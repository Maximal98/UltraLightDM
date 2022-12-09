
curl https://github.com/Maximal98/UltraLightDM/releases/latest/download/uldm -o /usr/bin/uldm
mkdir /etc/uldm

read -p 'TTY Number to start ULDM on (ex 1 or 7 for tty1 or tty7):' TTYNUM
TTY=/dev/tty$TTYNUM
curl https://raw.githubusercontent.com/Maximal98/UltraLightDM/main/systemd/uldm.service -o /etc/systemd/system/uldm@$TTYSELECTION.service
curl https://raw.githubusercontent.com/Maximal98/UltraLightDM/main/systemd/chvt.service -o /etc/systemd/system/chvt-$TTYNUM.service
echo "$TTY="$TTYNUM > /etc/uldm/ttyenv
systemctl enable uldm@$TTYSELECTION
systemctl enable chvt-$TTYNUM

echo Done!