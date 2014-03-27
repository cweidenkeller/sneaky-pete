#ifndef NOVA_REDIS_REDIS_MESSAGE_HANDLER_H
#define NOVA_REDIS_REDIS_MESSAGE_HANDLER_H

#include "guest.h"


namespace nova { namespace guest {


class RedisMessageHandler : public MessageHandler {

    public:

        RedisMessageHandler();
        virtual nova::JsonDataPtr handle_message(const GuestInput & input);

    private:

        RedisMessageHandler(const RedisMessageHandler &);
        RedisMessageHandler & operator = (const RedisMessageHandler &);

};


}}//end nova::redis namespace.
#endif //NOVA_REDIS_REDIS_MESSAGE_HANDLER_H
