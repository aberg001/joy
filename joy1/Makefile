SHELL = /bin/sh

HEADER_NAMES = globals
SOURCE_NAMES = interp main scan utils
HEADER_FILES = $(addprefix src/, $(addsuffix .h, $(HEADER_NAMES)))
SOURCE_FILES = $(addprefix src/, $(addsuffix .c, $(SOURCE_NAMES)))
OBJECT_FILES = $(addprefix src/, $(addsuffix .o, $(SOURCE_NAMES)))

LDLIBS += -lm

_DEFAULT: build

all: clean build test

build: bin/joy

bin/joy: $(OBJECT_FILES)
	$(CC) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)

$(OBJECT_FILES): $(HEADER_FILES)

test: build

clean:
	rm -f $(OBJECT_FILES) bin/joy
