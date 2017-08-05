CC      = gcc
PROG    = prog

# -Wall      Turns on all warnings about constructions.
# -Wextra    Turns on some extra warning missed by -Wall.
# -pedantic  Trigger all mandatory diagnostics listed in the C standard.
# -Werror    Convert warnings into errors.
# -std=c99   Use the 1999 C standard + GNU extensions.
# -Iinclude  Search in ./include for headers with #include "file"
CFLAGS += -Wall -Wextra -pedantic -Werror -std=c99 -Iinclude

vpath %.c src
vpath %.h include

OBJS = main.o

$(PROG): build_msg $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

.PHONY: build_msg clean tags

build_msg:
	@echo "make: building $(PROG)..."

clean:
	@echo "make: cleaning..."
	@rm -f *.o $(PROG) tags

tags:
	@echo "make: building tags..."
	@ctags -R

