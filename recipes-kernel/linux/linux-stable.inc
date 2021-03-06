DESCRIPTION = "Common Open Source Foundries Linux microPlatform Kernel"

PV = "${LINUX_VERSION}+git${SRCPV}"

DEPENDS += "${@bb.utils.contains('ARCH', 'x86', 'elfutils-native', '', d)}"
DEPENDS += "openssl-native util-linux-native"

SRC_URI = "git://git.toradex.com/linux-toradex.git;protocol=https;branch=${SRCBRANCH};name=kernel \
    file://defconfig \
    file://distro.scc \
    file://distro.cfg \
    file://ioaccounting.cfg \
    file://imx.scc \
    file://imx.cfg \
    file://0001-enable-lm816.patch \
"
export DTC_FLAGS = "-@"

LIC_FILES_CHKSUM = "file://COPYING;md5=bbea815ee2795b2f4230826c0c6b8814"

inherit kernel siteinfo
require recipes-kernel/linux/linux-yocto.inc

KCONFIG_MODE = "--alldefconfig"
KERNEL_DTS_PATH = "/usr/src/dts"

# You can define CMDLINE_DEBUG as "debug" in your local.conf or distro.conf
# to enable kernel debugging.
CMDLINE_DEBUG ?= ""
CMDLINE_append = " ${CMDLINE_DEBUG}"

KERNEL_INITRAMFS ?= '${@oe.utils.conditional("INITRAMFS_IMAGE_BUNDLE", "1", "1", "", d)}'

# No need for extra fragments besides what is provided by LMP
KERNEL_FEATURES_remove += " features/debug/printk.scc"

# Always have a package to allow forcing package dependency
ALLOW_EMPTY_${KERNEL_PACKAGE_NAME}-devicetree = "1"

# Task kernel_configme requires bison-native to be available
# Upstream added bison-native to build-deps but not for the required task
do_kernel_configme[depends] += "bison-native:do_populate_sysroot"

python __anonymous () {
    import bb
    devicetree = d.getVar('KERNEL_DEVICETREE', True) or ''
    if devicetree:
        depends = d.getVar("DEPENDS", True)
        d.setVar("DEPENDS", "%s dtc-native" % depends)
}

# Set a variable in .configure
# $1 - Configure variable to be set
# $2 - value [n/y/value]
kernel_configure_variable() {
    # Remove the config
    CONF_SED_SCRIPT="$CONF_SED_SCRIPT /CONFIG_$1[ =]/d;"
    if test "$2" = "n"
    then
        echo "# CONFIG_$1 is not set" >> ${B}/.config
    else
        echo "CONFIG_$1=$2" >> ${B}/.config
    fi
}

do_configure_prepend() {
    mv -f ${B}/.config ${B}/.config.patched
    CONF_SED_SCRIPT=""

    # Set cmdline
    kernel_configure_variable CMDLINE "\"${CMDLINE}\""

    # Localversion
    kernel_configure_variable LOCALVERSION "\"\""
    kernel_configure_variable LOCALVERSION_AUTO n

    # Udev quirks
    # Newer versions of udev mandate that sysfs doesn't have deprecated entries
    kernel_configure_variable SYSFS_DEPRECATED n
    kernel_configure_variable SYSFS_DEPRECATED_V2 n
    kernel_configure_variable HOTPLUG y
    kernel_configure_variable UEVENT_HELPER_PATH "\"\""
    kernel_configure_variable UNIX y
    kernel_configure_variable SYSFS y
    kernel_configure_variable PROC_FS y
    kernel_configure_variable TMPFS y
    kernel_configure_variable INOTIFY_USER y
    kernel_configure_variable SIGNALFD y
    kernel_configure_variable TMPFS_POSIX_ACL y
    kernel_configure_variable BLK_DEV_BSG y
    kernel_configure_variable DEVTMPFS y
    kernel_configure_variable DEVTMPFS_MOUNT y

    # Systemd need cgroup support
    kernel_configure_variable CGROUP_SCHED y
    kernel_configure_variable CGROUPS y
    kernel_configure_variable CGROUP_NS y
    kernel_configure_variable CGROUP_FREEZER y
    kernel_configure_variable CGROUP_DEVICE y
    kernel_configure_variable CPUSETS y
    kernel_configure_variable PROC_PID_CPUSET y
    kernel_configure_variable CGROUP_CPUACCT y
    kernel_configure_variable RESOURCE_COUNTERS y

    # Systemd
    kernel_configure_variable RT_GROUP_SCHED n

    if [ ! -z "${KERNEL_INITRAMFS}" ]; then
        kernel_configure_variable BLK_DEV_INITRD y
        kernel_configure_variable INITRAMFS_SOURCE ""
        kernel_configure_variable RD_GZIP y
        kernel_configure_variable OVERLAY_FS y
        kernel_configure_variable SQUASHFS y
        kernel_configure_variable UBIFS_FS y
    fi

    # Activate CONFIG_LEGACY_PTYS
    kernel_configure_variable LEGACY_PTYS y
    # this module is built externally via drbd-utils
    kernel_configure_variable BLK_DEV_DRBD n

    # Keep this the last line
    # Remove all modified configs and add the rest to .config
    sed -e "${CONF_SED_SCRIPT}" < '${B}/.config.patched' >> '${B}/.config'
    rm -f ${B}/.config.patched
}

do_install_append() {
    install -d ${D}/boot
    make -C ${S} O=${B} ARCH=$ARCH dtbs || true
    install -m 0644 ${B}/arch/$ARCH/boot/dts/*.dtb ${D}/boot || true

    mkdir -p ${D}/${KERNEL_DTS_PATH}
    cp -r ${S}/arch/$ARCH/boot/dts/* ${D}/${KERNEL_DTS_PATH}
    cp -r --dereference ${S}/include/dt-bindings/ ${D}/${KERNEL_DTS_PATH}
    chown -R root:root ${D}/${KERNEL_DTS_PATH}
}

do_deploy_append() {
    # Workaround for usrmerge
    if [ ${MODULE_TARBALL_DEPLOY} = "1" ] && (grep -q -i -e '^CONFIG_MODULES=y$' .config); then
        mkdir -p ${D}${nonarch_base_libdir}
        tar -cvzf $deployDir/modules-${MODULE_TARBALL_NAME}.tgz -C ${D} ${nonarch_base_libdir#/}
        ln -sf modules-${MODULE_TARBALL_NAME}.tgz $deployDir/modules-${MODULE_TARBALL_LINK_NAME}.tgz
    fi
}

# Kernel config
KERNEL_CONFIG_NAME ?= "${KERNEL_PACKAGE_NAME}-config-${KERNEL_ARTIFACT_NAME}"
KERNEL_CONFIG_LINK_NAME ?= "${KERNEL_PACKAGE_NAME}-config"

do_deploy_append() {
    cp -a ${B}/.config ${DEPLOYDIR}/${KERNEL_CONFIG_NAME}
    ln -sf ${KERNEL_CONFIG_NAME} ${DEPLOYDIR}/${KERNEL_CONFIG_LINK_NAME}
}

PACKAGES =+ "${KERNEL_PACKAGE_NAME}-devicetree-overlays ${KERNEL_PACKAGE_NAME}-devicetree-source"
FILES_${KERNEL_PACKAGE_NAME}-devicetree-overlays = "${nonarch_base_libdir}/firmware/*.dtbo ${nonarch_base_libdir}/firmware/*.dts"
FILES_${KERNEL_PACKAGE_NAME}-devicetree += "/boot/*.dtb"
FILES_${KERNEL_PACKAGE_NAME}-devicetree-source += "${KERNEL_DTS_PATH}"

RDEPENDS_${KERNEL_PACKAGE_NAME}-image_append = " ${KERNEL_PACKAGE_NAME}-devicetree"
RRECOMMENDS_${KERNEL_PACKAGE_NAME}-image_append = " \
    ${KERNEL_PACKAGE_NAME}-devicetree-overlays \
    ${KERNEL_PACKAGE_NAME}-devicetree-source \
"
