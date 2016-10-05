CMAKE_MINIMUM_REQUIRED(VERSION 3.6.0)

include_directories( ./include )

file(GLOB C_SRC
	src/*.c
	src/netbuf/*.c)

add_executable(helloworld ${C_SRC})

