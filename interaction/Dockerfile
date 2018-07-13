FROM python:2.7-alpine

RUN apk update && apk add --no-cache git netcat-openbsd openssl-dev libffi-dev g++ linux-headers make bash
COPY requirements.txt /requirements.txt
RUN pip install -r requirements.txt
COPY exploit1.py /exploit1.py
COPY exploit2.sh /exploit2.sh

COPY check1.py /check1.py
COPY check2.sh /check2.sh


