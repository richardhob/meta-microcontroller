SUMMARY = "C library for use with GCC on Microchip (formerly Atmel) AVR microcontrollers"
HOMEPAGE = "http://www.nongnu.org/avr-libc/"
SECTION = "devel"

LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=8d91a8f153d3de715f67a5df0d841c43"

# We are cross compiling LIBC for AVR. The HOST and TARGET should both be AVR
HOST_SYS = "avr"
HOST_AS_ARCH = ""
HOST_CC_ARCH = ""
HOST_LD_ARCH = ""

TARGET_SYS = "avr"
TARGET_AS_ARCH = ""
TARGET_CC_ARCH = ""
TARGET_LD_ARCH = ""
+

inherit avr-toolchain-base autotools-brokensep gettext texinfo

DEPENDS = " \
    zlib-native \
    gawk-native \
    gmp-native \
    mpfr-native \
    libmpc-native \
    flex-native \
    avr-binutils \
    avr-gcc \
"

PV = "2.1.0+git${SRCPV}"
SRC_URI = " \
    git://github.com/avrdudes/avr-libc.git;protocol=https;branch=main \
    file://0001-Change-python-shebangs-to-python3.patch \
"
SRCREV = "55e8cac69935657bcd3e4d938750960c757844c3"

do_configure:prepend() {
    avr_env
    cd ${S}
    ./devtools/gen-avr-lib-tree.sh
    touch ChangeLog
}

FILES:${PN} += "${prefix}/avr"
FILES:${PN}-staticdev += " \
    ${prefix}/avr/lib/*.a \
    ${prefix}/avr/lib/*/*.a \
    ${prefix}/avr/lib/*/*/*.a \
"

RDEPENDS:${PN} += "${PN}-staticdev"

SYSROOT_DIRS:append = " ${prefix}/avr"
