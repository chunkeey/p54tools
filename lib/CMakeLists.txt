cmake_minimum_required(VERSION 2.8)

project(libp54eeprom)

set(p54eeprom_src main.c)

add_definitions("-std=gnu99 -Wall -Wextra -Wshadow -Wstrict-overflow=5 -Wunsafe-loop-optimizations -Wpointer-arith -Wcast-qual -Wcast-align")

add_library( p54eeprom STATIC ${p54eeprom_src})
