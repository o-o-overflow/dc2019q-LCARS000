FROM ubuntu:18.04
RUN apt-get update && apt-get install -y \
    python3 \
	xinetd
RUN touch /flag && chmod 644 /flag && echo "TESTFLAG" > /flag
COPY service.conf /service.conf

COPY src/template.py /template
RUN chmod 755 /template

EXPOSE 5000
CMD ["/usr/sbin/xinetd", "-filelog", "-", "-dontfork", "-f", "/service.conf"]
