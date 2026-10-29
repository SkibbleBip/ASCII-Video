# src/Makefile
CARGO      = cargo
CARGO_ROOT = ..

LIB      = libascii.a
RUST_LIB = $(CARGO_ROOT)/target/release/libascii.a

all: $(LIB)

# build the Rust staticlib (builder + player + all C bindings)
$(RUST_LIB): builder.rs encoder.rs player.rs lib.rs bindings.rs
	(cd $(CARGO_ROOT) && $(CARGO) build --release)

# copy the Rust archive out to the expected name/location
$(LIB): $(RUST_LIB)
	cp $(RUST_LIB) $(LIB)

clean:
	(cd $(CARGO_ROOT) && $(CARGO) clean)
	rm -f $(LIB)
