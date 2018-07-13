#!/bin/sh -e

RESULT=$(echo "adam" | nc "$1" "$2")

echo "$RESULT" | grep "awesome chall"
echo "$RESULT" | grep "who are you?"
echo "$RESULT" | grep "you said adam"

