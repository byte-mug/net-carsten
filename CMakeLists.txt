#
#   Copyright 2016 Simon Schmidt
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

CMAKE_MINIMUM_REQUIRED(VERSION 3.6.0)
include (TestBigEndian)

TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
	add_definitions(-D__HOST_ENDIAN_BIG)
endif()

add_definitions(-D__HAS_BSWAP__)

include_directories( ./include )

file(GLOB APP_SRC
	# App.
	src/app/*.c
	)

file(GLOB API_SRC
	# App.
	src/apiposix/*.c
	)

file(GLOB NET_SRC
	# Network stack
	src/netarp/*.c
	src/neticmp/*.c
	src/netif/*.c
	src/netipv4/*.c
	src/netipv6/*.c
	src/netpkt/*.c
	src/netprot/*.c
	)

add_executable(helloworld ${APP_SRC} ${API_SRC} ${NET_SRC})
target_link_libraries(helloworld pthread)
