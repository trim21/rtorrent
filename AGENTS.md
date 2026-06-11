# AGENTS.md

## Project Overview

This is a monorepo that wraps the upstream [rakshasa/rtorrent](https://github.com/rakshasa/rtorrent) and [rakshasa/libtorrent](https://github.com/rakshasa/libtorrent) projects via git submodules, with a Meson build system on top.

- **libtorrent/** — BitTorrent protocol library (submodule from `rakshasa/libtorrent`)
- **rtorrent/** — BitTorrent client application (submodule from `rakshasa/rtorrent`, depends on libtorrent)

## Build

The project uses Meson. libtorrent is built as a static library and linked into rtorrent.

### Quick Build

```bash
meson setup build
meson compile -C build
```

### Common Options

```bash
# Daemon mode (no ncurses)
meson setup build -Dncurses=disabled

# With Lua support
meson setup build -Dlua=enabled

# XML-RPC backend (default: auto -> tinyxml2)
meson setup build -Drpc-backend=tinyxml2
```

## Lint & Type Check

```bash
bear -- meson compile -C build   # generate compile_commands.json
clang-tidy -p build libtorrent/src/**/*.cc rtorrent/src/**/*.cc
```

## Tests

```bash
meson test -C build
```

## Project Structure

```
.
├── meson.build           # Top-level Meson build (manages both projects)
├── meson.options         # Build options
├── libtorrent/           # BitTorrent protocol library (submodule)
│   ├── src/              # Source code
│   │   ├── data/         # Chunk management, disk I/O
│   │   ├── dht/          # Distributed Hash Table
│   │   ├── download/     # Download management
│   │   ├── net/          # Networking (libcurl, udns vendored)
│   │   ├── protocol/     # BitTorrent protocol, encryption
│   │   ├── tracker/      # Tracker communication
│   │   ├── utils/        # Utilities
│   │   └── torrent/      # Public API headers
│   ├── test/             # CppUnit tests
│   └── scripts/          # Autoconf M4 macros
├── rtorrent/             # BitTorrent client (submodule)
│   ├── src/              # Source code
│   │   ├── core/         # Core application logic
│   │   ├── rpc/          # XML-RPC/JSON-RPC
│   │   ├── ui/           # ncurses interface
│   │   └── display/      # Display/rendering
│   ├── test/             # CppUnit tests
│   └── lua/              # Lua scripting support
```

## Key Technical Details

- **C++ standard:** C++20
- **Dependencies:** libcurl, zlib, OpenSSL, pthreads (required); ncurses, Lua/LuaJIT, systemd (optional)
- **Vendored code:** tinyxml2, nlohmann/json, udns
- **File extensions:** `.cc` (source), `.h` (headers)

## Upstream Sync

```bash
git submodule update --remote libtorrent
git submodule update --remote rtorrent
```

## Running

```bash
./build/rtorrent -n -o scgi_port=0.0.0.0:5101
```
