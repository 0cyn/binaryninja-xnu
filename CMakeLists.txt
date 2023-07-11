cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

# Plugin Name
set(PLUGIN_NAME binaryninja-xnu)

set(PLUGIN_SOURCE Standalone.cpp Workflows/DarwinKernel.cpp Workflows/DarwinKernel.h )
set(UI_PLUGIN_SOURCE UI/TypeSetter.cpp UI/TypeSetter.h )

# Non-user-serviceable parts below ---------------------------------------------

if (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
endif ()

project(${PLUGIN_NAME})

# BinaryNinja API Directory
set(BINJA_API_DIR ${PROJECT_SOURCE_DIR}/binaryninja-api)

add_subdirectory(${BINJA_API_DIR})
include_directories(${CMAKE_SOURCE_DIR})

set(_UI_PLUGIN_SOURCES )
set(UI_PLUGIN_LIBRARIES )
set(UI_COMPILE_DEFS )

if (UI_BUILD)

    if(NOT DEFINED CMAKE_PREFIX_PATH)
        if(APPLE)
            set(CMAKE_PREFIX_PATH "$ENV{HOME}/Qt/6.4.3/clang_64/lib/cmake")
        elseif(MSVC) # doesnt work on gh runners
            set(CMAKE_PREFIX_PATH "$ENV{HOME}/Qt/6.4.3/msvc2022_64/lib/cmake")
        else()
            set(CMAKE_PREFIX_PATH "$ENV{HOME}/Qt/6.4.3/gcc_64/lib/cmake")
        endif()
    endif()

    message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)

    set(_UI_PLUGIN_SOURCES ${UI_PLUGIN_SOURCE})
    set(UI_PLUGIN_LIBRARIES Qt6::Core Qt6::Gui Qt6::Widgets)
    set(UI_COMPILE_DEFS UI_BUILD=1)

    find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

    find_package(QT NAMES Qt6 COMPONENTS Widgets REQUIRED)
    find_package(Qt6 COMPONENTS Widgets REQUIRED)

endif()

set(_PLUGIN_SOURCE ${PLUGIN_SOURCE} ${_UI_PLUGIN_SOURCES})

set("${CMAKE_C_FLAGS} -include${PROJECT_SOURCE_DIR}/ProjectDefinitions.h")

add_library(${PLUGIN_NAME} SHARED ${_PLUGIN_SOURCE})

set(CMAKE_CXX_STANDARD 17)
target_compile_definitions(${PLUGIN_NAME} PRIVATE DEV_MODE=${DEV_MODE} ${UI_COMPILE_DEFS})

if(APPLE)
    target_link_options(${PLUGIN_NAME} PUBLIC -undefined dynamic_lookup)
elseif(MSVC)
    target_link_options(${PLUGIN_NAME} PUBLIC "LINKER:/FORCE:UNRESOLVED")
else()
    target_link_options(${PLUGIN_NAME} PUBLIC "LINKER:--allow-shlib-undefined")
endif()
target_link_libraries(${PLUGIN_NAME} binaryninjaapi ${UI_PLUGIN_LIBRARIES})

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    target_compile_options(${PLUGIN_NAME} PRIVATE "-fPIC")
endif()

list(APPEND fcl ${_PLUGIN_SOURCE})
list(LENGTH fcl file_count)
message(STATUS "")

if (UI_BUILD)
    message(STATUS "BinaryNinja UI Plugin Config -=-=-=-======")
else()
    message(STATUS "BinaryNinja Headless Plugin Config -=-=-=-======")
endif()

message(STATUS "Project Name: ${PLUGIN_NAME}")
message(STATUS "Tracked Source Files: ${file_count}")
message(STATUS "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-======")

message(STATUS "")

# Support cmake --install
bn_install_plugin(${PLUGIN_NAME})