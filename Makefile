TARGET = fajlowy
CC = gcc
SRC = graph1.c main1.c pliki.c split.c utils.c validation.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
