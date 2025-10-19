cmake_minimum_required(VERSION 3.31)

project(Microarch)
include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

FetchContent_Declare(
		vstl
		GIT_REPOSITORY https://github.com/magistermaks/lib-vstl
		GIT_TAG 75778e49a0d2ce95642472bd4ed8dcc8c1ba2793
)

FetchContent_MakeAvailable(vstl)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_library(microarch_common INTERFACE)

target_include_directories(microarch_common INTERFACE
		src/
)

add_executable(test
		entry/test.cpp
		${SOURCES}
)

target_link_libraries(test PRIVATE
		microarch_common
)

target_include_directories(test PRIVATE
		${vstl_SOURCE_DIR}
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

	# GCC debugging doodads
	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		message(STATUS "Compiling with debugging symbols")
		target_compile_options(microarch_common INTERFACE -g)
	endif()

	target_compile_options(microarch_common INTERFACE
			-Wall
			-Wextra
			-pedantic

			# Errors
			-Werror=return-type         # Fail if when return in non-void function is missing
			-Werror=return-local-addr   # Fail if a pointer to a local variable is returned
			-Werror=init-self           # Fail if a class member is initialized with itself
			-Werror=uninitialized       # Fail if a uninitialized variables is used
			-Werror=cast-qual           # Fail when you try to cast the const away
			-Werror=switch

			# Warnings
			-Wnoexcept                  # Warn if the lack of noexcept disables some optimizations
			-Wunreachable-code          # Warn if a section of code is unreachable
			-Wparentheses               # Warn if there is a high chance we forgot about some parentheses
			-Wlogical-op                # Warn if there is a high chance we messed up the logical operator
			-Wdisabled-optimization     # Warn if you managed to confuse the compiler :skull:
			-Wredundant-decls           # Warn if anything is declared more than once in the same scope
			-Wmissing-declarations      # Warn if a global function is defined without a previous declaration

			# Ignore some warnings that I deemed stupid
			-Wno-unused-parameter       # Not really an error to ignore something, we may enable it later
			-Wno-variadic-macros        # Allow so called 'variadic macros'
			-Wno-misleading-indentation # Don't complain about non-issues such as "if (thing) do();"
	)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")

	# we4715 - treat missing return statement as error
	target_compile_options(microarch_common INTERFACE /W4 /we4715)
endif()

add_executable(microarch
		entry/main.cpp
		${SOURCES}
)

target_link_libraries(microarch PRIVATE
		microarch_common
)

install(TARGETS microarch)
