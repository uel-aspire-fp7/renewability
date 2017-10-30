CC=gcc

INCLUDES_PATHS=-I/opt/ASCL/include/ \
    -I$(SRCDIR)/client \
    -I$(SRCDIR)/server \

CFLAGS=-Wall \
	-g \
	-Os \
	-fpic

LIBRARIES=-lwebsockets -lmysqlclient -lz -lm -lpthread -ldl

all: renewability_manager.o nodes.o ascl.o
	$(CC) -o renewability_manager renewability_manager.o ascl.o nodes.o $(LIBRARIES)

ascl.o:
	cp $(ASCL)/obj/ascl.o .

%.o: %.c
	$(CC) $(INCLUDES_PATHS) $(CFLAGS) -c $<

clean:
	rm *.o *.log *.a -f

test: clean all
	clear
	./renewability_manager
