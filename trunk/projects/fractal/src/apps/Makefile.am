noinst_PROGRAMS = encode decode packetloss autotune
noinst_LTLIBRARIES = libannexb.la libsnr.la

common_cflags = \
	-std=c99 -DNDEBUG \
	-I $(top_srcdir)/src/codec \
	-I $(top_srcdir)/libs/my_getopt-1.5

encode_CFLAGS = \
	$(common_cflags) \
	-I $(srcdir)/common/snr \
	-I $(top_srcdir)/src/codec/encode \
	-I $(top_srcdir)/src/codec/common

decode_CFLAGS = \
	$(common_cflags) \
	-I $(srcdir)/common/annexb \
	-I $(top_srcdir)/src/codec/decode \
	-I $(top_srcdir)/src/codec/common

packetloss_CFLAGS = \
	$(common_cflags) \
	-I $(srcdir)/common/annexb

autotune_CFLAGS = \
	$(common_cflags) \
	-I $(srcdir)/common/snr \
	-I $(top_srcdir)/src/codec/encode \
	-I $(top_srcdir)/src/codec/common

if COMPILER_ICC
autotune_CFLAGS += -openmp -openmp_report1
else
autotune_CFLAGS += -fopenmp
endif

if COMPILER_ICC
autotune_LDFLAGS = -openmp
else
autotune_LDFLAGS = -fopenmp
endif

libannexb_la_SOURCES = common/annexb/annexb.c
libsnr_la_SOURCES = common/snr/snr.c
encode_SOURCES = \
	encoder/encode.c \
	encoder/ssim.c
decode_SOURCES = decoder/decode.c
packetloss_SOURCES = packetloss/main.c
autotune_SOURCES = autotune/autotune.c

encode_LDADD = \
	$(top_builddir)/src/codec/libfractalcodec.la \
	$(top_builddir)/libs/my_getopt-1.5/libmygetopt.la \
	libsnr.la

decode_LDADD = \
	$(top_builddir)/src/codec/libfractalcodec.la \
	$(top_builddir)/libs/my_getopt-1.5/libmygetopt.la \
	libannexb.la

packetloss_LDADD = \
	$(top_builddir)/libs/my_getopt-1.5/libmygetopt.la \
	libannexb.la

autotune_LDADD = \
	$(top_builddir)/src/codec/libfractalcodec.la \
	libsnr.la
