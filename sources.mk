SRC_DIR = .
INCLUDE_DIR = .

#Add(12, 215, 215) your source files here
SRCS = $(wildcard $(SRC_DIR)/*.c)

INCLUDE = \
-I$(INCLUDE_DIR)
