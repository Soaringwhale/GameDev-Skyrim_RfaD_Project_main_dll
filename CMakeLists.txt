cmake_minimum_required(VERSION 3.20)

# Set your project name. This will be the name of your SKSE .dll file.
project(ManaShield VERSION 1.0.0 LANGUAGES CXX)

if(DEFINED ENV{SKYRIM_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_FOLDER}/Data")
    set(OUTPUT_FOLDER "$ENV{SKYRIM_FOLDER}/Data")
endif()

if(DEFINED ENV{SKYRIM_MODS_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_MODS_FOLDER}")
    set(OUTPUT_FOLDER "$ENV{SKYRIM_MODS_FOLDER}/${PROJECT_NAME}")
endif()

set(headers
        ${CMAKE_CURRENT_SOURCE_DIR}/include/logger.h)

set(sources
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp)

source_group(
        TREE ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
        ${headers}
        ${sources})

find_package(CommonLibSSE CONFIG)
add_commonlibsse_plugin(${PROJECT_NAME} SOURCES ${headers} ${sources}) # <--- specifies plugin.cpp
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23) # <--- use C++23 standard
target_precompile_headers(${PROJECT_NAME} PRIVATE PCH.h) # <--- PCH.h is required!

set(OUTPUT_FOLDER "E:/RfaDChicken/MO2/mods/5.1.9.3__RU_Patch_") #<--- your Skyrim folder

if(DEFINED OUTPUT_FOLDER)

    set(DLL_FOLDER "${OUTPUT_FOLDER}/SKSE/Plugins")

    message(STATUS "SKSE plugin output folder: ${DLL_FOLDER}")

    add_custom_command(
        TARGET "${PROJECT_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${DLL_FOLDER}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
        VERBATIM
    )

    # If you perform a "Debug" build, also copy .pdb file (for debug symbols)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_custom_command(
            TARGET "${PROJECT_NAME}"
            POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
            VERBATIM
        )
    endif()
endif()

target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)