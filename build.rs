const HEADER_PATH: &str = "ASCII.h";

fn main() {
    println!("cargo:rerun-if-changed={}", HEADER_PATH);

    let bindings = bindgen::Builder::default()
        .header(HEADER_PATH)
        .fit_macro_constants(true)
        .parse_callbacks(Box::new(bindgen::CargoCallbacks::new()))
        .allowlist_recursively(false)
        .allowlist_type("ASCIIheader")
        .allowlist_var("COMPRESS_.*|BLACK|GREY|WHITE|WIDTH|HEIGHT|BYTE_ELEMENT_SIZE")
        .generate()
        .expect("Unable to generate bindings");

    bindings
        .write_to_file("src/bindings.rs")
        .expect("Could not write bindings");
}
