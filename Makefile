TARGET = grafmaster
CC = gcc
SRC = graph1.c main1.c pliki.c split.c utils.c validation.c
OBJS = $(SRC:.c=.o)
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC)  -o $(TARGET) $(OBJS)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
