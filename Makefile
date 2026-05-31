CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -mavx2 -msse4.1
LDFLAGS ?= -lm
INCLUDES := -Iinclude

SRC := src/main.c src/bases.c src/simd_utils.c src/conversions.c src/wav_core.c src/wav_dsp.c src/wav_compression.c
OBJ := $(SRC:.c=.o)
BIN := codage
INPUT ?= input.wav

.PHONY: all clean run

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

run: $(BIN)
	./$(BIN) $(INPUT)

clean:
	rm -f $(OBJ) $(BIN)
