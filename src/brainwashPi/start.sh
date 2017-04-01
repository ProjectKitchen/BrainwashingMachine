cd /home/pi/brainwashPi
echo starting virmidi service
sudo modprobe snd-virmidi
sleep 1
echo starting brainwash application !
./brainwash
cd /home/pi
./start_both.sh
