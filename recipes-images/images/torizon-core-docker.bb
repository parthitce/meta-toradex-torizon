SUMMARY = "TorizonCore image with Docker runtime"
DESCRIPTION = "TorizonCore image, featuring complete Docker runtime and secure \
OTA update capabilities."

require torizon-core-container.inc

CORE_IMAGE_BASE_INSTALL += " \
    docker \
"

EXTRA_USERS_PARAMS += "\
usermod -a -G docker torizon; \
"
