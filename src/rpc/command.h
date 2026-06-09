#ifndef RTORRENT_RPC_COMMAND_H
#define RTORRENT_RPC_COMMAND_H

#include <functional>
#include <type_traits>
#include <torrent/common.h>
#include <torrent/object.h>
#include <torrent/data/file_list_iterator.h>

namespace core {
  class Download;
}

namespace rpc {

template <typename Target>
struct target_wrapper {
  typedef Target      target_type;
  typedef Target      cleaned_type;
};

template <>
struct target_wrapper<void> {
  struct no_type {};

  typedef void        target_type;
  typedef no_type*    cleaned_type;
};

template <typename T1, typename T2, typename T3>
struct rt_triple : private std::pair<T1, T2> {
  typedef std::pair<T1, T2> base_type;
  typedef T3                third_type;

  using base_type::first;
  using base_type::second;
  using typename base_type::first_type;
  using typename base_type::second_type;

  T3 third;

  rt_triple() : base_type(), third() {}

  rt_triple(const T1& a, const T2& b) :
    base_type(a, b), third() {}

  rt_triple(const T1& a, const T2& b, const T3& c) :
    base_type(a, b), third(c) {}

  rt_triple(const base_type& b) : base_type(b), third() {}

  rt_triple(const rt_triple& src) :
    base_type(src.first, src.second), third(src.third) {}
};

typedef rt_triple<int, void*, void*> target_type;

class command_base;

typedef std::function<torrent::Object (target_type, const torrent::Object&)> base_function;

class command_base {
public:
  typedef torrent::Object::value_type  value_type;
  typedef torrent::Object::string_type string_type;
  typedef torrent::Object::list_type   list_type;
  typedef torrent::Object::map_type    map_type;
  typedef torrent::Object::key_type    key_type;

  typedef const torrent::Object (*generic_slot)  (command_base*, const torrent::Object&);
  typedef const torrent::Object (*cleaned_slot)  (command_base*, target_wrapper<void>::cleaned_type, const torrent::Object&);
  typedef const torrent::Object (*any_slot)      (command_base*, target_type, const torrent::Object&);
  typedef const torrent::Object (*download_slot) (command_base*, core::Download*, const torrent::Object&);
  typedef const torrent::Object (*file_slot)     (command_base*, torrent::File*, const torrent::Object&);
  typedef const torrent::Object (*file_itr_slot) (command_base*, torrent::FileListIterator*, const torrent::Object&);
  typedef const torrent::Object (*peer_slot)     (command_base*, torrent::Peer*, const torrent::Object&);
  typedef const torrent::Object (*tracker_slot)  (command_base*, torrent::tracker::Tracker*, const torrent::Object&);

  typedef const torrent::Object (*download_pair_slot) (command_base*, core::Download*, core::Download*, const torrent::Object&);

  static const int target_generic  = 0;
  static const int target_any      = 1;
  static const int target_download = 2;
  static const int target_peer     = 3;
  static const int target_tracker  = 4;
  static const int target_file     = 5;
  static const int target_file_itr = 6;

  static const int target_download_pair = 7;

  static const unsigned int max_arguments = 10;

  struct stack_type {
    torrent::Object*       begin() { return reinterpret_cast<torrent::Object*>(buffer); }
    torrent::Object*       end()   { return reinterpret_cast<torrent::Object*>(buffer) + max_arguments; }

    const torrent::Object* begin() const { return reinterpret_cast<const torrent::Object*>(buffer); }
    const torrent::Object* end()   const { return reinterpret_cast<const torrent::Object*>(buffer) + max_arguments; }

    torrent::Object&       operator [] (unsigned int idx)       { return *(begin() + idx); }
    const torrent::Object& operator [] (unsigned int idx) const { return *(begin() + idx); }

    static stack_type* from_data(char* data) { return reinterpret_cast<stack_type*>(data); }

    char buffer[sizeof(torrent::Object) * max_arguments];
  };

  command_base() = default;
  ~command_base() = default;

  static torrent::Object* argument(unsigned int index) { return current_stack.begin() + index; }
  static torrent::Object& argument_ref(unsigned int index) { return *(current_stack.begin() + index); }

  static stack_type current_stack;

  static torrent::Object* stack_begin() { return current_stack.begin(); }
  static torrent::Object* stack_end()   { return current_stack.end(); }

  static torrent::Object* push_stack(const torrent::Object::list_type& args, stack_type* stack);
  static torrent::Object* push_stack(const torrent::Object* first_arg, const torrent::Object* last_arg, stack_type* stack);
  static void             pop_stack(stack_type* stack, torrent::Object* last_stack);

  void set_function(base_function f) { t_pod = std::move(f); }

  torrent::Object call(const target_type& t, const torrent::Object& o) const { return t_pod(t, o); }

protected:
  base_function t_pod;
};

template <typename T1 = void, typename T2 = void>
struct target_type_id {};

template <typename T> inline bool
is_target_compatible(const target_type& target) { return target.first == target_type_id<T>::value; }

inline bool is_target_pair(const target_type& target) { return target.first >= command_base::target_download_pair; }

template <typename T> inline T
get_target_cast(target_type target, [[maybe_unused]] int type = target_type_id<T>::value) { return (T)target.second; }

inline target_type get_target_left(const target_type& target)  { return target_type(target.first - 5, target.second); }
inline target_type get_target_right(const target_type& target) { return target_type(target.first - 5, target.third); }

torrent::Object::value_type convert_to_value_arg(const torrent::Object& obj, int base, int unit);
std::string              convert_to_string_arg(const torrent::Object& obj);
torrent::Object::list_type convert_to_list_arg(const torrent::Object& obj);

template<typename T>
inline T extract_target(const target_type& t) {
  if constexpr (std::is_same_v<T, target_type>) {
    return t;
  } else {
    if (!is_target_compatible<T>(t))
      throw torrent::input_error("Target of wrong type to command.");
    return static_cast<T>(t.second);
  }
}

}

#include "command_impl.h"

#endif
