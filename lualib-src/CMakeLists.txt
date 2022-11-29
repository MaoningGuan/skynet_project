file(GLOB SRC_LIST "*.cpp" "*.c")

set(TARGET demo)

ADD_EXECUTABLE(${TARGET} ${SRC_LIST})

install(TARGETS ${TARGET} DESTINATION luaclib)

ADD_SUBDIRECTORY(./sdbus)