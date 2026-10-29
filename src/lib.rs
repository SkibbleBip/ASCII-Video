use std::ffi::CStr;
use std::os::raw::{c_char, c_int};

mod bindings;

mod builder;
mod encoder;
mod player;

pub use builder::create_video_file;

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

pub use player::{open_ascii_video, play_file, render_frame, ASCIIfileHandle, PrinterT};

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

#[no_mangle]
pub extern "C" fn closeASCIIvideo(handle: *mut ASCIIfileHandle) {
    if handle.is_null() {
        return;
    }
    unsafe { drop(Box::from_raw(handle)) };
}

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
