#!/usr/bin/python
#coding:utf-8
from threading import Event
from threading import Thread


def test_event():
    e = Event()
    def test():
        for i in range(5):
            print('start wait')
            e.wait()
            e.set()
            e.clear()  #如果不调用clear 那么标记一直未true wait 就不会发生柱塞
            print i

    Thread(target=test).start()
    return e

e=test_event()
