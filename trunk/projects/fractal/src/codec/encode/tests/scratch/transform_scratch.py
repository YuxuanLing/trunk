#!/usr/bin/env python

from numpy import *

t = matrix ([ (1, 1, 1, 1),
              (2, 1, -1, -2),
              (1, -1, -1, 1),
              (1, -2, 2, -1) ])

t_inv = matrix ([ (1,    1,  1,  0.5),
                  (1,  0.5, -1,   -1),
                  (1, -0.5, -1,    1),
                  (1,   -1,  1, -0.5) ])

def luma_transform_4x4 (block):
    """ Performs transformation according to 8.6.1.1.
    Returns the 4x4 coeffisient matrix. """

    return t * matrix (block) * t.T

def luma_inverse_transform_4x4 (block):
    """ Performs inverse transformation according to 8.5.10.
    Returns the reconstruced block. """
    return ((t_inv * matrix (block) * t_inv.T) + 32) >> 6 #/ (2**6)



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



def main ():

    # Warning, inverse transform does not work for negative numbers since
    input_blocks = [
        [(0, 0, 0, 0),
         (0, 0, 0, 0),
         (0, 0, 0, 0),
         (0, 0, 0, 0)],

        [(1, 1, 1, 1),
         (1, 1, 1, 1),
         (1, 1, 1, 1),
         (1, 1, 1, 1)],

        [(255, 255, 255, 255),
         (255, 255, 255, 255),
         (255, 255, 255, 255),
         (255, 255, 255, 255)],

        [(  0, 255,   0, 255),
         (255,   0, 255,   0),
         (  0, 255,   0, 255),
         (255,   0, 255,   0)],

        [( 55, 158,  85, 108),
         ( 19, 220, 112, 189),
         ( 50, 121, 218, 218),
         (202, 236, 139, 195)],

        [( 31, 201, 251, 130),
         (170, 211,   5, 129),
         (150,  14,  37, 249),
         (236,  18,  17, 129)],

        [( -7, -17, -15,  -3),
         ( -4, -17, -11,  -9),
         (-25, -25, -27, -27),
         (-15, -18,  -8, -14)]
        ]

    for a in input_blocks:
        b = luma_transform_4x4 (a)
        print "Input block:"
        print_matrix_c (matrix(a))
        print "Output coeff:"
        print_matrix_c (b)
        c = luma_inverse_transform_4x4 (b)
        print "Reconstructed block:"
        print_matrix_c (c)
        print ""


        tmp_coeff = matrix ([(-920,  -156,    20,     0),
                       (-26,   -32,    26,   -32),
                       (-40,    78,     0,     0),
                       (  0,    32,     0,     0)])
        tmp_recon = luma_inverse_transform_4x4 (tmp_coeff)

        print "Tmp coeff"
        print_matrix_c (tmp_coeff)
        print "tmp recon"
        print_matrix_c (tmp_recon)



if __name__ == "__main__":
    main ()
