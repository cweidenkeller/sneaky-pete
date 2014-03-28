#ifndef NOVA_REDIS_REDIS_MESSAGE_HANDLER_H
#define NOVA_REDIS_REDIS_MESSAGE_HANDLER_H

#include "guest.h"
#include "nova/guest/monitoring/monitoring.h"
#include "nova/VolumeManager.h"

namespace nova { namespace guest {


class RedisMessageHandler : public MessageHandler {

    public:

        RedisMessageHandler(
            nova::guest::apt::AptGuestPtr apt,
            nova::guest::monitoring::MonitoringManagerPtr monitoring,
            VolumeManagerPtr volumeManager);

        virtual ~RedisMessageHandler();
        virtual nova::JsonDataPtr handle_message(const GuestInput & input);
        VolumeManagerPtr create_volume_manager();

    private:
        nova::guest::apt::AptGuestPtr apt;
        nova::guest::monitoring::MonitoringManagerPtr monitoring;
        VolumeManagerPtr volumeManager;
        RedisMessageHandler(const RedisMessageHandler &);
        RedisMessageHandler & operator = (const RedisMessageHandler &);

};


}}//end nova::redis namespace.
#endif //NOVA_REDIS_REDIS_MESSAGE_HANDLER_H
