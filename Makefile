CFILES=$(wildcard *.c)
CFLAGS=-DFIRMWARE_BUILD -DPHYS_ADDR_BASE=0xa0000000ull -DPHYS_ADDR_SIZE=0x80000000ull -DPAGE_SIZE=4096ull
ASMFILES=$(wildcard *.S)
OBJECTS=$(patsubst %.c, %.o, $(CFILES)) $(patsubst %.S, %.o, $(ASMFILES))
EXECUTABLE=remarkable-memtest

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
