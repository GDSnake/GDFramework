cmake_minimum_required(VERSION 3.15)
project(BlockFall)

set(CMAKE_CXX_STANDARD 17)

add_executable(BlockFall source/main.cpp)

# ---------------------------------------------------------
# Create a default config.json if missing
# ---------------------------------------------------------
set(CONFIG_FILE "${CMAKE_SOURCE_DIR}/config.json")
if (NOT EXISTS ${CONFIG_FILE})
    file(WRITE ${CONFIG_FILE} "{\n")
    file(APPEND ${CONFIG_FILE} "    \"title\": \"Default Game Name\",\n")
    file(APPEND ${CONFIG_FILE} "    \"window_width\": 1280,\n")
    file(APPEND ${CONFIG_FILE} "    \"window_height\": 720,\n")
    file(APPEND ${CONFIG_FILE} "    \"fullscreen\": false\n")
    file(APPEND ${CONFIG_FILE} "}\n")
    message(STATUS "Created default config.json")
endif()

include(FetchContent)

# ---------------------------------------------------------
# JSON Setup (download if missing, otherwise use local)
# ---------------------------------------------------------
set(JSON_LOCAL_PATH "${CMAKE_SOURCE_DIR}/libs/JSON/include/nlohmann/json.hpp")

if (EXISTS ${JSON_LOCAL_PATH})
    message(STATUS "Using local nlohmann/json.hpp")
    target_include_directories(BlockFall PRIVATE ${CMAKE_SOURCE_DIR}/libs/JSON/include)
else()
    message(STATUS "Downloading nlohmann/json ...")
    file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/libs/JSON)

    FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(json)
    file(COPY ${json_SOURCE_DIR}/include DESTINATION ${CMAKE_SOURCE_DIR}/libs/JSON)
    target_include_directories(BlockFall PRIVATE ${CMAKE_SOURCE_DIR}/libs/JSON/include)
endif()

# ---------------------------------------------------------
# SDL3 Setup (download if missing)
# ---------------------------------------------------------
set(SDL3_DIR "${CMAKE_SOURCE_DIR}/libs/SDL3")

if (EXISTS "${SDL3_DIR}/include/SDL3/SDL.h")
    message(STATUS "Using local SDL3 at ${SDL3_DIR}")
else()
    message(STATUS "Downloading SDL3 precompiled binaries ...")
    file(MAKE_DIRECTORY ${SDL3_DIR})

    if (WIN32)
        set(SDL3_URL https://github.com/libsdl-org/SDL/releases/download/release-3.2.18/SDL3-devel-3.2.18-VC.zip)
    else()
        set(SDL3_URL https://github.com/libsdl-org/SDL/releases/download/release-3.2.18/SDL3-3.2.18.tar.gz)
    endif()

    set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)
    FetchContent_Declare(
        SDL3
        URL ${SDL3_URL}
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_Populate(SDL3)

    message(STATUS "SDL3 downloaded to: ${sdl3_SOURCE_DIR}")
    file(GLOB CONTENTS "${sdl3_SOURCE_DIR}/*")
    message(STATUS "Contents: ${CONTENTS}")

    set(SDL3_COPY_DIR "")
    if (EXISTS "${sdl3_SOURCE_DIR}/include/SDL3/SDL.h")
        set(SDL3_COPY_DIR "${sdl3_SOURCE_DIR}")
    else()
        foreach(child ${CONTENTS})
            if (EXISTS "${child}/include/SDL3/SDL.h")
                set(SDL3_COPY_DIR "${child}")
                break()
            endif()
        endforeach()
    endif()

    if (SDL3_COPY_DIR STREQUAL "")
        message(FATAL_ERROR "SDL3 extraction failed: include/SDL3/SDL.h not found.")
    else()
        message(STATUS "Found SDL3 extracted in: ${SDL3_COPY_DIR}")
    endif()

    file(COPY "${SDL3_COPY_DIR}/include" DESTINATION "${SDL3_DIR}")
    if (EXISTS "${SDL3_COPY_DIR}/lib")
        file(COPY "${SDL3_COPY_DIR}/lib" DESTINATION "${SDL3_DIR}")
    endif()
    if (EXISTS "${SDL3_COPY_DIR}/bin")
        file(COPY "${SDL3_COPY_DIR}/bin" DESTINATION "${SDL3_DIR}")
    endif()
endif()

# ---------------------------------------------------------
# SDL3 Linking
# ---------------------------------------------------------
if (WIN32)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ARCH_DIR "x64")
    else()
        set(ARCH_DIR "x86")
    endif()

    include_directories(${SDL3_DIR}/include)

    set(SDL3_LIB_DIR "${SDL3_DIR}/lib/${ARCH_DIR}")

    target_link_directories(BlockFall PRIVATE ${SDL3_LIB_DIR})
    target_link_libraries(BlockFall PRIVATE SDL3)

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(SDL_DLL "SDL3d.dll")
    else()
        set(SDL_DLL "SDL3.dll")
    endif()

    if (NOT EXISTS "${SDL3_LIB_DIR}/${SDL_DLL}")
        set(SDL_DLL "SDL3.dll")
    endif()

    add_custom_command(TARGET BlockFall POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SDL3_LIB_DIR}/${SDL_DLL}"
            $<TARGET_FILE_DIR:BlockFall>
    )
else()
    find_package(SDL3 REQUIRED CONFIG)
    target_link_libraries(BlockFall PRIVATE SDL3::SDL3)
endif()

# ---------------------------------------------------------
# Copy config.json after build
# ---------------------------------------------------------
add_custom_command(TARGET BlockFall POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/config.json"
        $<TARGET_FILE_DIR:BlockFall>
)
