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

# 查找 Qt5 及其所需组件
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Network)

# 递归获取 src 及其所有子目录下的源码、头文件和 UI 文件
file(GLOB_RECURSE PROJECT_SOURCES
    "*.cpp"
    "*.h"
    "*.ui"
)

# 添加可执行文件
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} "core/SystemTrayManager.h" "core/SystemTrayManager.cpp")

target_compile_definitions(${PROJECT_NAME} PRIVATE 
    ORG_NAME=\"${MY_ORG_NAME}\"
    APP_NAME=\"${MY_APP_NAME}\"
)
set(OPENSSL_DLL_CRYPTO "${CMAKE_SOURCE_DIR}/openssl1.1.1-x64/libcrypto-1_1.dll")
set(OPENSSL_DLL_SSL "${CMAKE_SOURCE_DIR}/openssl1.1.1-x64/libssl-1_1.dll")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    # 复制原名文件 (libcrypto-1_1.dll)
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${OPENSSL_DLL_CRYPTO}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${OPENSSL_DLL_SSL}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    # 复制并改名为带 -x64 后缀的文件
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${OPENSSL_DLL_CRYPTO}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/libcrypto-1_1-x64.dll
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${OPENSSL_DLL_SSL}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/libssl-1_1-x64.dll
    COMMENT "正在将 OpenSSL 动态库拷贝至输出目录..."
)
if (MSVC)
target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
endif()

# 屏蔽在 Windows 运行时的终端黑框
set_target_properties(${PROJECT_NAME}
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        WIN32_EXECUTABLE TRUE
)

# 链接对应的 Qt 库，因为是独立的可执行文件，使用 PRIVATE 权限更加规范
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Network
)
