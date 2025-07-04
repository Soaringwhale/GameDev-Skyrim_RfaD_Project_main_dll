cmake_minimum_required(VERSION 3.21)

# ---- Cache build vars ----

macro(set_from_environment VARIABLE)
    if (NOT DEFINED ${VARIABLE} AND DEFINED ENV{${VARIABLE}})
        set(${VARIABLE} $ENV{${VARIABLE}})
    endif ()
endmacro()

# ---- Vcpkg check ----
set_from_environment(VCPKG_ROOT)
if (NOT DEFINED VCPKG_ROOT)
    message(
        FATAL_ERROR
        "Variable VCPKG_ROOT is not set."
    )
endif ()

# ---- Include guards ----
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(
        FATAL_ERROR
            "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
    )
endif()

set(Boost_USE_STATIC_RUNTIME OFF CACHE BOOL "")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "")

# ---- Project ----

project(
    refrain_nextgen
    VERSION 2.0.0
    LANGUAGES CXX
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
    @ONLY
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    @ONLY
)

# ---- Globals ----

if (MSVC)
    add_compile_definitions(
        _UNICODE
    )

    if (NOT ${CMAKE_GENERATOR} STREQUAL "Ninja")
        add_compile_options(
            /MP    # Build with Multiple Processes
        )
    endif ()
endif ()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

set(Boost_USE_STATIC_LIBS ON)

if (DEFINED UseUselessUtils AND UseUselessUtils)
    add_compile_definitions(USELESS_UTILS)
endif ()


# ---- Dependencies ----

if (DEFINED UseUselessUtils AND UseUselessUtils)
    add_subdirectory($ENV{UselessFenixUtils} ./UselessFenixUtils/build)
else ()
    add_subdirectory($ENV{CommonLibSSE} ./CommonLibSSE/build)
endif ()

find_package(spdlog REQUIRED CONFIG)

# ---- Add source files ----

set(SOURCES
    src/main.cpp
    src/ItemCardFixer.h
    src/ItemCardFixer.cpp
    src/ActionManager.h
    src/ActionManager.cpp
    src/gameplay.h
    src/gameplay.cpp
    src/core.h
    src/core.cpp
    src/utils.h
    src/utils.cpp
    src/ui.h
    src/ui.cpp
    src/Hooks.h
    src/Events.h
    src/SimpleIni.h
    src/PCH.h
)

source_group(
    TREE
        ${CMAKE_CURRENT_SOURCE_DIR}
    FILES
        ${SOURCES}
)

source_group(
    TREE
        ${CMAKE_CURRENT_BINARY_DIR}
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
)

# ---- Create DLL ----

add_library(
    ${PROJECT_NAME}
    SHARED
    ${SOURCES}
    ${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    .clang-format
    .editorconfig
)

target_compile_features(
    ${PROJECT_NAME}
    PRIVATE
        cxx_std_20
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        CommonLibSSE::CommonLibSSE
        spdlog::spdlog
)

if (DEFINED UseUselessUtils AND UseUselessUtils)
    target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
            UselessFenixUtils::UselessFenixUtils
    )
endif ()

if (MSVC)
    target_compile_options(
        ${PROJECT_NAME}
        PRIVATE
      /std:c17
      
            /sdl    # Enable Additional Security Checks
            /utf-8    # Set Source and Executable character sets to UTF-8
            /Zi    # Debug Information Format

            /permissive-    # Standards conformance

            /Zc:alignedNew    # C++17 over-aligned allocation
            /Zc:auto    # Deduce Variable Type
            /Zc:char8_t
            /Zc:__cplusplus    # Enable updated __cplusplus macro
            /Zc:externC
            /Zc:externConstexpr    # Enable extern constexpr variables
            /Zc:forScope    # Force Conformance in for Loop Scope
            /Zc:hiddenFriend
            /Zc:implicitNoexcept    # Implicit Exception Specifiers
            /Zc:lambda
            /Zc:noexceptTypes    # C++17 noexcept rules
            /Zc:preprocessor    # Enable preprocessor conformance mode
            /Zc:referenceBinding    # Enforce reference binding rules
            /Zc:rvalueCast    # Enforce type conversion rules
            /Zc:sizedDealloc    # Enable Global Sized Deallocation Functions
            /Zc:strictStrings    # Disable string literal type conversion
            /Zc:ternary    # Enforce conditional operator rules
            /Zc:threadSafeInit    # Thread-safe Local Static Initialization
            /Zc:tlsGuards
            /Zc:trigraphs    # Trigraphs Substitution
            /Zc:wchar_t    # wchar_t Is Native Type

            /external:anglebrackets
            /external:W0

            /W4    # Warning level
            /WX    # Warning level (warnings are errors)

            "$<$<CONFIG:DEBUG>:>"
            "$<$<CONFIG:RELEASE>:/Zc:inline;/JMC-;/Ob3>"
    )

    target_link_options(
        ${PROJECT_NAME}
        PRIVATE
            /WX    # Treat Linker Warnings as Errors

            "$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
            "$<$<CONFIG:RELEASE>:/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>"
    )
endif ()

target_precompile_headers(
    ${PROJECT_NAME}
    PRIVATE
        src/PCH.h
)

SET (DEPLOY_PATH "C:/ART/11/SKSE/Plugins/RfadPlugin")   #  DEPLOY_PATH

  add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${PROJECT_NAME}> ${DEPLOY_PATH}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_PDB_FILE:${PROJECT_NAME}> ${DEPLOY_PATH}
  )
