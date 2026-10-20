cmake_minimum_required(VERSION 3.25)

if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
endif()

project(GDFrameworkSamples LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(FRAMEWORK_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../DefaultGame/source")
set(FRAMEWORK_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../DefaultGame")

include(FetchContent)
set(FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/_deps")
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)
FetchContent_Declare(
    box2d
    GIT_REPOSITORY https://github.com/erincatto/box2d.git
    GIT_TAG v3.1.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(box2d)

if(WIN32)
    if(MSVC OR CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
        set(SDL_PACKAGE_FLAVOR "VC")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang)$")
        set(SDL_PACKAGE_FLAVOR "mingw")
    else()
        message(FATAL_ERROR "Unsupported Windows compiler: ${CMAKE_CXX_COMPILER_ID}")
    endif()

    function(ensure_sdl_package package version archive_prefix root config_file)
        if(EXISTS "${root}/cmake/${config_file}")
            return()
        endif()

        set(archive_name "${archive_prefix}-devel-${version}-${SDL_PACKAGE_FLAVOR}.zip")
        set(archive "${CMAKE_BINARY_DIR}/${archive_name}")
        set(url "https://github.com/libsdl-org/${package}/releases/download/release-${version}/${archive_name}")
        set(extract_dir "${CMAKE_BINARY_DIR}/_deps/${package}-${version}-extract")

        message(STATUS "Downloading prebuilt ${package} ${version} for samples...")
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

    set(SDL_PACKAGE_CACHE "${FRAMEWORK_ROOT_DIR}/libs/${SDL_PACKAGE_FLAVOR}")
    set(SDL_PACKAGE_BUILD "${CMAKE_BINARY_DIR}/_deps/sdl/${SDL_PACKAGE_FLAVOR}")

    set(SDL3_ROOT "${SDL_PACKAGE_CACHE}/SDL3")
    if(NOT EXISTS "${SDL3_ROOT}/cmake/SDL3Config.cmake")
        set(SDL3_ROOT "${SDL_PACKAGE_BUILD}/SDL3")
    endif()
    set(SDL3_image_ROOT "${SDL_PACKAGE_CACHE}/SDL3_image")
    if(NOT EXISTS "${SDL3_image_ROOT}/cmake/SDL3_imageConfig.cmake")
        set(SDL3_image_ROOT "${SDL_PACKAGE_BUILD}/SDL3_image")
    endif()
    set(SDL3_mixer_ROOT "${SDL_PACKAGE_CACHE}/SDL3_mixer")
    if(NOT EXISTS "${SDL3_mixer_ROOT}/cmake/SDL3_mixerConfig.cmake")
        set(SDL3_mixer_ROOT "${SDL_PACKAGE_BUILD}/SDL3_mixer")
    endif()
    set(SDL3_ttf_ROOT "${SDL_PACKAGE_CACHE}/SDL3_ttf")
    if(NOT EXISTS "${SDL3_ttf_ROOT}/cmake/SDL3_ttfConfig.cmake")
        set(SDL3_ttf_ROOT "${SDL_PACKAGE_BUILD}/SDL3_ttf")
    endif()

    ensure_sdl_package(SDL 3.4.14 SDL3 "${SDL3_ROOT}" SDL3Config.cmake)
    ensure_sdl_package(SDL_image 3.4.4 SDL3_image "${SDL3_image_ROOT}" SDL3_imageConfig.cmake)
    ensure_sdl_package(SDL_mixer 3.2.4 SDL3_mixer "${SDL3_mixer_ROOT}" SDL3_mixerConfig.cmake)
    ensure_sdl_package(SDL_ttf 3.2.2 SDL3_ttf "${SDL3_ttf_ROOT}" SDL3_ttfConfig.cmake)

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

add_library(SampleFramework STATIC
    "${FRAMEWORK_SOURCE_DIR}/Core/Renderer.cpp"
    "${FRAMEWORK_SOURCE_DIR}/Entities/Sprite.cpp"
    "${FRAMEWORK_SOURCE_DIR}/AssetManagers/SpriteManager.cpp"
    "${FRAMEWORK_SOURCE_DIR}/AssetManagers/AudioManager.cpp"
    "${FRAMEWORK_SOURCE_DIR}/AssetManagers/FontsManager.cpp"
    "${FRAMEWORK_SOURCE_DIR}/Game/Game.cpp"
    "${FRAMEWORK_SOURCE_DIR}/Game/Scenes/GameScene.cpp"
    "${FRAMEWORK_SOURCE_DIR}/Input/InputManager.cpp"
)
target_include_directories(SampleFramework PUBLIC
    "${FRAMEWORK_SOURCE_DIR}"
    "${FRAMEWORK_SOURCE_DIR}/Game/Scenes"
)
target_link_libraries(SampleFramework PUBLIC
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_mixer::SDL3_mixer
    SDL3_ttf::SDL3_ttf
    box2d
    nlohmann_json::nlohmann_json
)

if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    target_compile_options(SampleFramework PRIVATE /UUNICODE /U_UNICODE)
else()
    target_compile_options(SampleFramework PRIVATE -UUNICODE -U_UNICODE)
endif()

function(add_sample target source sample_directory)
    add_executable(${target} "${source}" ${ARGN})
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(${target} PRIVATE SampleFramework)
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${target}"
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${target}>"
    )

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${sample_directory}/config.json"
            "$<TARGET_FILE_DIR:${target}>/config.json"
        VERBATIM
    )

    if(EXISTS "${sample_directory}/assets")
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${sample_directory}/assets"
                "$<TARGET_FILE_DIR:${target}>/assets"
            VERBATIM
        )
    endif()

    if(EXISTS "${sample_directory}/maps")
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${sample_directory}/maps"
                "$<TARGET_FILE_DIR:${target}>/maps"
            VERBATIM
        )
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SampleAssets")
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/SampleAssets"
                "$<TARGET_FILE_DIR:${target}>/assets"
            VERBATIM
        )
    endif()

    if(EXISTS "${FRAMEWORK_ROOT_DIR}/assets/Fonts")
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${FRAMEWORK_ROOT_DIR}/assets/Fonts"
                "$<TARGET_FILE_DIR:${target}>/assets/Fonts"
            VERBATIM
        )
    endif()

    if(WIN32)
        foreach(SDL_TARGET SDL3::SDL3 SDL3_image::SDL3_image SDL3_mixer::SDL3_mixer SDL3_ttf::SDL3_ttf)
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "$<TARGET_FILE:${SDL_TARGET}>"
                    "$<TARGET_FILE_DIR:${target}>"
                VERBATIM
            )
        endforeach()
    endif()
endfunction()

add_sample(SampleSquare
    "${CMAKE_CURRENT_SOURCE_DIR}/Square/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Square"
    "${CMAKE_CURRENT_SOURCE_DIR}/Square/TestScene.cpp"
)
add_sample(SampleImageViewer
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageViewer/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageViewer"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageViewer/TestScene.cpp"
)
add_sample(SampleSceneChange
    "${CMAKE_CURRENT_SOURCE_DIR}/SceneChange/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/SceneChange"
    "${CMAKE_CURRENT_SOURCE_DIR}/SceneChange/MenuScene.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/SceneChange/SettingsScene.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/SceneChange/TestScene.cpp"
)
add_sample(SampleScreenScrolling
    "${CMAKE_CURRENT_SOURCE_DIR}/ScreenScrolling/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ScreenScrolling"
    "${CMAKE_CURRENT_SOURCE_DIR}/ScreenScrolling/TestScene.cpp"
)