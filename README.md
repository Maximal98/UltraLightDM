# UltraLightDM
WARNING: This Utility works on Linux ONLY, unless ported.

This is a Utility that Automaticaly starts the Desktop Environment. Written in C. it has a rather small footprint and is pretty dang fast.

# Installation
"Install" This by adding a command to execute it in /etc/profile or /etc/profile.d/
then, create a file called "config" in /etc/UL-DM/ and write
1. the Command used to start the Desktop (ex: startxfce4/startx)
2. the Process of the Desktop or a process that always runs with it (ex: xfce4-session)
3. Profit!
