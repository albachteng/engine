CC = g++
IDIR = ../include
CFLAGS = -I$(IDIR)

OBJDIR = obj
LIBDIR = ../lib

LIBS = -lsfml-graphics -lsfml-window -lsfml-system

# List source files
SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp, $(OBJDIR)/%.o, $(SRC))

# Ensure obj directory exists before compiling
$(OBJDIR)/%.o: %.cpp
	mkdir -p $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

test: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o test
 
