cmake_minimum_required(VERSION 3.16)
project(SmartLinkForDrCOM LANGUAGES CXX)
set(MY_ORG_NAME "tql")
set(MY_APP_NAME "SmartLinkForDrCOM")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Network)

file(GLOB_RECURSE PROJECT_SOURCES
    "*.cpp"
    "*.h"
    "*.ui"
    "*.qrc"
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} "core/KeepLiveManager.cpp" "core/NetworkDetector.cpp" "core/NetworkDetector.h")

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_autogen/include
)

target_compile_definitions(${PROJECT_NAME} PRIVATE 
    ORG_NAME="${MY_ORG_NAME}"
    APP_NAME="${MY_APP_NAME}"
)
if(WIN32)
set(OPENSSL_DLL_CRYPTO "${CMAKE_SOURCE_DIR}/openssl1.1.1-x64/libcrypto-1_1.dll")
set(OPENSSL_DLL_SSL "${CMAKE_SOURCE_DIR}/openssl1.1.1-x64/libssl-1_1.dll")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${OPENSSL_DLL_CRYPTO}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${OPENSSL_DLL_SSL}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${OPENSSL_DLL_CRYPTO}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/libcrypto-1_1-x64.dll
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${OPENSSL_DLL_SSL}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/libssl-1_1-x64.dll
)
endif()
if (MSVC)
target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
endif()

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        WIN32_EXECUTABLE TRUE
)

# 添加 windeployqt 自动打包步骤
if(WIN32)
    # 查找 windeployqt
    get_target_property(_qt5_core_location Qt5::Core LOCATION)
    get_filename_component(_qt5_bin_dir ${_qt5_core_location} DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt5_bin_dir}")

    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
            "--no-translations"
            "--no-compiler-runtime"
            "--no-system-d3d-compiler"
            "--no-opengl-sw"
            "$<TARGET_FILE:${PROJECT_NAME}>"
            COMMENT "Running windeployqt for ${PROJECT_NAME}..."
        )
    else()
        message(WARNING "windeployqt not found!" " ")
    endif()
endif()

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Network
)
