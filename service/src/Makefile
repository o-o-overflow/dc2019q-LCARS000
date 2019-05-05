.PHONEY: all clean

CFLAGS=-fPIC -Os -O2 -g -fno-stack-protector -I/usr/include/x86_64-linux-musl
LIBC=/usr/lib/x86_64-linux-musl/libc.a # apt-get install musl-dev
APPS=hello.bin echo.bin io.bin

all: mon $(APPS)

.intermediates: hello.elf echo.elf io.elf

clean:
	rm -rf *.o *.elf $(APPS)

mon: mon.o msg.o fs.o
	$(PREFIX)gcc -o $@ $^ -Wl,--Ttext-segment=0x100000000

%.elf: app.o %.o
	$(PREFIX)ld -T ld.x -o $@ $^ $(LIBC)
	# $(PREFIX)strip $@

%.o: %.c
	$(PREFIX)gcc -c $(CFLAGS) -o $@ $<

%.bin: %.elf
	$(PREFIX)objcopy $< -O binary -j .text -j .data $@
