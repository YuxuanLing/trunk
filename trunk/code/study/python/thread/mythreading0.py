#-!/usr/bin/python
#coding:utf-8
import threading
from time import sleep, time
loops=[4,2]

def loop(nloop,nsec):
    print('start loop %s at: %s'%(nloop, time()))
    sleep(nsec)
    print('loop %s done at: %s'%(nloop,time()))



def main():
    print('starting at:', time())
    threads = []
    nloops = range(len(loops))

    for i in nloops:
      t = threading.Thread(target=loop, args=(i,loops[i]))
      threads.append(t)



    for i in nloops:
      threads[i].start()


    for i in nloops:
      # wait for all thread to stop
      threads[i].join()

    print('all DONE at:' ,time())


if __name__=='__main__':
   main()
