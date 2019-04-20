# DEFCON 2019 Quals Template

[![Build Status](https://travis-ci.com/o-o-overflow/dc2019q-template.svg?token=6XM5nywRvLrMFwxAsXj3&branch=master)](https://travis-ci.com/o-o-overflow/dc2019q-template)

Use this chall as template (for the info.yaml, flag, deployment dir, etc.)

The service-manager script will build the service container and test it.
The way this works is:

## Writing the service

Your service will be built from `service/Dockerfile` dockerfile.
The contents of that dockerfile, and the security of the resulting container, are 100% up to you.
**NOTHING will be changed about it for the final game deployment: it will be used and deployed as-is.**
There are no expectations, except for:

1. Your service should `EXPOSE` its ports (using the `EXPOSE` dockerfile directive).
   Port deduplication across services can be handled on the host, so just using the port 31337 in the template is fine.
   If you have a commandline service, wrap it in xinetd.
   This template wraps a commandline service in xinetd, listening on port 31337.
   To expose UDP, use `EXPOSE 31337/udp`.
2. Your service should automatically run when `docker run -d your-service` is performed.
   The template does this by launching xinetd.

## Specifying public files.

Public files are specified in the yaml, relative to the git repo root. They will be exposed with individual links, but internally bundled into a `public_bundle.tar.gz`.

## Writing the exploit and test scripts

Your test scripts will be launched through a docker image built from the `interactions/Dockerfile` dockerfile.
This dockerfile should set up the necessary environment to run interactions.
Every script specified in your yaml's `interactions` list will be run.
If a filename starts with the word `exploit`, then it is assumed to be an exploit that prints out the flag (anywhere in stdout), and its output is checked for the flag.

The interaction image is instantiated as a persistent container in which the tests are run.
Your test scripts should *not* be its `entrypoint` or `cmd` directives.
An approximation of the test procedure is, roughly:

```
INTERACTION_CONTAINER=$(docker run --rm -i -d $INTERACTION_IMAGE)
docker exec $INTERACTION_CONTAINER /exploit1.py $SERVICE_IP $SERVICE_PORT) | grep OOO{blahblah}
docker exec $INTERACTION_CONTAINER /exploit2.sh $SERVICE_IP $SERVICE_PORT) | grep OOO{blahblah}
docker exec $INTERACTION_CONTAINER /check1.py $SERVICE_IP $SERVICE_PORT)
docker exec $INTERACTION_CONTAINER /check2.sh $SERVICE_IP $SERVICE_PORT)
docker kill $INTERACTION_CONTAINER
```

The service manager will stress-test your service by launching multiple concurrent interactions.
It'll also stress-test it by launching multiple connections that terminate immediately, and making sure your service cleanly exits in this scenario!

## Building

You can build and test your service with the service manager:

```
# do everything
./tester

#
# or, piecemeal:
#

# build the service and public bundle:
./tester build

# launch and test the service (interaction tests, stress tests, shortreads)
./tester test

# just launch the service (will print network endpoint info)
./tester launch

# just test the service
./tester launch $IP_ADDRESS $PORT
```

The following artifacts result from the build process:

1. a `dc2019q:your_service` image is created, containing your service
1. a `dc2019q:your_service-interactions` image is created, with your interaction scripts
1. `public_bundle.tar.gz` has the public files, extracted from the service image.
