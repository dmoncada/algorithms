CC     := gcc
OUT    := algs

SRCDIR := src
OBJDIR := obj

_OBJS  := $(subst .c,.o,$(wildcard $(SRCDIR)/*.c))
OBJS   := $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(_OBJS))

# -Wall      Turns on all warnings about constructions.
# -Wextra    Turns on some extra warning missed by -Wall.
# -pedantic  Triggers all mandatory diagnostics listed in the C standard.
# -Werror    Converts warnings into errors.
# -std=c99   Uses the 1999 C standard.
# -Iinclude  Searches in ./include for headers with #include "file".
CFLAGS += -Wall -Wextra -pedantic -Werror -std=c99 -Iinclude

# Link the math library if we're compiling in Linux.
ifeq '$(shell uname)' 'Linux'
	LDFLAGS += -lm
endif

# Produce debugging information.
ifneq '$(filter $(DEBUG),Y YES Yes y yes)' ''
	CFLAGS += -g
endif

.PHONY: all clean

all: $(OUT)

$(OUT): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@rm -fr $(OBJDIR) $(OUT)

print-%:
	@echo '$*=$($*)'
