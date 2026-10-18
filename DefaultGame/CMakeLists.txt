cmake_minimum_required(VERSION 3.25)

if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(GDFramework LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
    if(MSVC OR CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
        set(SDL_PACKAGE_FLAVOR "VC")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang)$")
        set(SDL_PACKAGE_FLAVOR "mingw")
    else()
        message(FATAL_ERROR "Unsupported Windows compiler: ${CMAKE_CXX_COMPILER_ID}")
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/MP)
endif()

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")
file(GLOB_RECURSE PROJECT_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.h")

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/source")
target_compile_definitions(${PROJECT_NAME} PRIVATE "$<$<CONFIG:Debug>:DEBUG_BUILD>")

if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE /UUNICODE /U_UNICODE)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -UUNICODE -U_UNICODE)
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${PROJECT_SOURCES} ${PROJECT_HEADERS})

include(FetchContent)
set(FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/_deps")

FetchContent_Declare(
    box2d
    GIT_REPOSITORY https://github.com/erincatto/box2d.git
    GIT_TAG v3.1.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(box2d)

set(JSON_LOCAL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs/JSON/include/nlohmann/json.hpp")
if(EXISTS "${JSON_LOCAL_PATH}")
    target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libs/JSON/include")
else()
    FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(json)
    target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
endif()

if(WIN32)
    function(download_sdl_package package version archive_prefix root config_file)
        if(EXISTS "${root}/cmake/${config_file}")
            message(STATUS "Using prebuilt ${package} ${version} from ${root}")
            return()
        endif()

        set(archive_name "${archive_prefix}-devel-${version}-${SDL_PACKAGE_FLAVOR}.zip")
        set(archive "${CMAKE_BINARY_DIR}/${archive_name}")
        set(url "https://github.com/libsdl-org/${package}/releases/download/release-${version}/${archive_name}")
        set(extract_dir "${CMAKE_BINARY_DIR}/${package}-${version}-extract")

        message(STATUS "Downloading prebuilt ${package} ${version} ...")
        file(DOWNLOAD "${url}" "${archive}" SHOW_PROGRESS STATUS download_status)
        list(GET download_status 0 download_code)
        list(GET download_status 1 download_message)
        if(NOT download_code EQUAL 0)
            message(FATAL_ERROR "Failed to download ${package} ${version}: ${download_message}")
        endif()

        file(REMOVE_RECURSE "${extract_dir}")
        file(MAKE_DIRECTORY "${extract_dir}" "${root}")
        file(ARCHIVE_EXTRACT INPUT "${archive}" DESTINATION "${extract_dir}")
        file(GLOB extracted_roots LIST_DIRECTORIES true "${extract_dir}/*")
        list(LENGTH extracted_roots extracted_root_count)
        if(NOT extracted_root_count EQUAL 1)
            message(FATAL_ERROR "Unexpected ${package} archive layout")
        endif()
        list(GET extracted_roots 0 extracted_root)
        file(COPY "${extracted_root}/" DESTINATION "${root}")

        if(NOT EXISTS "${root}/cmake/${config_file}")
            message(FATAL_ERROR "Downloaded ${package} ${version} but ${config_file} was not found")
        endif()
    endfunction()

    set(SDL3_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/libs/${SDL_PACKAGE_FLAVOR}/SDL3" CACHE PATH "SDL3 package root")
    set(SDL3_image_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/libs/${SDL_PACKAGE_FLAVOR}/SDL3_image" CACHE PATH "SDL3_image package root")
    set(SDL3_mixer_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/libs/${SDL_PACKAGE_FLAVOR}/SDL3_mixer" CACHE PATH "SDL3_mixer package root")
    set(SDL3_ttf_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/libs/${SDL_PACKAGE_FLAVOR}/SDL3_ttf" CACHE PATH "SDL3_ttf package root")

    download_sdl_package(SDL 3.4.14 SDL3 "${SDL3_ROOT}" SDL3Config.cmake)
    download_sdl_package(SDL_image 3.4.4 SDL3_image "${SDL3_image_ROOT}" SDL3_imageConfig.cmake)
    download_sdl_package(SDL_mixer 3.2.4 SDL3_mixer "${SDL3_mixer_ROOT}" SDL3_mixerConfig.cmake)
    download_sdl_package(SDL_ttf 3.2.2 SDL3_ttf "${SDL3_ttf_ROOT}" SDL3_ttfConfig.cmake)

    find_package(SDL3 3.4.14 EXACT CONFIG REQUIRED HINTS "${SDL3_ROOT}")
    find_package(SDL3_image 3.4.4 EXACT CONFIG REQUIRED HINTS "${SDL3_image_ROOT}")
    find_package(SDL3_mixer 3.2.4 EXACT CONFIG REQUIRED HINTS "${SDL3_mixer_ROOT}")
    find_package(SDL3_ttf 3.2.2 EXACT CONFIG REQUIRED HINTS "${SDL3_ttf_ROOT}")
else()
    find_package(SDL3 CONFIG REQUIRED)
    find_package(SDL3_image CONFIG REQUIRED)
    find_package(SDL3_mixer CONFIG REQUIRED)
    find_package(SDL3_ttf CONFIG REQUIRED)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    box2d
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_mixer::SDL3_mixer
    SDL3_ttf::SDL3_ttf
)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    )

    foreach(SDL_TARGET SDL3::SDL3 SDL3_image::SDL3_image SDL3_mixer::SDL3_mixer SDL3_ttf::SDL3_ttf)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:${SDL_TARGET}>"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            VERBATIM
        )
    endforeach()
endif()

set(CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/config.json")
if(NOT EXISTS "${CONFIG_FILE}")
    file(WRITE "${CONFIG_FILE}" [=[
{
    "title": "Default Game Name",
    "window_width": 1280,
    "window_height": 720,
    "fullscreen": false
}
]=])
    message(STATUS "Created default config.json")
endif()

add_custom_target(${PROJECT_NAME}Config ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CONFIG_FILE}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    DEPENDS "${CONFIG_FILE}"
    VERBATIM
)
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}Config)

set(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")
file(MAKE_DIRECTORY "${ASSETS_DIR}")
add_custom_target(${PROJECT_NAME}Assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${ASSETS_DIR}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    VERBATIM
)
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}Assets)
