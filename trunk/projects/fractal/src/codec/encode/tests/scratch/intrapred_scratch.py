#!/usr/bin/env python

from numpy import *
from random import randint, seed

block = matrix(zeros ((8,8), int))
pred = matrix(zeros ((8,8), int))

block16 = matrix(zeros ((16,16), int))
pred16 = matrix(zeros ((16,16), int))

def print_matrix_c (m):
    """ Print the given matrix in a c-code format """
    rows, cols = m.shape
    print "{",
    for i in range (rows):
        if i > 0:
            print " ",
        print "{",
        for j in range (cols):
            print "%5d" % m[i, j],
            if j < cols-1:
                print ",",
        print "}",
        if i < rows-1:
            print ","
    print "};"

def sad (m1, m2):
    sad = 0
    rows, cols = m1.shape
    for i in xrange (rows):
        for j in xrange (cols):
            sad = sad + abs (m1 [i,j] - m2 [i,j])
    return sad


def print_pblock_dc (dc1, dc2, dc3, dc4):
    pred[:4,:4] = dc1
    pred[:4,4:] = dc2
    pred[4:,:4] = dc3
    pred[4:,4:] = dc4

    print "-- Predicted block (dc = %d %d %d %d) --" % (dc1, dc2, dc3, dc4)
    print_matrix_c (pred)
    print "sad = %d" % sad (pred, block)


def print_pblock_vert (row):
    for j in xrange(8):
       pred[:, j] = row [j]
    print "-- Predicted block (vert) --"
    print_matrix_c (pred)
    print "sad = %d" % sad (pred, block)


def print_pblock_hor (col):
    for i in xrange(8):
       pred[i,:] = col [i]
    print "-- Predicted block (hor) --"
    print_matrix_c (pred)
    print "sad = %d" % sad (pred, block)



def print_pblock_dc_16 (dc):
    pred16[:16,:16] = dc

    print "-- Predicted block (dc = %d) --" % dc
    print_matrix_c (pred16)
    print "sad = %d" % sad (pred16, block16)


def print_pblock_vert_16 (row):
    for j in xrange(16):
       pred16[:, j] = row [j]
    print "-- Predicted block (vert) --"
    print_matrix_c (pred16)
    print "sad = %d" % sad (pred16, block16)


def print_pblock_hor_16 (col):
    for i in xrange(16):
       pred16[i,:] = col [i]
    print "-- Predicted block (hor) --"
    print_matrix_c (pred16)
    print "sad = %d" % sad (pred16, block16)


seed (1)
# Fill block with random data
for i in xrange (0, 8) :
    for j in xrange (0, 8):
         block[i, j] = randint (0, 256)

for i in xrange (0, 16) :
    for j in xrange (0, 16):
         block16[i, j] = i * 16 + j


# print "-- Input block --"
# print_matrix_c (block)

# # Assume DC values (set cols and rows accordingly in test case)
# print_pblock_dc (128, 128, 128, 128)
# print_pblock_dc (53, 6, 106, 56)
# print_pblock_dc (15, 114, 15, 114)
# print_pblock_dc (15, 15, 114, 114)

# print_pblock_vert ([0, 10, 50, 100, 110, 2, 4, 5])
# print_pblock_hor ([0, 10, 50, 100, 110, 2, 4, 5])


print "-- Input block --"
print_matrix_c (block16)

print_pblock_dc_16 (128)
print_pblock_dc_16 (59)
print_pblock_dc_16 (9)
print_pblock_dc_16 (109)

print_pblock_vert_16 ([0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150])
print_pblock_hor_16 ([0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150])
