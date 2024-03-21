set(This server_bin)
set(Sources server.cpp)

message(STATUS  "PROJECT ROOT DIR \t=>\t${SOURCE_DIR}")
message(STATUS  "COMMON DIR \t\t=>\t${commonDir}")

add_executable(${This} ${Sources})
target_include_directories(${This} PRIVATE ${commonDir})
target_link_libraries(${This} pthread sctp)