CC := cc
CFLAGS := -Wall -g3 -rdynamic -lz
ZIGFLAGS := -lc -lz

BUILDDIR = build
CLIENT = pdf
ZIGCLIENT = zig-pdf
SRC = $(wildcard *.c)
OBJS = $(SRC:%.c=%.o)
OBJECTS = $(patsubst %,$(BUILDDIR)/%,$(OBJS))
EXE = $(BUILDDIR)/$(CLIENT)

$(EXE): build $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

.PHONY: clean test zig
clean:
	rm -rf $(BUILDDIR)
	rm -f test/tester.o test/tester
	rm -f vgcore.*

test: $(EXE)
	make -C test

$(BUILDDIR)/$(ZIGCLIENT): build
	zig build-exe $(ZIGFLAGS) *.c *.h -femit-bin=$(BUILDDIR)/$(ZIGCLIENT)
