cmake_minimum_required(VERSION 2.6)

include(../tools/arch.cmake)
include(../tools/os.cmake)
include(../tools/subdirs.cmake)

project(fibgit)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
	set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
	set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

file(GLOB_RECURSE src_list "test/*.c*")
add_library(fibgit ${src_list})

set(PROJECT_ROOT_DIR ${PROJECT_SOURCE_DIR}/../)
set(BIN_DIR ${PROJECT_ROOT_DIR}/bin/${OS}_${ARCH}_${BUILD_TYPE})
set(LIBRARY_OUTPUT_PATH ${BIN_DIR})

include(CheckIncludeFiles)
include(CheckCSourceCompiles)

set(CMAKE_C_FLAGS "${BUILD_OPTION}")

check_include_files(iconv.h HAVE_ICONV_H)
check_c_source_compiles("void posix_spawnp();
	__asm__(\".symver posix_spawnp,posix_spawnp@GLIBC_2.2.5\");
	void main(void){posix_spawnp();}" HAVE_GLIB_C_225_H)

check_c_source_compiles("void posix_spawnp();
	__asm__(\".symver posix_spawnp,posix_spawnp@GLIBC_2.2\");
	void main(void){posix_spawnp();}" HAVE_GLIB_C_22_H)

set(flags "-fsigned-char -fmessage-length=0 -fdata-sections -ffunction-sections -fno-exceptions -D_FILE_OFFSET_BITS=64")
set(ccflags "-std=c++11")
set(link_flags "-lcurl ")

if(${OS} STREQUAL "Darwin")
	set(link_flags "${link_flags} -mmacosx-version-min=10.9 -framework Carbon -framework IOKit -framework Security")
	set(flags "${flags} -mmacosx-version-min=10.9")
endif()

if(${BUILD_TYPE} STREQUAL "release")
	set(flags "${flags} -O3 -s ${BUILD_OPTION} -w -fvisibility=hidden")

	if(${OS} STREQUAL "FreeBSD")
		set(flags "${flags} -fno-omit-frame-pointer")
	else()
		set(flags "${flags} -fomit-frame-pointer")
	endif()

	set(link_flags "${link_flags} ${BUILD_OPTION}")
	add_definitions(-DNDEBUG=1)

	if(HAVE_GLIB_C_225_H)
		set(link_flags "${link_flags} -Wl,--wrap=memcpy")
	endif()

	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		set(link_flags "${link_flags} -static-libgcc -static-libstdc++ -Wl,--no-as-needed")
	endif()
endif()

if(${BUILD_TYPE} STREQUAL "debug")
	set(flags "${flags} -g -O0 ${BUILD_OPTION} -Wall -Wno-overloaded-virtual")
	set(link_flags "${link_flags} ${BUILD_OPTION}")
	add_definitions(-DDEBUG=1)
endif()

set(CMAKE_C_FLAGS "${flags}")
set(CMAKE_CXX_FLAGS "${flags} ${ccflags}")

include_directories(
	"${PROJECT_ROOT_DIR}/include"
	"${PROJECT_ROOT_DIR}/libgit2/include"
	"${PROJECT_ROOT_DIR}/fibjs/fibjs/include"
	"${PROJECT_ROOT_DIR}/fibjs/vender"
	"${PROJECT_ROOT_DIR}/fibjs/vender/v8"
	"${PROJECT_ROOT_DIR}/fibjs/vender/v8/include"
	"${PROJECT_ROOT_DIR}/fibjs/vender/mbedtls"
	"${PROJECT_ROOT_DIR}/fibjs/vender/zlib/include"
	"${CMAKE_CURRENT_BINARY_DIR}"
)
