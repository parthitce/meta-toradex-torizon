product_id=$(cat /proc/device-tree/toradex,product-id)
serial=$(cat /proc/device-tree/serial-number)

#if serial nubmer is empty we append no-serial-number string
if [ -z "$serial" -a "$serial" != " "  ]; then
    serial="no-serial-number"
fi

case "${product_id}" in
        "0039"|"0033"|"0032")
            machine="colibri-imx7"
        ;;
        "0027"|"0028"|"0029"|"0035")
            machine="apalis-imx6"
        ;;
        "0014"|"0015"|"0017")
            machine="colibri-imx6"
        ;;
        "0044"|"0040"|"0045"|"0036")
            machine="colibri-imx6ull"
        ;;
        *)
            machine="unsupported-device"
        ;;
esac
hostname=${machine}"-"${serial}
echo  ${hostname} > /etc/hostname

/usr/bin/hostname -F /etc/hostname
