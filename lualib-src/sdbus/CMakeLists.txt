file(GLOB SRC_LIST "*.cpp" "*.c")

set(TARGET ldbus)

add_library(${TARGET} SHARED ${SRC_LIST})

set(ENV{PKG_CONFIG_PATH} /usr/lib/pkgconfig)
find_package(PkgConfig)
pkg_search_module(DBUS IMPORTED_TARGET dbus-1)

message(STATUS "found library:${DBUS_LIBRARIES}")

message(WARNING "555555")

target_link_libraries(${TARGET} PRIVATE PkgConfig::DBUS)

set_target_properties(${TARGET} PROPERTIES PREFIX "")

install(TARGETS ${TARGET} DESTINATION luaclib)