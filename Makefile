CC := cc
CFLAGS := -Wall

BUILDDIR = build
CLIENT = pdf
SRC = $(CLIENT:%=%.c)
OBJS = $(SRC:%.c=%.o)
OBJECTS = $(patsubst %,$(BUILDDIR)/%,$(OBJS))
EXE = $(BUILDDIR)/$(CLIENT)

$(EXE): build $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

.PHONY: clean
clean:
	rm -rf $(BUILDDIR)

