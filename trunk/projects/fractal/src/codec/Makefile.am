lib_LTLIBRARIES = libfractalcodec.la
noinst_LTLIBRARIES = libfractalcodecssse3.la

iccoptions = -fargument-noalias -unroll-aggressive -D NDEBUG -fPIC -DPIC
if COMPILER_ICC
iccoptions += -fp-model fast=2 -ansi-alias
endif

noinst_HEADERS =

common_cflags = \
		$(iccoptions) \
		-I $(top_srcdir)/src/codec/common \
		-I $(top_srcdir)/src/codec/encode \
		-I $(top_srcdir)/src/codec/decode

libfractalcodec_la_CFLAGS = $(common_cflags)

sse3flags =
ssse3flags = -DHAVE_SSSE3
if COMPILER_ICC
sse3flags += -axAVX,SSE3
ssse3flags += -axAVX,SSSE3
else
sse3flags += -msse3
ssse3flags += -mssse3

# XXX: hw: hack to compile using gcc
sse3flags += -mssse3
endif

# SSE3 is implicit on OS X on x86-64 when using the Intel compiler.
if ENV_DARWIN      # Platform
if ! HAVE_X86_64
libfractalcodec_la_CFLAGS += $(sse3flags)
endif
else               # Platform
libfractalcodec_la_CFLAGS += $(sse3flags)
endif              # Platforms

libfractalcodecssse3_la_CFLAGS = $(common_cflags) $(sse3flags) $(ssse3flags)

libfractalcodec_la_LDFLAGS = -static $(TETRIS_LIBRARY_LDFLAGS)

libfractalcodec_la_LIBADD = libfractalcodecssse3.la

libfractalcodecssse3_la_SOURCES = \
	encode/enc_init_mb.c \
	encode/enc_mb.c \
	common/com_deblock.c

libfractalcodec_la_SOURCES = \
	encode/enc.c \
	encode/enc_control.c \
	encode/enc_dpb.c \
	encode/enc_gop.c \
	encode/enc_headers.c \
	encode/enc_init_mb.c \
	encode/enc_interpolate.c \
	encode/enc_load_mb.c \
	encode/enc_mb.c \
	encode/enc_motionsearch.c \
	encode/enc_putvlc.c \
	encode/enc_ratecontrol.c \
	encode/enc_sei.c \
	encode/enc_writebits.c \
	encode/taah264enc.c \
	decode/dec.c \
	decode/dec_dpb.c \
	decode/dec_getvlc.c \
	decode/dec_headers.c \
	decode/dec_intrapred.c \
	decode/dec_quant.c \
	decode/dec_readbits.c \
	decode/dec_sei.c \
	decode/dec_transform.c \
	decode/taah264dec.c \
	common/com_cpuinfo.c \
	common/com_deblock.c \
	common/com_dpb.c \
	common/com_error.c \
	common/com_interpolation.c \
	common/com_interpolation_ref.c \
	common/com_prediction.c
