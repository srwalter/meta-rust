DESCRIPTION = "A Rust library with native bindings to the types and functions commonly found on various systems, including libc."
HOMEPAGE = "https://github.com/rust-lang/libc"
LICENSE = "MIT | Apache-2.0"
LIC_FILES_CHKSUM = "\
	file://LICENSE-MIT;md5=615cc94ba6f721c4ed3d6988605e85ca \
	file://LICENSE-APACHE;md5=1836efb2eb779966696f473ee8540542 \
"

inherit rust-bin

# SRC_URI = "git://git@github.com:rust-lang/libc.git;protocol=https"
# libc lives in rust-lang/rust which is a submodule of rust-lang/libc
SRC_URI = "gitsm://github.com/rust-lang/rust.git;protocol=https"
SRCREV = "8b7c17db2235a2a3f2c71242b11fc429a8d05a90"

S = "${WORKDIR}/git"

# This defeats the "only use the version from crates.io" check
RUSTC_FLAGS += "--cfg feature='"cargo-build"'"
LIB_SRC = "${S}/src/liblibc/lib.rs"

do_compile () {
	oe_compile_rust_lib
}

do_install () {
	oe_install_rust_lib
}
