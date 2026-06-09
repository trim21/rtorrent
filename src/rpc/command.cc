#include "config.h"

#include "core/download.h"
#include "parse.h"

#include "command.h"

namespace rpc {

int64_t
convert_to_value_arg(const torrent::Object& obj, int base, int unit) {
  const torrent::Object& arg = convert_to_single_argument(obj);

  if (arg.type() == torrent::Object::TYPE_STRING) {
    torrent::Object::value_type val;

    if (!parse_whole_value_nothrow(arg.as_string().c_str(), &val, base, unit))
      throw torrent::input_error("Not a value.");

    return val;
  }

  return unit * arg.as_value();
}

std::string
convert_to_string_arg(const torrent::Object& obj) {
  const torrent::Object& arg = convert_to_single_argument(obj);

  if (arg.type() == torrent::Object::TYPE_RAW_STRING)
    return arg.as_raw_string().as_string();

  return arg.as_string();
}

torrent::Object::list_type
convert_to_list_arg(const torrent::Object& obj) {
  if (obj.type() != torrent::Object::TYPE_LIST) {
    torrent::Object::list_type arg;

    if (!obj.is_empty())
      arg.push_back(obj);

    return arg;
  }

  return obj.as_list();
}

}
