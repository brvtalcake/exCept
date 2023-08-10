CC = gcc
CFLAGS = -Wall -Wextra -Werror $(USER_CFLAGS)

ifndef DEBUG
CFLAGS += -O3 -flto
else
CFLAGS += -ggdb3 -O0
endif

INC ?= -I./metalang99/include/ -I./chaos-pp/ -I./ 

TEST_FILES = $(wildcard tests/*.c)
TESTS = $(TEST_FILES:tests/%.c=build/%)

.PHONY : all static shared clean test demo

all : static shared test demo

clean :
	@find ./build/ -type f -printf 'Deleting %h/%f\n' -delete
	@touch ./build/shared/.gitkeep
	@touch ./build/static/.gitkeep

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

build/demo : build/static/libexCept.a demo.c
	$(CC) $(CFLAGS) -Wno-error=clobbered $(INC) demo.c -o $@ -lexCept -L./build/static/

build/% : tests/%.c build/static/libexCept.a
	$(CC) $(CFLAGS) $(INC) $< -o $@ -lexCept -L./build/static/

test : build/static/libexCept.a $(TESTS)
	@for test in $(TESTS); do \
		echo "Running $$test"; \
		$$test; \
	done

demo : build/static/libexCept.a build/demo
	./build/demo