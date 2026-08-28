use std::ffi::CStr;
use std::os::raw::{c_char, c_int};

mod bindings;

mod builder;
mod encoder;
// mod frame;
// mod bitpack;
// mod bmp;
// mod header;


pub use builder::create_video_file;


#[no_mangle]
pub extern "C" fn createVideoFile(
    input: *const c_char,
    output: *const c_char,
    fps: u8,
) -> c_int{
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
