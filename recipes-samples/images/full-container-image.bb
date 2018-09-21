SUMMARY = "TordyOS image featuring OTA and container capabilities"
DESCRIPTION = "Basic TordyOS console image, featuring complete Docker runtime and SOTA update capabilities."
PV = "0.1"

require image-common.inc

IMAGE_FEATURES += "ssh-server-openssh"

# Enough free space for a full image update
IMAGE_OVERHEAD_FACTOR = "2.3"

# Base packages
CORE_IMAGE_BASE_INSTALL += " \
    coreutils \
    cpufrequtils \
    ldd \
    gptfdisk \
    hostapd \
    htop \
    iptables \
    kernel-modules \
    networkmanager-nmtui \
    ${@bb.utils.contains("MACHINE_FEATURES", "optee", "optee-test optee-client", "", d)} \
    rsync \
    sshfs-fuse \
"

CORE_IMAGE_BASE_INSTALL += " \
    docker \
    less \
    openssh-sftp-server \
    packagegroup-core-full-cmdline-utils \
    packagegroup-core-full-cmdline-extended \
    packagegroup-core-full-cmdline-multiuser \
    python3-compression \
    python3-distutils \
    python3-docker \
    python3-docker-compose \
    python3-json \
    python3-netclient \
    python3-pkgutil \
    python3-shell \
    python3-unixadmin \
    pciutils \
    strace \
    tcpdump \
    vim-tiny \
"

# docker pulls runc/containerd, which in turn recommend lxc unecessarily
BAD_RECOMMENDATIONS_append = " lxc"

EXTRA_USERS_PARAMS += "\
usermod -a -G docker tordy; \
"