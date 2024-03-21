cmake_minimum_required(VERSION 3.27)
project(sctp_poc
        DESCRIPTION "Minimal Working Example for SCTP client && server"
        LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

set(SOURCE_DIR "${PROJECT_SOURCE_DIR}")

set(commonDir
        "${SOURCE_DIR}/common"
)

add_subdirectory(client)
add_subdirectory(server)
