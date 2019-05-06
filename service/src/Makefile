.PHONEY: all clean

CFLAGS=-fPIC -Os -g -O2 -fno-stack-protector -I/usr/include/x86_64-linux-musl
CFLAGS+=-Wno-multichar
LIBC=/usr/lib/x86_64-linux-musl/libc.a # apt-get install musl-dev
LIBWOLFSSL=/usr/lib/x86_64-linux-gnu/libwolfssl.a # apt-get install libwolfssl-dev
SERVICES=hello.sys echo.sys init.sys loader.sys crypto.sys
APPLETS=hello.app untrusted.app

all: mon $(SERVICES) $(APPLETS)

.intermediates: hello.elf echo.elf init.elf loader.elf crypto.elf hello.raw untrusted.raw

clean:
	rm -rf *.o *.elf $(SERVICES) $(APPLETS)

mon: mon.o msg.o fs.o
	$(PREFIX)gcc -o $@ $^ -Wl,--Ttext-segment=0x100000000

%.elf: app.o %.o
	$(PREFIX)ld -T papp.x -o $@ $^ $(LIBWOLFSSL) $(LIBC)
	# $(PREFIX)strip $@

%.o: %.c
	$(PREFIX)gcc -c $(CFLAGS) -o $@ $<

%.sys: %.elf
	$(PREFIX)objcopy $< -O binary -j .text -j .data $@

%.app: app.o %.o
	$(PREFIX)ld -T uapp.x -o $*.raw $^ $(LIBC)
	$(PREFIX)objcopy $*.raw -O binary -j .text -j .data $*.raw
	python app.py $* $*.raw $@
