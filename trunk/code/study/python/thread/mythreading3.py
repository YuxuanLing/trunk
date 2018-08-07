#!/usr/bin/python
#coding:utf-8
import threading
from threading import Thread, current_thread
from time import sleep

lock = threading.RLock()


def show(nsec):
   with lock:
        print current_thread().name, (nsec + 1)
        sleep(0.1)


def test():
    with lock:
        for i in range(3):
            show(i)



for i in range(2):
    Thread(target=test).start()
