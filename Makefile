CC = gcc

TARGET = a65

CFLAGS = -g -std=c89 -Wall -Wvla -O0 -MMD

OBJECTS =    a65.o \
			 a65eval.o \
			 a65util.o

DEPENDS = $(OBJECTS:.o=.d)

all: $(TARGET)

clean:
	rm *.o *.d

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

-include $(DEPENDS)
