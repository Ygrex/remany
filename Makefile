gtk_cflags := $(shell pkg-config --cflags gtk+-3.0)
gtk_libs := $(shell pkg-config --libs gtk+-3.0)
obj_flags = -Wall -Wextra -g -c
link_flags = -fPIC -g

aout=build/remany

remany: main.o
	$(CC) build/*.o $(link_flags) -o $(aout) -ldl $(gtk_libs)
	chmod 0755 $(aout)

main.o: build src/main.c
	$(CC) -c src/main.c $(obj_flags) -o build/main.o -I include $(gtk_cflags)

build:
	mkdir build

