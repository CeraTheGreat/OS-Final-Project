CC = gcc
CT = ctags

CFLAGS = -O2
DFLAGS = -g -Wall
LFLAGS = -lutil 

PROG_NAME = main
PT1_DIR = ./part1
PT2_DIR = ./part2
PT3_DIR = ./part3

SRC_DIR = ./src
BIN_DIR = ./bin

SRC1_LIST = $(wildcard $(PT1_DIR)/*.c)
HED1_LIST = $(wildcard $(PT1_DIR)/*.h)

SRC2_LIST = $(wildcard $(PT2_DIR)/*.c)
HED2_LIST = $(wildcard $(PT2_DIR)/*.h)

SRC3_LIST = $(wildcard $(PT3_DIR)/*.c)
HED3_LIST = $(wildcard $(PT3_DIR)/*.h)

.PHONY: all part1 part2 part3 clean $(PROG_NAME) debug development

default: all

all: part1 part2 part3

part1: $(SRC1_LIST) $(HED1_LIST)
	$(CC) $(DFLAGS) -o $(PT1_DIR)/$(PROG_NAME) $(SRC1_LIST) $(LFLAGS)
	etags -o $(PT1_DIR)/.tags $(SRC1_LIST) $(HED1_LIST)

part2: $(SRC2_LIST) $(HED2_LIST)
	$(CC) $(DFLAGS) -o $(PT2_DIR)/$(PROG_NAME) $(SRC2_LIST) $(LFLAGS)
	etags -o $(PT2_DIR)/.tags $(SRC2_LIST) $(HED2_LIST)

part3: $(SRC3_LIST) $(HED3_LIST)
	$(CC) $(DFLAGS) -o $(PT3_DIR)/$(PROG_NAME) $(SRC3_LIST) $(LFLAGS)
	etags -o $(PT3_DIR)/.tags $(SRC3_LIST) $(HED3_LIST)

clean:
	rm -f $(PT1_DIR)/$(PROG_NAME)
	rm $(PT1_DIR)/.tags
	rm -f $(PT2_DIR)/$(PROG_NAME)
	rm $(PT2_DIR)/.tags
	rm -f $(PT3_DIR)/$(PROG_NAME)
	rm $(PT3_DIR)/.tags

