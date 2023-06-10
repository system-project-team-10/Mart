#!/usr/bin/env python

import RPi.GPIO as GPIO
from mfrc522 import SimpleMFRC522
from time import sleep
import sys
import fcntl

reader = SimpleMFRC522()

def write_file(text):
    f = open("rfid.txt", "w")
    fcntl.flock(f.fileno(), fcntl.LOCK_SH)
    f.write(text)
    print(text)
    fcntl.flock(f.fileno(), fcntl.LOCK_UN)
    f.close()

while (True):
    _, text = reader.read()
    write_file(text+"\n")
    sleep(3)
    write_file("nodata\n")
    sleep(3)

GPIO.cleanup()