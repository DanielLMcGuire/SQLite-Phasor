cmake_minimum_required(VERSION 3.10)
project(SQLite-Phasor C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include phasor)

add_library(sqlite-phs SHARED sqlite-phs.cpp sqlite/sqlite3.c)

if(WIN32)
    set(PLUGIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/plugins")
elseif(APPLE)
    set(PLUGIN_INSTALL_DIR "/Library/Application Support/org.Phasor.Phasor/plugins")
elseif(UNIX)
    set(PLUGIN_INSTALL_DIR "/opt/Phasor/plugins")
endif()

install(TARGETS sqlite-phs
        LIBRARY DESTINATION "${PLUGIN_INSTALL_DIR}"
        RUNTIME DESTINATION "${PLUGIN_INSTALL_DIR}"
)