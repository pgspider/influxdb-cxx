#! /bin/bash

HOME=$(pwd)

# InfluxDB 1.x systemtest config
export INFLUXDB_V1_SYSTEMTEST_HOST=127.0.0.1
export INFLUXDB_V1_SYSTEMTEST_PORT=18086
## for authentication server
export INFLUXDB_V1_SYSTEMTEST_HOST_AUTH=127.0.0.1
export INFLUXDB_V1_SYSTEMTEST_PORT_AUTH=28086
export INFLUXDB_V1_SYSTEMTEST_DATABASE=st_db
export INFLUXDB_V1_SYSTEMTEST_USER=root
export INFLUXDB_V1_SYSTEMTEST_PASS=root

# InfluxDB 2.x systemtest config
export INFLUXDB_V2_SYSTEMTEST_HOST=127.0.0.1
export INFLUXDB_V2_SYSTEMTEST_PORT=38086
export INFLUXDB_V2_SYSTEMTEST_DATABASE=st_db
export INFLUXDB_V2_SYSTEMTEST_TOKEN=mytoken
export INFLUXDB_V2_SYSTEMTEST_RP=autogen

# start docker
cd test/docker
./start.sh

# wait InfluxDB server start (may be need more time)
sleep 10

# create buket and database mapping for v2
TESTDB=$INFLUXDB_V2_SYSTEMTEST_DATABASE
TESTDB_ID=$(docker exec influx2server influx bucket create -n $TESTDB | grep $TESTDB | cut -f 1)
docker exec influx2server influx v1 dbrp create --bucket-id $TESTDB_ID --db $TESTDB -rp autogen --default

cd $HOME
# build
rm -rf build
mkdir build
cd build
cmake ..

# run test
make unittest
make systemtest

# clean up
cd test/docker
docker-compose down
