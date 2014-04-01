#include <string>
namespace nova { namespace redis {
struct response
/* This struct is used to represent responses from the redis server.
    * It returns the status. A predefined set of responses can be found in
    * redis_constants.h.
    * A breif description of the response and the data if any the response
    * contained.
    */
{
    std::string status;
    std::string description;
    std::string data;
    response(std::string _status, std::string _data);
};
}}//end nova::redis namespace.
