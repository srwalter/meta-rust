SUMMARY = "Rust runtime libaries"
HOMEPAGE = "http://www.rust-lang.org"
SECTION = "devel"
LICENSE = "MIT | Apache-2.0"

inherit rust-bin

INHIBIT_DEFAULT_RUST_DEPS = "1"
DEPENDS += "virtual/${TARGET_PREFIX}rust"

rustlib="${libdir}/${TUNE_PKGARCH}${TARGET_VENDOR}-${TARGET_OS}/rustlib/${HOST_SYS}/lib"

do_install () {
	mkdir -p ${D}/${rustlib}
	cp ${STAGING_DIR_NATIVE}/${rustlib}/*.so ${D}/${rustlib}
}

# This has no license file
python do_qa_configure() {
}

FILES_${PN} += "${rustlib}/*.so"
FILES_${PN}-dbg += "${rustlib}/.debug"
