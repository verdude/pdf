CC := cc
CFLAGS := -Wall

BUILDDIR = build
CLIENT = pdf
SRC = $(wildcard *.c)
OBJS = $(SRC:%.c=%.o)
OBJECTS = $(patsubst %,$(BUILDDIR)/%,$(OBJS))
EXE = $(BUILDDIR)/$(CLIENT)

$(EXE): build $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

.PHONY: clean test
clean:
	rm -rf $(BUILDDIR)
	rm test/tester.o test/tester

test: $(EXE)
	make -C test

