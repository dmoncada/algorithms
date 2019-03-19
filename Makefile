# The Makefile used for building the algorithms library.
# Author: David Moncada

CC      ?= clang
OUT     := algs

SRC_DIR := src
OBJ_DIR := obj

SRC     := $(wildcard $(SRC_DIR)/*.c)
OBJ     := $(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/%,$(SRC:.c=.o))

# -Wall      Turns on all warnings about constructions.
# -Wextra    Turns on some extra warning missed by -Wall.
# -pedantic  Triggers all mandatory diagnostics listed in the C standard.
# -Werror    Converts warnings into errors.
# -std=c99   Uses the 1999 C standard.
# -Iinclude  Searches in ./include for headers with #include "file".
CFLAGS  += -Wall -Wextra -pedantic -Werror -std=c99 -Iinclude

# Link the math library if we're compiling in Linux.
ifeq '$(shell uname)' 'Linux'
	LDFLAGS += -lm
endif

# Produce debugging information.
ifneq '$(filter $(DEBUG),Y YES Yes y yes)' ''
	CFLAGS += -g
endif

.PHONY: all clean help tags

# #######
# Targets
# #######

all: tags $(OUT)

clean:
	@rm -fr $(OBJ_DIR) $(OUT) tags

help:
	@echo 'Targets:'
	@echo ''
	@echo ' all      - Builds the app and all targets marked with [*].'
	@echo ' clean    - Removes all generated files.'
	@echo ' help     - Show this help message.'
	@echo ' *tags    - Builds tags for vim.'
	@echo ' print-%  - Prints the value of variable %.'

tags:
	@find include -type f -and -iname '*.h' | xargs ctags
	@find $(SRC_DIR) -type f -and -iname '*.c' | xargs ctags -a

$(OUT): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

print-%:
	@echo '$*=$($*)'
