inherit rust

RUSTLIB_DEP ?= " rustlib"
DEPENDS .= "${RUSTLIB_DEP}"
DEPENDS += "patchelf-native"

export rustlibdir = "${libdir}/rust"
FILES_${PN} += "${rustlibdir}/*.so"
FILES_${PN}-dev += "${rustlibdir}/*.rlib"
FILES_${PN}-dbg += "${rustlibdir}/.debug"

RUSTC_ARCHFLAGS += "-C opt-level=3 -L ${STAGING_DIR_HOST}/${rustlibdir}"
EXTRA_OEMAKE += 'RUSTC_ARCHFLAGS="${RUSTC_ARCHFLAGS}"'
# Libc requires extra work to explicitly override the standard library
LIBC_FLAGS ?= "${@base_contains('DEPENDS', 'libc-rs', '--extern libc=${STAGING_DIR_HOST}/${rustlibdir}/liblibc.rlib', '', d)}"
RUSTC_FLAGS += "${LIBC_FLAGS}"

rustlib="${libdir}/${TUNE_PKGARCH}${TARGET_VENDOR}-${TARGET_OS}/rustlib/${HOST_SYS}/lib"
CRATE_NAME ?= "${@d.getVar('BPN', True).replace('-rs', '').replace('-', '_')}"
LIBNAME ?= "lib${CRATE_NAME}"
CRATE_TYPE ?= "dylib"
LIB_SRC ?= "${S}/src/lib.rs"

oe_compile_rust_lib () {
    rm -rf ${LIBNAME}.{rlib,so}
    oe_runrustc ${LIB_SRC} --crate-name=${CRATE_NAME} --crate-type=${CRATE_TYPE}
}

oe_install_rust_lib () {
    for lib in $(ls ${LIBNAME}.{so,rlib} 2>/dev/null); do
        echo Installing $lib
        install -D -m 644 $lib ${D}/${rustlibdir}/$lib
    done
}

do_rust_bin_fixups() {
    for f in `find ${PKGD} -name '*.so*'`; do
        echo "Strip rust note: $f"
        ${OBJCOPY} -R .note.rustc $f $f
    done

    for f in `find ${PKGD}`; do
        file "$f" | grep -q ELF || continue
        readelf -d "$f" | grep RPATH | grep -q rustlib || continue
        echo "Set rpath:" "$f"
        patchelf --set-rpath '$ORIGIN:'${rustlibdir}:${rustlib} "$f"
    done
}

PACKAGE_PREPROCESS_FUNCS += "do_rust_bin_fixups"
