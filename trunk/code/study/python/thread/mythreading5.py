#!/usr/bin/python
#coding:utf-8
from threading import Condition, current_thread, Thread
from time import sleep
con =  Condition()

def tc1():
    with con:
        for i in range(5):
            print current_thread().name, i
            sleep(0.3)
            if i == 3:
                con.wait()  #释放锁定 进入condition的等待线程池


def tc2():
    with con:
        for i in range(5):
            print current_thread().name, i
            sleep(1)
            con.notify()      #此方法不会释放锁定
            sleep(2)

Thread(target=tc1).start()
Thread(target=tc2).start()

