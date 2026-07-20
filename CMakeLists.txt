cmake_minimum_required(VERSION 3.20)
project(pi_forwarder CXX C)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT forwarder)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(PI_FORWARDER_ASIO_CANDIDATE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/third-party/asio-1.36.0/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/third-party/asio/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/asio/asio/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/asio/include"
)

set(PI_FORWARDER_ASIO_INCLUDE_DIR "")
foreach(asio_candidate_dir IN LISTS PI_FORWARDER_ASIO_CANDIDATE_DIRS)
    if(EXISTS "${asio_candidate_dir}/asio.hpp")
        set(PI_FORWARDER_ASIO_INCLUDE_DIR "${asio_candidate_dir}")
        break()
    endif()
endforeach()

add_library(asio INTERFACE)
target_compile_definitions(asio INTERFACE ASIO_STANDALONE)
if(WIN32)
    target_compile_definitions(asio INTERFACE _WIN32_WINNT=0x0A00)
endif()

if(PI_FORWARDER_ASIO_INCLUDE_DIR)
    target_include_directories(asio INTERFACE "${PI_FORWARDER_ASIO_INCLUDE_DIR}")
    message(STATUS "Using vendored standalone ASIO from: ${PI_FORWARDER_ASIO_INCLUDE_DIR}")
else()
    message(STATUS "Standalone ASIO headers not found yet. Place them under third-party/asio-1.36.0/include, third-party/asio/include, or third_party/asio/ before adding real ASIO-based code.")
endif()

# nlohmann/json (single header)
set(PI_FORWARDER_NLOHMANN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party/nlohmann")
if(EXISTS "${PI_FORWARDER_NLOHMANN_DIR}/json.hpp")
    add_library(nlohmann_json INTERFACE)
    target_include_directories(nlohmann_json INTERFACE "${PI_FORWARDER_NLOHMANN_DIR}")
    message(STATUS "Found nlohmann/json at: ${PI_FORWARDER_NLOHMANN_DIR}")
else()
    message(FATAL_ERROR "nlohmann/json.hpp not found at ${PI_FORWARDER_NLOHMANN_DIR}/json.hpp")
endif()

# Vendored RSSP1 C library — always compiled
file(GLOB_RECURSE RSSP1_SOURCES CONFIGURE_DEPENDS
    src/GM_RSSPI_V2.0.14/*.c
)

add_library(rssp1 STATIC ${RSSP1_SOURCES})
target_include_directories(rssp1 PUBLIC
    src/GM_RSSPI_V2.0.14
    src/GM_RSSPI_V2.0.14/CFM
    src/GM_RSSPI_V2.0.14/SFM
    src/GM_RSSPI_V2.0.14/common
)

# Single-threaded: disable the vendored C stack's mutex hooks
target_compile_definitions(rssp1 PUBLIC GM_RSSP1_DISABLE_LOCK OS_64)
if(NOT WIN32)
    target_compile_definitions(rssp1 PUBLIC LINUX)
endif()

# Suppress C4819 (Unicode-in-CP936) in vendored C source files
if(MSVC)
    target_compile_options(rssp1 PRIVATE /wd4819)
endif()

if(WIN32)
    target_link_libraries(rssp1 PUBLIC ws2_32 mswsock)
endif()

# Forwarder executable
file(GLOB FORWARDER_SOURCES CONFIGURE_DEPENDS
    src/main.cpp
    src/forwarder/*.cpp
)

add_executable(forwarder ${FORWARDER_SOURCES})
target_include_directories(forwarder PRIVATE src src/forwarder)
target_link_libraries(forwarder PRIVATE asio rssp1 nlohmann_json)

if(WIN32)
    target_link_libraries(forwarder PRIVATE ws2_32 mswsock)
endif()
