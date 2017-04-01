# amixer cset numid=3 1
#fluidsynth -s -i -a pulseaudio wash3.sf2 &
#fluidsynth -s -i -a alsa wash3.sf2 &
echo stating qsynth
qsynth &
echo waiting for qsynth
sleep 4
echo starting virmidi service
sudo modprobe snd-virmidi
sleep 1
echo connecting virmidi:24 with fluidsynth:128
aconnect 24 128
echo getting focus back to bash window
wmctrl -a brainwashPi

