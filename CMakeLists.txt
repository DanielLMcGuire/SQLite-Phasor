cmake_minimum_required(VERSION 3.10)
project(SQLite-Phasor VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include phasor)

add_library(sqlite-phs SHARED sqlite-phs.cpp sqlite/sqlite3.c)

if(WIN32)
    set(PLUGIN_INSTALL_DIR "plugins")
elseif(APPLE)
    set(CMAKE_INSTALL_PREFIX "/Library/Application Support/org.Phasor.Phasor" CACHE PATH "Install prefix" FORCE)
    set(PLUGIN_INSTALL_DIR "plugins")
elseif(UNIX)
    if(NOT CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(PLUGIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/plugins")
    else()
        set(CMAKE_INSTALL_PREFIX "/opt/Phasor" CACHE PATH "Install prefix" FORCE)
        set(PLUGIN_INSTALL_DIR "plugins")
    endif()
endif()

install(TARGETS sqlite-phs
        LIBRARY DESTINATION "${PLUGIN_INSTALL_DIR}"
        RUNTIME DESTINATION "${PLUGIN_INSTALL_DIR}"
)
