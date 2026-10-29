//! Rust library for reading, rendering, and generating ASCII video files.
//!
//! The crate exposes both a Rust API and a small C-compatible FFI layer so the
//! same video format can be consumed from native code and terminal UIs.

use flate2::read::GzDecoder;
use std::fs::File;
use std::io::Read;
use std::os::raw::c_char;
use std::time::{Duration, Instant};

use crate::bindings::*;

/// Function pointer type used when a consumer wants to print terminal output.
pub type PrinterT = extern "C" fn(*const c_char);

/// Runtime handle for an open ASCII video file.
pub struct ASCIIfileHandle {
    /// Video header describing width, height, FPS, and frame count.
    pub header: ASCIIheader,
    /// Gzip decoder used to read compressed frame payloads.
    decoder: GzDecoder<File>,
    /// Callback invoked to render each decoded frame row.
    printer: PrinterT,
    /// Number of frames the player has skipped due to timing pressure.
    frame_skip: u8,
    /// Total number of frames rendered so far.
    frame_count: u16,
}

/// Opens an ASCII video file from disk and prepares it for playback.
///
/// # Arguments
///
/// * `path` - Filesystem path to a serialized ASCII video.
/// * `funct` - Callback used to emit terminal output while rendering frames.
pub fn open_ascii_video(path: &str, funct: PrinterT) -> std::io::Result<Box<ASCIIfileHandle>> {
    let mut file = File::open(path)?;

    let mut header_bytes = [0u8; std::mem::size_of::<ASCIIheader>()];
    file.read_exact(&mut header_bytes)?;

    let header: ASCIIheader = unsafe { std::ptr::read(header_bytes.as_ptr() as *const _) };

    if &header.header_field != b"ASCII" {
        return Err(std::io::Error::new(
            std::io::ErrorKind::InvalidData,
            "Invalid header of file",
        ));
    }
    if header.compression != COMPRESS_5_BIT as u8 {
        return Err(std::io::Error::new(
            std::io::ErrorKind::InvalidData,
            "Non 5-bit stuffing not supported",
        ));
    }

    Ok(Box::new(ASCIIfileHandle {
        header,
        decoder: GzDecoder::new(file),
        printer: funct,
        frame_skip: 0,
        frame_count: 0,
    }))
}

/// Decodes a single 5-bit packed byte into a 6-character terminal raster snippet.
fn decode(byte: u8) -> [u8; 6] {
    let mut v = byte;
    let mut out = [b' '; 6];

    for idx in 0..5 {
        out[idx] = match v % 3 {
            0 => b' ',
            1 => b'*',
            _ => b'#',
        };
        v /= 3;
    }
    out[5] = 0;
    out
    // return null-terminated string
}

/// Prints a decoded frame row to the configured terminal callback.
fn display(frame: &[u8], x: u8, printer: PrinterT) {
    let mut pos: usize = 5;

    for b in frame.iter().copied() {
        let tmp = decode(b);
        printer(tmp.as_ptr() as *const c_char);
        pos = pos + 5;
        if pos % x as usize == 0 {
            printer(b"\n\0".as_ptr() as *const c_char);
        }
    }
}

/// Plays a complete ASCII video sequence through the registered printer callback.
pub fn play_file(h: &mut ASCIIfileHandle) -> Result<(), String> {
    let frames = u16::from_le(h.header.frames);
    let fps = h.header.fps.max(1);
    let x = h.header.x_size;
    let buff_len = {
        let n: usize = h.header.x_size as usize * h.header.y_size as usize;
        n.div_ceil(BYTE_ELEMENT_SIZE as usize)
    };
    let frame_dur = Duration::from_micros(1_000_000 / fps as u64);
    let printer = h.printer;

    if frames == 1 {
        let uncompressed_frame = decompress_frame(h, buff_len)?;
        printer(b"\x1b[0;0H\0".as_ptr() as *const c_char);
        display(&uncompressed_frame, x, printer);
        return Ok(());
    }

    h.frame_skip = 0;
    for _ in 0..frames {
        let start = Instant::now();
        let uncompressed_frame = decompress_frame(h, buff_len)?;

        if h.frame_skip > 0 {
            h.frame_skip -= 1;
            continue;
        }

        printer(b"\x1b[0;0H\0".as_ptr() as *const c_char);
        display(&uncompressed_frame, x, printer);

        let elapsed = start.elapsed();
        if elapsed >= frame_dur {
            let over = elapsed.as_micros() as u64 - frame_dur.as_micros() as u64;
            h.frame_skip = (over / frame_dur.as_micros().max(1) as u64) as u8 + 1;
        } else {
            std::thread::sleep(frame_dur - elapsed);
        }
    }

    printer(b"\n\n\n\0".as_ptr() as *const c_char);
    Ok(())
}

/// Renders a single frame while honoring a requested playback rate multiplier.
pub fn render_frame(h: &mut ASCIIfileHandle, speed: f32) -> Result<u16, String> {
    let frames = u16::from_le(h.header.frames);
    let fps = h.header.fps.max(1);
    let x = h.header.x_size;
    let buff_len = {
        let n: usize = h.header.x_size as usize * h.header.y_size as usize;
        n.div_ceil(BYTE_ELEMENT_SIZE as usize)
    };

    let printer = h.printer;

    let mut consumed: u16 = 0;

    if frames == 1 {
        let uncompressed_frame = decompress_frame(h, buff_len)?;
        printer(b"\x1b[0;0H\0".as_ptr() as *const c_char);
        display(&uncompressed_frame, x, printer);
        return Ok(0);
    }

    if speed != 0.0 {
        let start = Instant::now();
        while h.frame_skip > 0 {
            let _ = decompress_frame(h, buff_len)?;
            h.frame_skip -= 1;
            consumed += 1;
        }

        let uncompressed_frame = decompress_frame(h, buff_len)?;
        consumed += 1;
        printer(b"\x1b[0;0H\0".as_ptr() as *const c_char);
        display(&uncompressed_frame, x, printer);

        let frame_us = 1_000_000i64 / fps as i64;
        let elapsed = start.elapsed().as_micros() as i64;
        let diff = frame_us - elapsed;

        if diff < 0 {
            h.frame_skip = ((-diff as f64) / frame_us as f64).ceil() as u8;
            let wait = h.frame_skip as i64 * frame_us - elapsed;
            let sleep_duration = ((wait.abs() as f32) / speed) as u64;
            std::thread::sleep(Duration::from_micros(sleep_duration));
        } else {
            let sleep_duration = ((diff as f32) / speed) as u64;
            std::thread::sleep(Duration::from_micros(sleep_duration));
        }

        h.frame_count += consumed;
        return Ok(frames.saturating_sub(h.frame_count));
    }

    // speed == 0, go nuts and print
    printer(b"\x1b[0;0H\0".as_ptr() as *const c_char);
    let uncompressed_frame = decompress_frame(h, buff_len)?;
    display(&uncompressed_frame, x, printer);
    h.frame_count += 1;
    Ok(frames.saturating_sub(h.frame_count + 1))
}

/// Reads a single compressed frame from the underlying gzip decoder.
fn decompress_frame(h: &mut ASCIIfileHandle, len: usize) -> Result<Vec<u8>, String> {
    let mut buf = vec![0u8; len];
    match h.decoder.read_exact(&mut buf) {
        Ok(()) => Ok(buf),
        Err(e) if e.kind() == std::io::ErrorKind::UnexpectedEof => {
            Err("Early EOF reached".to_string())
        }
        Err(e) => Err(format!("Decompression error-{}", e)),
    }
}
