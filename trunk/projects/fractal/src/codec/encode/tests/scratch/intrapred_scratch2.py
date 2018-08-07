#!/usr/bin/env python

from numpy import *
from random import randint, seed


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
    return sum (abs(pred - block))
#     sad = 0
#     rows, cols = m1.shape
#     for i in xrange (rows):
#         for j in xrange (cols):
#             sad = sad + abs (m1 [i,j] - m2 [i,j])
#     return sad


# Assume DC values (set cols and rows accordingly in test case)

block = matrix (((0, 10, 20, 30),
                (40,  50,  60,  70),
                (80,  85,  90,  95),
                (100, 128, 200, 255)))


pred = matrix(zeros ((4,4), int))

pred[:,:] = 128
print_matrix_c (pred)
print "sad %d\n\n" % sad (block, pred)

pred[:,:] = 53
print_matrix_c (pred)
print "sad %d\n\n" % sad (block, pred)

pred[:,:] = 15
print_matrix_c (pred)
print "sad %d\n\n" % sad (block, pred)


pred[:,0] = 1
pred[:,1] = 2
pred[:,2] = 3
pred[:,3] = 4
print_matrix_c (pred)
print "sad %d\n\n" % sad (block, pred)

pred[:,0] = 10
pred[:,1] = 20
pred[:,2] = 25
pred[:,3] = 5
print_matrix_c (pred)
print "sad %d\n\n" % sad (block, pred)


pred[0,:] = 101
pred[1,:] = 102
pred[2,:] = 103
pred[3,:] = 105
print_matrix_c (pred)
print "sad %d\n\n" % sad (block, pred)


pred[0,:] = 10
pred[1,:] = 20
pred[2,:] = 25
pred[3,:] = 5
print_matrix_c (pred)
print "sad %d\n\n" % sad (block, pred)
