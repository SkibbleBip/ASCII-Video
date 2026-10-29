//! C-compatible ASCII video library.
//!
//! This crate exposes the ASCII video format as a small Rust library with a
//! stable C ABI. It is designed to be called from native code that needs to
//! generate ASCII video files or open/play existing ones without depending on
//! Rust-specific name mangling or type layout.
//!
//! The public interface intentionally targets raw C pointers and integer return
//! codes, mirroring the original C-style API used by the project. In Rust code,
//! the same functionality is also available through higher-level `&str` and
//! handle-based APIs.

use std::ffi::CStr;
use std::os::raw::{c_char, c_int};

mod bindings;

mod builder;
mod encoder;
mod player;

/// Re-exports the Rust file-creation API.
///
/// This function is the Rust-native entry point for generating an ASCII video
/// from a BMP file or a directory of BMP frames. It is also re-used by the C ABI
/// wrapper exported below.
pub use builder::create_video_file;

/// Creates an ASCII video file from a C string path and writes it to disk.
///
/// This function exposes the Rust implementation through a C-compatible ABI so
/// foreign code can call it without Rust name mangling. It validates both input
/// pointers, converts them from C strings to Rust strings, and delegates to
/// `create_video_file`.
///
/// # Parameters
///
/// * `input` - Pointer to a null-terminated C string containing the source BMP
///   file path or the directory containing frame images.
/// * `output` - Pointer to a null-terminated C string for the destination file.
/// * `fps` - Frames per second to store in the output file header.
///
/// # Returns
///
/// * `0` on success
/// * `1` on an internal create failure
/// * `-1` if either pointer is null or the C strings cannot be decoded
///
/// # Safety
///
/// The caller must ensure that both pointers are valid, non-null, and point to
/// properly terminated C strings with a lifetime that lasts for the duration of
/// the call.
#[no_mangle]
pub extern "C" fn createVideoFile(input: *const c_char, output: *const c_char, fps: u8) -> c_int {
    if input.is_null() || output.is_null() {
        return -1;
    }

    let input_str = unsafe {
        match CStr::from_ptr(input).to_str() {
            Ok(s) => s,
            Err(_) => return -1,
        }
    };

    let output_str = unsafe {
        match CStr::from_ptr(output).to_str() {
            Ok(s) => s,
            Err(_) => return -1,
        }
    };

    match create_video_file(input_str, output_str, fps) {
        Ok(()) => 0,
        Err(_) => 1,
    }
}

/// Re-exports the Rust playback API used by the C binding layer.
///
/// This includes the player handle type, the callback signature for terminal
/// output, and the frame-rendering helpers that are useful from both Rust and C
/// integrations.
pub use player::{open_ascii_video, play_file, render_frame, ASCIIfileHandle, PrinterT};

/// Opens an ASCII video file and returns a pointer-backed playback handle.
///
/// The handle is allocated as a Rust `Box<ASCIIfileHandle>` and stored into the
/// caller-provided pointer slot. This allows C code to manage the lifetime of the
/// playback state without needing to know the internal Rust representation.
///
/// # Parameters
///
/// * `path` - Pointer to a null-terminated C string containing the video path.
/// * `fileHandle` - Pointer to a pointer variable that receives the handle.
/// * `funct` - Printer callback used to render ASCII frames to the terminal.
///
/// # Returns
///
/// * The frame count on success
/// * `-1` if the file could not be opened or the header is invalid
///
/// # Safety
///
/// The caller must provide a valid pointer for `fileHandle` and a valid C string
/// for `path`. The pointer stored in `fileHandle` must be released later by
/// calling `closeASCIIvideo`.
#[no_mangle]
pub extern "C" fn openASCIIvideo(
    path: *const c_char,
    fileHandle: *mut *mut ASCIIfileHandle,
    funct: PrinterT,
) -> c_int {
    if path.is_null() {
        return -1;
    }

    let path_str = unsafe {
        match CStr::from_ptr(path).to_str() {
            Ok(s) => s,
            Err(_) => return -1,
        }
    };

    match open_ascii_video(path_str, funct) {
        Ok(handle) => {
            let frame_count: c_int = handle.header.frames as c_int;
            unsafe { *fileHandle = Box::into_raw(handle) };
            frame_count
        }
        Err(_) => -1,
    }
}

/// Releases a playback handle previously created by `openASCIIvideo`.
///
/// This function takes ownership of the raw pointer and destroys the Rust
/// `ASCIIfileHandle` instance, ensuring the associated gzip decoder and state are
/// cleaned up properly.
///
/// # Safety
///
/// The supplied pointer must be a valid handle created by `openASCIIvideo`; a
/// null pointer is ignored.
#[no_mangle]
pub extern "C" fn closeASCIIvideo(handle: *mut ASCIIfileHandle) {
    if handle.is_null() {
        return;
    }
    unsafe { drop(Box::from_raw(handle)) };
}

/// Starts playback of an already opened ASCII video handle.
///
/// This is the C ABI wrapper around the Rust `play_file` function. It returns a
/// status code that is compatible with native callers expecting simple integer
/// success indicators.
///
/// # Returns
///
/// * `0` on successful playback
/// * `-1` on an error reported by the decoder or renderer
/// * `1` if the supplied handle pointer is null
#[no_mangle]
pub extern "C" fn playFile(handle: *mut ASCIIfileHandle) -> c_int {
    if handle.is_null() {
        return 1;
    }
    let h = unsafe { &mut *handle };

    match play_file(h) {
        Ok(()) => 0,
        Err(e) => {
            eprintln!("{}", e);
            -1
        }
    }
}

/// Renders one frame from an open video handle using a playback speed factor.
///
/// The `speed` parameter is a floating-point multiplier applied to the frame
/// timing. A value of `0.0` requests the fastest possible rendering, while a
/// positive value slows playback relative to the recording's nominal frame rate.
///
/// # Returns
///
/// * The remaining number of frames to render on success
/// * `-1` if an internal rendering error occurs
/// * `0` if the handle is null
#[no_mangle]
pub extern "C" fn renderFrame(handle: *mut ASCIIfileHandle, speed: f32) -> i32 {
    if handle.is_null() {
        return 0;
    }
    let h = unsafe { &mut *handle };
    match render_frame(h, speed) {
        Ok(ret) => ret as i32,
        Err(e) => {
            eprintln!("{}", e);
            -1 as i32
        }
    }
}
