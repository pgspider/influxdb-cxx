#! /bin/bash

# start InfluxDB server on docker
docker-compose down
docker-compose up --force-recreate -V -d
