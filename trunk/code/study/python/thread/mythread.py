#!/usr/bin/python
#coding:utf-8
import thread
from time import sleep, time

loops=[4,2]

def loop(nloop,nsec,lock):
    print('start loop %s at: %s'%(nloop, time()))
    sleep(nsec)
    print('loop %s done at: %s'%(nloop,time()))
    lock.release()

def main():
  print('start at:',time())
  locks = []
  nloops = range(len(loops))
 # 为所有的线程申请锁，之所以不在线程里申请锁因为1：想实现线程同步，所以要让所有的马同时冲出去
 # 2：获取锁需要一些时间，如果你的线程退出得“太快”，可能导致还没有获得锁，线程已经退出结束了
  for i in nloops:
    lock = thread.allocate_lock()
    lock.acquire()
    locks.append(lock)

  for i in nloops:
    thread.start_new_thread(loop, (i,loops[i], locks[i]))

 # 主线程一直循环直到所有的锁都释放为止
  for i in nloops:
    while locks[i].locked():pass

  print('all DONE at:',time())

if __name__=='__main__':
  main()
