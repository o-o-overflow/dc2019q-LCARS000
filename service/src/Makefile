.PHONEY: all clean

CFLAGS=-fPIC -Os -O2 -g -fno-stack-protector
LIBC=/usr/lib/x86_64-linux-musl/libc.a # apt-get install musl-dev
APPS=hello

all: mon $(APPS)

clean:
	rm -rf *.o *.elf $(APP)

mon: mon.o
	$(PREFIX)gcc -o $@ $^ -Wl,--Ttext-segment=0x100000000

hello: hello.elf
	$(PREFIX)objcopy $@.elf -O binary -j .text -j .data $@

%.elf: app.o %.o
	$(PREFIX)ld -T ld.x -o $@ $^ $(LIBC)
	# $(PREFIX)strip $@

%.o: %.c
	$(PREFIX)gcc -c $(CFLAGS) -o $@ $<
