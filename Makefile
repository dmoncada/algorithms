CC      = gcc
PROG    = prog
CFLAGS += -Wall -Wextra -pedantic -Werror -std=c99 -I include

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

