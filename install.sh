if [ "$1" == "-nosystemd" ]
then
	echo "systemd is the default and only (atm) supported option."
	echo "ULDM will be installed but will not automatically boot."
fi


curl https://github.com/Maximal98/UltraLightDM/releases/latest/download/uldm -o /usr/bin/uldm
mkdir /etc/uldm

if [ "$1" != "-nosystemd" ]
then
	read -p 'TTY to start ULDM on (tty1, tty7):' TTYSELECTION
	read -p 'TTY Number to start ULDM on (1, 7):' TTYNUM
	curl https://raw.githubusercontent.com/Maximal98/UltraLightDM/main/systemd/uldm.service -o /etc/systemd/system/uldm@$TTYSELECTION.service
	curl https://raw.githubusercontent.com/Maximal98/UltraLightDM/main/systemd/chvt.service -o /etc/systemd/system/chvt-$TTYNUM.service
	echo "$TTY="$TTYNUM > /etc/uldm/ttyenv
	systemctl enable uldm@$TTYSELECTION
	systemctl enable chvt-$TTYNUM
fi

echo Done!