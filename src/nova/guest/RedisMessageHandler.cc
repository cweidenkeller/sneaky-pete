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
RedisMessageHandler::RedisMessageHandler() {

}
JsonDataPtr RedisMessageHandler::handle_message(const GuestInput & input) {
    if (input.method_name == "prepare") {
        NOVA_LOG_INFO("Calling prepare...");
        const auto packages = get_packages_argument(input.args);
        //const auto config_contents = input.args->get_string("config_contents");
        const auto overrides = input.args->get_optional_string("overrides");
        NOVA_LOG_INFO("Skipping Monitoring Agent as no endpoints were supplied.");
        return JsonData::from_null();
        }
        return JsonData::from_null();

    }


}}//end namespace nova::guest
