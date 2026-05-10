add_library(fcl_reflect INTERFACE)
target_include_directories(fcl_reflect INTERFACE ${FCL_PUBLIC_INCLUDE_DIRS})
target_link_libraries(fcl_reflect INTERFACE Boost::headers)
