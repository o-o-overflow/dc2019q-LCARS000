from ubuntu:18.04
run apt-get -qq update && apt-get install -qq clang python3 xinetd
# python3-pip, etc.

copy src/config.py /
copy src/service.c /service.c
copy service /service
# Currently anything that goes in public files must be pre-built and checked in git
# Can still build other things locally though
#run clang -std=gnu99 -o /service /service.c \
#    && rm /service.c \
#    && apt-get -y purge clang \
#    && apt-get -y autoremove

# Or, include the flag file in the repository
run touch /flag && chmod 644 /flag && echo "OOO{this is a test flag}" > /flag

copy service.conf /service.conf
copy wrapper /wrapper

expose 5000
cmd ["/usr/sbin/xinetd", "-filelog", "-", "-dontfork", "-f", "/service.conf"]
