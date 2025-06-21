CC = g++
IDIR = include
CFLAGS = -I$(IDIR) -Wall -Wextra

OBJDIR = obj
LIBDIR = lib

# link libraries
LIBS = -lGL -lGLEW -lsfml-graphics -lsfml-window -lsfml-system
TEST_LIBS = -lgtest -lgtest_main -pthread
TEST_CFLAGS = -DGTEST_HAS_PTHREAD=1

# List source files
CPP_SRC = $(wildcard ./src/*.cpp)
C_SRC = ./src/glad.c
OBJ = $(patsubst ./src/%.cpp, $(OBJDIR)/%.o, $(CPP_SRC)) $(patsubst ./src/%.c, $(OBJDIR)/%.o, $(C_SRC))

# Test source files
TEST_SRC = $(wildcard ./tests/*.cpp)
TEST_OBJ = $(patsubst ./tests/%.cpp, $(OBJDIR)/test_%.o, $(TEST_SRC))

# Library object files (exclude main/test.cpp for linking with tests)
LIB_SRC = $(filter-out ./src/test.cpp, $(CPP_SRC))
LIB_OBJ = $(patsubst ./src/%.cpp, $(OBJDIR)/%.o, $(LIB_SRC)) $(patsubst ./src/%.c, $(OBJDIR)/%.o, $(C_SRC))

# Ensure obj directory exists before compiling
$(OBJDIR)/%.o: ./src/%.cpp
	mkdir -p $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJDIR)/%.o: ./src/%.c
	mkdir -p $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

# Test compilation rules
$(OBJDIR)/test_%.o: ./tests/%.cpp
	mkdir -p $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS) $(TEST_CFLAGS)

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

# Test target
run_tests: $(LIB_OBJ) $(TEST_OBJ)
	$(CC) -o run_tests $^ $(CFLAGS) $(LIBS) $(TEST_LIBS)

.PHONY: clean tests
clean:
	rm -f $(OBJDIR)/*.o $(OBJDIR)/*.d test run_tests

tests: run_tests
	./run_tests
 
