#!/usr/bin/python
#coding:utf-8
from threading import Thread
from Queue import Queue
from random import randint
from time import sleep, time



class MyThread(Thread):

    def __init__(self, func, argcs, name=""):
        super(MyThread,self).__init__()
        self.name = name
        self.func = func
        self.args = argcs

    def getResult(self):
        return self.res

    def run(self):
       # print('starting    %s at : %s'%(self.name, time()))
        self.res=self.func(*self.args)
       # print('finishing   %s at : %s'%(self.name, time()))


def writeQ(queue):
    queue.put('xxx', 1)
    print("producing object for Q...size now", queue.qsize())


def readQ(queue):
    queue.get(1)
    print("consumed object from Q...sizo now", queue.qsize())


def writer(queue,loops):
    #只做一件事情，一次往队列里放一个对象，等一会，再放
    for i in range(loops):
        writeQ(queue)
        sleep(1)

def reader(queue,loops):
    for i in range(loops):
        readQ(queue)
        sleep(randint(2,5))


#funcs=[writer,reader]
funcs=[reader,writer]
#funcs=[reader]
nfuncs=range(len(funcs))


def main():
    nloops = 4  # randint(10,20)
    q=Queue(2)
    threads=[]

    for i in nfuncs:
        t = MyThread(funcs[i],(q,nloops),funcs[i].__name__)
        threads.append(t)

    for i in nfuncs:
        threads[i].start()

    for i in nfuncs:
        threads[i].join()
        print threads[i].getResult()

    print 'all Done'


if __name__=='__main__':
    main()
