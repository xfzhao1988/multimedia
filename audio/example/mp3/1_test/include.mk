CC = gcc
RM = rm -f
AR = ar
CP = cp
MAKE = make
HEADER_PATH = $(ROOT)/include/
LIB_PATH = $(ROOT)/lib/
BIN_PATH = $(ROOT)/bin/
SRC_PATH = $(ROOT)/src
CCFLAGS = -I$(HEADER_PATH) -Wall -Werror -Wno-error=restrict
LIBFLAGS = -L$(LIB_PATH)
