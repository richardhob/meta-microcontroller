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
