project(
    'rtorrent-mono',
    'cpp',
    version: '0.16.13',
    license: 'GPL-2.0-or-later',
    default_options: [
        'cpp_std=c++20',
        'warning_level=2',
        'buildtype=debugoptimized',
    ],
)

cpp = meson.get_compiler('cpp')
conf = configuration_data()

conf.set('HAVE_CONFIG_H', 1)
conf.set('HAVE_CXX20', 1)

# ── Project version constants (match configure.ac) ──
conf.set_quoted('PACKAGE', meson.project_name())
conf.set_quoted('VERSION', meson.project_version())
conf.set_quoted('PACKAGE_VERSION', meson.project_version())
conf.set_quoted(
    'PACKAGE_DATADIR',
    get_option('prefix') / get_option('datadir') / 'rtorrent',
)
conf.set('USER_AGENT', 'std::string(PACKAGE "/" VERSION)')

# libtorrent peer identification
conf.set_quoted('PEER_NAME', '-lt100C-')
conf.set_quoted('PEER_VERSION', 'lt\x10\x0C')

# API version
conf.set('API_VERSION', 21)

# ── Compiler flags ──
if get_option('debug')
    conf.set('DEBUG', 1)
    conf.set('USE_EXTRA_DEBUG', 1)
else
    conf.set('NDEBUG', 1)
endif


# ── Dependencies ──
libcurl_dep = dependency('libcurl')
zlib_dep = dependency('zlib')
openssl_dep = dependency('libcrypto')
conf.set('HAVE_OPENSSL', 1)

pthread_dep = dependency('threads')

# ── ncurses ──
ncurses_opt = get_option('ncurses')
if ncurses_opt.disabled()
    have_ncurses = false
    ncurses_dep = []
else
    ncurses_dep = dependency('ncursesw', required: false)
    if not ncurses_dep.found()
        ncurses_dep = dependency('ncurses', required: ncurses_opt.enabled())
    endif
    have_ncurses = ncurses_dep.found()
endif
if have_ncurses
    foreach ch : [
        ['ncursesw/curses.h', 'HAVE_NCURSESW_CURSES_H'],
        ['ncursesw.h', 'HAVE_NCURSESW_H'],
        ['ncurses/curses.h', 'HAVE_NCURSES_CURSES_H'],
        ['ncurses.h', 'HAVE_NCURSES_H'],
        ['curses.h', 'HAVE_CURSES_H'],
    ]
        if cpp.has_header(ch[0], dependencies: ncurses_dep)
            conf.set(ch[1], 1)
            break
        endif
    endforeach
else
    conf.set('HAVE_NO_NCURSES', 1)
endif
# ── Atomic support ──
atomic_test = '''
#include <cstdint>
#include <atomic>
int main() { std::atomic<uint64_t> x(0); return x.load(); }
'''
atomic_dep = []
if not cpp.links(atomic_test, name: '64-bit atomic without -latomic')
    atomic_dep = cpp.find_library('atomic', required: true)
endif

# ── epoll ──
epoll_test = '''
#include <sys/epoll.h>
int main() { int fd = epoll_create(100); (void)fd; return 0; }
'''
use_epoll = false
if get_option('epoll').allowed()
    if cpp.compiles(epoll_test, name: 'epoll support')
        use_epoll = true
        conf.set('USE_EPOLL', 1)
    elif get_option('epoll').enabled()
        error('epoll explicitly requested but not available')
    endif
endif

# ── kqueue ──
kqueue_test = '''
#include <sys/time.h>
#include <sys/event.h>
int main() { int fd = kqueue(); (void)fd; return 0; }
'''
use_kqueue = false
if get_option('kqueue').allowed()
    if cpp.links(kqueue_test, name: 'kqueue support')
        use_kqueue = true
        conf.set('USE_KQUEUE', 1)
    elif get_option('kqueue').enabled()
        error('kqueue explicitly requested but not available')
    endif
endif

if not use_epoll and not use_kqueue
    error('Must enable at least one of either kqueue or epoll')
endif

# ── inotify ──
inotify_test = '''
#include <sys/inotify.h>
int main(int, const char**) { return (-1 == inotify_init()); }
'''
if get_option('inotify').allowed()
    if cpp.compiles(inotify_test, name: 'inotify support')
        conf.set('USE_INOTIFY', 1)
    elif get_option('inotify').enabled()
        error('inotify explicitly requested but not available')
    endif
endif

# ── madvise ──
madvise_test = '''
#include <sys/types.h>
#include <sys/mman.h>
int main() { static char test[1024]; madvise((void*)test, sizeof(test), MADV_NORMAL); return 0; }
'''
if cpp.compiles(madvise_test, name: 'madvise')
    conf.set('USE_MADVISE', 1)
endif

# ── posix_fadvise ──
fadvise_test = '''
#include <fcntl.h>
int main() { posix_fadvise(0, 0, 0, POSIX_FADV_RANDOM); return 0; }
'''
if get_option('posix-fallocate').allowed()
    if cpp.compiles(fadvise_test, name: 'posix_fadvise')
        conf.set('USE_POSIX_FADVISE', 1)
    elif get_option('posix-fallocate').enabled()
        error('posix_fadvise explicitly requested but not available')
    endif
endif

# ── fallocate / posix_fallocate ──
if cpp.has_function('fallocate', prefix: '#include <fcntl.h>')
    conf.set('HAVE_FALLOCATE', 1)
endif
if cpp.has_function('posix_fallocate', prefix: '#include <fcntl.h>')
    conf.set('HAVE_POSIX_FALLOCATE', 1)
endif

# ── mincore signedness ──
mincore_unsigned_test = '''
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
int main() { mincore((char*)0, 0, (unsigned char*)0); return 0; }
'''
mincore_signed_test = '''
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
int main() { mincore((char*)0, 0, (char*)0); return 0; }
'''
if cpp.compiles(mincore_unsigned_test, name: 'mincore (unsigned)')
    conf.set('USE_MINCORE', 1)
    conf.set('USE_MINCORE_UNSIGNED', 1)
elif cpp.compiles(mincore_signed_test, name: 'mincore (signed)')
    conf.set('USE_MINCORE', 1)
    conf.set('USE_MINCORE_UNSIGNED', 0)
endif

# ── execinfo / backtrace ──
if get_option('execinfo').allowed()
    if cpp.has_header('execinfo.h')
        conf.set('HAVE_EXECINFO_H', 1)
        # Check if backtrace is available (may need -lexecinfo on BSD)
        if cpp.has_function('backtrace', prefix: '#include <execinfo.h>')
            conf.set('HAVE_BACKTRACE', 1)
        else
            execinfo_lib = cpp.find_library('execinfo', required: false)
            if execinfo_lib.found() and cpp.has_function(
                'backtrace',
                prefix: '#include <execinfo.h>',
                dependencies: execinfo_lib,
            )
                conf.set('HAVE_BACKTRACE', 1)
            endif
        endif
    elif get_option('execinfo').enabled()
        error('execinfo explicitly requested but not available')
    endif
endif

# ── visibility attribute ──
vis_test = '''
void __attribute__((visibility("default"))) f() {}
int main() { f(); return 0; }
'''
if get_option('visibility').allowed()
    if cpp.compiles(
        vis_test,
        args: '-fvisibility=hidden',
        name: 'visibility attribute',
    )
        conf.set('SUPPORT_ATTRIBUTE_VISIBILITY', 1)
    elif get_option('visibility').enabled()
        error('visibility attribute explicitly requested but not available')
    endif
endif

# ── cacheline size (LT_SMP_CACHE_BYTES) ──
host_cpu = host_machine.cpu_family()
if host_cpu == 'x86_64' or host_cpu == 'x86'
    conf.set('LT_SMP_CACHE_BYTES', 64)
elif host_cpu == 'aarch64' or host_cpu == 'arm' or host_cpu == 'ppc' or host_cpu == 'ppc64' or host_cpu == 's390x'
    conf.set('LT_SMP_CACHE_BYTES', 128)
else
    conf.set('LT_SMP_CACHE_BYTES', 64)
endif

# ── endian / 64-bit byte swap ──
endian_header = ''
if cpp.has_header('endian.h')
    endian_header = '<endian.h>'
elif cpp.has_header('sys/endian.h')
    endian_header = '<sys/endian.h>'
elif cpp.has_header('libkern/OSByteOrder.h')
    endian_header = '<libkern/OSByteOrder.h>'
else
    endian_header = '<stdint.h>'
endif
conf.set('CUSTOM_ENDIAN_HEADER', endian_header)
be64_test_endian = '''#include <stdint.h>
#include <endian.h>
int main() { uint64_t x = be64toh(1); (void)x; return 0; }'''
be64_test_sys_endian = '''#include <stdint.h>
#include <sys/endian.h>
int main() { uint64_t x = be64toh(1); (void)x; return 0; }'''
has_native_be64 = false
if endian_header == '<endian.h>'
    has_native_be64 = cpp.links(be64_test_endian, name: 'be64toh via endian.h')
elif endian_header == '<sys/endian.h>'
    has_native_be64 = cpp.links(
        be64_test_sys_endian,
        name: 'be64toh via sys/endian.h',
    )
endif


if has_native_be64
    conf.set('custom_ntohll(x)', '(be64toh(x))')
    conf.set('custom_htonll(x)', '(htobe64(x))')
elif endian_header == '<libkern/OSByteOrder.h>'
    conf.set('custom_ntohll(x)', '(OSSwapBigToHostInt64(x))')
    conf.set('custom_htonll(x)', '(OSSwapHostToBigInt64(x))')
else
    if host_machine.endian() == 'big'
        conf.set('custom_ntohll(x)', '((uint64_t)(x))')
        conf.set('custom_htonll(x)', '((uint64_t)(x))')
    else
        conf.set('custom_ntohll(x)', '(__builtin_bswap64((uint64_t)(x)))')
        conf.set('custom_htonll(x)', '(__builtin_bswap64((uint64_t)(x)))')
    endif
endif

# ── pthread_setname_np ──
setname_generic_test = '''
#define _GNU_SOURCE
#include <pthread.h>
int main() { pthread_t t; pthread_setname_np(t, "foo"); return 0; }
'''
setname_darwin_test = '''
#include <pthread.h>
int main() { pthread_t t; pthread_setname_np("foo"); (void)t; return 0; }
'''
if get_option('pthread-setname-np').allowed()
    if cpp.links(setname_generic_test, name: 'pthread_setname_np (generic)')
        conf.set('HAS_PTHREAD_SETNAME_NP_GENERIC', 1)
    elif cpp.links(setname_darwin_test, name: 'pthread_setname_np (darwin)')
        conf.set('HAS_PTHREAD_SETNAME_NP_DARWIN', 1)
    elif get_option('pthread-setname-np').enabled()
        error('pthread_setname_np explicitly requested but not available')
    endif
endif

# ── pthread_setstacksize ──
if get_option('pthread-setstacksize').allowed()
    # Auto-enable for musl
    is_musl = host_machine.system() == 'linux' and cpp.has_function(
        '__libc_start_main',
    ) == false
    if get_option('pthread-setstacksize').enabled() or is_musl
        conf.set('USE_PTHREAD_SETSTACKSIZE', 1)
        conf.set('DEFAULT_PTHREAD_SETSTACKSIZE', '(8 * 1024 * 1024)')
    endif
endif

# ── address space size ──
addr_space = get_option('address-space')
if addr_space > 0
    conf.set('DEFAULT_ADDRESS_SPACE_SIZE', addr_space)
else
    if host_machine.cpu_family() == 'x86_64' or host_machine.cpu_family() == 'aarch64' or host_machine.cpu_family() == 'ppc64' or host_machine.cpu_family() == 's390x'
        conf.set('DEFAULT_ADDRESS_SPACE_SIZE', 32768)
    else
        conf.set('DEFAULT_ADDRESS_SPACE_SIZE', 1024)
    endif
endif

# ── instrumentation ──
if get_option('instrumentation')
    conf.set('LT_INSTRUMENTATION', 1)
endif

# ── RPC backend (rtorrent) ──
rpc_backend = get_option('rpc-backend')
if rpc_backend == 'auto'
    rpc_backend = 'tinyxml2'
endif
if rpc_backend == 'tinyxml2'
    conf.set('HAVE_XMLRPC_TINYXML2', 1)
elif rpc_backend == 'xmlrpc-c'
    conf.set('HAVE_XMLRPC_C', 1)
endif

# ── Lua ──
have_lua = false
lua_dep = []
if get_option('lua').allowed()
    foreach name : ['lua', 'lua5.4', 'lua5.3', 'luajit', 'luajit-5.1']
        lua_dep = dependency(name, required: false)
        if lua_dep.found()
            break
        endif
    endforeach
    if lua_dep.found()
        conf.set('HAVE_LUA', 1)
        have_lua = true
        conf.set_quoted(
            'LUA_DATADIR',
            get_option('prefix') / get_option('datadir') / 'rtorrent' / 'lua',
        )
    elif get_option('lua').enabled()
        error('Lua support requested but not found')
    endif
endif

# ── systemd ──
have_systemd = false
systemd_dep = []
if get_option('systemd').allowed()
    systemd_dep = dependency('libsystemd', required: get_option('systemd'))
    if systemd_dep.found()
        conf.set('HAVE_SYSTEMD', 1)
        have_systemd = true
    endif
endif

# ── Common headers ──
foreach h : [
    'sys/inotify.h',
    'endian.h',
    'sys/endian.h',
    'libkern/OSByteOrder.h',
    'execinfo.h',
    'pthread.h',
    'sys/mman.h',
    'fcntl.h',
]
    if cpp.has_header(h)
        conf.set('HAVE_' + h.underscorify().to_upper(), 1)
    endif
endforeach

# ── Generate config.h ──
config_h = configure_file(output: 'config.h', configuration: conf)



# ──────────────────────────────────────────────────────────
#  libtorrent
# ──────────────────────────────────────────────────────────

libtorrent_sources = files(
    # src/ top-level
    'libtorrent/src/manager.cc',
    'libtorrent/src/runtime.cc',
    'libtorrent/src/thread_main.cc',
    # data/
    'libtorrent/src/data/chunk.cc',
    'libtorrent/src/data/chunk_list.cc',
    'libtorrent/src/data/chunk_part.cc',
    'libtorrent/src/data/hash_check_queue.cc',
    'libtorrent/src/data/hash_chunk.cc',
    'libtorrent/src/data/hash_queue.cc',
    'libtorrent/src/data/hash_queue_node.cc',
    'libtorrent/src/data/hash_torrent.cc',
    'libtorrent/src/data/memory_chunk.cc',
    'libtorrent/src/data/socket_file.cc',
    'libtorrent/src/data/thread_disk.cc',
    # dht/
    'libtorrent/src/dht/dht_bucket.cc',
    'libtorrent/src/dht/dht_node.cc',
    'libtorrent/src/dht/dht_router.cc',
    'libtorrent/src/dht/dht_server.cc',
    'libtorrent/src/dht/dht_tracker.cc',
    'libtorrent/src/dht/dht_transaction.cc',
    'libtorrent/src/dht/transactions/dht_announce.cc',
    'libtorrent/src/dht/transactions/dht_search.cc',
    # download/
    'libtorrent/src/download/available_list.cc',
    'libtorrent/src/download/chunk_selector.cc',
    'libtorrent/src/download/chunk_statistics.cc',
    'libtorrent/src/download/delegator.cc',
    'libtorrent/src/download/download_constructor.cc',
    'libtorrent/src/download/download_main.cc',
    'libtorrent/src/download/download_wrapper.cc',
    # net/
    'libtorrent/src/net/address_list.cc',
    'libtorrent/src/net/curl_get.cc',
    'libtorrent/src/net/curl_socket.cc',
    'libtorrent/src/net/curl_stack.cc',
    'libtorrent/src/net/dns_buffer.cc',
    'libtorrent/src/net/dns_cache.cc',
    'libtorrent/src/net/event_fd.cc',
    'libtorrent/src/net/listen.cc',
    'libtorrent/src/net/socket_datagram.cc',
    'libtorrent/src/net/socket_stream.cc',
    'libtorrent/src/net/thread_net.cc',
    'libtorrent/src/net/throttle_internal.cc',
    'libtorrent/src/net/throttle_list.cc',
    'libtorrent/src/net/udns_library.cc',
    'libtorrent/src/net/udns_resolver.cc',
    # protocol/
    'libtorrent/src/protocol/extensions.cc',
    'libtorrent/src/protocol/handshake.cc',
    'libtorrent/src/protocol/handshake_encryption.cc',
    'libtorrent/src/protocol/handshake_manager.cc',
    'libtorrent/src/protocol/initial_seed.cc',
    'libtorrent/src/protocol/peer_connection_base.cc',
    'libtorrent/src/protocol/peer_connection_leech.cc',
    'libtorrent/src/protocol/peer_connection_metadata.cc',
    'libtorrent/src/protocol/peer_factory.cc',
    'libtorrent/src/protocol/request_list.cc',
    # tracker/
    'libtorrent/src/tracker/thread_tracker.cc',
    'libtorrent/src/tracker/tracker_controller.cc',
    'libtorrent/src/tracker/tracker_dht.cc',
    'libtorrent/src/tracker/tracker_http.cc',
    'libtorrent/src/tracker/tracker_list.cc',
    'libtorrent/src/tracker/tracker_udp.cc',
    'libtorrent/src/tracker/tracker_worker.cc',
    'libtorrent/src/tracker/udp_router.cc',
    # utils/
    'libtorrent/src/utils/diffie_hellman.cc',
    'libtorrent/src/utils/instrumentation.cc',
    # torrent/ (public API)
    'libtorrent/src/torrent/bitfield.cc',
    'libtorrent/src/torrent/chunk_manager.cc',
    'libtorrent/src/torrent/connection_manager.cc',
    'libtorrent/src/torrent/download.cc',
    'libtorrent/src/torrent/event.cc',
    'libtorrent/src/torrent/exceptions.cc',
    'libtorrent/src/torrent/object.cc',
    'libtorrent/src/torrent/object_static_map.cc',
    'libtorrent/src/torrent/object_stream.cc',
    'libtorrent/src/torrent/path.cc',
    'libtorrent/src/torrent/rate.cc',
    'libtorrent/src/torrent/throttle.cc',
    'libtorrent/src/torrent/torrent.cc',
    # torrent/data/
    'libtorrent/src/torrent/data/block.cc',
    'libtorrent/src/torrent/data/block_list.cc',
    'libtorrent/src/torrent/data/chunk_utils.cc',
    'libtorrent/src/torrent/data/download_data.cc',
    'libtorrent/src/torrent/data/file.cc',
    'libtorrent/src/torrent/data/file_list.cc',
    'libtorrent/src/torrent/data/file_list_iterator.cc',
    'libtorrent/src/torrent/data/file_manager.cc',
    'libtorrent/src/torrent/data/file_utils.cc',
    'libtorrent/src/torrent/data/transfer_list.cc',
    # torrent/download/
    'libtorrent/src/torrent/download/choke_group.cc',
    'libtorrent/src/torrent/download/choke_queue.cc',
    'libtorrent/src/torrent/download/download_manager.cc',
    'libtorrent/src/torrent/download/resource_manager.cc',
    # torrent/net/
    'libtorrent/src/torrent/net/address_info.cc',
    'libtorrent/src/torrent/net/connection_state.cc',
    'libtorrent/src/torrent/net/fd.cc',
    'libtorrent/src/torrent/net/http_get.cc',
    'libtorrent/src/torrent/net/http_stack.cc',
    'libtorrent/src/torrent/net/resolver.cc',
    'libtorrent/src/torrent/net/socket_address.cc',
    'libtorrent/src/torrent/net/socket_address_key.cc',
    # torrent/peer/
    'libtorrent/src/torrent/peer/client_info.cc',
    'libtorrent/src/torrent/peer/client_list.cc',
    'libtorrent/src/torrent/peer/connection_list.cc',
    'libtorrent/src/torrent/peer/peer.cc',
    'libtorrent/src/torrent/peer/peer_info.cc',
    'libtorrent/src/torrent/peer/peer_list.cc',
    # torrent/runtime/
    'libtorrent/src/torrent/runtime/network_config.cc',
    'libtorrent/src/torrent/runtime/network_manager.cc',
    'libtorrent/src/torrent/runtime/runtime.cc',
    'libtorrent/src/torrent/runtime/socket_manager.cc',
    # torrent/system/
    'libtorrent/src/torrent/system/poll_epoll.cc',
    'libtorrent/src/torrent/system/poll_kqueue.cc',
    'libtorrent/src/torrent/system/system.cc',
    'libtorrent/src/torrent/system/thread.cc',
    # torrent/tracker/
    'libtorrent/src/torrent/tracker/dht_controller.cc',
    'libtorrent/src/torrent/tracker/manager.cc',
    'libtorrent/src/torrent/tracker/tracker.cc',
    'libtorrent/src/torrent/tracker/wrappers.cc',
    # torrent/types/
    'libtorrent/src/torrent/types/string_utf8.cc',
    # torrent/utils/
    'libtorrent/src/torrent/utils/directory_events.cc',
    'libtorrent/src/torrent/utils/log.cc',
    'libtorrent/src/torrent/utils/log_buffer.cc',
    'libtorrent/src/torrent/utils/option_strings.cc',
    'libtorrent/src/torrent/utils/random.cc',
    'libtorrent/src/torrent/utils/resume.cc',
    'libtorrent/src/torrent/utils/scheduler.cc',
    'libtorrent/src/torrent/utils/string_manip.cc',
    'libtorrent/src/torrent/utils/uri_parser.cc',
)


libtorrent_deps = [libcurl_dep, zlib_dep, openssl_dep, pthread_dep, atomic_dep]

libtorrent_lib = static_library(
    'torrent',
    libtorrent_sources,
    include_directories: include_directories('libtorrent', 'libtorrent/src', 'libtorrent/src/torrent'),
    dependencies: libtorrent_deps,
)

libtorrent_dep_internal = declare_dependency(
    link_with: libtorrent_lib,
    include_directories: include_directories('libtorrent', 'libtorrent/src', 'libtorrent/src/torrent'),
)
# ──────────────────────────────────────────────────────────

rtorrent_sources = files(
    'rtorrent/src/main.cc',
    # command_*
    'rtorrent/src/command_download.cc',
    'rtorrent/src/command_dynamic.cc',
    'rtorrent/src/command_events.cc',
    'rtorrent/src/command_file.cc',
    'rtorrent/src/command_groups.cc',
    'rtorrent/src/command_helpers.cc',
    'rtorrent/src/command_ip.cc',
    'rtorrent/src/command_local.cc',
    'rtorrent/src/command_logging.cc',
    'rtorrent/src/command_network.cc',
    'rtorrent/src/command_peer.cc',
    'rtorrent/src/command_scheduler.cc',
    'rtorrent/src/command_throttle.cc',
    'rtorrent/src/command_tracker.cc',
    'rtorrent/src/command_ui.cc',
    # control, globals, etc.
    'rtorrent/src/control.cc',
    'rtorrent/src/globals.cc',
    'rtorrent/src/option_parser.cc',
    'rtorrent/src/setup.cc',
    'rtorrent/src/signal_handler.cc',
    # core/
    'rtorrent/src/core/dht_manager.cc',
    'rtorrent/src/core/download.cc',
    'rtorrent/src/core/download_factory.cc',
    'rtorrent/src/core/download_list.cc',
    'rtorrent/src/core/http_queue.cc',
    'rtorrent/src/core/manager.cc',
    'rtorrent/src/core/view.cc',
    'rtorrent/src/core/view_manager.cc',
    # display/
    'rtorrent/src/display/canvas.cc',
    'rtorrent/src/display/frame.cc',
    'rtorrent/src/display/manager.cc',
    'rtorrent/src/display/text_element_list.cc',
    'rtorrent/src/display/text_element_string.cc',
    'rtorrent/src/display/text_element_value.cc',
    'rtorrent/src/display/utils.cc',
    'rtorrent/src/display/window.cc',
    'rtorrent/src/display/window_download_chunks_seen.cc',
    'rtorrent/src/display/window_download_list.cc',
    'rtorrent/src/display/window_download_statusbar.cc',
    'rtorrent/src/display/window_download_transfer_list.cc',
    'rtorrent/src/display/window_file_list.cc',
    'rtorrent/src/display/window_http_queue.cc',
    'rtorrent/src/display/window_input.cc',
    'rtorrent/src/display/window_log.cc',
    'rtorrent/src/display/window_log_complete.cc',
    'rtorrent/src/display/window_peer_list.cc',
    'rtorrent/src/display/window_statusbar.cc',
    'rtorrent/src/display/window_string_list.cc',
    'rtorrent/src/display/window_text.cc',
    'rtorrent/src/display/window_title.cc',
    'rtorrent/src/display/window_tracker_list.cc',
    # input/
    'rtorrent/src/input/bindings.cc',
    'rtorrent/src/input/input_event.cc',
    'rtorrent/src/input/manager.cc',
    'rtorrent/src/input/path_input.cc',
    'rtorrent/src/input/text_input.cc',
    # rpc/
    'rtorrent/src/rpc/command.cc',
    'rtorrent/src/rpc/command_map.cc',
    'rtorrent/src/rpc/command_scheduler.cc',
    'rtorrent/src/rpc/command_scheduler_item.cc',
    'rtorrent/src/rpc/exec_file.cc',
    'rtorrent/src/rpc/jsonrpc.cc',
    'rtorrent/src/rpc/lua.cc',
    'rtorrent/src/rpc/object_storage.cc',
    'rtorrent/src/rpc/parse.cc',
    'rtorrent/src/rpc/parse_commands.cc',
    'rtorrent/src/rpc/parse_options.cc',
    'rtorrent/src/rpc/rpc_manager.cc',
    'rtorrent/src/rpc/scgi.cc',
    'rtorrent/src/rpc/scgi_task.cc',
    'rtorrent/src/rpc/tinyxml2/tinyxml2.cc',
    'rtorrent/src/rpc/xmlrpc.cc',
    'rtorrent/src/rpc/xmlrpc_c.cc',
    'rtorrent/src/rpc/xmlrpc_tinyxml2.cc',
    # scgi/
    'rtorrent/src/scgi/thread_scgi.cc',
    # session/
    'rtorrent/src/session/download_storer.cc',
    'rtorrent/src/session/session_manager.cc',
    'rtorrent/src/session/thread_session.cc',
    # ui/
    'rtorrent/src/ui/download.cc',
    'rtorrent/src/ui/download_list.cc',
    'rtorrent/src/ui/element_base.cc',
    'rtorrent/src/ui/element_chunks_seen.cc',
    'rtorrent/src/ui/element_download_list.cc',
    'rtorrent/src/ui/element_file_list.cc',
    'rtorrent/src/ui/element_log_complete.cc',
    'rtorrent/src/ui/element_menu.cc',
    'rtorrent/src/ui/element_peer_list.cc',
    'rtorrent/src/ui/element_string_list.cc',
    'rtorrent/src/ui/element_text.cc',
    'rtorrent/src/ui/element_tracker_list.cc',
    'rtorrent/src/ui/element_transfer_list.cc',
    'rtorrent/src/ui/root.cc',
    # utils/
    'rtorrent/src/utils/base64.cc',
    'rtorrent/src/utils/directory.cc',
    'rtorrent/src/utils/file_status_cache.cc',
    'rtorrent/src/utils/gzip.cc',
    'rtorrent/src/utils/lockfile.cc',
    'rtorrent/src/utils/watch_ready_queue.cc',
)

if not have_ncurses
    rtorrent_sources += files('rtorrent/src/display/curses_stub.cc')
endif

rtorrent_deps = [libtorrent_dep_internal, zlib_dep, pthread_dep, atomic_dep]
if have_ncurses
    rtorrent_deps += ncurses_dep
endif
if rpc_backend == 'xmlrpc-c'
    xmlrpc_dep = dependency('xmlrpc-c', required: true)
    rtorrent_deps += xmlrpc_dep
endif
if have_lua
    rtorrent_deps += lua_dep
endif
if have_systemd
    rtorrent_deps += systemd_dep
endif

rtorrent_inc = include_directories(
    'rtorrent',
    'rtorrent/src',
    'libtorrent',
    'libtorrent/src',
    'libtorrent/src/torrent',
)

rtorrent_exe = executable(
    'rtorrent',
    rtorrent_sources,
    include_directories: rtorrent_inc,
    dependencies: rtorrent_deps,
    install: true,
)

# ── Install rtorrent data files ──
install_data(
    'rtorrent/lua/rtorrent.lua',
    install_dir: get_option('datadir') / 'rtorrent' / 'lua',
)

# ──────────────────────────────────────────────────────────
#  Tests
# ──────────────────────────────────────────────────────────

if get_option('tests').allowed()
    cppunit_dep = dependency('cppunit', required: get_option('tests'))
    if cppunit_dep.found()
        test_cflags = ['-DLT_CPPUNIT_TESTING']

        # ── libtorrent tests ──
        lt_test_common = files(
            'libtorrent/test/main.cc',
            'libtorrent/test/helpers/mock_function.cc',
            'libtorrent/test/helpers/network.cc',
            'libtorrent/test/helpers/progress_listener.cc',
            'libtorrent/test/helpers/protectors.cc',
            'libtorrent/test/helpers/test_fixture.cc',
            'libtorrent/test/helpers/test_main_thread.cc',
            'libtorrent/test/helpers/test_thread.cc',
            'libtorrent/test/helpers/tracker_test.cc',
        )

        lt_test_inc = include_directories('libtorrent/test', 'libtorrent/src')

        lt_test_args = [
            [
                lt_test_common + files(
                    'libtorrent/test/torrent/net/test_address_info.cc',
                    'libtorrent/test/torrent/net/test_fd.cc',
                    'libtorrent/test/torrent/net/test_socket_address.cc',
                ),
                'LibTorrent_Test_Torrent_Net',
            ],
            [
                lt_test_common + files(
                    'libtorrent/test/torrent/utils/test_extents.cc',
                    'libtorrent/test/torrent/utils/test_log.cc',
                    'libtorrent/test/torrent/utils/test_log_buffer.cc',
                    'libtorrent/test/torrent/utils/directory_events_test.cc',
                    'libtorrent/test/torrent/utils/test_option_strings.cc',
                    'libtorrent/test/torrent/utils/test_queue_buckets.cc',
                    'libtorrent/test/torrent/utils/test_thread_base.cc',
                    'libtorrent/test/torrent/utils/test_uri_parser.cc',
                ),
                'LibTorrent_Test_Torrent_Utils',
            ],
            [
                lt_test_common + files(
                    'libtorrent/test/torrent/object_test.cc',
                    'libtorrent/test/torrent/object_test_utils.cc',
                    'libtorrent/test/torrent/object_static_map_test.cc',
                    'libtorrent/test/torrent/object_stream_test.cc',
                    'libtorrent/test/torrent/test_tracker_controller.cc',
                    'libtorrent/test/torrent/test_tracker_controller_features.cc',
                    'libtorrent/test/torrent/test_tracker_controller_requesting.cc',
                    'libtorrent/test/torrent/test_tracker_list.cc',
                    'libtorrent/test/torrent/test_tracker_list_features.cc',
                    'libtorrent/test/torrent/test_tracker_timeout.cc',
                ),
                'LibTorrent_Test_Torrent',
            ],
            [
                lt_test_common + files(
                    'libtorrent/test/data/test_chunk_list.cc',
                    'libtorrent/test/data/test_hash_check_queue.cc',
                    'libtorrent/test/data/test_hash_queue.cc',
                ),
                'LibTorrent_Test_Data',
            ],
            [lt_test_common, 'LibTorrent_Test_Net'],
            [
                lt_test_common + files(
                    'libtorrent/test/tracker/test_tracker_http.cc',
                ),
                'LibTorrent_Test_Tracker',
            ],
            [
                lt_test_common + files(
                    'libtorrent/test/rak/ranges_test.cc',
                    'libtorrent/test/protocol/test_request_list.cc',
                ),
                'LibTorrent_Test',
            ],
        ]


        foreach t : lt_test_args
            exe = executable(
                t[1],
                t[0],
                include_directories: lt_test_inc,
                link_with: libtorrent_lib,
                dependencies: [cppunit_dep, pthread_dep, atomic_dep],
                cpp_args: test_cflags,
            )
            test(t[1], exe)
        endforeach

        # ── rtorrent tests ──
        # Static lib for rtorrent tests (mirrors libsub_root.a)
        rtorrent_test_lib = static_library(
            'rtorrent_test_lib',
            rtorrent_sources,
            include_directories: rtorrent_inc,
            dependencies: rtorrent_deps,
        )

        rt_test_common = files(
            'rtorrent/test/main.cc',
            'rtorrent/test/helpers/mock_function.cc',
            'rtorrent/test/helpers/progress_listener.cc',
            'rtorrent/test/helpers/protectors.cc',
            'rtorrent/test/helpers/test_fixture.cc',
            'rtorrent/test/helpers/test_main_thread.cc',
            'rtorrent/test/helpers/test_thread.cc',
        )

        rt_test_inc = include_directories(
            'rtorrent/test',
            'rtorrent/src',
            'libtorrent/src',
        )

        rt_test_args = [
            [
                rt_test_common + files(
                    'rtorrent/test/rpc/test_command.cc',
                    'rtorrent/test/rpc/test_command_map.cc',
                    'rtorrent/test/rpc/test_jsonrpc.cc',
                    'rtorrent/test/rpc/test_xmlrpc.cc',
                    'rtorrent/test/rpc/test_command_slot.cc',
                    'rtorrent/test/rpc/test_object_storage.cc',
                    'rtorrent/test/rpc/test_parse_options.cc',
                ),
                'rtorrent_Test_Rpc',
            ],
            [
                rt_test_common + files(
                    'rtorrent/test/src/test_command_dynamic.cc',
                    'rtorrent/test/src/test_watch_ready_queue.cc',
                ),
                'rtorrent_Test_Src',
            ],
        ]

        foreach t : rt_test_args
            exe = executable(
                t[1],
                t[0],
                include_directories: rt_test_inc,
                link_with: [rtorrent_test_lib, libtorrent_lib],
                dependencies: [cppunit_dep, pthread_dep, atomic_dep],
                cpp_args: test_cflags,
            )
            test(t[1], exe)
        endforeach
    endif
endif
