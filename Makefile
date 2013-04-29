COMPILER = gcc


GCFLAGS = -O2 -std=gnu99 
GCFLAGS += -Wno-strict-aliasing -Wstrict-prototypes -Wundef -Wall -Wextra  
GCFLAGS += -fstrict-aliasing -funsigned-char
GCFLAGS += -lSDLmain -lSDL -lSDL_image 

UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
	GCFLAGS+= -framework Cocoa -framework OpenGL -L/usr/X11/lib/
endif

ifeq ($(UNAME), Linux)
	GCFLAGS+= -lGL -lGLU
endif


all: ship

clean:
	rm ship

ship: main.c Makefile 
	$(COMPILER) $(GCFLAGS) main.c -o ship

.PHONY : clean all 

