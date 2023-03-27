
find_package(Catch2 REQUIRED)
find_package(trompeloeil REQUIRED)

if( NOT TARGET trompeloeil::trompeloeil )
    add_library(trompeloeil::trompeloeil INTERFACE IMPORTED)
    target_link_libraries(trompeloeil::trompeloeil INTERFACE trompeloeil)
endif()


add_library(TestMain STATIC TestMain.cxx)
target_link_libraries(TestMain PUBLIC Catch2::Catch2 trompeloeil::trompeloeil)

add_subdirectory("mock")


function(add_unittest name)
    add_executable(${name} ${name}.cxx)
    target_link_libraries(${name} PRIVATE TestMain InfluxDB)
    target_include_directories(${name}
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/mock
            )
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_unittest(PointTest)
target_compile_options(PointTest PRIVATE $<$<NOT:$<BOOL:${MSVC}>>:-Wno-deprecated-declarations>)

add_unittest(LineProtocolTest)
target_link_libraries(LineProtocolTest PRIVATE InfluxDB-Internal)

add_unittest(InfluxDBTest)
add_unittest(InfluxDBFactoryTest)

add_unittest(HttpTest)
target_link_libraries(HttpTest PRIVATE InfluxDB-Http CurlMock json)
target_sources(HttpTest PRIVATE ${PROJECT_SOURCE_DIR}/src/ConnectionInfo.cxx ${PROJECT_SOURCE_DIR}/src/Query.cxx)

add_unittest(NoBoostSupportTest)
target_sources(NoBoostSupportTest PRIVATE ${PROJECT_SOURCE_DIR}/src/NoBoostSupport.cxx)
target_sources(NoBoostSupportTest PRIVATE ${PROJECT_SOURCE_DIR}/src/ConnectionInfo.cxx)

add_unittest(QueryTest)
target_link_libraries(QueryTest PRIVATE json)
target_sources(QueryTest PRIVATE ${PROJECT_SOURCE_DIR}/src/Query.cxx)

add_unittest(InfluxDBParamsTest)
target_link_libraries(InfluxDBParamsTest PRIVATE json)
target_sources(InfluxDBParamsTest PRIVATE ${PROJECT_SOURCE_DIR}/src/InfluxDBParams.cxx)

if (INFLUXCXX_WITH_BOOST)
    add_unittest(BoostSupportTest)
    target_link_libraries(BoostSupportTest PRIVATE InfluxDB-BoostSupport Threads::Threads Boost::system date)
    target_sources(BoostSupportTest PRIVATE ${PROJECT_SOURCE_DIR}/src/ConnectionInfo.cxx)
endif()


add_custom_target(unittest PointTest
    COMMAND LineProtocolTest
    COMMAND InfluxDBTest
    COMMAND InfluxDBFactoryTest
    COMMAND HttpTest
    COMMAND NoBoostSupportTest
    COMMAND QueryTest
    COMMAND InfluxDBParamsTest
    COMMAND $<$<BOOL:${INFLUXCXX_WITH_BOOST}>:BoostSupportTest>

    COMMENT "Running unit tests\n\n"
    VERBATIM
    )


if (INFLUXCXX_WITH_BOOST)
    add_dependencies(unittest BoostSupportTest)
endif()


if (INFLUXCXX_SYSTEMTEST)
    add_subdirectory(system)
endif()
