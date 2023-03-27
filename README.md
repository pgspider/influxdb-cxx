# influxdb-cxx

[![CI](https://github.com/offa/influxdb-cxx/workflows/ci/badge.svg)](https://github.com/offa/influxdb-cxx/actions)
[![GitHub release](https://img.shields.io/github/release/offa/influxdb-cxx.svg)](https://github.com/offa/influxdb-cxx/releases)
[![License](https://img.shields.io/badge/license-MIT-yellow.svg)](LICENSE)
![C++](https://img.shields.io/badge/c++-17-green.svg)


InfluxDB C++ client library
 - Support both InfluxDB 1.x and InfluxDB 2.x (An InfluxDB 2.x bucket must be mapped to a database and retention policy (DBRP) before it can be queried using InfluxQL or write data by influxdb-cxx):
   - Batch write
   - Data exploration
 - Support InfluxDB 1.x with transports
   - HTTP/HTTPS with Basic Auth
   - UDP
   - Unix datagram socket
 - Support InfluxDB 2.x with transports
   - HTTP/HTTPS with Token Auth


## Installation

 __Build requirements__
 - CMake 3.12+
 - C++17 compiler

__Dependencies__
 - CURL (required)
 - boost 1.57+ (optional – see [Transports](#transports))

### Generic
 ```bash
mkdir build && cd build
cmake ..
sudo make install
 ```

### Cmake build options
|Name                   |Description                          |Default value|
|:----------------------|:------------------------------------|:-----------:|
|BUILD_SHARED_LIBS      |Build shared versions of libraries   |           ON|
|INFLUXCXX_WITH_BOOST   |Build with Boost support enabled     |           ON|
|INFLUXCXX_TESTING      |Enable testing for this component    |           ON|
|INFLUXCXX_SYSTEMTEST   |Enable system tests                  |           ON|
|INFLUXCXX_COVERAGE     |Enable Coverage                      |          OFF|

For example: disable Boost library and disable testing:
 ```bash
mkdir build && cd build
cmake .. -DINFLUXCXX_WITH_BOOST=OFF -DINFLUXCXX_TESTING=OFF
sudo make install
 ```

## Quick start

### Include in CMake project

The InfluxDB library is exported as target `InfluxData::InfluxDB`.

```cmake
project(example)

find_package(InfluxDB)

add_executable(example-influx main.cpp)
target_link_libraries(example-influx PRIVATE InfluxData::InfluxDB)
```

This target is also provided when the project is included as a subdirectory.

```cmake
project(example)
add_subdirectory(influxdb-cxx)
add_executable(example-influx main.cpp)
target_link_libraries(example-influx PRIVATE InfluxData::InfluxDB)
```

### Basic write

```cpp
// Provide complete URI
// For InfluxDB 1.x
auto influxdb = influxdb::InfluxDBFactory::GetV1("http://localhost", 8086, "test");
// For InfluxDB 2.x
auto influxdb = influxdb::InfluxDBFactory::GetV2("http://localhost", 8086, "test", "auth_token");

influxdb->write(influxdb::Point{"test"}
  .addField("value", 10)
  .addTag("host", "localhost")
);
```

### Batch write

```cpp
// For InfluxDB 1.x
auto influxdb = influxdb::InfluxDBFactory::GetV1("http://localhost", 8086, "test");
// For InfluxDB 2.x
auto influxdb = influxdb::InfluxDBFactory::GetV2("http://localhost", 8086, "test", "auth_token");

// Write batches of 100 points
influxdb->batchOf(100);

for (;;) {
  influxdb->write(influxdb::Point{"test"}.addField("value", 10));
}
```

###### Note:

When batch write is enabled, call `flushBatch()` to flush pending batches.
This is of particular importance to ensure all points are written prior to destruction.

```cpp
// For InfluxDB 1.x
auto influxdb = influxdb::InfluxDBFactory::GetV1("http://localhost", 8086, "test");
// For InfluxDB 2.x
auto influxdb = influxdb::InfluxDBFactory::GetV2("http://localhost", 8086, "test", "auth_token");

influxdb->batchOf(3);
influxdb->write(influxdb::Point{"test"}.addField("value", 1));
influxdb->write(influxdb::Point{"test"}.addField("value", 2));

// Flush batches, both points are written
influxdb->flushBatch();
```


### Query

```cpp
// Available over HTTP only
// For InfluxDB 1.x
auto influxdb = influxdb::InfluxDBFactory::GetV1("http://localhost", 8086, "test");
// For InfluxDB 2.x
auto influxdb = influxdb::InfluxDBFactory::GetV2("http://localhost", 8086, "test", "auth_token");

/// Pass an IFQL to get list of points
std::vector<influxdb::InfluxDBTable> result = idb->query("SELECT * FROM test");
/// Pass an IFQL and param
influxdb::InfluxDBParams params = influxdb::InfluxDBParams{}.addParam("param1", 1);
std::vector<influxdb::InfluxDBTable> result = idb->query("SELECT * FROM test WHERE c1 = $param1", params);
```

### Authentication
```cpp
// Available over HTTP/HTTPs only
// For InfluxDB 1.x: basic authentication
auto influxdb = influxdb::InfluxDBFactory::GetV1("http://localhost", 8086, "test", "username", "password");
// For InfluxDB 2.x: token authentication and retention policy
auto influxdb = influxdb::InfluxDBFactory::GetV2("http://localhost", 8086, "test", "auth_token", "my_-_rp");
```

## Transports

List of InfluxDB 1.x supported transport is following:

| Name        | Dependency  | URI protocol   | Sample                                     |
| ----------- |:-----------:|:--------------:| ------------------------------------------:|
| HTTP        | cURL        | `http`/`https` | `GetV1("http://localhost", 8086, "db")`    |
| UDP         | boost       | `udp`          | `GetV1("http://localhost", 8094)`          |
| Unix socket | boost       | `unix`         | `GetV1("unix:///tmp/telegraf.sock")`       |

List of InfluxDB 2.x supported transport is following:
| Name        | Dependency  | URI protocol   | Sample                                     |
| ----------- |:-----------:|:--------------:| ------------------------------------------:|
| HTTP        | cURL        | `http`/`https` | `GetV2("http://localhost", 8086, "db")`    |
