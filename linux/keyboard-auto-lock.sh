#!/bin/bash

last_info=''
#mitigation_type='lock'
mitigation_type='disable'

function lock_system()
{
    loginctl lock-sessions
    ret="$?"
    if [ "${ret}" -eq 0 ]; then
        return
    fi
    /bin/loginctl lock-sessions
    ret="$?"
    if [ "${ret}" -eq 0 ]; then
        return
    fi
    vlock -a
    ret="$?"
    if [ "${ret}" -eq 0 ]; then
        return
    fi
    /bin/vlock -a
    ret="$?"
    if [ "${ret}" -eq 0 ]; then
        return
    fi
}

if [ "$(id -u)" -ne 0 ]; then
    echo "Error: You have to run as root"
    exit 1
fi

if ! which inotifywait > /dev/null 2>&1; then
    echo
    echo "Utility inotifywait not found. Please install first"
    echo "Run: yum/apt install inotify-tools"
    echo
    exit 2
fi

if [ "${mitigation_type}" == "disable" ]; then
    if ! which xinput > /dev/null 2>&1; then
        echo
	echo "Mitigation type set to 'disable' but xinput not found in PATH."
	echo "Please change mitigation type or install xinput binary."
	echo
	exit 3
    fi
fi

action="$1"
if [ "${action}" == "--install" ]; then
    copy=1
    if [ -f "/bin/keyboard-auto-lock" ]; then
        f1="$(stat --format=%i /bin/keyboard-auto-lock)"
        f2="$(stat --format=%i "$0")"

        if [ "${f1}" == "${f2}" ]; then
            copy=0
        fi
    fi

    if [ "${copy}" -eq 1 ]; then
        sudo rm -f /bin/keyboard-auto-lock
        sudo cp "$0" /bin/keyboard-auto-lock
    fi
    link="$(readlink /etc/rc.d/rc.local)"
    if [ -z "${link}" ]; then
        link="/etc/rc.d/rc.local"
    fi
    if [ -f "${link}" ]; then
        if ! grep "keyboard-auto-lock" /etc/rc.d/rc.local > /dev/null 2>&1; then
            sudo sed -i 's/exit 0/\/bin\/keyboard-auto-lock --run \&\nexit 0/g' /etc/rc.d/rc.local
        fi
        if ! grep "keyboard-auto-lock" /etc/rc.d/rc.local > /dev/null 2>&1; then
           echo "/bin/keyboard-auto-lock --run &" >> /etc/rc.d/rc.local
        fi
        chmod 755 /etc/rc.d/rc.local
        sudo systemctl enable rc-local.service > /dev/null 2>&1
        if ! sudo systemctl is-enabled rc-local.service | grep -E "enabled|static" > /dev/null 2>&1; then
            echo "Warning: Rc.local service cannot be enabled"
        fi
    fi
    sudo /bin/keyboard-auto-lock --run &
    echo "Installed"
elif [ "${action}" == "--uninstall" ]; then
    if [ -f "/etc/rc.d/rc.local" ]; then
        sudo sed -i '\/bin\/keyboard-auto-lock/d' /etc/rc.d/rc.local
    fi
    echo "Removed"
elif [ "${action}" == "--run" ]; then
    if [ "${mitigation_type}" == 'disable' ]; then
        orig="$(xinput list --id-only)"
    fi
    while [ 1 ]
    do
        dev="$(inotifywait --format=%f -q -e CREATE /dev)"
        info="$(dmesg | grep "input,${dev}" | tail -n 1)"
        if [ "${last_info}" != "${info}" ]; then
            if [[ "${info}" == *Keyboard* ]] || [[ "${info}" == *Mouse* ]]; then
                if [ "${mitigation_type}" == 'disable' ]; then
 		    remaining=20 # 2 seconds
		    new_id=''
		    while [ "${remaining}" -gt 0 ]
		    do
		       data="$(xinput list --id-only)"
		       new_id="$(diff -up <(echo "$orig") <(echo "$data") | tail -n 1 | tr -d '+')"
		       if [[ "${new_id}" == *-* ]]; then
			   new_id=''
		       fi
		       if [ ! -z "${new_id}" ]; then
		           break
		       fi
		       sleep 0.1
		       remaining=$((remaining-1))
	            done
		    if [ ! -z "${new_id}" ]; then
			xinput disable "${new_id}"
			orig="$(xinput list --id-only | grep -v "${new_id}")"
		        logger -t "$(basename "$0")" "New input device has been automatically disabled. Use 'xinput enable ${new_id}' to re-enable"
		    fi
		elif [ "${mitigation_type}" == 'lock' ]; then
                   logger -t "$(basename "$0")" "Found new keyboard or mouse device. Locking ..."
                   lock_system
	        fi
            fi
            last_info="${info}"
        fi
    done
else
    echo "Syntax: $(basename "$0") <--install|--run|--uninstall>"
    exit 1
fi

exit 0
