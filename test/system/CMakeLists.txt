add_executable(InfluxDBST InfluxDBST.cxx)
target_link_libraries(InfluxDBST PRIVATE InfluxDB InfluxDB-Http CURL::libcurl TestMain json)
target_include_directories(InfluxDBST PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_sources(InfluxDBST PRIVATE ${PROJECT_SOURCE_DIR}/src/ConnectionInfo.cxx ${PROJECT_SOURCE_DIR}/src/Query.cxx)

add_custom_target(systemtest InfluxDBST
        COMMENT "Running system tests\n\n"
        VERBATIM
        )
