CFILES=$(wildcard *.c)
ASMFILES=$(wildcard *.S)
OBJECTS=$(patsubst %.c, %.o, $(CFILES)) $(patsubst %.S, %.o, $(ASMFILES))
EXECUTABLE=remarkable-memtest

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

firmware: $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) -DFIRMWARE_BUILD -DPHYS_ADDR_SIZE=0x80000000

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
