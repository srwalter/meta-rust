inherit rust

RUSTLIB_DEP ?= " rustlib"
DEPENDS .= "${RUSTLIB_DEP}"

export rustlibdir = "${libdir}/rust"
FILES_${PN} += "${rustlibdir}/*.so"
FILES_${PN}-dbg += "${rustlibdir}/.debug"

RUSTC_ARCHFLAGS += "-C opt-level=3 -L ${STAGING_DIR_HOST}/${rustlibdir}"
EXTRA_OEMAKE += 'RUSTC_ARCHFLAGS="${RUSTC_ARCHFLAGS}"'

do_strip_rust_note() {
    for f in `find ${PKGD} -name '*.so*'`; do
        echo "Strip rust note: $f"
        ${OBJCOPY} -R .note.rustc $f $f
    done
}

PACKAGEBUILDPKGD_append = "do_strip_rust_note"
