cmake_minimum_required(VERSION 3.6 FATAL_ERROR)
cmake_policy(VERSION 3.6)
include("cmake/defaults.cmake")
set(NAME Celestia)

project(${NAME})

if (NOT "${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    message( FATAL_ERROR "Only 64 bit builds supported." )
endif()

include("cmake/compiler.cmake")

find_package(Threads REQUIRED)

# This define is specific to Vulkan has a depth range of [0, 1], unlike OpenGL which has a [-1, 1] 
add_definitions(-DGLM_FORCE_DEPTH_ZERO_TO_ONE)

if (NOT ANDROID)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin_debug")
    set(CMAKE_DEBUG_POSTFIX "d")

    add_custom_target(SetupRelease ALL ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin)
    set_target_properties(SetupRelease PROPERTIES FOLDER "CMakeTargets")
    add_custom_target(SetupDebug ALL ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin_debug)
    set_target_properties(SetupDebug PROPERTIES FOLDER "CMakeTargets")
endif()

if (ANDROID)
    add_definitions(-DVULKAN_HPP_NO_SMART_HANDLE)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")
    set(APP_GLUE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
    include_directories(${APP_GLUE_DIR})
    add_library(app-glue STATIC ${APP_GLUE_DIR}/android_native_app_glue.c)
endif()

if (ANDROID)
    add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR)
elseif (WIN32)
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
else()
    add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
    find_package(XCB REQUIRED)
    link_libraries(${XCB_LIBRARIES})
endif()



set(SHADER_DIR "${PROJECT_SOURCE_DIR}/resources/shaders")
file(GLOB_RECURSE SHADERS
    ${SHADER_DIR}/*.vert
    ${SHADER_DIR}/*.frag
    ${SHADER_DIR}/*.comp
    ${SHADER_DIR}/*.tesc
    ${SHADER_DIR}/*.tese
    ${SHADER_DIR}/*.geom
)
foreach(SHADER ${SHADERS})
    compile_spirv_shader(${SHADER})
    list(APPEND COMPILED_SHADERS ${COMPILE_SPIRV_SHADER_RETURN})
    source_group("compiled" FILES ${COMPILE_SPIRV_SHADER_RETURN})
endforeach()
add_custom_target(shaders SOURCES ${SHADERS} ${COMPILED_SHADERS})
set_target_properties(shaders PROPERTIES FOLDER "common")



add_definitions(-DUSE_SPICE=1)
add_subdirectory(libraries)
add_subdirectory(app)
add_subdirectory(tools/cmod2gltf)
add_subdirectory(tools/testCore)