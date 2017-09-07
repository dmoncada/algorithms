CC      = gcc
PROG    = prog

# -Wall      Turns on all warnings about constructions.
# -Wextra    Turns on some extra warning missed by -Wall.
# -pedantic  Triggers all mandatory diagnostics listed in the C standard.
# -Werror    Converts warnings into errors.
# -std=c99   Uses the 1999 C standard.
# -Iinclude  Searches in ./include for headers with #include "file".
CFLAGS += -Wall -Wextra -pedantic -Werror -std=c99 -Iinclude

# Link the math library if we're compiling in Linux.
ifeq "$(shell uname)" "Linux"
	LDFLAGS += -lm
endif

# Produce debugging information.
ifeq "$(DEBUG)" "y"
	CFLAGS += -g
endif

vpath %.c src
vpath %.h include

OBJS = main.o

$(PROG): clean build_msg $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

.PHONY: build_msg clean tags

build_msg:
	@echo "make: building $(PROG)..."

clean:
	@echo "make: cleaning..."
	@rm -f *.o $(PROG) tags

tags:
	@echo "make: building tags..."
	@ctags -R

