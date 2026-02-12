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
