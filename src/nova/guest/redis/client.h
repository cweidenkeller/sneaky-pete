#include <string>
#include "response.h"
#include "commands.h"
#include "control.h"
#include "config.h"
#ifndef CLIENT_H
#define CLIENT_H
namespace nova { namespace redis {
class Client
    /*
     * This is the redis Client class it handles all communication
     * and process control of a redis instance.
     * This object is thread safe and handles closing
     * of any open file descriptors or sockets on deconstruction.
     */
{
    private:
        bool _authed;
        bool _name_set;
        int _socket;
        std::string _port;
        std::string _host;
        std::string _client_name;
        std::string _config_file;
        std::string _config_command;
        Commands*_commands;
        /*
         * Connects to the redis server using nova::redis::get_socket.
         * found
         */
        response _connect();
        response _send_redis_message(std::string message);
        response _get_redis_response();
        response _set_client();
        response _auth();
        response _reconnect();
        void _find_config_command();
    public:
        Control* control;
        Config* config;
        Client(std::string host, std::string port, std::string client_name,
               std::string config_file);
        ~Client();
        response ping();
        response bgsave();
        response save();
        response last_save();
        response config_get(std::string name);
        response config_set(std::string name, std::string value);
        response config_rewrite();
};
}}//end nova::redis namespace
#endif /* CLIENT_H */
