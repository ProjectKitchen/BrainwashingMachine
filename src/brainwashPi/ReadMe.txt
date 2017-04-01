important files for startup / settings:

/etc/rc.local:
added GPIO listener
added RFCOMM bind (BT serial port)

/home/pi/.config/lxsession/LXDE/autostart
added @lxterminal -e /home/pi/brainwshPi/start.sh
load qsynth, aconnect midi devices etc.

used music player is located in 
/home/pi/Mouseplayer/src
(reacts to GPIO input for washbuttons!)


