CC := gcc
LIB_FLAGS := -shared -fPIC

all: loader.c injected.c
	make -C injector
	$(CC) -I./injector/include/ -o loader loader.c ./injector/src/linux/libinjector.a
	$(CC) $(LIB_FLAGS) -o injected.so injected.c

clean:
	rm loader injected.so
	make -C injector clean

.PHONY: clean