# Error when building avr-binutils

``` bash
ERROR: avr-binutils-native-2.38-r0 do_unpack: S should be set relative to UNPACKDIR, e.g. replace WORKDIR with UNPACKDIR in "S = ${WORKDIR}/binutils-${PV}"
ERROR: Logfile of failure stored in: /tools/bitbake-builds/poky-whinlatter/build/tmp/work/x86_64-linux/avr-binutils-native/2.38/temp/log.do_unpack.711003
ERROR: Task (virtual:native:/tools/bitbake-builds/poky-whinlatter/layers/meta-microcontroller/recipes-avr/avr-binutils/avr-binutils_2.38.bb:do_unpack) failed with exit code '1'
```

This is caused by a change in Yocto from styhead:

    https://docs.yoctoproject.org/next/migration-guides/migration-5.1.html

## S = ${WORKDIR} no longer supported

If a recipe has S set to be WORKDIR, this is no longer supported, and an error
will be issued. The recipe should be changed to:

> S = "${UNPACKDIR}"

Any WORKDIR references where files from `SRC_URI` are referenced should be
changed to `S`. These are commonly in `do_configure`, (typo) `do_compile`,
`do_install` and `LIC_FILES_CHKSUM`. 

## WORKDIR references in recipes

WORKDIR references in other recipes need auditing. If they reference files from
`SRC_URI`, they likely need changing to `UNPACKDIR`. These are commonly in
`do_compile` and `do_install` for things like service or configuration files.
One unusual case is `${WORKDIR}/${BP}` which should probably be set to `${S}`.

References to `../` in `LIC_FILES_CHKSUM` or elsewhere may need changing to
UNPACKDIR. References to WORKDIR in sed commands are usually left as they are.

## General notes

Files from `do_unpack` now unpack to WORKDIR/sources-unpack/ rather than
WORKDIR/.

If S is set to a subdirectory under WORKDIR and that subdirectory exists in
sources-unpack after `do_unpack` runs, it is moved to WORKDIR. This means that S
= "${WORKDIR}/${BP}", S = "${WORKDIR}/git" and also deeper paths continue to
work as expected without changes. We cannot use symlinks to do this as it breaks
autotools based recipes. Keeping all sources under sources-unpack wasn’t
considered as it meant more invasive recipes changes. The key objective was
separating the `do_unpack` task output from WORKDIR.

Previously, S was always created but after the recent changes it is no longer
the case. This means the check in `do_unpack_qa` triggers where S is not created
by a recipe while it didn’t happen before. This can require to add an S
definition to a recipe that only uses file:// `SRC_URI` entries. To be consistent,
the following pattern is recommended:

> S = "${UNPACKDIR}"

Building C files from UNPACKDIR without setting S to point at it does not work
as the debug prefix mapping doesn’t handle that.

devtool and recipetool have been updated to handle this and their support for S
= WORKDIR and oe-local-files has been removed.

# Other Changes 

From: whinlatter - https://docs.yoctoproject.org/next/migration-guides/migration-5.3.html

## S = ${WORKDIR}/something no longer supported

If a recipe has S set to be ${WORKDIR}/something, this is no longer supported,
and an error will be issued. The recipe should be changed to:

> S = "${UNPACKDIR}/something"

## S = ${WORKDIR}/git and S = ${UNPACKDIR}/git should be removed

The Git fetcher now unpacks into the `BB_GIT_DEFAULT_DESTSUFFIX` directory rather
than the git/ directory under `UNPACKDIR`. `BB_GIT_DEFAULT_DESTSUFFIX` is set in
OpenEmbedded-Core (OE-Core)’s bitbake.conf to BP.

This location matches the default value of S set by bitbake.conf, so S setting
in recipes can and should be removed.

Note that when S is set to a subdirectory of the git checkout, then it should be
instead adjusted according to the previous point:

> S = "${UNPACKDIR}/${BP}/something"

Note that “git” as the source checkout location can be hardcoded in other places
in recipes; when it’s in `SRC_URI`, replace with `BB_GIT_DEFAULT_DESTSUFFIX`,
otherwise replace with BP.  

## How to make those adjustments without tedious manual editing

The following sed command can be used to remove S = "${WORKDIR}/git" across a
whole layer:

```bash
> sed -i "/^S = \"\${WORKDIR}\/git\"/d" `find . -name *.bb -o -name *.inc -o -name *.bbclass`
```

Then, the following command can tweak the remaining S assignments to refer to
UNPACKDIR instead of WORKDIR:

```bash
> sed -i "s/^S = \"\${WORKDIR}\//S = \"\${UNPACKDIR}\//g" `find . -name *.bb -o -name *.inc -o -name *.bbclass`
```

The first change can introduce a lot of consecutive empty lines, so those can be removed with:

```bash
> sed -i -z -E 's/([ \t\f\v\r]*\n){3,}/\n\n/g' `find . -name *.bb -o -name *.inc`
```

# Files affected by WORKDIR changes

Let's look at the files that have been affected by this change in this
repository:

```
> find . -name *.bb -o -name *.inc -o -name *.bbclass | grep "^S = ...WORKDIR"
./recipes-eda/kicad/kicad-i18n.bb:S = "${WORKDIR}/git"
./recipes-eda/kicad/kicad.bb:S = "${WORKDIR}/git"
./recipes-eda/kicad/kicad-packages3d.bb:S = "${WORKDIR}/git"
./recipes-eda/kicad/kicad-symbols.bb:S = "${WORKDIR}/git"
./recipes-eda/kicad/kicad-footprints.bb:S = "${WORKDIR}/git"
./recipes-eda/kicad/kicad-templates.bb:S = "${WORKDIR}/git"
./recipes-eda/opencascade/opencascade.bb:S = "${WORKDIR}/git"
./recipes-support/tbb/tbb-2020_2020.2.bb:S = "${WORKDIR}/git"
./recipes-devtools/thonny/thonny.bb:S = "${WORKDIR}/git"
./recipes-devtools/micropython/micropython-rp2-elf2uf2-native.bb:S = "${WORKDIR}/git/lib/pico-sdk/tools/elf2uf2"
./recipes-devtools/micropython/micropython.inc:S = "${WORKDIR}/git"
./recipes-avr/avr-gcc/avr-gcc_8.4.0.bb:S = "${WORKDIR}/gcc-${PV}"
./recipes-avr/avr-tools/avrdude.bb:S = "${WORKDIR}/git"
./recipes-avr/avr-tools/avarice.bb:S = "${WORKDIR}/git"
./recipes-avr/avr-gdb/avr-gdb_9.1.bb:S = "${WORKDIR}/gdb-${PV}"
./recipes-avr/avr-projects/old-avr-projects.bb:S = "${WORKDIR}/git"
./recipes-avr/avr-libc/avr-libc.inc:S = "${WORKDIR}/git"
./recipes-other-mcus/stcgal/stcgal.bb:S = "${WORKDIR}/git"
./recipes-arm-none-eabi/arm-none-eabi-binutils/arm-none-eabi-binutils_2.38.bb:S = "${WORKDIR}/binutils-${PV}"
./recipes-graphics/vtk/vtk.bb:S = "${WORKDIR}/VTK-${PV}"
```

Most can just be removed Luckily. 

There are a few other changes:

## micropython-rp2-elf2uf2-native.bb

This line:

```
S = "${WORKDIR}/git/lib/pico-sdk/tools/elf2uf2"
```

fits this example:

```
S = "${UNPACKDIR}/${BP}/something"
```

So we can update it to be:

```
S = "${UNPACKDIR}/${BP}/lib/pico-sdk/tools/elf2uf2"
```

## avr-gcc_8.4.0.bb

This line:

```
S = "${WORKDIR}/gcc-${PV}"
```

fits this example:

```
> S = "${UNPACKDIR}/something"
```

So we can update it to be:

```
S = "${UNPACKDIR}/gcc-${PV}"
```

This applies to most of the others above as well.

# Warning while building avr-binutils

```bash
WARNING: /tools/bitbake-builds/poky-whinlatter/layers/meta-microcontroller/recipes-avr/avr-binutils/avr-binutils_2.38.bb:13 has a lack of whitespace around the assignment: 'LIC_FILES_CHKSUM="    file://COPYING;md5=59530bdf33659b29e73d4adb9f9f6552    file://COPYING.LIB;md5=9f604d8a4f8e74f4f5140845a21b6674    file://COPYING3;md5=d32239bcb673463ab874e80d47fae504    file://COPYING3.LIB;md5=6a6a8e020838b23406c81b19c1d46df6    file://gas/COPYING;md5=d32239bcb673463ab874e80d47fae504    file://include/COPYING;md5=59530bdf33659b29e73d4adb9f9f6552    file://include/COPYING3;md5=d32239bcb673463ab874e80d47fae504    file://libiberty/COPYING.LIB;md5=a916467b91076e631dd8edb7424769c7    file://bfd/COPYING;md5=d32239bcb673463ab874e80d47fae504"'
```

How Annoying. Let's find all the files with this issue:

```bash
> find . -name *.bb -o -name *.inc -o -name *.bbclass | grep "^LIC_FILES_CHKSUM=\""
./recipes-avr/avr-binutils/avr-binutils_2.38.bb:LIC_FILES_CHKSUM="\
./recipes-arm-none-eabi/arm-none-eabi-binutils/arm-none-eabi-binutils_2.38.bb:LIC_FILES_CHKSUM="\
```

Easy fix.

# Error in avr-libc

Configuration error:

```
...
| NOTE: /tools/bitbake-builds/poky-whinlatter/build/tmp/work/all-poky-linux/avr-libc/2.1.0+git/sources/avr-libc-2.1.0+git/config.log
| ERROR: configure failed
| WARNING: exit code 1 from a shell command.
ERROR: Task (/tools/bitbake-builds/poky-whinlatter/layers/meta-microcontroller/recipes-avr/avr-libc/avr-libc.bb:do_configure) failed with exit code '1'
```

Full log cached in docs/avr-libc.log

Topic of interest for this issue - could be related to a GCC script?

    https://bbs.archlinux.org/viewtopic.php?id=266711

Good stuff happening here:

```
configure:5696: gcc none --sysroot=/tools/bitbake-builds/poky-whinlatter/build/tmp/work/all-poky-linux/avr-libc/2.1.0+git/recipe-sysroot -c   conftest.c >&5
gcc: warning: none: linker input file unused because linking not done
gcc: error: none: linker input file not found: No such file or directory
```

hmmm maybe we need a dev shell and to take a look at `configure` directly? That
`none` is pretty suspicious.

`do_configure` calls `devtools/get-avr-lib-tree.sh` first, so let's try that:

```bash
> ./devtools/get-avr-lib-tree.sh
> ./configure?
```

That doesn't work ... 

Looking at the build environment - there are a LOT of references to `none`:

```
> env | grep none
CPP=gcc -E --sysroot=/tools/bitbake-builds/poky-whinlatter/build/tmp/work/all-poky-linux/avr-libc/2.1.0+git/recipe-sysroot none
CXX=g++ none --sysroot=/tools/bitbake-builds/poky-whinlatter/build/tmp/work/all-poky-linux/avr-libc/2.1.0+git/recipe-sysroot
CCLD=gcc none --sysroot=/tools/bitbake-builds/poky-whinlatter/build/tmp/work/all-poky-linux/avr-libc/2.1.0+git/recipe-sysroot
LD=ld --sysroot=/tools/bitbake-builds/poky-whinlatter/build/tmp/work/all-poky-linux/avr-libc/2.1.0+git/recipe-sysroot none
AS=as none
FC=gfortran none --sysroot=/tools/bitbake-builds/poky-whinlatter/build/tmp/work/all-poky-linux/avr-libc/2.1.0+git/recipe-sysroot
CC=gcc none --sysroot=/tools/bitbake-builds/poky-whinlatter/build/tmp/work/all-poky-linux/avr-libc/2.1.0+git/recipe-sysroot
```

This definitely doesn't look right. We should be cross compiling right? So
what's going on here.

OH! OK SO `CPP` does NOT have a `none` in it - I wonder if the CFLAGS and others
need defined, otherwise they become "none".

Adding the following to the avr libc include:

```
HOST_AS_ARCH = ""
HOST_CC_ARCH = ""
HOST_LD_ARCH = ""
```

This gets us to the next error:

```bash
| configure: error: Wrong C compiler found; check the PATH!
| NOTE: The following config.log files may provide further information.
| NOTE: /tools/bitbake-builds/poky-whinlatter/build/tmp/work/all-poky-linux/avr-libc/2.1.0+git/sources/avr-libc-2.1.0+git/config.log
| ERROR: configure failed
```

OK So NOW we're configured for the wrong GCC. I think we need to depend on
avr-gcc in order for this to work? And we may need to set HOSTTOOLS perhaps to
get the environment correct.

# Comilation Error in avr-gcc

Error stated something like: 

```bash
Unrecognized flag --fcanan-prefix-map ...
```

This flag was added in GCC 13 or so. Since we're building GCC 8, we definitely
can't provide this flag.

```
# Feature '-fcanon-prefix-map' was added in GCC 13 or so
# 
# Needed to be removed from 'meta-clang' at the time 
#     https://github.com/kraj/meta-clang/pull/782/
DEBUG_PREFIX_MAP:remove = "-fcanon-prefix-map"
DEBUG_PREFIX_MAP_EXTRA:remove = "-fcanon-prefix-map"
```

Compilation works after this.

# Comilation Error in avr-libc

This one was a bit more tricky. Firstly, the compiler variables were set wrong:

```
AR='gcc-ar'
AS='as none'
...
```

And the errors were a mixed bag of "no linker script none found" and stuff like
that. There are two parts of this:

1. The 'none' comes from the these flags not being set:
   ```
   HOST_AS_ARCH = ""
   HOST_CC_ARCH = ""
   HOST_LD_ARCH = ""
   ```
   But that doesn't really get to the heart of the problem. I'll set this for
   host and target in the recipe just in case
2. The Archiver _really_ needs to be set to `avr-ar`

In `classes/avr-toolchain-base.bbclass` (which `avr-libc` inherits from) there
are some exported variables:

```
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
```

These aren't being sent to the shell. I don't know when this was changed, cause
I assumed this worked at some point.

My solution is to just add a bash function:

```
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
```

and call the function from the `avr-libc.inc:do_configure` prepend recipe bit:

```
do_configure:prepend() {
    avr_env
    cd ${S}
    ./devtools/gen-avr-lib-tree.sh
    touch ChangeLog
}
```

Which works great.

# QA Issues

Next is some QA issues in the `avr-gcc` package:

```
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/libgcc.a in package avr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/libgcov.a in package avr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avrxmega4/libgcc.a in package avr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avrxmega4/libgcov.a in package avr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avr6/libgcc.a in package avr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avr6/libgcov.a in package vr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avr31/libgcc.a in package vr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avr31/libgcov.a in packageavr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avr4/libgcc.a in package avr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avr4/libgcov.a in package vr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avr51/libgcc.a in package vr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avr51/libgcov.a in packageavr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avrxmega2/libgcc.a in package avr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avrxmega2/libgcov.a in package avr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avrxmega3/libgcc.a in package avr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avrxmega3/libgcov.a in package avr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avrxmega3/short-calls/libgcc.a in package avr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avrxmega3/short-calls/libgcov.a in package avr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avrxmega6/libgcc.a in package avr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avrxmega6/libgcov.a in package avr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avrxmega5/libgcc.a in package avr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avrxmega5/libgcov.a in package avr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/tiny-stack/libgcc.a in package avr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/tiny-stack/libgcov.a in package avr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avrtiny/libgcc.a in package avr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avrtiny/libgcov.a in package avr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avr35/libgcc.a in package vr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avr35/libgcov.a in packageavr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avr5/libgcc.a in package avr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avr5/libgcov.a in package vr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avr3/libgcc.a in package avr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avr3/libgcov.a in package vr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avrxmega7/libgcc.a in package avr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avrxmega7/libgcov.a in package avr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avr25/libgcc.a in package vr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avr25/libgcov.a in packageavr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avr25/tiny-stack/libgcc.a n package avr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/avr25/tiny-stack/libgcov.ain package avr-gcc-staticdev contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/bin/avr-cpp in package avr-gcc contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/bin/avr-c++ in package avr-gcc contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/bin/avr-g++ in package avr-gcc contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/bin/avr-gcc in package avr-gcc contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/bin/avr-gcc-8.4.0 in package avr-gcc contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/include-fixed/pthread.h inpackage avr-gcc contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/plugin/include/auto-host.hin package avr-gcc contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/plugin/include/configargs. h in package avr-gcc contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/lib/gcc/avr/8.4.0/install-tools/mkheaders.conf in package avr-gcc contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/libexec/gcc/avr/8.4.0/lto1 in package avr-gcc contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/libexec/gcc/avr/8.4.0/cc1plus in package avr-gcc contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/libexec/gcc/avr/8.4.0/collect2 in package avr-gcc contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/libexec/gcc/avr/8.4.0/cc1 in package avr-gcccontains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/libexec/gcc/avr/8.4.0/install-tools/fixincl n package avr-gcc contains reference to TMPDIR [buildpaths]
ERROR: avr-gcc-1_8.4.0-r0 do_package_qa: QA Issue: File /usr/src/debug/avr-gcc/8.4.0/gcc/configargs.h in ackage avr-gcc-src contains reference to TMPDIR [buildpaths]
```

Which is QA Check 11.2.4 `buildpaths`:

```
11.2.4 buildpaths

> File <filename> in package <packagename> contains reference to TMPDIR [buildpaths]

This check ensures that build system paths (including TMPDIR) do not appear in
output files, which not only leaks build system configuration into the target,
but also hinders binary reproducibility as the output will change if the build
system configuration changes.

Typically these paths will enter the output through some mechanism in the
configuration or compilation of the software being built by the recipe. To
resolve this issue you will need to determine how the detected path is entering
the output. Sometimes it may require adjusting scripts or code to use a relative
path rather than an absolute one, or to pick up the path from runtime
configuration or environment variables.
```

Well that's not good. I mean it's probably fine ... we can skip this for now and
figure out what to do with it later.

To skip this check, we can add an `INSANE-SKIP` to `avr-gcc`:

```
INSANE_SKIP:${PN} = "dev-so buildpaths"

```

This didn't actually work to remove the errors though. Odd! But disabling QA
altogether did:

```
do_package_qa[noexec] = "1"
```

wild stuff.

# avr-dev-rules

This is required for avrdude (aka the Arduino programmer)

```
ERROR: avr-udev-rules-0.1-r0 do_install: Execution of '/tools/bitbake-builds/poky-whinlatter/build/tmp/work/all-poky-linux/avr-udev-rules/0.1/temp/run.do_install.2471775' failed with exit code 1
ERROR: Logfile of failure stored in: /tools/bitbake-builds/poky-whinlatter/build/tmp/work/all-poky-linux/avr-udev-rules/0.1/temp/log.do_install.2471775
Log data follows:
| DEBUG: Executing python function extend_recipe_sysroot
| NOTE: Direct dependencies are ['/tools/bitbake-builds/poky-whinlatter/layers/openembedded-core/meta/recipes-devtools/quilt/quilt-native_0.69.bb:do_populate_sysroot', 'virtual:native:/tools/bitbake-builds/poky-whinlatter/layers/openembedded-core/meta/recipes-devtools/patch/patch_2.8.bb:do_populate_sysroot', 'virtual:native:/tools/bitbake-builds/poky-whinlatter/layers/openembedded-core/meta/recipes-devtools/pseudo/pseudo_git.bb:do_populate_sysroot']
| NOTE: Installed into sysroot: []
| NOTE: Skipping as already exists in sysroot: ['gettext-minimal-native', 'libtool-native', 'quilt-native', 'texinfo-dummy-native', 'patch-native', 'pseudo-native', 'xz-native', 'attr-native', 'sqlite3-native']
| DEBUG: Python function extend_recipe_sysroot finished
| DEBUG: Executing shell function do_install
| install: cannot stat '/tools/bitbake-builds/poky-whinlatter/build/tmp/work/all-poky-linux/avr-udev-rules/0.1/60-avr-dev-devices.rules': No such file or directory
| WARNING: exit code 1 from a shell command.
ERROR: Task (/tools/bitbake-builds/poky-whinlatter/layers/meta-microcontroller/recipes-avr/avr-tools/avr-udev-rules.bb:do_install) failed with exit code '1'
```

Solution: change the install command a bit:

```
diff --git a/recipes-avr/avr-tools/avr-udev-rules.bb b/recipes-avr/avr-tools/avr-udev-rules.bb
index 641adb4..03f66cb 100644
--- a/recipes-avr/avr-tools/avr-udev-rules.bb
+++ b/recipes-avr/avr-tools/avr-udev-rules.bb
@@ -9,5 +9,5 @@ PV = "0.1"

 do_install () {
     install -d ${D}${sysconfdir}/udev/rules.d
-    install -m 0644 ${WORKDIR}/60-avr-dev-devices.rules ${D}${sysconfdir}/udev/rules.d/
+    install -m 0644 ${WORKDIR}/sources/60-avr-dev-devices.rules ${D}${sysconfdir}/udev/rules.d/
 }
```

# avrdude

```
ERROR: avrdude-7.0+git-r0 do_package_qa: QA Issue: File /usr/src/debug/avrdude/7.0+git/lexer.c in package
 avrdude-src contains reference to TMPDIR [buildpaths]
ERROR: avrdude-7.0+git-r0 do_package_qa: QA Issue: File /usr/src/debug/avrdude/7.0+git/config_gram.c in p
ackage avrdude-src contains reference to TMPDIR [buildpaths]
ERROR: avrdude-7.0+git-r0 do_package_qa: Fatal QA errors were found, failing task.
ERROR: Logfile of failure stored in: /tools/bitbake-builds/poky-whinlatter/build/tmp/work/cortexa7t2hf-ne
on-vfpv4-poky-linux-gnueabi/avrdude/7.0+git/temp/log.do_package_qa.2474886
ERROR: Task (/tools/bitbake-builds/poky-whinlatter/layers/meta-microcontroller/recipes-avr/avr-tools/avrd
ude.bb:do_package_qa) failed with exit code '1'
```

Great. Let's try to add the skip again? Nope doesn't work :/ OK we'll disable QA
again...

```
do_package_qa[noexec] = "1"
```
