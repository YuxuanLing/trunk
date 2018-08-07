#! /usr/bin/python

import sys
#this module used for summing time log
#information in log is lines
#every line looks method=name gputime=9.5 cputime=8.3
class Info:
	def __init__(self):
		self.method=""
		self.gputime=-1.0
		self.cputime=-1.0
		self.occupancy=-1.0

	def parseString(self, line, lineNo):
		line=line.strip('\n').strip(" ")
		if(line.startswith("#")):
			self = None
			return self
		line = line.replace("[ ", "")
#		print(line)
		line = line.replace(" ]", "")
#		print(line)
		l = line.split(" ")
#		print(l)
		for item in l:
			if(item.startswith("method=")):
				self.method = item.split("=")[1]
			elif(item.startswith("gputime=")):
				self.gputime = float(item.split("=")[1])
			elif(item.startswith("cputime=")):
				self.cputime = float(item.split("=")[1])
			elif(item.startswith("occupancy=")):
				self.occupancy = float(item.split("=")[1])
			else:
			 	print(item+" in line "+str(lineNo)+" is not regular format")
				self = None
				return self

		return self

	def toString(self):
		ret="method=[ "+self.method+" ] "+"gputime=[ "+str(self.gputime)+" ] "+"cputime=[ "+str(self.cputime)+" ]"
		if(-1.0 != self.occupancy):
			ret += " "+"occupancy=[ "+str(self.occupancy)+" ]"

		return ret

class KernelTime:
	def __init__(self, name, totalTime):
		self.name=name
		self.totalTime=totalTime
		self.callTimes=1
		self.minTime=totalTime
		self.maxTime=totalTime
		
	def add(self, kt):
		if(kt.name != self.name):
			print(kt.name +" is not equal to "+ self.name)
		else:
			self.totalTime += kt.totalTime
			self.callTimes += kt.callTimes
			self.minTime = min(self.minTime, kt.minTime)
			self.maxTime = max(self.maxTime, kt.maxTime)

def parseFile(filename):
	fp = open(filename, "r")

	l = []
	lineNo = 0
	for line in fp:
		lineNo = lineNo + 1
		info = Info()
		info = info.parseString(line, lineNo)
		if(info is None):
#			print("I am none")
			continue
		else:			
			l.append(info)

	fp.close()

	return l
#	for item in l:		
#		print(item.toString())		 

def formatOutput(l):
	print("........................................................................................................................................")	
	print("%12s\t\t%12s\t\t%12s\t\t%12s\t\t%12s\t\t%12s"%("%", "avg time", "total time", "calls", "min time", "max time"))	
	print("........................................................................................................................................")	
	
	total = 0.0
	for item in l:
		total = total + item.totalTime

	if(total > 0.0):
		for item in l:
			print("%s:\n%10.2f\t\t %10.2f\t\t %10.2f\t\t%10d\t\t %10.2f\t\t %10.2f"%(item.name, 100*item.totalTime/total, item.totalTime/item.callTimes, item.totalTime, item.callTimes, item.minTime, item.maxTime))

def analyse(infoList):
	cpuTimeMap=dict()
	gpuTimeMap=dict()

	for info in infoList:
#		print(info.toString())
		if(-1.0 != info.cputime):
			ckt = KernelTime(info.method, info.cputime)
#			if(ckt is None):
#				pirnt("ckt is none")
			if(cpuTimeMap.has_key(info.method)):
				cpuTimeMap[info.method].add(ckt)
			else:
				cpuTimeMap[info.method] = ckt

		if(-1.0 != info.gputime):
			gkt = KernelTime(info.method, info.gputime)
			if(gpuTimeMap.has_key(info.method)):
				gpuTimeMap[info.method].add(gkt)
			else:
				gpuTimeMap[info.method] = gkt

	if(len(cpuTimeMap) != 0):			
		l = cpuTimeMap.values()
		l = sorted(l, reverse=True, key=lambda x:x.totalTime)
		formatOutput(l)

	if(len(gpuTimeMap) != 0):
		l = gpuTimeMap.values()
		l = sorted(l, reverse=True, key=lambda x:x.totalTime)
		formatOutput(l)

def help(argv):
	print("usage %s filename\n"%(argv[0]))
	exit(0)

def main(argv):
	if(len(argv) != 2):
		help(argv)	
	else:
		l = parseFile(argv[1])	
		analyse(l)

main(sys.argv)

