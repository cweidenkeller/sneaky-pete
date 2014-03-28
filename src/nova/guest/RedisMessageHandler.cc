#include "pch.hpp"
#include "nova/guest/apt.h"
#include "nova/guest/backup/BackupRestore.h"
#include <boost/format.hpp>
#include "nova/guest/guest.h"
#include "nova/guest/GuestException.h"
#include "nova/process.h"
#include "RedisMessageHandler.h"
#include "nova/Log.h"
#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/assign/std/list.hpp>
#include <boost/optional.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/variant.hpp>
#include "nova/VolumeManager.h"
#include "nova/guest/redis/client.h"

using namespace boost::assign;

using nova::guest::apt::AptGuest;
using nova::guest::backup::BackupRestoreInfo;
using boost::format;
using nova::guest::GuestException;
using boost::lexical_cast;
using nova::Log;
using nova::JsonData;
using nova::JsonDataPtr;
using nova::JsonObject;
using nova::JsonObjectPtr;
using nova::json_string;
using boost::optional;
using namespace std;
using boost::tie;

namespace nova { namespace guest {

namespace {
vector<string> get_packages_argument(JsonObjectPtr obj) {
    try {
        const auto packages = obj->get_array("packages")->to_string_vector();
        return packages;
    } catch(const JsonException) {
        NOVA_LOG_DEBUG("Interpretting \"packages\" as a single string.");
        vector<string> packages;
        packages.push_back(obj->get_string("packages"));
        return packages;
    }
}
}
RedisMessageHandler::RedisMessageHandler(
    nova::guest::apt::AptGuestPtr apt,
    nova::guest::monitoring::MonitoringManagerPtr monitoring,
    VolumeManagerPtr volumeManager)
:   apt(apt),
    monitoring(monitoring),
    volumeManager(volumeManager) {

}
JsonDataPtr RedisMessageHandler::handle_message(const GuestInput & input) {
    if (input.method_name == "prepare") {
        const auto packages = get_packages_argument(input.args);
        const auto config_contents = input.args->get_string("config_contents");
        const auto overrides = input.args->get_optional_string("overrides");
        // Mount volume
        if (volumeManager) {
            const auto device_path = input.args->get_optional_string("device_path");
            const auto mount_point = volumeManager->get_mount_point();
            if (device_path && device_path.get().length() > 0) {
                NOVA_LOG_INFO("Mounting volume for prepare call...");
                bool write_to_fstab = true;
                VolumeManagerPtr volume_manager = this->create_volume_manager();
                VolumeDevice vol_device = volume_manager->create_volume_device(device_path.get());
                vol_device.format();                                                                                                                                          vol_device.mount(mount_point, write_to_fstab);
                NOVA_LOG_INFO("Mounted the volume.");
            }
        }
        // Restore the database?
        optional<BackupRestoreInfo> restore;
        const auto backup_url = input.args->get_optional_string("backup_url");
        if (backup_url && backup_url.get().length() > 0) {
            NOVA_LOG_INFO("Calling Restore...")
            if (!input.token) {
                NOVA_LOG_ERROR("No token given! Cannot do this restore!");
                throw GuestException(GuestException::MALFORMED_INPUT);
            }
            const auto token = input.token;
            const auto backup_checksum = input.args->get_optional_string("backup_checksum");
            restore = optional<BackupRestoreInfo>(BackupRestoreInfo(token.get(), backup_url.get(), backup_checksum.get()));
        }
        // installation of monitoring
        const auto monitoring_info = input.args->get_optional_object("monitoring_info");
        if (monitoring_info) {
            NOVA_LOG_INFO("Installing Monitoring Agent following successful prepare");
            const auto token = monitoring_info->get_string("token");
            const auto endpoints = monitoring_info->get_string("endpoints");
            monitoring->install_and_configure_monitoring_agent(
                *this->apt, token, endpoints);
        } else {
            NOVA_LOG_INFO("Skipping Monitoring Agent as no endpoints were supplied.");
        }
        return JsonData::from_null();
    }

}
}}//end namespace nova::guest
