extra_libs = dl
gtk_cflags := $(shell pkg-config --cflags gtk+-3.0)
gtk_libs := $(shell pkg-config --libs gtk+-3.0)
obj_flags = -Wall -Wextra -g -c
link_flags = -fPIC -g

dir_build=build
dir_src=src
dir_include=include

OBJECTS=$(addprefix $(dir_build)/,data.o log.o main.o sprite.o)
HEADERS=$(addprefix $(dir_include)/,data.h log.h main.h sprite.h)

aout=$(dir_build)/remany

.PHONY : all
.PHONY : clean

all: $(SOURCES) $(aout)

$(aout): $(dir_build) $(OBJECTS)
	$(CC) $(OBJECTS) $(link_flags) $(CFLAGS) -o $(aout) $(addprefix -l,$(extra_libs)) $(gtk_libs)
	chmod 0755 $(aout)

$(OBJECTS): $(dir_build)/%.o: $(dir_src)/%.c $(HEADERS)
	$(CC) $< $(obj_flags) $(CFLAGS) -o $@ -I $(dir_include) $(gtk_cflags)

$(dir_build):
	mkdir $@

clean:
	rm -rf $(dir_build)

