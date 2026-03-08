# Base AVR toolchain to build avr-libc

DEPENDS:append = " \
    avr-binutils-native \
    avr-gcc-native \
"

export AR = "avr-ar"
export AS = "avr-as"
export CC = "avr-gcc --sysroot=${STAGING_DIR_NATIVE}"
export CXX = "avr-g++ --sysroot=${STAGING_DIR_NATIVE}"
export CFLAGS = ""
export CXXFLAGS = ""
export CPPFLAGS = ""
export LDFLAGS = ""
export LD = "avr-ld --sysroot=${STAGING_DIR_NATIVE}"
export NM = "avr-nm"
export OBJCOPY = "avr-objcopy"
export OBJDUMP = "avr-objdump"
export RANLIB = "avr-ranlib"
export READELF = "avr-readelf"
export STRINGS = "avr-strings"
export STRIP = "avr-strip"

avr_env() {
    AR="avr-ar"
    AS="avr-as"
    CC="avr-gcc --sysroot=${STAGING_DIR_NATIVE}"
    CXX="avr-g++ --sysroot=${STAGING_DIR_NATIVE}"
    CFLAGS=""
    CXXFLAGS=""
    CPPFLAGS=""
    LDFLAGS=""
    LD="avr-ld --sysroot=${STAGING_DIR_NATIVE}"
    NM="avr-nm"
    OBJCOPY="avr-objcopy"
    OBJDUMP="avr-objdump"
    RANLIB="avr-ranlib"
    READELF="avr-readelf"
    STRINGS="avr-strings"
    STRIP="avr-strip"
}

EXPORT_FUNCTIONS avr_env
