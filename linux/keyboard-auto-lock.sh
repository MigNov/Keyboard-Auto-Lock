#!/bin/bash

last_info=''

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
    vlock
    ret="$?"
    if [ "${ret}" -eq 0 ]; then
        return
    fi
    /bin/vlock
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
    if [ -f "/etc/rc.d/rc.local" ]; then
        if ! grep "keyboard-auto-lock" /etc/rc.d/rc.local > /dev/null 2>&1; then
            sudo sed -i 's/exit 0/\/bin\/keyboard-auto-lock --run \&\nexit 0/g' /etc/rc.d/rc.local
        fi
        sudo systemctl enable rc-local.service > /dev/null 2>&1
        if ! sudo systemctl is-enabled rc-local.service | grep enabled > /dev/null 2>&1; then
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
    while [ 1 ]
    do
        dev="$(inotifywait --format=%f -q -e CREATE /dev)"
        info="$(dmesg | grep "input,${dev}" | tail -n 1)"
        if [ "${last_info}" != "${info}" ]; then
            if [[ "${info}" == *Keyboard* ]] || [[ "${info}" == *Mouse* ]]; then
                logger -t "$(basename "$0")" "Found new keyboard or mouse device. Locking ..."
                lock_system
            fi
            last_info="${info}"
        fi
    done
else
    echo "Syntax: $(basename "$0") <--install|--run|--uninstall>"
    exit 1
fi

exit 0
