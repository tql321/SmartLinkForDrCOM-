# 🌐 SmartLinkForDrCOM (校园网无感认证助手)

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey.svg)
![Qt](https://img.shields.io/badge/Qt-5.12+-green.svg)

SmartLinkForDrCOM 是一款专为广西师范大学校园网 （及DrCOM 哆点认证系统的学校环境）设计的自动化登录与保活客户端。它能够智能探测网络状态，支持 Windows 桌面图形界面以及 Linux 服务器，让你告别频繁手动登录校园网的烦恼。

## ✨ 主要特性

*   **自动登录**：智能检测校园网未认证状态（通过网关及 204 状态测试），并自动提交凭据登录。
*   **验证服务器地址一键解析**：自动解析并验证 DrCOM 认证服务器地址。
*   **断线重连**：实时侦测底层网络连通性，遭遇掉线时可强制重新连接。
*   **稳定保活**：支持自定义心跳时间间隔，通过轻量级探测或者模拟网页浏览请求防止被网关踢出休眠。
*   **单例防护**：采用安全的进程锁（跨平台兼容），避免重复启动占用资源。
** **跨平台支持**：完美兼容 Windows 环境与 Linux 系统 (Ubuntu 等主流发行版)。
* **轻量级 UI**：基于 C++ 与 Qt 构建，内存占用极低，支持系统托盘后台运行。

## 🛠️ 编译与构建

项目基于 CMake 与 Qt 构建系统，前置要求：
*   C++17 编译器 (MSVC / GCC / Clang)
*   CMake >= 3.16
*   Qt 5 (包含必要模块: Core, Gui, Widgets, Network)
*   OpenSSL (用于处理 HTTPS 的网关认证请求)

### 构建步骤 (通用)

```bash
mkdir build
cd build
cmake ..
cmake --build .
```
*(注：Windows 平台下的打包已在 CMake 中自动集成 `windeployqt` 以及 OpenSSL DLL 的拷贝)。*
## 🚀 下载

### Windows 用户
1. 前往 [Releases](../../releases) 页面。
2. 下载最新的 Windows 压缩包，解压后双击运行即可。

### Linux 用户 (AppImage 免安装版)
1. 前往 [Releases](../../releases) 页面，下载最新的 `SmartLinkForDrCOM-x86_64.AppImage`。
2. 在终端中赋予执行权限并运行：
## 🚀 使用指南

### 1. 图形界面模式 (Windows / Linux)
直接双击运行程序，在界面中：
1.在校园网需要验证身份的网络环境下解析登录接口地址
2. 填入你的账号和密码及网络类型（校园网，电信，联通，移动）。
3. 根据需要勾选“自动登录”和“掉线强制重连”。
4. 点击登录即可完成配置并生效。

程序会自动将最新登录成功的账户凭据安全存储在本地，并在运行时常驻系统托盘，不打扰你的正常工作。


## 📝 注意事项

*   *   *首次使用*：默认验证服务器地址为广西师大育才校区的 DrCOM 认证服务器地址。如学校和校区不是育才校区，请在校园网需要验证身份的网络环境下解析登录接口地址。
    *   *Windows*：编译时会自动将 `libcrypto` 和 `libssl` 拷贝到运行目录。
    *   *Linux*：如果出现 SSL 握手失败报错，请安装 OpenSSL (例如 `sudo apt install libssl1.1` 或 `libssl3` 根据 Qt5 的编译依赖而定)。
*   **多网卡或虚拟机代理干扰**：程序已内置对常见虚拟网卡（VMware、VPN、WSL、Clash 等）的屏蔽，如果依然遇到探测不准的情况，请排查相关路由规则。

## 📜 许可证

本项目采用 [MIT License](LICENSE) 授权。欢迎提交 Issue 或 Pull Request 改善本项目！
