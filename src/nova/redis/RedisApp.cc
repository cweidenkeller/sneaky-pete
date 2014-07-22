#include "pch.hpp"
#include "RedisApp.h"
#include "RedisException.h"
#include "nova/guest/redis/client.h"
#include "nova/utils/io.h"
#include <iostream>
#include <boost/assign/list_of.hpp>
#include <boost/assign/std/list.hpp>
#include "nova/process.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using nova::guest::backup::BackupRestoreInfo;
using boost::format;
using nova::utils::io::is_file;
using std::ofstream;
using boost::optional;
using nova::redis::RedisAppStatusPtr;
using nova::process::shell;
using std::string;


// Macros are bad, but better than duplicating these two lines everywhere.
#define THROW_PASSWORD_ERROR(msg) { \
        NOVA_LOG_ERROR(msg); \
        throw RedisException(RedisException::CHANGE_PASSWORD_ERROR); }

#define THROW_PREPARE_ERROR(msg) { \
        NOVA_LOG_ERROR(msg); \
        throw RedisException(RedisException::PREPARE_ERROR); }


namespace nova { namespace redis {


RedisApp::RedisApp(RedisAppStatusPtr app_status)
:   app_status(app_status) {
}

RedisApp::~RedisApp() {
}



void RedisApp::change_password(const string & password) {
    NOVA_LOG_INFO("Connecting to redis");
    nova::redis::Client client;
    client.config_set("requirepass", password);
    client.config_rewrite();
}


void RedisApp::prepare(const optional<string> & json_root_password,
                       const string & config_contents,
                       const optional<string> & overrides,
                       optional<BackupRestoreInfo> restore)
{
    if (!json_root_password) {
        NOVA_LOG_ERROR("Missing root password!");
        throw RedisException(RedisException::MISSING_ROOT_PASSWORD);
    }
    const string root_password = json_root_password.get();

    shell("sudo rm /etc/redis/redis.conf");
    shell("sudo chmod -R 777 /etc/redis");

    NOVA_LOG_INFO("Creating new config file.");
    {
        ofstream fd;
        fd.open("/etc/redis/redis.conf");
        NOVA_LOG_INFO("Writing config contents.");
        NOVA_LOG_DEBUG(config_contents.c_str());
        if (fd.good())
        {
            const string requirepass = str(format("requirepass %1%") 
                % root_password);
            fd << requirepass << config_contents;
            fd.close();
        }
        else
        {
            THROW_PREPARE_ERROR("Couldn't open config file.");
        }
    }
    shell("sudo chmod 644 /etc/redis/redis.conf");
    shell("mkdir /etc/redis/conf.d");
    if (is_file("/etc/redis/conf.d/local.conf")) {
        shell("rm /etc/redis/conf.d/local.conf");
    }
    nova::redis::Client client;
    NOVA_LOG_INFO("Opening /etc/redis/conf.d/local.conf for writing");
    {
        ofstream fd;
        fd.open("/etc/redis/conf.d/local.conf");
        if (fd.is_open())
        {
            const string local_conf = str(format(
                        "maxmemory %1%\n"
                        "dir %2%\n"
                        "pidfile %3%\n"
                        "dbfilename %4%\n"
                        "daemonize yes\n"
                        "port %5%\n"
                        "logfile %6%\n"
                        "appendfilename %7%")
                        % client.config->get_max_memory()
                        % client.config->get_db_dir()
                        % client.config->get_pidfile()
                        % client.config->get_db_filename()
                        % client.config->get_port()
                        % client.config->get_log_file()
                        % client.config->get_append_filename());

            fd << local_conf;
            fd.close();
            fd.clear();
        }
        else
        {
            THROW_PREPARE_ERROR("Unable to open /etc/redis/conf.d/local.conf");
        }
    }
    shell("sudo chmod 644 /etc/redis/conf.d/local.conf");
    shell("sudo chmod 755 /etc/redis");
    if (system("sudo chmod 755 /etc/redis/conf.d") == -1)
    {
        THROW_PREPARE_ERROR("Unable to chmod /etc/redis/conf.d to 755");
    }
    shell("sudo chown -R redis:redis /etc/redis");
    NOVA_LOG_INFO("Connecting to redis instance.");
    NOVA_LOG_INFO("Stopping redis instance.");
    if (client.control->stop() != 0)
    {
        NOVA_LOG_INFO("Unable to stop redis instance.");
        //TODO(tim.simpson): If we can't stop, is this just ok?
    }
    NOVA_LOG_INFO("Starting redis instance.");
    if (client.control->start() != 0)
    {
        THROW_PREPARE_ERROR("Unable to start redis instance!");
    }
}


void RedisApp::restart() {
    NOVA_LOG_INFO("Entering restart call.");
    nova::redis::Client client;
    NOVA_LOG_INFO("Stopping redis instance.");
    if (client.control->stop() != 0)
    {
        NOVA_LOG_ERROR("Unable to stop redis instance!");
        throw RedisException(RedisException::COULD_NOT_STOP);
    }
    NOVA_LOG_INFO("Starting redis instance.");
    if (client.control->start() != 0)
    {
        NOVA_LOG_ERROR("Unable to start redis instance!");
        throw RedisException(RedisException::COULD_NOT_START);
    }
    NOVA_LOG_INFO("Redis instance restarted successfully.");
}


void RedisApp::start_with_conf_changes(const std::string & config_contents) {
    //TODO (cweid): I need to add in HA persistance to the upgrade calls.
    // Will add in the Am.
    NOVA_LOG_INFO("Entering start_db_with_conf_changes call.");
    nova::redis::Client client;
    const string root_password = client.config->get_require_pass();
    shell("sudo rm /etc/redis/redis.conf");
    shell("sudo chmod 777 /etc/redis");
    NOVA_LOG_INFO("Creating new config file.");
    {
        ofstream fd;
        fd.open("/etc/redis/redis.conf");
        NOVA_LOG_INFO("Writing config contents.");
        NOVA_LOG_DEBUG(config_contents.c_str());
        if (fd.good())
        {
            const string requirepass = str(format("requirepass %1%")
                % root_password);
            fd << requirepass << config_contents;
            fd.close();
        }
        else
        {
            THROW_PREPARE_ERROR("Couldn't open config file.");
        }
    }
    if (client.control->start() != 0)
    {
        NOVA_LOG_ERROR("Unable to start redis instance!");
        throw RedisException(RedisException::COULD_NOT_START);
    }
    shell("sudo chmod 755 /etc/redis");
    shell("sudo chmod 644 /etc/redis/redis.conf");
    shell("sudo chmod -R redis:redis /etc/redis");
    NOVA_LOG_INFO("Redis instance started.");
}


void RedisApp::stop() {
    NOVA_LOG_INFO("Entering stop_db call.");
    nova::redis::Client client;
    NOVA_LOG_INFO("Stopping redis instance.");
    if (client.control->stop() != 0)
    {
        NOVA_LOG_ERROR("Unable to stop redis instance!");
        throw RedisException(RedisException::COULD_NOT_STOP);
    }
    NOVA_LOG_INFO("Redis instance stopped.");
}

} }  // end namespace