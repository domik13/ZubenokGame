cmake_minimum_required(VERSION 3.26)

project(
	ZubrenokGame
	VERSION 0.1.0
	LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)

# Use a full (non-shallow) clone so cmake/ templates are never missing mid-configure.
FetchContent_Declare(
	SFML
	GIT_REPOSITORY https://github.com/SFML/SFML.git
	GIT_TAG 3.0.2
	SYSTEM
)

set(SQLITECPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SQLITECPP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SQLITECPP_RUN_CPPLINT OFF CACHE BOOL "" FORCE)
set(SQLITECPP_RUN_CPPCHECK OFF CACHE BOOL "" FORCE)
set(SQLITECPP_INTERNAL_SQLITE ON CACHE BOOL "" FORCE)

FetchContent_Declare(
	SQLiteCpp
	GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
	GIT_TAG 3.3.3
)

FetchContent_MakeAvailable(SFML SQLiteCpp)

add_executable(
	ZubrenokGame
	src/main.cpp
	src/app/Application.cpp
	src/persistence/Database.cpp
	src/entities/Player.cpp
	src/levels/Levels.cpp
	include/zubrenok/entities/Player.h
	include/zubrenok/game/GameTypes.h
	include/zubrenok/game/ContrillersMoves.h
	include/zubrenok/levels/Level.h
 "include/zubrenok/entities/Enemy.h" "src/entities/Enemy.cpp")

target_include_directories(
	ZubrenokGame
	PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(
	ZubrenokGame
	PRIVATE
	SFML::Graphics
	SFML::Audio
	SQLiteCpp
)

if(MSVC)
	target_compile_options(
		ZubrenokGame
		PRIVATE
		/W4
		/permissive-
		/utf-8
	)
else()
	target_compile_options(
		ZubrenokGame
		PRIVATE
		-Wall
		-Wextra
		-Wpedantic
	)
endif()

add_custom_command(
	TARGET ZubrenokGame
	POST_BUILD
	COMMAND
		${CMAKE_COMMAND}
		-E
		copy_directory
		"${CMAKE_CURRENT_SOURCE_DIR}/assets"
		"$<TARGET_FILE_DIR:ZubrenokGame>/assets"
)

set_property(
	DIRECTORY
	"${CMAKE_CURRENT_SOURCE_DIR}"
	PROPERTY
	VS_STARTUP_PROJECT ZubrenokGame
)

set_property(
	TARGET ZubrenokGame
	PROPERTY
	VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:ZubrenokGame>"
)
