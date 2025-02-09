CC=g++
IDIR =../include
CFLAGS=-I$(IDIR)

ODIR=obj
LDIR =../lib

LIBS=-lsfml-graphics -lsfml-window -lsfml-system

_DEPS = 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = test.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

testmake: test.o
	$(CC) test.cpp -o test $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~

