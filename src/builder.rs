//! Utilities for creating binary ASCII video files from source bitmaps.
//!
//! The builder module reads BMP input, converts each frame to the project's
//! compact 5-bit representation, and writes the resulting video data into a valid
//! `ASCIIheader`-prefixed output file.

use std::fs::File;
use std::io::{Seek, SeekFrom, Write};

use crate::bindings::*;
use crate::encoder::VideoEncoder;

/// Converts an `ASCIIheader` value into a byte slice suitable for writing to disk.
pub(crate) fn struct_to_bytes(header: &ASCIIheader) -> &[u8] {
    unsafe {
        std::slice::from_raw_parts(
            header as *const ASCIIheader as *const u8,
            std::mem::size_of::<ASCIIheader>(),
        )
    }
}

/// Creates an ASCII video file from a single BMP image or a directory of BMP frames.
///
/// # Arguments
///
/// * `read_path` - Path to a single BMP file or a directory containing numbered BMP frames.
/// * `output_path` - Destination path for the generated ASCII video file.
/// * `fps` - Playback speed to store in the output file header.
pub fn create_video_file(read_path: &str, output_path: &str, fps: u8) -> std::io::Result<()> {
    let mut output_file = File::create(output_path)?;

    let header = ASCIIheader {
        header_field: *b"ASCII",
        compression: COMPRESS_5_BIT as u8,
        frames: 0,
        fps,
        x_size: WIDTH as u8,
        y_size: HEIGHT as u8,
        reserved: 0,
    };

    let mut frame_count: u16 = 0;

    let metadata = std::fs::metadata(read_path)?;
    let is_single = if metadata.is_file() {
        true
    } else if metadata.is_dir() {
        // todo handle folder of bmps
        false
    } else {
        return Err(std::io::Error::new(
            std::io::ErrorKind::InvalidInput,
            "Input path is invalid",
        ));
    };

    output_file.write_all(struct_to_bytes(&header))?;

    let mut encoder = VideoEncoder::new(output_file);

    if is_single {
        let frame_data = std::fs::read(read_path)?;
        let frame = get_frame(&frame_data)?;

        encoder.write_frame(&frame)?;
        frame_count += 1;
    } else {
        let mut c: u16 = 1;
        loop {
            let frame_path = format!("{}/{:03}.bmp", read_path, c);

            let frame_data = match std::fs::read(&frame_path) {
                Ok(d) => d,
                Err(ref e) if e.kind() == std::io::ErrorKind::NotFound => break, // break out after file not found
                Err(e) => return Err(e), // actual error, propigate
            };

            let frame = get_frame(&frame_data)?;
            encoder.write_frame(&frame)?;

            println!("Processed frame {}...", c);
            frame_count += 1;
            c += 1;
        }
    }

    let mut output_file = encoder.finish()?;

    output_file.seek(SeekFrom::Start(6))?;
    output_file.write_all(&frame_count.to_le_bytes())?;

    Ok(())
}

const BMP_HEADER_SIZE: usize = 14;
const DIB_HEADER_SIZE: usize = 40;

/// Converts a BMP byte stream into the project's packed 5-bit ASCII frame format.
fn get_frame(data: &[u8]) -> std::io::Result<Vec<u8>> {
    let mut cursor: usize = 0;

    // bitmap header stuff
    let bmp_header_raw = read_bytes(data, &mut cursor, BMP_HEADER_SIZE + DIB_HEADER_SIZE)?;

    let width = i32::from_le_bytes(
        bmp_header_raw[BMP_HEADER_SIZE + 4..BMP_HEADER_SIZE + 8]
            .try_into()
            .unwrap(),
    );
    let height = i32::from_le_bytes(
        bmp_header_raw[BMP_HEADER_SIZE + 8..BMP_HEADER_SIZE + 12]
            .try_into()
            .unwrap(),
    );
    let bpp = u16::from_le_bytes(
        bmp_header_raw[BMP_HEADER_SIZE + 14..BMP_HEADER_SIZE + 16]
            .try_into()
            .unwrap(),
    );
    let pad = (width as usize * (bpp as usize / 8) as usize) % 4;
    let row_len = (width as usize * (bpp as usize / 8) + pad) as usize;

    let col = height as usize;
    let sample_w = (width / WIDTH as i32) as usize;
    let sample_h = (height / HEIGHT as i32) as usize;

    let mut total = [[BLACK; HEIGHT as usize]; WIDTH as usize];

    let mut idx: usize = 0;
    while idx < col {
        for jdx in (0..row_len).step_by(sample_w as usize * 3) {
            let pix = read_bytes(data, &mut cursor, 3)?;
            let (blue, green, red) = (pix[0], pix[1], pix[2]);

            let b = get_lit(blue, green, red);

            let x = jdx / (3 * sample_w);
            let y = idx / sample_h;
            total[x][y] = b;

            let step = (sample_w) * 3;
            let burner = if (row_len - jdx) as usize >= step - 3 {
                step - 3
            } else {
                (row_len - jdx) as usize
            };

            let _burned = read_bytes(data, &mut cursor, burner)?;
        } // for

        let mut _burned = read_bytes(data, &mut cursor, pad)?;
        // burn any buffer bytes
        _burned = read_bytes(data, &mut cursor, (sample_h as usize - 1) * row_len)?;

        idx += sample_h;
    } // while col

    let mut buff = [BLACK; (WIDTH as usize * HEIGHT as usize)];
    let mut cdx = 0 as usize;
    for y in (0..HEIGHT as usize).rev() {
        for x in 0..WIDTH as usize {
            buff[cdx] = total[x][y];
            cdx += 1;
        }
    }

    let mut array_size: usize = WIDTH as usize * HEIGHT as usize;
    while array_size % BYTE_ELEMENT_SIZE as usize != 0 {
        array_size += 1;
    }
    array_size /= BYTE_ELEMENT_SIZE as usize;

    let mut packed = Vec::with_capacity(array_size);
    let mut idx: usize = 0;
    for _ in 0..array_size {
        let mut clump = [BLACK; 5];
        let remaining = if (WIDTH as usize * HEIGHT as usize) - idx < 5 {
            (WIDTH as usize * HEIGHT as usize) - idx
        } else {
            5
        };
        for q in 0..remaining {
            clump[q] = buff[idx];
            idx += 1;
        }
        packed.push(bit_compress(&clump));
    }

    Ok(packed)
}

/// Reads a slice of raw bytes from the input data and advances the cursor.
fn read_bytes<'a>(
    data: &'a [u8],
    cursor: &mut usize,
    bytes_count: usize,
) -> std::io::Result<&'a [u8]> {
    let end = *cursor + bytes_count;
    if end > data.len() {
        return Err(std::io::Error::new(
            std::io::ErrorKind::UnexpectedEof,
            "Unexpected end of BMP data",
        ));
    }

    let slice = &data[*cursor..end];
    *cursor = end;
    Ok(slice)
}

/// Maps a pixel triplet to the project's 3-level terminal brightness scale.
fn get_lit(b: u8, g: u8, r: u8) -> u8 {
    if (b | g | r) == 0 {
        BLACK as u8 // ' ' SPACE
    } else if (b & g & r) == 255 {
        WHITE as u8 // # WHITE
    } else {
        GREY as u8 // * GREY
    }
}

/// Packs a 5-pixel sample into the single-byte 5-bit representation used by the file format.
fn bit_compress(clump: &[u8; 5]) -> u8 {
    const THREE: u8 = 3;

    clump[0] + THREE * (clump[1] + THREE * (clump[2] + THREE * (clump[3] + THREE * (clump[4]))))
}
