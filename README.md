# ASCII-VIDEO

## What is this?
This is a file formatting and video player to display monochromatic non-audio videos in ASCII format on the terminal. 
## How does this work? 
The video "encoder" and file generator takes in the location of a directory that contains each individual frame of the video saved as an uncompressed 24-bit BMP file in the name formatting as `%03d[...]`.

## Usages
If a user were to make a terminal-based game and wanted a little cutscene, they could use this implementation of this video player. Also, I have gotten "video" to work on embedded devices that do not have a full desktop graphics stack.

## Compiling
This project is now built primarily as a Rust crate, while keeping a C-compatible ABI for compatibility with existing native code. The library is exported from `src/lib.rs` using `extern "C"` functions, and the crate is configured in `Cargo.toml` to build as both a static library and a shared library.

From the repository root, compile it with Cargo:

```bash
cargo build
```

For an optimized release build:

```bash
cargo build --release
```

The compiled artifacts will be placed in `target/debug` or `target/release`. Depending on your platform, you will typically see:

- `libascii.a` for a static library
- `libascii.so`, `libascii.dylib`, or `ascii.dll` for a shared library

This means the Rust implementation is the main source of truth, but C code can still link against the same library through the original FFI entry points. If you are consuming the library from C, link against the generated static or dynamic artifact in the target directory.

## License
Copyright (C) 2021 SkibbleBip

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
