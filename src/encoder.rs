



use std::fs::File;
use std::io::{self, Write};

use flate2::Compression;
use flate2::write::GzEncoder;

pub(crate) struct VideoEncoder {
    encoder: GzEncoder<File>,
}


impl VideoEncoder {
    pub(crate) fn new(file: File) -> Self{
        Self {
            encoder: GzEncoder::new(file, Compression::default()),
        }

    }



    pub(crate) fn write_frame(&mut self, frame: &[u8]) -> io::Result<()> {
        self.encoder.write_all(frame)
    }

    pub(crate) fn finish(self) -> io::Result<File> {
        self.encoder.finish()
    }


}
