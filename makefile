CC = g++
IDIR = include
CFLAGS = -I$(IDIR) -Wall -Wextra

OBJDIR = obj
LIBDIR = lib

# link libraries
LIBS = -lGL -lGLEW -lsfml-graphics -lsfml-window -lsfml-system

# List source files
CPP_SRC = $(wildcard ./src/*.cpp)
C_SRC = ./src/glad.c
OBJ = $(patsubst ./src/%.cpp, $(OBJDIR)/%.o, $(CPP_SRC)) $(patsubst ./src/%.c, $(OBJDIR)/%.o, $(C_SRC))

# Ensure obj directory exists before compiling
$(OBJDIR)/%.o: ./src/%.cpp
	mkdir -p $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJDIR)/%.o: ./src/%.c
	mkdir -p $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

# generate dependency files
DEPS = $(OBJ:.o=.d)

$(OBJDIR)/%.d: ./src/%.cpp
	mkdir -p $(OBJDIR)
	$(CC) -MM -MT $(OBJDIR)/$*.o $(CFLAGS) $< > $(OBJDIR)/$*.d

$(OBJDIR)/%.d: ./src/%.c
	mkdir -p $(OBJDIR)
	$(CC) -MM -MT $(OBJDIR)/$*.o $(CFLAGS) $< > $(OBJDIR)/$*.d

# include the dependencies 
-include $(DEPS)

test: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o $(OBJDIR)/*.d test
 
