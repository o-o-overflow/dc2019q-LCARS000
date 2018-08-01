FROM ubuntu:18.04
RUN apt-get update \
    && apt-get install -y \
        clang \
        python3 \
	    xinetd
RUN touch /flag && chmod 644 /flag && echo "TESTFLAG" > /flag
COPY service.conf /service.conf

COPY src/config.py /
COPY src/service.c /service.c
RUN clang -std=gnu99 -o /service /service.c \
    && rm /service.c \
    && apt-get -y purge clang \
    && apt-get -y autoremove

EXPOSE 5000
CMD ["/usr/sbin/xinetd", "-filelog", "-", "-dontfork", "-f", "/service.conf"]
