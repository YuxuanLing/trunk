#!/usr/bin/python
#coding:utf-8
from threading import Thread
from time import sleep, time
loops=[4,2]
loops=[2,4]


class MyThread(Thread):  #MyThread 类继承了 Thread 类？
    def __init__(self, func, argcs, name=""):
        super(MyThread,self).__init__()
        self.name = name
        self.func = func
        self.args = argcs

    def getResult(self):
        return self.res

    def run(self):
        print('starting    %s at : %s'%(self.name, time()))
        self.res=self.func(*self.args)
        print('finishing   %s at : %s'%(self.name, time()))


def loop(nloop, nsec):
    print('start loop %s at: %s'%(nloop, time()))
    sleep(nsec)
    print('stop  loop %s at: %s'%(nloop,time()))


def main():
    print('starting at:', time())
    threads = []
    nloops = range(len(loops))

    for i in nloops:
        t = MyThread(loop, (i,loops[i]), loop.__name__)
        threads.append(t)

    for i in nloops:
        #start threads
        threads[i].start()

    for i in nloops:
       # join 会等到线程结束 或者使用了timeout参数时候，等到超时为止
       # 只有等到了 thread[i]结束才会继续往下执行
        print('Start wait for thread %s'%(i))
        threads[i].join()  #thread to finish
        print('Stop  wait for thread %s'%(i))

    print('All Done at:',time())



if __name__ == '__main__':
   main()
