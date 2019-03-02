from ubuntu:18.04
run apt-get update && apt-get install -y clang python3 xinetd

copy src/config.py /
copy src/service.c /service.c
copy service /service
#run clang -std=gnu99 -o /service /service.c \
#    && rm /service.c \
#    && apt-get -y purge clang \
#    && apt-get -y autoremove

run touch /flag && chmod 644 /flag && echo "OOO{this is a test flag}" > /flag
copy service.conf /service.conf
copy wrapper /wrapper

expose 5000
cmd ["/usr/sbin/xinetd", "-filelog", "-", "-dontfork", "-f", "/service.conf"]
