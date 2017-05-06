################################################################################
# DESCRIPTION:  GNU Makefile for Video Decoder Firmware
# # AUTHOR:       Hongjie Guan
# # DATE:         09/26/2007
###################################################################################
WA_ROOT ?= $(shell while [ `pwd` != / -a ! -f wa.root  ];do cd ..;done;pwd -P)
MKCOMM_DIR ?= $(WA_ROOT)/coding/make/comm

SRC_DIRS   += $(WA_ROOT)/coding/dbg
SRC_DIRS   += $(WA_ROOT)/coding/xmem

SRC        += xmem.c

OUT         = lib_xmem.a

#------------------------------------------------------------------------------
# Project-specific rules
#------------------------------------------------------------------------------

.PHONY: all
all: default_all

.PHONY: clean
clean: default_clean

################################################################################
include $(MKCOMM_DIR)/xrules.mkinc
################################################################################
# End of Makefile
################################################################################



