# Docker-Based Packaging Steps

## Build Dockerfile
```shell script
sudo docker build -f Dockerfile -t asap-packaging .
```
The target image will be named as "asap-packaging".

## Run build_ASAP.sh in Docker
```shell script
sudo docker run -it -v "$(pwd)":/artifacts asap-packaging
```
A data volume should be attached to the docker image in order to retrieve
the built package.
