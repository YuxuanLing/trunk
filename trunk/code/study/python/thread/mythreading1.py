#!/usr/bin/python
#coding:utf-8
from threading import Thread
from time import sleep, time
loops=[4,2]
loops=[2,4]


class ThreadFunc(object):
    def __init__(self, func, argcs, name=""):
        self.name = name
        self.func = func
        self.args = argcs

    def __call__(self):
        # 创建新线程的时候
        # Thread对象会调用我们的ThreadFunc对象，这时候会用到一个特殊的函数__call__()
        #
        self.func(*self.args)

def loop(nloop, nsec):
    print('start loop %s at: %s'%(nloop, time()))
    sleep(nsec)
    print('loop %s done at: %s'%(nloop,time()))


def main():
    print('starting at:', time())
    threads = []
    nloops = range(len(loops))

    for i in nloops:
        t = Thread(target=ThreadFunc(loop, (i,loops[i]), loop.__name__))
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
