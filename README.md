# UltraLightDM
WARNING: This Utility has only been tested on linux, it may not work on something like BSD

This is a Utility that Automaticaly starts the Desktop Environment. Written in C. it has a rather small footprint and is pretty dang fast.

# Installation
"Install" This by adding a command to execute it in /etc/profile or /etc/profile.d/
then, create a file called "config" in /etc/UL-DM/ and write
1. the Command used to start the Desktop (ex: startxfce4/startx)
2. the Process of the Desktop or a process that always runs with it (ex: xfce4-session)
3. Profit!



Warning: This project is mostly me just coming to grips with C, so the Code is probably bad.

# Why would you do this?
The reason i made this is beceause i noticed that lightdm used a whole 100 Megabytes of RAM while being inactive (on a computer with 16 gigabytes of RAM), so i decided to 1. make a bash script that does this and then 2. turn it into a C program because why the hell not?
