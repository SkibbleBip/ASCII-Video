use std::ffi::CStr;
use std::os::raw::{c_char, c_int};

mod bindings;

mod builder;
mod encoder;
mod player;

/// Creates an ASCII video file from a BMP input path and writes it to disk.
pub use builder::create_video_file;

/// Creates an ASCII video file from a C-style path string.
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

/// Exposes the player API and runtime handle types for external callers.
pub use player::{open_ascii_video, play_file, render_frame, ASCIIfileHandle, PrinterT};

/// Opens an ASCII video file from a C path and returns a handle for playback.
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

/// Releases the memory backing an open ASCII video handle.
#[no_mangle]
pub extern "C" fn closeASCIIvideo(handle: *mut ASCIIfileHandle) {
    if handle.is_null() {
        return;
    }
    unsafe { drop(Box::from_raw(handle)) };
}

/// Starts playback for an existing ASCII video handle.
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

/// Renders a single frame using the supplied playback speed.
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
