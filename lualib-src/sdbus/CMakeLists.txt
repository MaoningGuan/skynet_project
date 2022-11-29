file(GLOB SRC_LIST "*.cpp" "*.c")

set(TARGET sdbus)

add_library(${TARGET} SHARED ${SRC_LIST})

set_target_properties(${TARGET} PROPERTIES PREFIX "")

install(TARGETS ${TARGET} DESTINATION luaclib)