SUMMARY = "Rust compiler and runtime libaries"
HOMEPAGE = "http://www.rust-lang.org"
SECTION = "devel"
LICENSE = "MIT | Apache-2.0"
LIC_FILES_CHKSUM = "file://COPYRIGHT;md5=93a95682d51b4cb0a633a97046940ef0"

inherit rust
inherit cargo_common

SRC_URI += "file://riscv-march.patch"
SRC_URI += "file://rv64gc.patch"

DEPENDS += "file-native python3-native"
DEPENDS_append_class-native = " rust-llvm-native"

S = "${RUSTSRC}"

# We generate local targets, and need to be able to locate them
export RUST_TARGET_PATH="${WORKDIR}/targets/"

export FORCE_CRATE_HASH="${BB_TASKHASH}"

export YOCTO_ALTERNATE_EXE_PATH = "${STAGING_LIBDIR}/llvm-rust/bin/llvm-config"
export YOCTO_ALTERNATE_MULTILIB_NAME = "/${BASELIB}"

# We don't want to use bitbakes vendoring because the rust sources do their
# own vendoring.
CARGO_DISABLE_BITBAKE_VENDORING = "1"

# We can't use RUST_BUILD_SYS here because that may be "musl" if
# TCLIBC="musl". Snapshots are always -unknown-linux-gnu
SNAPSHOT_BUILD_SYS = "${BUILD_ARCH}-unknown-linux-gnu"
setup_cargo_environment () {
    # The first step is to build bootstrap and some early stage tools,
    # these are build for the same target as the snapshot, e.g.
    # x86_64-unknown-linux-gnu.
    # Later stages are build for the native target (i.e. target.x86_64-linux)
    cargo_common_do_configure

    printf '[target.%s]\n' "${SNAPSHOT_BUILD_SYS}" >> ${CARGO_HOME}/config
    printf "linker = '%s'\n" "${RUST_BUILD_CCLD}" >> ${CARGO_HOME}/config
}

# Right now this is focused on arm-specific tune features.
# We get away with this for now as one can only use x86-64 as the build host
# (not arm).
# Note that TUNE_FEATURES is _always_ refering to the target, so we really
# don't want to use this for the host/build.
def llvm_features_from_tune(d):
    f = []
    feat = d.getVar('TUNE_FEATURES')
    if not feat:
        return []
    feat = frozenset(feat.split())

    mach_overrides = d.getVar('MACHINEOVERRIDES')
    mach_overrides = frozenset(mach_overrides.split(':'))

    if 'vfpv4' in feat:
        f.append("+vfp4")
    if 'vfpv3' in feat:
        f.append("+vfp3")
    if 'vfpv3d16' in feat:
        f.append("+d16")

    if 'vfpv2' in feat or 'vfp' in feat:
        f.append("+vfp2")

    if 'neon' in feat:
        f.append("+neon")

    if 'mips32' in feat:
        f.append("+mips32")

    if 'mips32r2' in feat:
        f.append("+mips32r2")

    if target_is_armv7(d):
        f.append('+v7')

    if ('armv6' in mach_overrides) or ('armv6' in feat):
        f.append("+v6")

    if 'dsp' in feat:
        f.append("+dsp")

    if 'thumb' in feat:
        if d.getVar('ARM_THUMB_OPT') is "thumb":
            if target_is_armv7(d):
                f.append('+thumb2')
            f.append("+thumb-mode")

    if 'cortexa5' in feat:
        f.append("+a5")
    if 'cortexa7' in feat:
        f.append("+a7")
    if 'cortexa9' in feat:
        f.append("+a9")
    if 'cortexa15' in feat:
        f.append("+a15")
    if 'cortexa17' in feat:
        f.append("+a17")
    if ('riscv64' in feat) or ('riscv32' in feat):
        f.append("+a,+c,+d,+f,+m")
    return f

# TARGET_CC_ARCH changes from build/cross/target so it'll do the right thing
# this should go away when https://github.com/rust-lang/rust/pull/31709 is
# stable (1.9.0?)
def llvm_features_from_cc_arch(d):
    f = []
    feat = d.getVar('TARGET_CC_ARCH')
    if not feat:
        return []
    feat = frozenset(feat.split())

    if '-mmmx' in feat:
        f.append("+mmx")
    if '-msse' in feat:
        f.append("+sse")
    if '-msse2' in feat:
        f.append("+sse2")
    if '-msse3' in feat:
        f.append("+sse3")
    if '-mssse3' in feat:
        f.append("+ssse3")
    if '-msse4.1' in feat:
        f.append("+sse4.1")
    if '-msse4.2' in feat:
        f.append("+sse4.2")
    if '-msse4a' in feat:
        f.append("+sse4a")
    if '-mavx' in feat:
        f.append("+avx")
    if '-mavx2' in feat:
        f.append("+avx2")

    return f

def llvm_features_from_target_fpu(d):
    # TARGET_FPU can be hard or soft. +soft-float tell llvm to use soft float
    # ABI. There is no option for hard.

    fpu = d.getVar('TARGET_FPU', True)
    return ["+soft-float"] if fpu == "soft" else []

def llvm_features(d):
    return ','.join(llvm_features_from_tune(d) +
                    llvm_features_from_cc_arch(d) +
                    llvm_features_from_target_fpu(d))

## arm-unknown-linux-gnueabihf
DATA_LAYOUT[arm] = "e-m:e-p:32:32-i64:64-v128:64:128-a:0:32-n32-S64"
LLVM_TARGET[arm] = "${RUST_TARGET_SYS}"
TARGET_ENDIAN[arm] = "little"
TARGET_POINTER_WIDTH[arm] = "32"
TARGET_C_INT_WIDTH[arm] = "32"
MAX_ATOMIC_WIDTH[arm] = "64"
FEATURES[arm] = "+v6,+vfp2"

## armv7-unknown-linux-gnueabihf
DATA_LAYOUT[armv7] = "e-m:e-p:32:32-i64:64-v128:64:128-a:0:32-n32-S64"
LLVM_TARGET[armv7] = "${RUST_TARGET_SYS}"
TARGET_ENDIAN[armv7] = "little"
TARGET_POINTER_WIDTH[armv7] = "32"
TARGET_C_INT_WIDTH[armv7] = "32"
MAX_ATOMIC_WIDTH[armv7] = "64"
FEATURES[armv7] = "+v7,+vfp2,+thumb2"

## aarch64-unknown-linux-{gnu, musl}
DATA_LAYOUT[aarch64] = "e-m:e-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128"
LLVM_TARGET[aarch64] = "${RUST_TARGET_SYS}"
TARGET_ENDIAN[aarch64] = "little"
TARGET_POINTER_WIDTH[aarch64] = "64"
TARGET_C_INT_WIDTH[aarch64] = "32"
MAX_ATOMIC_WIDTH[aarch64] = "128"

## x86_64-unknown-linux-{gnu, musl}
DATA_LAYOUT[x86_64] = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
LLVM_TARGET[x86_64] = "${RUST_TARGET_SYS}"
TARGET_ENDIAN[x86_64] = "little"
TARGET_POINTER_WIDTH[x86_64] = "64"
TARGET_C_INT_WIDTH[x86_64] = "32"
MAX_ATOMIC_WIDTH[x86_64] = "64"

## i686-unknown-linux-{gnu, musl}
DATA_LAYOUT[i686] = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
LLVM_TARGET[i686] = "${RUST_TARGET_SYS}"
TARGET_ENDIAN[i686] = "little"
TARGET_POINTER_WIDTH[i686] = "32"
TARGET_C_INT_WIDTH[i686] = "32"
MAX_ATOMIC_WIDTH[i686] = "64"

## XXX: a bit of a hack so qemux86 builds, clone of i686-unknown-linux-{gnu, musl} above
DATA_LAYOUT[i586] = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
LLVM_TARGET[i586] = "${RUST_TARGET_SYS}"
TARGET_ENDIAN[i586] = "little"
TARGET_POINTER_WIDTH[i586] = "32"
TARGET_C_INT_WIDTH[i586] = "32"
MAX_ATOMIC_WIDTH[i586] = "64"

## mips-unknown-linux-{gnu, musl}
DATA_LAYOUT[mips] = "E-m:m-p:32:32-i8:8:32-i16:16:32-i64:64-n32-S64"
LLVM_TARGET[mips] = "${RUST_TARGET_SYS}"
TARGET_ENDIAN[mips] = "big"
TARGET_POINTER_WIDTH[mips] = "32"
TARGET_C_INT_WIDTH[mips] = "32"
MAX_ATOMIC_WIDTH[mips] = "32"

## mipsel-unknown-linux-{gnu, musl}
DATA_LAYOUT[mipsel] = "e-m:m-p:32:32-i8:8:32-i16:16:32-i64:64-n32-S64"
LLVM_TARGET[mipsel] = "${RUST_TARGET_SYS}"
TARGET_ENDIAN[mipsel] = "little"
TARGET_POINTER_WIDTH[mipsel] = "32"
TARGET_C_INT_WIDTH[mipsel] = "32"
MAX_ATOMIC_WIDTH[mipsel] = "32"

## mips64-unknown-linux-{gnu, musl}
DATA_LAYOUT[mips64] = "E-m:e-i8:8:32-i16:16:32-i64:64-n32:64-S128"
LLVM_TARGET[mips64] = "${RUST_TARGET_SYS}"
TARGET_ENDIAN[mips64] = "big"
TARGET_POINTER_WIDTH[mips64] = "64"
TARGET_C_INT_WIDTH[mips64] = "64"
MAX_ATOMIC_WIDTH[mips64] = "64"

## mips64el-unknown-linux-{gnu, musl}
DATA_LAYOUT[mips64el] = "e-m:e-i8:8:32-i16:16:32-i64:64-n32:64-S128"
LLVM_TARGET[mips64el] = "${RUST_TARGET_SYS}"
TARGET_ENDIAN[mips64el] = "little"
TARGET_POINTER_WIDTH[mips64el] = "64"
TARGET_C_INT_WIDTH[mips64el] = "64"
MAX_ATOMIC_WIDTH[mips64el] = "64"

## powerpc-unknown-linux-{gnu, musl}
DATA_LAYOUT[powerpc] = "E-m:e-p:32:32-i64:64-n32"
LLVM_TARGET[powerpc] = "${RUST_TARGET_SYS}"
TARGET_ENDIAN[powerpc] = "big"
TARGET_POINTER_WIDTH[powerpc] = "32"
TARGET_C_INT_WIDTH[powerpc] = "32"
MAX_ATOMIC_WIDTH[powerpc] = "32"

## powerpc64le-unknown-linux-{gnu, musl}
DATA_LAYOUT[powerpc64le] = "e-m:e-i64:64-n32:64-v256:256:256-v512:512:512"
LLVM_TARGET[powerpc64le] = "${RUST_TARGET_SYS}"
TARGET_ENDIAN[powerpc64le] = "little"
TARGET_POINTER_WIDTH[powerpc64le] = "64"
TARGET_C_INT_WIDTH[powerpc64le] = "64"
MAX_ATOMIC_WIDTH[powerpc64le] = "64"

## riscv32-unknown-linux-{gnu, musl}
DATA_LAYOUT[riscv32] = "e-m:e-p:32:32-i64:64-n32-S128"
LLVM_TARGET[riscv32] = "${RUST_TARGET_SYS}"
TARGET_ENDIAN[riscv32] = "little"
TARGET_POINTER_WIDTH[riscv32] = "32"
TARGET_C_INT_WIDTH[riscv32] = "32"
MAX_ATOMIC_WIDTH[riscv32] = "32"

## riscv64-unknown-linux-{gnu, musl}
DATA_LAYOUT[riscv64] = "e-m:e-p:64:64-i64:64-i128:128-n64-S128"
LLVM_TARGET[riscv64] = "${RUST_TARGET_SYS}"
TARGET_ENDIAN[riscv64] = "little"
TARGET_POINTER_WIDTH[riscv64] = "64"
TARGET_C_INT_WIDTH[riscv64] = "64"
MAX_ATOMIC_WIDTH[riscv64] = "64"

def sys_for(d, thing):
    return d.getVar('{}_SYS'.format(thing))

def prefix_for(d, thing):
    return d.getVar('{}_PREFIX'.format(thing))

# Convert a normal arch (HOST_ARCH, TARGET_ARCH, BUILD_ARCH, etc) to something
# rust's internals won't choke on.
def arch_to_rust_target_arch(arch):
    if arch == "i586" or arch == "i686":
        return "x86"
    elif arch == "mipsel":
        return "mips"
    elif arch == "mip64sel":
        return "mips64"
    elif arch == "armv7":
        return "arm"
    else:
        return arch

# generates our target CPU value
def llvm_cpu(d):
    # First check if TUNE_CCARGS gives us a specific CPU to build for (via -march).
    # Translate that GCC -march flag to a Rust/LLVM CPU.
    trans = {}
    trans['btver2'] = "btver2"
    trans['core2'] = "core2"
    trans['mips32'] = "mips32"
    trans['mips32r2'] = "mips32r2"
    trans['nehalem'] = "nehalem"
    trans['skylake'] = "skylake"

    for arg in (d.getVar('TUNE_CCARGS') or '').split():
        if arg.startswith('-march='):
            march = arg[7:]
            cpu = trans.get(march)
            if cpu:
                return cpu

    # If we don't have -march in TUNE_CCARGS, check TRANSLATED_TARGET_ARCH.
    # This must also be translated into a Rust/LLVM CPU.
    trans = {}
    trans['i586'] = "i586"
    trans['i686'] = "i686"
    trans['mips64'] = "mips64"
    trans['mips64el'] = "mips64"
    trans['powerpc'] = "ppc"
    trans['powerpc64'] = "ppc64"
    trans['riscv32'] = "generic-rv32"
    trans['riscv64'] = "generic-rv64"
    trans['x86-64'] = "x86-64"

    target = d.getVar('TRANSLATED_TARGET_ARCH')
    cpu = trans.get(target)
    if cpu:
        return cpu

    # If we still didn't get a target CPU, choose "generic".
    # Further optimization can still happen via llvm_features.
    return "generic"

TARGET_LLVM_CPU="${@llvm_cpu(d)}"
TARGET_LLVM_FEATURES = "${@llvm_features(d)}"

# class-native implies TARGET=HOST, and TUNE_FEATURES only describes the real
# (original) target.
TARGET_LLVM_FEATURES_class-native = "${@','.join(llvm_features_from_cc_arch(d))}"

def rust_gen_target(d, thing, wd, features, cpu, arch):
    import json
    sys = sys_for(d, thing)
    prefix = prefix_for(d, thing)

    features = features or d.getVarFlag('FEATURES', arch) or ""
    features = features.strip()

    # build tspec
    tspec = {}
    tspec['llvm-target'] = d.getVarFlag('LLVM_TARGET', arch)
    tspec['data-layout'] = d.getVarFlag('DATA_LAYOUT', arch)
    tspec['max-atomic-width'] = int(d.getVarFlag('MAX_ATOMIC_WIDTH', arch))
    tspec['target-pointer-width'] = d.getVarFlag('TARGET_POINTER_WIDTH', arch)
    tspec['target-c-int-width'] = d.getVarFlag('TARGET_C_INT_WIDTH', arch)
    tspec['target-endian'] = d.getVarFlag('TARGET_ENDIAN', arch)
    tspec['arch'] = arch_to_rust_target_arch(arch)
    tspec['os'] = "linux"
    if "musl" in tspec['llvm-target']:
        tspec['env'] = "musl"
    else:
        tspec['env'] = "gnu"
    if "riscv64" in tspec['llvm-target']:
        tspec['llvm-abiname'] = "lp64d"
    if "riscv32" in tspec['llvm-target']:
        tspec['llvm-abiname'] = "ilp32d"
    tspec['vendor'] = "unknown"
    tspec['target-family'] = "unix"
    tspec['linker'] = "{}{}gcc".format(d.getVar('CCACHE'), prefix)
    tspec['ar'] = "{}ar".format(prefix)
    tspec['cpu'] = cpu
    if features is not "":
        tspec['features'] = features
    tspec['dynamic-linking'] = True
    tspec['executables'] = True
    tspec['linker-is-gnu'] = True
    tspec['linker-flavor'] = "gcc"
    tspec['has-rpath'] = True
    tspec['has-elf-tls'] = True
    tspec['position-independent-executables'] = True
    tspec['panic-strategy'] = d.getVar("RUST_PANIC_STRATEGY")

    # write out the target spec json file
    with open(wd + sys + '.json', 'w') as f:
        json.dump(tspec, f, indent=4)

python do_rust_gen_targets () {
    wd = d.getVar('WORKDIR') + '/targets/'
    build_arch = d.getVar('BUILD_ARCH')
    rust_gen_target(d, 'BUILD', wd, "", "generic", build_arch)
}

addtask rust_gen_targets after do_patch before do_compile
do_rust_gen_targets[dirs] += "${WORKDIR}/targets"

do_rust_setup_snapshot () {
    for installer in "${WORKDIR}/rust-snapshot-components/"*"/install.sh"; do
        "${installer}" --prefix="${WORKDIR}/rust-snapshot" --disable-ldconfig
    done

    # Some versions of rust (e.g. 1.18.0) tries to find cargo in stage0/bin/cargo
    # and fail without it there.
    mkdir -p ${RUSTSRC}/build/${BUILD_SYS}
    ln -sf ${WORKDIR}/rust-snapshot/ ${RUSTSRC}/build/${BUILD_SYS}/stage0
}
addtask rust_setup_snapshot after do_unpack before do_configure
do_rust_setup_snapshot[dirs] += "${WORKDIR}/rust-snapshot"


python do_configure() {
    import json
    try:
        import configparser
    except ImportError:
        import ConfigParser as configparser

    # toml is rather similar to standard ini like format except it likes values
    # that look more JSON like. So for our purposes simply escaping all values
    # as JSON seem to work fine.

    e = lambda s: json.dumps(s)

    config = configparser.RawConfigParser()

    # [target.ARCH-poky-linux]
    target_section = "target.{}".format(d.getVar('TARGET_SYS', True))
    config.add_section(target_section)

    llvm_config = d.expand("${YOCTO_ALTERNATE_EXE_PATH}")
    config.set(target_section, "llvm-config", e(llvm_config))

    config.set(target_section, "cxx", e(d.expand("${RUST_TARGET_CXX}")))
    config.set(target_section, "cc", e(d.expand("${RUST_TARGET_CC}")))

    # If we don't do this rust-native will compile it's own llvm for BUILD.
    # [target.${BUILD_ARCH}-unknown-linux-gnu]
    target_section = "target.{}".format(d.getVar('SNAPSHOT_BUILD_SYS', True))
    config.add_section(target_section)

    config.set(target_section, "llvm-config", e(llvm_config))

    config.set(target_section, "cxx", e(d.expand("${RUST_BUILD_CXX}")))
    config.set(target_section, "cc", e(d.expand("${RUST_BUILD_CC}")))

    # [rust]
    config.add_section("rust")
    config.set("rust", "rpath", e(True))
    config.set("rust", "channel", e("stable"))

    # Whether or not to optimize the compiler and standard library
    config.set("rust", "optimize", e(True))

    # [build]
    config.add_section("build")
    config.set("build", "submodules", e(False))
    config.set("build", "docs", e(False))

    rustc = d.expand("${WORKDIR}/rust-snapshot/bin/rustc")
    config.set("build", "rustc", e(rustc))

    cargo = d.expand("${WORKDIR}/rust-snapshot/bin/cargo")
    config.set("build", "cargo", e(cargo))

    config.set("build", "vendor", e(True))

    targets = [d.getVar("TARGET_SYS", True)]
    config.set("build", "target", e(targets))

    hosts = [d.getVar("HOST_SYS", True)]
    config.set("build", "host", e(targets))

    # We can't use BUILD_SYS since that is something the rust snapshot knows
    # nothing about when trying to build some stage0 tools (like fabricate)
    config.set("build", "build", e(d.getVar("SNAPSHOT_BUILD_SYS", True)))

    # [install]
    config.add_section("install")
    # ./x.py install doesn't have any notion of "destdir"
    # but we can prepend ${D} to all the directories instead
    config.set("install", "prefix",  e(d.getVar("D", True) + d.getVar("prefix", True)))
    config.set("install", "bindir",  e(d.getVar("D", True) + d.getVar("bindir", True)))
    config.set("install", "libdir",  e(d.getVar("D", True) + d.getVar("libdir", True)))
    config.set("install", "datadir", e(d.getVar("D", True) + d.getVar("datadir", True)))
    config.set("install", "mandir",  e(d.getVar("D", True) + d.getVar("mandir", True)))

    with open("config.toml", "w") as f:
        f.write('changelog-seen = 2\n\n')
        config.write(f)

    # set up ${WORKDIR}/cargo_home
    bb.build.exec_func("setup_cargo_environment", d)
}


rust_runx () {
    echo "COMPILE ${PN}" "$@"

    # CFLAGS, LDFLAGS, CXXFLAGS, CPPFLAGS are used by rust's build for a
    # wide range of targets (not just TARGET). Yocto's settings for them will
    # be inappropriate, avoid using.
    unset CFLAGS
    unset LDFLAGS
    unset CXXFLAGS
    unset CPPFLAGS

    oe_cargo_fix_env

    python3 src/bootstrap/bootstrap.py ${@oe.utils.parallel_make_argument(d, '-j %d')} "$@" --verbose
}
rust_runx[vardepsexclude] += "PARALLEL_MAKE"

do_compile () {
    rust_runx build
}

rust_do_install () {
    mkdir -p ${D}${bindir}
    cp build/${HOST_SYS}/stage2/bin/* ${D}${bindir}

    mkdir -p ${D}${libdir}/rustlib
    cp -pRd build/${HOST_SYS}/stage2/lib/* ${D}${libdir}
    # Remove absolute symlink so bitbake doesn't complain
    rm -f ${D}${libdir}/rustlib/src/rust
}

rust_install_targets() {
    # Install our custom target.json files
    local td="${D}${libdir}/rustlib/"
    install -d "$td"
    for tgt in "${WORKDIR}/targets/"* ; do
        install -m 0644 "$tgt" "$td"
    done
}


do_install () {
    rust_do_install
    rust_install_targets
}
# ex: sts=4 et sw=4 ts=8
