#! /usr/bin/python
import sys

verbose = False

class KernelCode:
	def __init__(self):
		self.name = ""
		self.lineNos = []
		self.codes = []

	def append(self, code, lineNo):
		self.codes.append(code)
		self.lineNos.append(lineNo)

	def clean(self):
		self.name=""
		self.lineNos = []
		self.codes=[]

def getAllKernelCodes(lines):
	header = "function"
	tail = "................"

	kernelStart = False
	kernelEnd = False

	l = []
	kc = KernelCode() 
	lineNo = 0

	for line in lines:
		line = line.strip('\n').lower().strip()
		lineNo = lineNo + 1
		if(0 == len(line)):
			continue
		if(line.startswith(header)):
			kernelStart = True
			kernelEnd = False
#			kc.clean()
			kc.name=line.split(":")[1]
			continue
		elif(line.startswith(tail)):			
			kernelEnd = True
			kernelStart = False
			l.append(kc)
			kc = KernelCode()
			continue

		if(kernelStart and (not kernelEnd)):
			kc.append(line, lineNo)

	return l

def getInstFromLine(line):
	inst = line.strip('\n').split(';')[0]
	inst = inst.split('/')
	if(len(inst) == 1):
		return None
	elif(2 == len(inst)):
		print(inst[1])
#	print(inst[0])
	return inst[2].strip()

def countFlopsInLine(line, lineNo):
	global verbose

	inst = getInstFromLine(line)
	if(inst is None):
		return 0
#	print(inst)
	count = 0
	inst_exe=''
	if(inst.startswith('@')):
		inst_exe = inst.split()[1]
	else:
	 	inst_exe = inst

	if (inst_exe.startswith('ffma')):
		count = 2
		if(verbose):
			print("%d %s"%(lineNo, inst))
	elif (inst_exe.startswith('f')):
		count = 1
		if(verbose):
			print("%d %s"%(lineNo, inst))

	return count


def countFlopsInFile(filename):
	count = 0
	lineNo = 0
	fp = open(filename, "r")
	lines = fp.readlines()
	fp.close()

	kernelCodes = getAllKernelCodes(lines)
	for kernel in kernelCodes:
#		print(kernel.name)
		count = 0
		for i in range(0, len(kernel.codes)):
			count = count + countFlopsInLine(kernel.codes[i], kernel.lineNos[i])

		print(kernel.name+" has "+str(count) +" FLop")


def main(argv):
	global verbose

	if(len(argv) == 3):
		verbose = True
	countFlopsInFile(argv[1])

main(sys.argv)	
