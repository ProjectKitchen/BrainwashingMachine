INCLUDEFLAGS=-I/opt/vc/include -I/opt/vc/include/interface/vmcs_host/linux -I/opt/vc/include/interface/vcos/pthreads -Iopenvg
LIBFLAGS=-L/opt/vc/lib -lbcm2835 -lasound -lm -lEGL -lGLESv2 -lbcm_host -lpthread -ljpeg

CC=gcc
CFLAGS=-Wall -std=gnu99 $(INCLUDEFLAGS) -DT_LINUX
DEPS = serial.h signal.h midi.h filters.h draw.h

OBJ = brainwash.o serial.o signals.o midi.o filters.o fidlib.o draw.o openvg/libshapes.o openvg/oglinit.o

all: brainwash

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< 

brainwash: $(OBJ)
	$(CC) -o $@ $^ $(INCLUDEFLAGS)  $(LIBFLAGS)

.PHONY: clean

clean:
	rm -f *.o brainwash 
	
run:	brainwash
	sudo ./brainwash
