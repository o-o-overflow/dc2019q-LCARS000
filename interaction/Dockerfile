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
COPY exp1.uapp /exp1.uapp
COPY exploit2.py /exploit2.py
COPY flag1.papp /flag1.papp
COPY exploit3.py /exploit3.py
COPY exp3.papp /exp3.papp
