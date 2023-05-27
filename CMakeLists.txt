cmake_minimum_required(VERSION 3.22)

project(Log VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions(-DLOG_LIBRARY)
find_package(Qt6 REQUIRED COMPONENTS
    Core
)

qt_add_library(Log ${LIB_COMPILE_MODE})


target_link_libraries(Log PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

#file(GLOB SRC_FILES
#    "*.cpp"
#)
#file(GLOB H_FILES
#    "*.h"
#)

target_sources(Log PRIVATE
    log.cpp
)
#INSTALL(TARGETS Log
#        LIBRARY DESTINATION ${CMAKE_BINARY_DIR}/output/libpath
#        PUBLIC_HEADER DESTINATION ${CMAKE_BINARY_DIR}/output/includepath
#)
