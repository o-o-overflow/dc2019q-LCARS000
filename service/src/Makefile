.PHONEY: all clean clean-all release

CFLAGS=-fPIC -Os -g -O2 -fno-stack-protector -I/usr/include/x86_64-linux-musl
CFLAGS+=-Wno-multichar
LIBC=/usr/lib/x86_64-linux-musl/libc.a # apt-get install musl-dev
LIBWOLFSSL=/usr/lib/x86_64-linux-gnu/libwolfssl.a # apt-get install libwolfssl-dev
SERVICES=echo.sys init.sys loader.sys crypto.sys
APPLETS=svc.papp perm.uapp perm.papp perm.sapp flag1.papp exp1.uapp exp3.papp exp4.app
KEYS=system.pub system.priv platform.pub platform.priv
CERTS=certs.h certs-test.h
FLAGS=flag1.h

all: $(FLAGS) $(CERTS) mon $(SERVICES) $(APPLETS)

.intermediates: $(KEYS) \
	echo.sys.elf init.sys.elf loader.sys.elf crypto.sys.elf \
	svc.app.elf perm.app.elf flag1.app.elf \
	svc.app perm.app flag1.app

clean:
	rm -rf *.o *.elf *.sys *.app *.bpf $(SERVICES) $(APPLETS)

clean-all: clean
	rm $(KEYS) $(CERTS)

release: all
	cp mon echo.sys crypto.sys loader.sys init.sys ../public
	cp exp1.uapp flag1.papp exp3.papp exp4.app ../../interaction

mon: mon.o msg.o fs.o scmp.o
	$(PREFIX)gcc -o $@.elf $^ -Wl,--Ttext-segment=0x100000000
	cp $@.elf $@
	$(PREFIX)strip $@

%.sys.elf: app.o %.o scmp.o
	$(PREFIX)ld -T sys.x -o $@ $^ $(LIBC)

%.sys: %.sys.elf
	$(PREFIX)objcopy $< -O binary -j .text -j .data $@

%.app.elf: app.o %.o scmp.o
	$(PREFIX)ld -T app.x -o $@ $^ $(LIBC)

%.app: %.app.elf
	$(PREFIX)objcopy $< -O binary -j .text -j .data $@

crypto.sys.elf: app.o crypto.o gnu-c.o certs.h
	$(PREFIX)ld -T sys.x -o $@ app.o crypto.o $(LIBWOLFSSL) gnu-c.o scmp.o $(LIBC) \
		/usr/lib/gcc/x86_64-linux-gnu/7/libgcc.a \
		--ignore-unresolved-symbol __strcat_chk \
		--ignore-unresolved-symbol __snprintf_chk \
		--ignore-unresolved-symbol __strncat_chk \
		--ignore-unresolved-symbol __strncpy_chk \
		--ignore-unresolved-symbol __fprintf_chk \
		--error-unresolved-symbols

%.o: %.c
	$(PREFIX)gcc -c $(CFLAGS) -o $@ $<

scmp.o: scmp.c policy.h
	$(PREFIX)gcc -c $(CFLAGS) -o $@ $<

%.sapp: %.app app.py
	python app.py $* $< 0 > $@

%.papp: %.app app.py
	python app.py $* $< 1 > $@

%.uapp: %.app app.py
	python app.py $* $< 2 > $@

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
	rm test_system
	xxd -i test_platform >> $@
	rm test_platform
	sed -i 's/unsigned/const unsigned/' $@

flag%.h: flag%.txt
	xxd -i $< > $@
	sed -i 's/unsigned/const unsigned/' $@

policy.h: policy.py
	python policy.py
	xxd -i untrusted.bpf > $@
	xxd -i platform.bpf >> $@
	xxd -i system.bpf >> $@
	sed -i 's/unsigned/const unsigned/' $@
