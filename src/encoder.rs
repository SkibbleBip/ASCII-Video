//! Video encoding helpers that compress raw frame bytes with gzip.
//!
//! `VideoEncoder` wraps a file handle and a `GzEncoder`, allowing the builder to
//! stream each frame straight into the output file before finalizing the archive.

use std::fs::File;
use std::io::{self, Write};

use flate2::write::GzEncoder;
use flate2::Compression;

/// Encodes frame data to a gzip-compressed output file.
pub(crate) struct VideoEncoder {
    encoder: GzEncoder<File>,
}

impl VideoEncoder {
    /// Creates a new encoder bound to the supplied output file.
    pub(crate) fn new(file: File) -> Self {
        Self {
            encoder: GzEncoder::new(file, Compression::default()),
        }
    }

    /// Writes a raw frame payload into the compressed stream.
    pub(crate) fn write_frame(&mut self, frame: &[u8]) -> io::Result<()> {
        self.encoder.write_all(frame)
    }

    /// Finalizes the gzip stream and returns the underlying file handle.
    pub(crate) fn finish(self) -> io::Result<File> {
        self.encoder.finish()
    }
}
