CC = g++
IDIR = ../include
CFLAGS = -I$(IDIR) -Wall -Wextra

OBJDIR = obj
LIBDIR = ../lib

# link libraries
LIBS = -lGL -lGLEW -lsfml-graphics -lsfml-window -lsfml-system

# List source files
SRC = $(wildcard *.cpp) ./glad.c
OBJ = $(patsubst %.cpp, $(OBJDIR)/%.o, $(SRC))

# Ensure obj directory exists before compiling
$(OBJDIR)/%.o: %.cpp
	mkdir -p $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

# generate dependency files
$(OBJDIR)/%d: %.cpp
	$(CC) -MM -MT $(OBJDIR)/$*.o $(CFLAGS) $< > $(OBJDIR)/$*.d

# include the dependencies 
-include $(OBJDIR)/*.d

test: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o $(OBJDIR)/*.d test
 
