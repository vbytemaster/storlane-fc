add_library(fcl_raw INTERFACE)
target_include_directories(fcl_raw INTERFACE ${FCL_PUBLIC_INCLUDE_DIRS})
target_link_libraries(fcl_raw INTERFACE Boost::headers)
