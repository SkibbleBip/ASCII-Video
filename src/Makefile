#**************************************************************************
#Copyright (C) 2026 SkibbleBip
#
#This program is free software; you can redistribute it and/or
#modify it under the terms of the GNU General Public License
#as published by the Free Software Foundation; either version 2
#of the License, or (at your option) any later version.
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with this program; if not, write to the Free Software
#Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
#**************************************************************************/


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
