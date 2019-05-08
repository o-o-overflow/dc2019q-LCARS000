.PHONEY: all clean

CFLAGS=-fPIC -Os -g -O2 -fno-stack-protector -I/usr/include/x86_64-linux-musl
CFLAGS+=-Wno-multichar
LIBC=/usr/lib/x86_64-linux-musl/libc.a # apt-get install musl-dev
LIBWOLFSSL=/usr/lib/x86_64-linux-gnu/libwolfssl.a # apt-get install libwolfssl-dev
SERVICES=echo.sys init.sys loader.sys crypto.sys
APPLETS=test.app untrusted.app system.app platform.app
KEYS=system.pub system.priv platform.pub platform.priv
CERTS=certs.h certs-test.h

all: $(CERTS) mon $(SERVICES) $(APPLETS)

.intermediates: test.elf echo.elf init.elf loader.elf crypto.elf test.raw untrusted.raw $(KEYS)

clean:
	rm -rf *.o *.elf *.raw $(SERVICES) $(APPLETS)

clean-all: clean
	rm $(KEYS) $(CERTS)

mon: mon.o msg.o fs.o
	$(PREFIX)gcc -o $@ $^ -Wl,--Ttext-segment=0x100000000

%.elf: app.o %.o
	$(PREFIX)ld -T papp.x -o $@ $^ $(LIBC)
	# $(PREFIX)strip $@

crypto.elf: app.o crypto.o gnu-c.o certs.h
	$(PREFIX)ld -T papp.x -o $@ app.o crypto.o $(LIBWOLFSSL) gnu-c.o $(LIBC) \
		/usr/lib/gcc/x86_64-linux-gnu/7/libgcc.a \
		--ignore-unresolved-symbol __strcat_chk \
		--ignore-unresolved-symbol __snprintf_chk \
		--ignore-unresolved-symbol __strncat_chk \
		--ignore-unresolved-symbol __strncpy_chk \
		--ignore-unresolved-symbol __fprintf_chk \
		--error-unresolved-symbols

%.o: %.c
	$(PREFIX)gcc -c $(CFLAGS) -o $@ $<

%.sys: %.elf
	$(PREFIX)objcopy $< -O binary -j .text -j .data $@

%.raw: app.o %.o
	$(PREFIX)ld -T uapp.x -o $@.elf $^ $(LIBC)
	$(PREFIX)objcopy $@.elf -O binary -j .text -j .data $@

%.app: %.raw app.py
	python app.py $* $< 2 > $@

system.app: untrusted.raw app.py
	python app.py $@ $< 0 > $@

platform.app: untrusted.raw app.py
	python app.py $@ $< 1 > $@

%.priv:
	openssl genrsa -out $@ 2048

%.pub:	%.priv
	openssl rsa -in $< -out $@ -outform der -pubout

certs.h: system.pub platform.pub
	xxd -i system.pub > $@
	xxd -i platform.pub >> $@
	sed -i 's/unsigned/const unsigned/' $@

certs-test.h: app.py
	python app.py 0 system_app > test_system
	python app.py 1 platform_app > test_platform
	xxd -i test_system > $@
	xxd -i test_platform >> $@
	sed -i 's/unsigned/const unsigned/' $@
