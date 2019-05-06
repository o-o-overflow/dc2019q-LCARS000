.PHONEY: all clean

CFLAGS=-fPIC -Os -O2 -fno-stack-protector -I/usr/include/x86_64-linux-musl
CFLAGS+=-Wno-multichar
LIBC=/usr/lib/x86_64-linux-musl/libc.a # apt-get install musl-dev
LIBWOLFSSL=/usr/lib/x86_64-linux-gnu/libwolfssl.a # apt-get install libwolfssl-dev
PLATFORM_APPS=hello.bin echo.bin init.bin loader.bin crypto.bin
UNTRUSTED_APPS=hello.app

all: mon $(PLATFORM_APPS) $(UNTRUSTED_APPS)

.intermediates: hello.elf echo.elf init.elf loader.elf crypto.elf hello.raw

clean:
	rm -rf *.o *.elf $(PLATFORM_APPS) $(UNTRUSTED_APPS)

mon: mon.o msg.o fs.o
	$(PREFIX)gcc -o $@ $^ -Wl,--Ttext-segment=0x100000000

%.elf: app.o %.o
	$(PREFIX)ld -T papp.x -o $@ $^ $(LIBWOLFSSL) $(LIBC)
	# $(PREFIX)strip $@

%.o: %.c
	$(PREFIX)gcc -c $(CFLAGS) -o $@ $<

%.bin: %.elf
	$(PREFIX)objcopy $< -O binary -j .text -j .data $@

%.app: app.o %.o
	$(PREFIX)ld -T uapp.x -o $*.raw $^ $(LIBC)
	$(PREFIX)objcopy $*.raw -O binary -j .text -j .data $*.raw
	python app.py $* $*.raw $@
