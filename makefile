CC = gcc
CFLAGS = -c -Wall -std=c11
LDFLAGS =
SOURCES = main2.c vector2.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = vectorprog

all: $(SOURCES) $(EXECUTABLE)

# pull in dependency info for *existing* .o files
-include $(OBJECTS:.o=.d)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
	$(CC) -MM $< > $*.d

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE) *.d