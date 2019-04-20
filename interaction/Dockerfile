FROM python:2.7-alpine

ENV TERM linux

run echo "travis_fold:start:DIapt\033[33;1minteractions Dockerfile pkgs\033[0m" && \
    apk update && apk add --no-cache git netcat-openbsd openssl-dev libffi-dev g++ linux-headers make bash && \
    echo "\ntravis_fold:end:DIapt\r"

COPY requirements.txt /requirements.txt
run echo "travis_fold:start:DIpip\033[33;1minteractions Dockerfile pip\033[0m" && \
    pip install -r requirements.txt && \
    echo "\ntravis_fold:end:DIpip\r"

COPY exploit1.py /exploit1.py
COPY exploit2.sh /exploit2.sh

COPY check1.py /check1.py
COPY check2.sh /check2.sh


