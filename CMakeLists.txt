cmake_minimum_required(VERSION 3.13)
project(raw)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Og -g -fno-rtti -fno-exceptions -Wall -Wno-unknown-pragmas -Wshadow -Wundef -Wwrite-strings -Wnon-virtual-dtor -Wno-multichar")

option(BYPASS_PROTECTION "Skip the code wheel screen" ON)

set(SYS_IMPLEMENTATION "SDL2" CACHE STRING "Which platform backend the game should use: 'SDL2', 'N64'")
set(GAME_TITLE "Out Of This World" CACHE STRING "Which game title to display: 'Out Of This World', 'Another World'")

add_executable(raw
        src/bank.cpp
        src/engine.cpp
        src/file.cpp
        src/main.cpp
        src/parts.cpp
        src/resource.cpp
        src/serializer.cpp
        src/sfxplayer.cpp
        src/staticres.cpp
        src/util.cpp
        src/video.cpp
        src/vm.cpp
)

target_compile_definitions(raw PRIVATE GAME_TITLE="${GAME_TITLE}")

if(BYPASS_PROTECTION)
	target_compile_definitions(raw PRIVATE BYPASS_PROTECTION)
endif()

if(SYS_IMPLEMENTATION MATCHES "SDL2")
	target_sources(raw PRIVATE
		src/mixerSDL.cpp
		src/sysSDL.cpp
	)
	find_package(SDL2 REQUIRED)
	include_directories(${SDL2_INCLUDE_DIRS})
	target_link_libraries(raw PRIVATE SDL2::Main)
	target_compile_definitions(raw PRIVATE AUTO_DETECT_PLATFORM)
elseif(SYS_IMPLEMENTATION MATCHES "N64")
	target_sources(raw PRIVATE
		src/mixerN64.cpp
		src/sysN64.cpp
	)
	set_target_properties(raw PROPERTIES SUFFIX ".elf")
else()
	message(FATAL_ERROR "Invalid SYS_IMPLEMENTATION selected")
endif()

target_link_libraries(raw PRIVATE z)
