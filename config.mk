PREFIX = /usr/local

CPPFLAGS = -D_POSIX_C_SOURCE
CFLAGS = -O2 -std=c99 -Wall -pedantic $(CPPFLAGS)
LDFLAGS = -s -static

CC = cc
