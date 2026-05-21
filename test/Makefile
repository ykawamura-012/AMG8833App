TARGET = amg8833_test

CC ?= gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS =

SRCS = main.c amg8833.c heatmap.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)