// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2005-2011, Jari Sundell <jaris@ifi.uio.no>

#include <torrent/download/choke_group.h>
#include <torrent/download/choke_queue.h>
#include <torrent/download/resource_manager.h>
#include <torrent/utils/option_strings.h>

#include "rpc/parse.h"
#include "rpc/parse_commands.h"
#include "ui/root.h"

#include "command_helpers.h"
#include "control.h"
#include "globals.h"

// For cg_d_group.
#include "core/download.h"

// A hack to allow testing of the new choke_group API without the
// working parts present.
#define USE_CHOKE_GROUP 0

#if USE_CHOKE_GROUP

int64_t
cg_get_index(const torrent::Object& raw_args) {
  const torrent::Object& arg =
    (raw_args.is_list() && !raw_args.as_list().empty())
      ? raw_args.as_list().front()
      : raw_args;

  int64_t index = 0;

  if (arg.is_string()) {
    if (!rpc::parse_whole_value_nothrow(arg.as_string().c_str(), &index))
      return torrent::resource_manager()->group_index_of(arg.as_string());

  } else {
    index = arg.as_value();
  }

  if (index < 0)
    index = (int64_t)torrent::resource_manager()->group_size() + index;

  return std::min<uint64_t>(index, torrent::resource_manager()->group_size());
}

torrent::choke_group*
cg_get_group(const torrent::Object& raw_args) {
  return torrent::resource_manager()->group_at(cg_get_index(raw_args));
}

int64_t
cg_d_group(core::Download* download) {
  return torrent::resource_manager()->entry_at(download->main()).group();
}

const std::string&
cg_d_group_name(core::Download* download) {
  return torrent::resource_manager()
    ->group_at(torrent::resource_manager()->entry_at(download->main()).group())
    ->name();
}

void
cg_d_group_set(core::Download* download, const torrent::Object& arg) {
  torrent::resource_manager()->set_group(
    torrent::resource_manager()->find_throw(download->main()),
    cg_get_index(arg));
}

torrent::Object
apply_cg_list() {
  torrent::Object::list_type result;

  for (torrent::ResourceManager::group_iterator
         itr  = torrent::resource_manager()->group_begin(),
         last = torrent::resource_manager()->group_end();
       itr != last;
       itr++)
    result.push_back((*itr)->name());

  return torrent::Object::from_list(result);
}

torrent::Object
apply_cg_insert(const std::string& arg) {
  int64_t dummy;

  if (rpc::parse_whole_value_nothrow(arg.c_str(), &dummy))
    throw torrent::input_error(
      "Cannot use a value string as choke group name.");

  torrent::resource_manager()->push_group(arg);

  return torrent::Object();
}

//
// The hacked version:
//
#else

std::vector<torrent::choke_group*> cg_list_hack;

int64_t
cg_get_index(const torrent::Object& raw_args) {
  const torrent::Object& arg =
    (raw_args.is_list() && !raw_args.as_list().empty())
      ? raw_args.as_list().front()
      : raw_args;

  int64_t index = 0;

  if (arg.is_string()) {
    if (!rpc::parse_whole_value_nothrow(arg.as_string().c_str(), &index)) {
      std::vector<torrent::choke_group*>::iterator itr =
        std::find_if(cg_list_hack.begin(),
                     cg_list_hack.end(),
                     [arg](torrent::choke_group* c) {
                       return arg.as_string() == c->name();
                     });

      if (itr == cg_list_hack.end())
        throw torrent::input_error("Choke group not found.");

      return std::distance(cg_list_hack.begin(), itr);
    }

  } else {
    index = arg.as_value();
  }

  if (index < 0)
    index = (int64_t)cg_list_hack.size() + index;

  if ((size_t)index >= cg_list_hack.size())
    throw torrent::input_error("Choke group not found.");

  return index;
}

torrent::choke_group*
cg_get_group(const torrent::Object& raw_args) {
  int64_t index = cg_get_index(raw_args);

  if ((size_t)index >= cg_list_hack.size())
    throw torrent::input_error("Choke group not found.");

  return cg_list_hack.at(index);
}

int64_t
cg_d_group(core::Download* download) {
  return download->group();
}

const std::string&
cg_d_group_name(core::Download* download) {
  return cg_list_hack.at(download->group())->name();
}

void
cg_d_group_set(core::Download* download, const torrent::Object& arg) {
  download->set_group(cg_get_index(arg));
}

torrent::Object
apply_cg_list() {
  torrent::Object::list_type result;

  for (std::vector<torrent::choke_group*>::iterator itr  = cg_list_hack.begin(),
                                                    last = cg_list_hack.end();
       itr != last;
       itr++)
    result.push_back((*itr)->name());

  return torrent::Object::from_list(result);
}

torrent::Object
apply_cg_insert(const std::string& arg) {
  int64_t dummy;

  if (rpc::parse_whole_value_nothrow(arg.c_str(), &dummy))
    throw torrent::input_error(
      "Cannot use a value string as choke group name.");

  if (arg.empty() || std::find_if(cg_list_hack.begin(),
                                  cg_list_hack.end(),
                                  [arg](torrent::choke_group* cg) {
                                    return arg == cg->name();
                                  }) != cg_list_hack.end())
    throw torrent::input_error("Duplicate name for choke group.");

  cg_list_hack.push_back(new torrent::choke_group());
  cg_list_hack.back()->set_name(arg);

  cg_list_hack.back()->up_queue()->set_heuristics(
    torrent::choke_queue::HEURISTICS_UPLOAD_LEECH);
  cg_list_hack.back()->down_queue()->set_heuristics(
    torrent::choke_queue::HEURISTICS_DOWNLOAD_LEECH);

  return torrent::Object();
}

torrent::Object
apply_cg_index_of(const std::string& arg) {
  std::vector<torrent::choke_group*>::iterator itr =
    std::find_if(cg_list_hack.begin(),
                 cg_list_hack.end(),
                 [arg](torrent::choke_group* cg) { return arg == cg->name(); });

  if (itr == cg_list_hack.end())
    throw torrent::input_error("Choke group not found.");

  return std::distance(cg_list_hack.begin(), itr);
}

//
// End of choke group hack.
//
#endif

torrent::Object
apply_cg_max_set(const torrent::Object::list_type& args, bool is_up) {
  if (args.size() != 2)
    throw torrent::input_error("Incorrect number of arguments.");

  int64_t second_arg = 0;
  rpc::parse_whole_value(args.back().as_string().c_str(), &second_arg);

  if (is_up)
    cg_get_group(args.front())->up_queue()->set_max_unchoked(second_arg);
  else
    cg_get_group(args.front())->down_queue()->set_max_unchoked(second_arg);

  return torrent::Object();
}

torrent::Object
apply_cg_heuristics_set(const torrent::Object::list_type& args, bool is_up) {
  if (args.size() != 2)
    throw torrent::input_error("Incorrect number of arguments.");

  int t = torrent::option_find_string(
    is_up ? torrent::OPTION_CHOKE_HEURISTICS_UPLOAD
          : torrent::OPTION_CHOKE_HEURISTICS_DOWNLOAD,
    args.back().as_string().c_str());

  if (is_up)
    cg_get_group(args.front())
      ->up_queue()
      ->set_heuristics((torrent::choke_queue::heuristics_enum)t);
  else
    cg_get_group(args.front())
      ->down_queue()
      ->set_heuristics((torrent::choke_queue::heuristics_enum)t);

  return torrent::Object();
}

torrent::Object
apply_cg_tracker_mode_set(const torrent::Object::list_type& args) {
  if (args.size() != 2)
    throw torrent::input_error("Incorrect number of arguments.");

  int t = torrent::option_find_string(torrent::OPTION_TRACKER_MODE,
                                      args.back().as_string().c_str());

  cg_get_group(args.front())
    ->set_tracker_mode((torrent::choke_group::tracker_mode_enum)t);

  return torrent::Object();
}

/*

<cg_index> -> '0'..'(choke_group.size)'
           -> '-1'..'-(choke_group.size)'
           -> '<group_name>'

(choke_group.list) -> List of group names.
(choke_group.size) -> Number of groups.

(choke_group.insert,"group_name")

Adds a new group with default settings, use index '-1' to accessing it
immediately afterwards.

(choke_group.index_of,"group_name") -> <group_index>

Throws if the group name was not found.

(choke_group.general.size,<cg_index>) -> <size>

Number of torrents in this group.

(choke_group.tracker.mode,<cg_index>) -> "tracker_mode"
(choke_group.tracker.mode.set,<cg_index>,"tracker_mode")

Decide on how aggressive a tracker should be, see
'strings.tracker_mode' for list of available options

(choke_group.up.rate,<cg_index>) -> <bytes/second>
(choke_group.down.rate,<cg_index>) -> <bytes/second>

Upload / download rate for the aggregate of all torrents in this
particular group.

(choke_group.up.max,<cg_index>) -> <max_upload_slots>
(choke_group.up.max.unlimited,<cg_index>) -> <max_upload_slots>
(choke_group.up.max.set,<cg_index>, <max_upload_slots>)
(choke_group.down.max,<cg_index>) -> <max_download_slots>
(choke_group.down.max.unlimited,<cg_index>) -> <max_download_slots>
(choke_group.down.max.set,<cg_index>, <max_download_slots)

Number of unchoked upload / download peers regulated on a group basis.

(choke_group.up.total,<cg_index>) -> <number of queued and unchoked interested
peers> (choke_group.up.queued,<cg_index>) -> <number of queued interested peers>
(choke_group.up.unchoked,<cg_index>) -> <number of unchoked uploads>
(choke_group.down.total,<cg_index>) -> <number of queued and unchoked interested
peers> (choke_group.down.queued,<cg_index>) -> <number of queued interested
peers> (choke_group.down.unchoked,<cg_index>) -> <number of unchoked uploads>

(choke_group.up.heuristics,<cg_index>) -> "heuristics"
(choke_group.up.heuristics.set,<cg_index>,"heuristics")
(choke_group.down.heuristics,<cg_index>) -> "heuristics"
(choke_group.down.heuristics.set,<cg_index>,"heuristics")

Heuristics are used for deciding what peers to choke and unchoke, see
'strings.choke_heuristics{,_download,_upload}' for a list of available
options.

(d.group) -> <choke_group_index>
(d.group.name) -> "choke_group_name"
(d.group.set,<cg_index>)

 */

void
initialize_command_groups() {
  CMD2_ANY("choke_group.list",
           [](const auto&, const auto&) { return apply_cg_list(); });
  CMD2_ANY_STRING("choke_group.insert", [](const auto&, const auto& arg) {
    return apply_cg_insert(arg);
  });

#if USE_CHOKE_GROUP
  CMD2_ANY("choke_group.size", [](const auto&, const auto&) {
    return torrent::resource_manager()->group_size();
  });
  CMD2_ANY_STRING("choke_group.index_of", [](const auto&, const auto& name) {
    return torrent::resource_manager()->group_index_of(name);
  });
#else
  apply_cg_insert("default");

  CMD2_ANY("choke_group.size",
           [](const auto&, const auto&) { return cg_list_hack.size(); });
  CMD2_ANY_STRING("choke_group.index_of", [](const auto&, const auto& arg) {
    return apply_cg_index_of(arg);
  });
#endif

  // Commands specific for a group. Supports as the first argument the
  // name, the index or a negative index.
  CMD2_ANY("choke_group.general.size", [](const auto&, const auto& raw_args) {
    return cg_get_group(raw_args)->size();
  });

  CMD2_ANY("choke_group.tracker.mode", [](const auto&, const auto& raw_args) {
    return torrent::option_as_string(torrent::OPTION_TRACKER_MODE,
                                     cg_get_group(raw_args)->tracker_mode());
  });
  CMD2_ANY_LIST("choke_group.tracker.mode.set",
                [](const auto&, const auto& args) {
                  return apply_cg_tracker_mode_set(args);
                });

  CMD2_ANY("choke_group.up.rate", [](const auto&, const auto& raw_args) {
    return cg_get_group(raw_args)->up_rate();
  });
  CMD2_ANY("choke_group.down.rate", [](const auto&, const auto& raw_args) {
    return cg_get_group(raw_args)->down_rate();
  });

  CMD2_ANY("choke_group.up.max.unlimited",
           [](const auto&, const auto& raw_args) {
             return cg_get_group(raw_args)->up_queue()->is_unlimited();
           });
  CMD2_ANY("choke_group.up.max", [](const auto&, const auto& raw_args) {
    return cg_get_group(raw_args)->up_queue()->max_unchoked_signed();
  });
  CMD2_ANY_LIST("choke_group.up.max.set", [](const auto&, const auto& args) {
    return apply_cg_max_set(args, true);
  });

  CMD2_ANY("choke_group.up.total", [](const auto&, const auto& raw_args) {
    return cg_get_group(raw_args)->up_queue()->size_total();
  });
  CMD2_ANY("choke_group.up.queued", [](const auto&, const auto& raw_args) {
    return cg_get_group(raw_args)->up_queue()->size_queued();
  });
  CMD2_ANY("choke_group.up.unchoked", [](const auto&, const auto& raw_args) {
    return cg_get_group(raw_args)->up_queue()->size_unchoked();
  });
  CMD2_ANY("choke_group.up.heuristics", [](const auto&, const auto& raw_args) {
    return torrent::option_as_string(
      torrent::OPTION_CHOKE_HEURISTICS,
      cg_get_group(raw_args)->up_queue()->heuristics());
  });
  CMD2_ANY_LIST("choke_group.up.heuristics.set",
                [](const auto&, const auto& args) {
                  return apply_cg_heuristics_set(args, true);
                });

  CMD2_ANY("choke_group.down.max.unlimited",
           [](const auto&, const auto& raw_args) {
             return cg_get_group(raw_args)->down_queue()->is_unlimited();
           });
  CMD2_ANY("choke_group.down.max", [](const auto&, const auto& raw_args) {
    return cg_get_group(raw_args)->down_queue()->max_unchoked_signed();
  });
  CMD2_ANY_LIST("choke_group.down.max.set", [](const auto&, const auto& args) {
    return apply_cg_max_set(args, false);
  });

  CMD2_ANY("choke_group.down.total", [](const auto&, const auto& raw_args) {
    return cg_get_group(raw_args)->down_queue()->size_total();
  });
  CMD2_ANY("choke_group.down.queued", [](const auto&, const auto& raw_args) {
    return cg_get_group(raw_args)->down_queue()->size_queued();
  });
  CMD2_ANY("choke_group.down.unchoked", [](const auto&, const auto& raw_args) {
    return cg_get_group(raw_args)->down_queue()->size_unchoked();
  });
  CMD2_ANY("choke_group.down.heuristics",
           [](const auto&, const auto& raw_args) {
             return torrent::option_as_string(
               torrent::OPTION_CHOKE_HEURISTICS,
               cg_get_group(raw_args)->down_queue()->heuristics());
           });
  CMD2_ANY_LIST("choke_group.down.heuristics.set",
                [](const auto&, const auto& args) {
                  return apply_cg_heuristics_set(args, false);
                });
}

void
cleanup_command_groups() {
#if USE_CHOKE_GROUP
#else
  while (!cg_list_hack.empty()) {
    auto cg = cg_list_hack.back();
    delete cg;
    cg_list_hack.pop_back();
  }
#endif
}
