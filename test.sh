#!/bin/sh

set -e
set -x

# quick a dirty extract of the name from the info.yml
SERVICE_NAME=$(cat info.yml | grep "^service_name:" | cut -f 2 -d '"')

SERVICE_IMAGE="$SERVICE_NAME-service"
INTERACTION_IMAGE="$SERVICE_NAME-interaction"
SERVICE_TAG=$(git rev-parse HEAD)

cd service && docker build . -t "$SERVICE_IMAGE:$SERVICE_TAG"  && cd -

cd interaction && docker build . -t "$INTERACTION_IMAGE:$SERVICE_TAG" && cd -

SERVICE_ID=$(docker run -d --rm "$SERVICE_IMAGE:$SERVICE_TAG")

# TODO: EXPLOIT_SCRIPTS=$(get_info.py exploit_scripts)
EXPLOIT_SCRIPTS="/exploit1.py /exploit2.sh"


# TODO: SLA_SCRIPTS=$(get_info.py sla_scripts)
#SLA_SCRIPTS="/check1.py /check2.sh"
SLA_SCRIPTS="/check2.sh"

# TODO: SERVICE_PORT=$(get_info.py service_port)
SERVICE_PORT=5000

# TODO: Set new flag before testing

IP=$(docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' "$SERVICE_ID")

for script in $EXPLOIT_SCRIPTS
do
	RESULT=$(docker run --rm "$INTERACTION_IMAGE:$SERVICE_TAG" "$script" "$IP" "$SERVICE_PORT")
	echo "$RESULT" | grep "FLAG:"
	echo "$RESULT" | grep "FLAG: TESTFLAG"	
done

for script in $SLA_SCRIPTS
do
	RESULT=$(docker run -it --rm "$INTERACTION_IMAGE:$SERVICE_TAG" "$script" "$IP" "$SERVICE_PORT")
done

docker kill "$SERVICE_ID"


