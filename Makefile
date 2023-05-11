CC = gcc
CFLAGS = -O3 -Wall -Wextra -Werror $(USER_CFLAGS)
INC = -I./metalang99/include/

.PHONY : all static shared clean

all : static shared

clean :
	find ./build/ -type f -delete
	touch ./build/shared/.gitkeep
	touch ./build/static/.gitkeep

static : build/static/libexCept.a

shared : build/shared/libexCept.so

build/static/libexCept.a : build/static_exCept.o
	ar -rsc $@ $<

build/static_exCept.o : exCept.c exCept.h
	$(CC) $(CFLAGS) $(INC) $< -c -o $@