#!/usr/bin/env python

from numpy import *

def print_matrix_c (m):
    """ Print the given matrix in a c-code format """
    rows, cols = m.shape
    print "{",
    for i in range (rows):
        if i > 0:
            print " ",
        print "{",
        for j in range (cols):
            print "%3d," % m[i, j],
#             if j < cols-1:
#                 print ",",
        print "}",
        if i < rows-1:
            print ","
    print "};"


hor = matrix(zeros ((32,32), int))
vert = matrix(zeros ((32,32), int))

val = 0
for i in xrange(16):
    for j in xrange(16):
        hor_val = val
        vert_val = 255 - val
        val += 1

        hor  [i*2 : i*2+2, j*2 : j*2+2] = hor_val
        vert [i*2 : i*2+2, j*2 : j*2+2] = vert_val

print_matrix_c (hor)
print_matrix_c (vert)
