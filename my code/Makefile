CC=g++

CFLAGS = -std=c++11 -g -O0 #-Wall -Wextra -Werror

DEPS = anim.cpp
OBJ = anim.o 
SLIBS = 
LDFLAGS = 
LDLIBS = -lstdc++ -lGL -lglut -lGLEW

all: anim

%.o: %.c* $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $< 

anim: $(OBJ) $(SLIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(OBJ) anim *~ [#]*[#] .\#*
