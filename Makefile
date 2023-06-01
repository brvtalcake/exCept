CC = gcc
CFLAGS = -O3 -Wall -Wextra -Werror $(USER_CFLAGS)
INC = -I./metalang99/include/ -I./chaos-pp/ 

.PHONY : all static shared clean test

all : static shared test

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

build/shared/libexCept.so : build/shared_exCept.o
	$(CC) $(CFLAGS) $(INC) -shared -fPIC $< -o $@

build/shared_exCept.o : exCept.c exCept.h
	$(CC) $(CFLAGS) $(INC) -fPIC $< -c -o $@

test : build/static/libexCept.a test.c
	$(CC) $(CFLAGS) -Wno-error=clobbered $(INC) test.c -o build/test -lexCept -L./build/static/
	./build/test