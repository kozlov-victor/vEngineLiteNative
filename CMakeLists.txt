
cmake_minimum_required(VERSION 4.0)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(vEngineLiteNative LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================
# Dependencies
# ============================================================

set(DEPS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies")

# ============================================================
# ANGLE
# ============================================================

set(ANGLE_ROOT "${DEPS}/angle")
set(ANGLE_INCLUDE_DIR "${ANGLE_ROOT}/include")
set(ANGLE_LIB_DIR "${ANGLE_ROOT}/lib/x64")

if(EXISTS "${ANGLE_LIB_DIR}/libEGL.dll.lib")
    set(ANGLE_EGL_LIB "${ANGLE_LIB_DIR}/libEGL.dll.lib")
elseif(EXISTS "${ANGLE_LIB_DIR}/libEGL.lib")
    set(ANGLE_EGL_LIB "${ANGLE_LIB_DIR}/libEGL.lib")
else()
    message(FATAL_ERROR
            "ANGLE EGL import library not found in ${ANGLE_LIB_DIR}")
endif()

if(EXISTS "${ANGLE_LIB_DIR}/libGLESv2.dll.lib")
    set(ANGLE_GLES_LIB "${ANGLE_LIB_DIR}/libGLESv2.dll.lib")
elseif(EXISTS "${ANGLE_LIB_DIR}/libGLESv2.lib")
    set(ANGLE_GLES_LIB "${ANGLE_LIB_DIR}/libGLESv2.lib")
else()
    message(FATAL_ERROR
            "ANGLE GLESv2 import library not found in ${ANGLE_LIB_DIR}")
endif()

set(ANGLE_EGL_DLL "${ANGLE_LIB_DIR}/libEGL.dll")
set(ANGLE_GLES_DLL "${ANGLE_LIB_DIR}/libGLESv2.dll")

if(NOT EXISTS "${ANGLE_EGL_DLL}")
    message(FATAL_ERROR "ANGLE DLL not found: ${ANGLE_EGL_DLL}")
endif()

if(NOT EXISTS "${ANGLE_GLES_DLL}")
    message(FATAL_ERROR "ANGLE DLL not found: ${ANGLE_GLES_DLL}")
endif()

# ============================================================
# V8 — static monolithic library
# Structure:
# dependencies/v8/include/
# dependencies/v8/lib/x64/v8_monolith.lib
# ============================================================

set(V8_ROOT "${DEPS}/v8")
set(V8_INCLUDE_DIR "${V8_ROOT}/include")
set(V8_MONOLITH_LIB "${V8_ROOT}/lib/x64/v8_monolith.lib")

if(NOT EXISTS "${V8_INCLUDE_DIR}/v8.h")
    message(FATAL_ERROR
            "V8 header not found: ${V8_INCLUDE_DIR}/v8.h")
endif()

if(NOT EXISTS "${V8_MONOLITH_LIB}")
    message(FATAL_ERROR
            "V8 static library not found: ${V8_MONOLITH_LIB}")
endif()

message(STATUS "V8 include directory: ${V8_INCLUDE_DIR}")
message(STATUS "V8 monolithic library: ${V8_MONOLITH_LIB}")

# ============================================================
# Application
# ============================================================

add_executable(vEngineLiteNative
        main.cpp
        app/console/Console.h app/console/Console.cpp
        app/logger/Logger.h app/logger/Logger.win.cpp
        app/gl/GlFields.h app/gl/GlFields.cpp
        app/gl/GlFunctions.h app/gl/GlFunctions.cpp
        app/js/Js.h app/js/Js.cpp
        app/external_functions/ExternalFunctions.h app/external_functions/ExternalFunctions.cpp
        app/external_functions/external_functions_impl/ExternalFunctionsImpl.h
        app/external_functions/external_functions_impl/ExternalFunctionsImpl.win.cpp
        app/js_compilation_result/JsCompilationResult.h app/js_compilation_result/JsCompilationResult.cpp
        app/misc/fun.hpp
        app/v_engine/v_engine.h app/v_engine/v_engine.cpp
        app/bootstrap/bootstrap.hpp
        app/misc/win/env.h
        app/misc/win/misc.h app/misc/win/misc.cpp
        app/misc/win/bitmap/bitmap.h app/misc/win/bitmap/bitmap.cpp
        app/misc/win/keyboard/keyboard.h app/misc/win/keyboard/keyboard.cpp
)

target_include_directories(vEngineLiteNative PRIVATE
        "${ANGLE_INCLUDE_DIR}"
        "${V8_INCLUDE_DIR}"
)


target_link_libraries(vEngineLiteNative PRIVATE
        "${ANGLE_EGL_LIB}"
        "${ANGLE_GLES_LIB}"
        "${V8_MONOLITH_LIB}"
        dbghelp
        winmm
        ws2_32
        windowscodecs
        ole32
)

# ============================================================
# Compiler settings
# ============================================================

if(MSVC)
    target_compile_options(vEngineLiteNative PRIVATE
            /Zc:__cplusplus
            /utf-8
    )
    target_compile_definitions(vEngineLiteNative PRIVATE
            V8_COMPRESS_POINTERS
            NOMINMAX
            WIN32_LEAN_AND_MEAN
    )
endif()

# ============================================================
# Copy runtime DLLs next to executable
# ============================================================

add_custom_command(TARGET vEngineLiteNative POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${SDL_DLL}"
        "$<TARGET_FILE_DIR:vEngineLiteNative>"

        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${ANGLE_EGL_DLL}"
        "$<TARGET_FILE_DIR:vEngineLiteNative>"

        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${ANGLE_GLES_DLL}"
        "$<TARGET_FILE_DIR:vEngineLiteNative>"

        COMMENT "Copying SDL3 and ANGLE runtime DLLs"
        VERBATIM
)
