import signal
import time
import os
import fcntl
from threading import Thread, Event
import RPi.GPIO as GPIO


def wait_mouse_click():
    mouse = file('/dev/input/mice')
    playtime=0
    fd=mouse.fileno()
    flag = fcntl.fcntl (fd, fcntl.F_GETFL)
    fcntl.fcntl(fd,fcntl.F_SETFL, flag | os.O_NONBLOCK)
    while True:
        try:
            status, dx, dy = tuple(ord(c) for c in mouse.read(3))
            def to_signed(n):
                return n - ((0x80 & n) << 1)

            dx = to_signed(dx)
            dy = to_signed(dy)
            # print "status=%s / %#02x, %d, %d" % (status, status, dx, dy)
            if status in [9, 10, 12]:
                return 1
        except:
            pass #print "nothing to read"
            

        #print GPIO.input (4)
        if GPIO.input (17):
            while GPIO.input (17):
                pass

            if playtime > 10:
                playtime=0
                return 1

        if GPIO.input (27):
            return 2

        time.sleep (0.2)
        playtime=playtime+1


class MouseClickThread(Thread):
    def __init__(self, callback_click, callback_stop):
        Thread.__init__(self)
        self.daemon = True
        self.callback_click = callback_click
        self.callback_stop = callback_stop
        self.stop_event = Event()
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(17, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)  #wheel
        GPIO.setup(27, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)  #stop
        #GPIO.setup(22, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)  #start

        #print GPIO.input (4)
        # GPIO.add_event_detect(4, GPIO.RISING)
        # def button4_pushed():
        #     print "button4 pushed"

        # GPIO.add_event_callback(4, button4_pushed)       

    def run(self):
        while not self.stop_event.is_set():
            # Continually wait for a mouse click
            if wait_mouse_click() == 1:
                self.callback_click()

            else:
                self.callback_stop()
                self.stop_event.set()


if __name__ == "__main__":
    # while True:
    #     wait_mouse_click()
    #     print "mouse clicked"
    def clicked():
        print "clicked"

    def shutdown():
        print "clicked"

    thread = MouseClickThread(clicked, shutdown)
    thread.start()

    print "waiting for signal..."
    signal.pause()
    print "bye"
