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

file(GLOB_RECURSE src_list "src/*.c*")
add_executable(fibgit ${src_list})

set(PROJECT_ROOT_DIR ${PROJECT_SOURCE_DIR}/../..)
set(ARCH_DIRNAME ${OS}_${ARCH}_${BUILD_TYPE})
set(BIN_DIR ${PROJECT_ROOT_DIR}/bin/${ARCH_DIRNAME})
set(FIBJS_DIR ${PROJECT_ROOT_DIR}/fibjs/bin/${ARCH_DIRNAME})
set(LIBGIT2_DIR ${PROJECT_ROOT_DIR}/libgit2/${LIBGIT2_BUILD_PATH})
set(EXECUTABLE_OUTPUT_PATH ${BIN_DIR})

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
	target_link_libraries(fibgit dl iconv stdc++)
endif()

if(${OS} STREQUAL "Linux")
	target_link_libraries(fibgit dl rt)
endif()

if(${OS} STREQUAL "FreeBSD")
	find_library(execinfo execinfo "/usr/local/lib" "/usr/lib")
	target_link_libraries(fibgit ${execinfo})
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
	"${PROJECT_ROOT_DIR}/fibgit/include"
	"${PROJECT_ROOT_DIR}/libgit2/include"
	"${PROJECT_ROOT_DIR}/fibjs/fibjs/include"
	"${PROJECT_ROOT_DIR}/fibjs/vender"
	"${PROJECT_ROOT_DIR}/fibjs/vender/v8"
	"${PROJECT_ROOT_DIR}/fibjs/vender/v8/include"
	"${PROJECT_ROOT_DIR}/fibjs/vender/mbedtls"
	"${PROJECT_ROOT_DIR}/fibjs/vender/zlib/include"
	"${CMAKE_CURRENT_BINARY_DIR}"
)

set(libs fibjs expat gumbo gd tiff jpeg png webp zlib leveldb snappy ev pcre sqlite mongo umysql uuid exif mbedtls v8 zmq unzip editline exlib)

foreach(lib ${libs})
	target_link_libraries(fibgit "${FIBJS_DIR}/lib${lib}.a")
endforeach()

target_link_libraries(fibgit "${LIBGIT2_DIR}/${LIBGIT2_OUTPUTNAME}.a")

target_link_libraries(fibgit "${BIN_DIR}/libfibgit.a")

target_link_libraries(fibgit pthread)

set_target_properties(fibgit PROPERTIES LINK_FLAGS ${link_flags})
