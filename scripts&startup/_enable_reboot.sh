#!/bin/bash
pin=21
gpio -1 mode $pin in
gpio -1 wfi $pin both
logger Shutdown button pressed
shutdown -h now

