from ubuntu:18.04
run echo "travis_fold:start:Dapt\033[33;1mservice Dockerfile apt\033[0m" && \
    apt-get -qq update && apt-get install -qq clang python3 xinetd && \
    echo "\ntravis_fold:end:Dapt\r"
# python3-pip, etc.

run apt-get install -qq gdb strace

copy public/LCARS /LCARS
copy public/init.sys /init.sys
copy public/loader.sys /loader.sys
copy public/crypto.sys /crypto.sys
copy public/echo.sys /echo.sys
# Currently anything that goes in public files must be pre-built and checked in git
# Can still build other things locally though
#run clang -std=gnu99 -o /service /service.c \
#    && rm /service.c \
#    && apt-get -y purge clang \
#    && apt-get -y autoremove

# Or, include the flag file in the repository
copy src/root.key /root.key
copy src/flag22.txt /flag22.txt
copy src/flag333.txt /flag333.txt
copy src/flag4.txt /flag4.txt

copy service.conf /service.conf
copy banner_fail /
copy wrapper /wrapper

expose 5000
cmd ["/usr/sbin/xinetd", "-filelog", "-", "-dontfork", "-f", "/service.conf"]
