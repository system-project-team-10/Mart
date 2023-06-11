#!/usr/bin/env python

import RPi.GPIO as GPIO
from mfrc522 import SimpleMFRC522
from time import sleep
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
    id2, text = reader.read()
    # TAG 자체 문제로 인한 hardcode.
    if id2 == 712322887692:
        write_file("Item: Ramen, 1000\n")
    elif id2 == 657940568739:
        write_file("Item: Coffee, 2000\n")
    else:
        write_file(text+"\n")
    sleep(3)
    write_file("nodata\n")

GPIO.cleanup()