# src/Makefile
CC         = gcc
AR         = ar
CFLAGS     = -I.. -O3 -Wall -c
CARGO      = cargo
CARGO_ROOT = ..

BUILD    = build
LIB      = libascii.a
RUST_LIB = $(CARGO_ROOT)/target/release/libascii.a

# ONLY the player is still C now (ASCII_build.c is replaced by Rust)
SRC  = ASCII_play.c
OBJ  = $(SRC:%.c=$(BUILD)/%.o)

all: $(LIB)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: %.c | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $<

# build the Rust staticlib (builder + std runtime)
$(RUST_LIB): builder.rs encoder.rs lib.rs bindings.rs
	(cd $(CARGO_ROOT) && $(CARGO) build --release)

# start from the Rust archive, then add the C player object into it
$(LIB): $(RUST_LIB) $(OBJ)
	cp $(RUST_LIB) $(LIB)
	$(AR) r $(LIB) $(OBJ)
	ranlib $(LIB)

clean:
	(cd $(CARGO_ROOT) && $(CARGO) clean)
	rm -rf $(BUILD) $(LIB)
