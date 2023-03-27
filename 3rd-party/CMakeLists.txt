add_library(date INTERFACE)
target_include_directories(date INTERFACE "${CMAKE_CURRENT_LIST_DIR}/date")
add_library(json INTERFACE)
target_include_directories(json INTERFACE "${CMAKE_CURRENT_LIST_DIR}/json")
