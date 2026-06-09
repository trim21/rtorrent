#ifndef RTORRENT_UTILS_COMMAND_HELPERS_H
#define RTORRENT_UTILS_COMMAND_HELPERS_H

#include "rpc/command.h"
#include "rpc/parse_commands.h"
#include "rpc/object_storage.h"

void initialize_commands();

namespace rpc {

template<typename Slot>
inline auto wrap_typed_slot(Slot&& slot) {
    return [slot = std::forward<Slot>(slot)](const rpc::target_type& t, const torrent::Object& o) -> torrent::Object {
        return slot(t, o);
    };
}

template<typename Slot>
inline auto wrap_void_slot(Slot&& slot) {
    return [slot = std::forward<Slot>(slot)](const rpc::target_type& t, const torrent::Object& o) {
        slot(t, o);
    };
}

template<typename Slot>
inline auto wrap_list_void_slot(Slot&& slot) {
    return [slot = std::forward<Slot>(slot)](const rpc::target_type& t, const torrent::Object& o) {
        auto l = rpc::convert_to_list_arg(o);
        slot(t, l);
    };
}

template<typename Slot>
inline auto wrap_value_void_slot(Slot&& slot) {
    return [slot = std::forward<Slot>(slot)](const rpc::target_type& t, const torrent::Object& o) {
        auto v = rpc::convert_to_value_arg(o, 0, 1);
        slot(t, v);
    };
}

template<typename Slot>
inline auto wrap_value_kb_void_slot(Slot&& slot) {
    return [slot = std::forward<Slot>(slot)](const rpc::target_type& t, const torrent::Object& o) {
        auto v = rpc::convert_to_value_arg(o, 0, 1024);
        slot(t, v);
    };
}

template<typename Slot>
inline auto wrap_string_void_slot(Slot&& slot) {
    return [slot = std::forward<Slot>(slot)](const rpc::target_type& t, const torrent::Object& o) {
        auto s = rpc::convert_to_string_arg(o);
        slot(t, s);
    };
}

template<typename T, typename Slot>
inline auto wrap_target_void_slot(Slot&& slot) {
    return [slot = std::forward<Slot>(slot)](const rpc::target_type& t, const torrent::Object& o) {
        slot(rpc::extract_target<T>(t), o);
    };
}

template<typename T, typename Slot>
inline auto wrap_target_value_void_slot(Slot&& slot) {
    return [slot = std::forward<Slot>(slot)](const rpc::target_type& t, const torrent::Object& o) {
        auto v = rpc::convert_to_value_arg(o, 0, 1);
        slot(rpc::extract_target<T>(t), v);
    };
}

template<typename T, typename Slot>
inline auto wrap_target_string_void_slot(Slot&& slot) {
    return [slot = std::forward<Slot>(slot)](const rpc::target_type& t, const torrent::Object& o) {
        auto s = rpc::convert_to_string_arg(o);
        slot(rpc::extract_target<T>(t), s);
    };
}

template<typename T, typename Slot>
inline auto wrap_target_slot(Slot&& slot) {
    return [slot = std::forward<Slot>(slot)](const rpc::target_type& t, const torrent::Object& o) -> torrent::Object {
        return slot(rpc::extract_target<T>(t), o);
    };
}

template<typename T, typename Slot>
inline auto wrap_target_value_slot(Slot&& slot) {
    return [slot = std::forward<Slot>(slot)](const rpc::target_type& t, const torrent::Object& o) -> torrent::Object {
        auto v = rpc::convert_to_value_arg(o, 0, 1);
        return slot(rpc::extract_target<T>(t), v);
    };
}

template<typename T, typename Slot>
inline auto wrap_target_string_slot(Slot&& slot) {
    return [slot = std::forward<Slot>(slot)](const rpc::target_type& t, const torrent::Object& o) -> torrent::Object {
        auto s = rpc::convert_to_string_arg(o);
        return slot(rpc::extract_target<T>(t), s);
    };
}

template<typename T, typename Slot>
inline auto wrap_target_list_slot(Slot&& slot) {
    return [slot = std::forward<Slot>(slot)](const rpc::target_type& t, const torrent::Object& o) -> torrent::Object {
        auto l = rpc::convert_to_list_arg(o);
        return slot(rpc::extract_target<T>(t), l);
    };
}

template<typename Slot>
inline auto wrap_value_slot(Slot&& slot) {
    return [slot = std::forward<Slot>(slot)](const rpc::target_type& t, const torrent::Object& o) -> torrent::Object {
        auto v = rpc::convert_to_value_arg(o, 0, 1);
        return slot(t, v);
    };
}

template<typename Slot>
inline auto wrap_value_kb_slot(Slot&& slot) {
    return [slot = std::forward<Slot>(slot)](const rpc::target_type& t, const torrent::Object& o) -> torrent::Object {
        auto v = rpc::convert_to_value_arg(o, 0, 1024);
        return slot(t, v);
    };
}

template<typename Slot>
inline auto wrap_string_slot(Slot&& slot) {
    return [slot = std::forward<Slot>(slot)](const rpc::target_type& t, const torrent::Object& o) -> torrent::Object {
        auto s = rpc::convert_to_string_arg(o);
        return slot(t, s);
    };
}

template<typename Slot>
inline auto wrap_list_slot(Slot&& slot) {
    return [slot = std::forward<Slot>(slot)](const rpc::target_type& t, const torrent::Object& o) -> torrent::Object {
        auto l = rpc::convert_to_list_arg(o);
        return slot(t, l);
    };
}

}

// --- ANY (slot takes target_type, Object) ---
#define CMD2_ANY(key, ...) \
  rpc::commands.insert_slot(key, rpc::wrap_typed_slot(__VA_ARGS__), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

#define CMD2_ANY_P(key, ...) \
  rpc::commands.insert_slot(key, rpc::wrap_typed_slot(__VA_ARGS__), rpc::CommandMap::flag_dont_delete, NULL, NULL);

#define CMD2_ANY_VOID(key, ...) \
  rpc::commands.insert_slot(key, object_convert_void(rpc::wrap_void_slot(__VA_ARGS__)), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

#define CMD2_ANY_V(key, ...) \
  rpc::commands.insert_slot(key, object_convert_void(rpc::wrap_list_void_slot(__VA_ARGS__)), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

#define CMD2_ANY_L(key, ...) \
  rpc::commands.insert_slot(key, rpc::wrap_list_slot(__VA_ARGS__), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

#define CMD2_ANY_VALUE(key, ...) \
  rpc::commands.insert_slot(key, rpc::wrap_value_slot(__VA_ARGS__), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

#define CMD2_ANY_STRING(key, ...) \
  rpc::commands.insert_slot(key, rpc::wrap_string_slot(__VA_ARGS__), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

#define CMD2_ANY_VALUE_V(key, ...) \
  rpc::commands.insert_slot(key, object_convert_void(rpc::wrap_value_void_slot(__VA_ARGS__)), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

#define CMD2_ANY_VALUE_KB(key, ...) \
  rpc::commands.insert_slot(key, object_convert_void(rpc::wrap_value_kb_void_slot(__VA_ARGS__)), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

#define CMD2_ANY_STRING_V(key, ...) \
  rpc::commands.insert_slot(key, object_convert_void(rpc::wrap_string_void_slot(__VA_ARGS__)), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

#define CMD2_ANY_LIST(key, ...) \
  rpc::commands.insert_slot(key, rpc::wrap_list_slot(__VA_ARGS__), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

// --- DL (Download) ---
#define CMD2_DL(key, ...) \
  rpc::commands.insert_slot(key, rpc::wrap_target_slot<core::Download*>(__VA_ARGS__), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

#define CMD2_DL_V(key, ...) \
  rpc::commands.insert_slot(key, object_convert_void(rpc::wrap_target_void_slot<core::Download*>(__VA_ARGS__)), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

#define CMD2_DL_VALUE(key, ...) \
  rpc::commands.insert_slot(key, rpc::wrap_target_value_slot<core::Download*>(__VA_ARGS__), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

#define CMD2_DL_VALUE_V(key, ...) \
  rpc::commands.insert_slot(key, object_convert_void(rpc::wrap_target_value_void_slot<core::Download*>(__VA_ARGS__)), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

#define CMD2_DL_STRING(key, ...) \
  rpc::commands.insert_slot(key, rpc::wrap_target_string_slot<core::Download*>(__VA_ARGS__), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

#define CMD2_DL_STRING_V(key, ...) \
  rpc::commands.insert_slot(key, object_convert_void(rpc::wrap_target_string_void_slot<core::Download*>(__VA_ARGS__)), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

#define CMD2_DL_LIST(key, ...) \
  rpc::commands.insert_slot(key, rpc::wrap_target_list_slot<core::Download*>(__VA_ARGS__), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

#define CMD2_DL_VALUE_P(key, ...) \
  rpc::commands.insert_slot(key, rpc::wrap_target_value_slot<core::Download*>(__VA_ARGS__), rpc::CommandMap::flag_dont_delete, NULL, NULL);

#define CMD2_DL_STRING_P(key, ...) \
  rpc::commands.insert_slot(key, rpc::wrap_target_string_slot<core::Download*>(__VA_ARGS__), rpc::CommandMap::flag_dont_delete, NULL, NULL);

// --- FILE ---
#define CMD2_FILE(key, ...) \
  rpc::commands.insert_slot(key, rpc::wrap_target_slot<torrent::File*>(__VA_ARGS__), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

#define CMD2_FILE_V(key, ...) \
  rpc::commands.insert_slot(key, object_convert_void(rpc::wrap_target_void_slot<torrent::File*>(__VA_ARGS__)), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

#define CMD2_FILE_VALUE_V(key, ...) \
  rpc::commands.insert_slot(key, object_convert_void(rpc::wrap_target_value_void_slot<torrent::File*>(__VA_ARGS__)), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

#define CMD2_FILEITR(key, ...) \
  rpc::commands.insert_slot(key, rpc::wrap_target_slot<torrent::FileListIterator*>(__VA_ARGS__), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

// --- PEER ---
#define CMD2_PEER(key, ...) \
  rpc::commands.insert_slot(key, rpc::wrap_target_slot<torrent::Peer*>(__VA_ARGS__), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

#define CMD2_PEER_V(key, ...) \
  rpc::commands.insert_slot(key, object_convert_void(rpc::wrap_target_void_slot<torrent::Peer*>(__VA_ARGS__)), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

#define CMD2_PEER_VALUE_V(key, ...) \
  rpc::commands.insert_slot(key, object_convert_void(rpc::wrap_target_value_void_slot<torrent::Peer*>(__VA_ARGS__)), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

// --- TRACKER ---
#define CMD2_TRACKER(key, ...) \
  rpc::commands.insert_slot(key, rpc::wrap_target_slot<torrent::tracker::Tracker*>(__VA_ARGS__), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

#define CMD2_TRACKER_V(key, ...) \
  rpc::commands.insert_slot(key, object_convert_void(rpc::wrap_target_void_slot<torrent::tracker::Tracker*>(__VA_ARGS__)), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

#define CMD2_TRACKER_VALUE_V(key, ...) \
  rpc::commands.insert_slot(key, object_convert_void(rpc::wrap_target_value_void_slot<torrent::tracker::Tracker*>(__VA_ARGS__)), rpc::CommandMap::flag_dont_delete | rpc::CommandMap::flag_public_rpc, NULL, NULL);

// --- VAR macros ---
#define CMD2_VAR_BOOL(key, value)                                       \
  control->object_storage()->insert_c_str(key, int64_t(value), rpc::object_storage::flag_bool_type); \
  CMD2_ANY(key, [storage = control->object_storage(), k = torrent::raw_string::from_c_str(key)](const auto&, const auto&) { return storage->get(k); }); \
  CMD2_ANY_VALUE(key ".set", [storage = control->object_storage(), k = torrent::raw_string::from_c_str(key)](const auto&, const auto& v) { return storage->set_bool(k, v); });

#define CMD2_VAR_VALUE(key, value)                                      \
  control->object_storage()->insert_c_str(key, int64_t(value), rpc::object_storage::flag_value_type); \
  CMD2_ANY(key, [storage = control->object_storage(), k = torrent::raw_string::from_c_str(key)](const auto&, const auto&) { return storage->get(k); }); \
  CMD2_ANY_VALUE(key ".set", [storage = control->object_storage(), k = torrent::raw_string::from_c_str(key)](const auto&, const auto& v) { return storage->set_value(k, v); });

#define CMD2_VAR_STRING(key, value)                                     \
  control->object_storage()->insert_c_str(key, value, rpc::object_storage::flag_string_type); \
  CMD2_ANY(key, [storage = control->object_storage(), k = torrent::raw_string::from_c_str(key)](const auto&, const auto&) { return storage->get(k); }); \
  CMD2_ANY_STRING(key ".set", [storage = control->object_storage(), k = torrent::raw_string::from_c_str(key)](const auto&, const auto& v) { return storage->set_string(k, v); });

#define CMD2_VAR_C_STRING(key, value)                                   \
  control->object_storage()->insert_c_str(key, value, rpc::object_storage::flag_string_type); \
  CMD2_ANY(key, [storage = control->object_storage(), k = torrent::raw_string::from_c_str(key)](const auto&, const auto&) { return storage->get(k); });

#define CMD2_VAR_LIST(key)                                              \
  control->object_storage()->insert_c_str(key, torrent::Object::create_list(), rpc::object_storage::flag_list_type); \
  CMD2_ANY(key, [storage = control->object_storage(), k = torrent::raw_string::from_c_str(key)](const auto&, const auto&) { return storage->get(k); }); \
  CMD2_ANY_LIST(key ".set", [storage = control->object_storage(), k = torrent::raw_string::from_c_str(key)](const auto&, const auto& v) { return storage->set_list(k, v); }); \
  CMD2_ANY_VOID(key ".push_back", [storage = control->object_storage(), k = torrent::raw_string::from_c_str(key)](const auto&, const auto& v) { storage->list_push_back(k, v); });

#define CMD2_FUNC_SINGLE(key, cmds)                                     \
  CMD2_ANY(key, [cmd = torrent::Object(torrent::raw_string::from_c_str(cmds))](const auto& target, const auto& args) { return rpc::command_function_call_object(cmd, target, args); });

// --- REDIRECT macros ---
#define CMD2_REDIRECT(from_key, to_key)                                 \
  rpc::commands.create_redirect(from_key, to_key, rpc::CommandMap::flag_public_rpc | rpc::CommandMap::flag_dont_delete);
#define CMD2_REDIRECT_NO_EXPORT(from_key, to_key)                       \
  rpc::commands.create_redirect(from_key, to_key, rpc::CommandMap::flag_dont_delete);
#define CMD2_REDIRECT_MUTABLE(from_key, to_key)                         \
  rpc::commands.create_redirect(from_key, to_key, rpc::CommandMap::flag_public_rpc);
#define CMD2_REDIRECT_STR(from_key, to_key)                             \
  rpc::commands.create_redirect(from_key, to_key, rpc::CommandMap::flag_public_rpc);
#define CMD2_REDIRECT_STR_NO_EXPORT(from_key, to_key)                   \
  rpc::commands.create_redirect(from_key, to_key, 0);
#define CMD2_REDIRECT_FILE(from_key, to_key) \
  rpc::commands.create_redirect(from_key, to_key, rpc::CommandMap::flag_public_rpc | rpc::CommandMap::flag_file_target | rpc::CommandMap::flag_dont_delete);
#define CMD2_REDIRECT_TRACKER(from_key, to_key) \
  rpc::commands.create_redirect(from_key, to_key, rpc::CommandMap::flag_public_rpc | rpc::CommandMap::flag_tracker_target | rpc::CommandMap::flag_dont_delete);

//
// Conversion of return types:
//

template <typename Functor, typename Result>
struct object_convert_type;

template <typename Functor>
struct object_convert_type<Functor, void> {

  template <typename Signature> struct result {
    typedef torrent::Object type;
  };

  object_convert_type(Functor s) : m_slot(s) {}

  torrent::Object operator () () const { m_slot(); return torrent::Object(); }
  template <typename Arg1>
  torrent::Object operator () (Arg1& arg1) const { m_slot(arg1); return torrent::Object(); }
  template <typename Arg1, typename Arg2>
  torrent::Object operator () (const Arg1& arg1) const { m_slot(arg1); return torrent::Object(); }
  template <typename Arg1, typename Arg2>
  torrent::Object operator () (Arg1& arg1, Arg2& arg2) const { m_slot(arg1, arg2); return torrent::Object(); }
  template <typename Arg1, typename Arg2>
  torrent::Object operator () (const Arg1& arg1, const Arg2& arg2) const { m_slot(arg1, arg2); return torrent::Object(); }
  template <typename Arg1, typename Arg2>
  torrent::Object operator () (const Arg1& arg1, Arg2& arg2) const { m_slot(arg1, arg2); return torrent::Object(); }

  Functor m_slot;
};

template <typename T>
object_convert_type<T, void>
object_convert_void(T f) { return f; }

#endif
