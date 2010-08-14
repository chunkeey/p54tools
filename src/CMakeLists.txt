cmake_minimum_required(VERSION 2.8)

project(tools)

set(tools checksum view edit)

foreach(tool ${tools})
	add_executable( ${tool} ${tool}.c )
	add_definitions("-std=gnu99 -Wall -Wextra -Wshadow -Wstrict-overflow=5 -Wunsafe-loop-optimizations -Wpointer-arith -Wcast-qual -Wcast-align")
	target_link_libraries( ${tool} p54eeprom )
endforeach()
