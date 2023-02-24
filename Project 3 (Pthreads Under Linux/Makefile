CC = g++
CFLAGS = -lpthread -lglut -lGLU -lGL -lm

all: factory

factory: factory.o
	$(CC) factory.o  -o factory $(CFLAGS)

factory.o: factory.cpp
	$(CC) -c factory.cpp $(CFLAGS)

run: factory
	./factory arg.txt

clean:
	rm -f *.o factory