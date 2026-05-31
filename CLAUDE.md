# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

TeamSpeak 3 语音监控插件，用于监听频道内用户的语音活动并实时显示发言者昵称。使用 C 语言编写，基于 TeamSpeak 3 Plugin SDK。

## 构建命令

### Windows
```bash
# 使用构建脚本
build.bat

# 手动构建
cmake -B build -G "MinGW Makefiles" .
cmake --build build --config Release
```

### Linux/macOS
```bash
# 使用构建脚本
chmod +x build.sh
./build.sh

# 手动构建
cmake -B build .
cmake --build build --config Release
```

### 输出文件
- Windows: `build/VoiceMonitorPlugin_win64.dll`
- Linux: `build/VoiceMonitorPlugin_linux_amd64.so`
- macOS: `build/VoiceMonitorPlugin_mac.dylib`

## 前置依赖

- CMake 3.10+
- TeamSpeak 3 Plugin SDK (位于 `../ts3client-pluginsdk-master`)
- Windows: MinGW-w64 或 Visual Studio
- Linux: GCC/G++
- macOS: Xcode Command Line Tools

## 架构说明

### 核心文件
- `src/plugin.c` - 插件主要实现，包含所有回调函数
- `src/plugin.h` - 插件头文件，定义导出函数声明
- `CMakeLists.txt` - CMake 构建配置

### 关键回调函数
- `ts3plugin_onTalkStatusChangeEvent()` - **正确的语音监控方式**，当用户开始/停止发言时触发
- `ts3plugin_onEditPlaybackVoiceDataEvent()` - **必须保持为空实现**！此回调每秒触发数百次，调用 printMessageToCurrentTab 会导致 TS3 崩溃
- `ts3plugin_setFunctionPointers()` - 接收 TeamSpeak SDK 函数指针
- `ts3plugin_init()` / `ts3plugin_shutdown()` - 插件生命周期管理

### SDK 函数使用
- `ts3Functions.getClientDisplayName()` - 获取用户显示名称（推荐，更高效）
- `ts3Functions.getClientVariableAsString()` - 获取用户昵称
- `ts3Functions.printMessageToCurrentTab()` - 输出消息到聊天窗口
- `ts3Functions.freeMemory()` - 释放 SDK 分配的内存

### 必需的头文件
```c
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_errors.h"
#include "teamspeak/public_rare_definitions.h"
#include "teamspeak/public_errors_rare.h"
#include "ts3_functions.h"
```

## 安装部署

将构建产物复制到 TeamSpeak 3 插件目录：
- Windows: `%APPDATA%\TS3Client\plugins\`
- Linux: `~/.ts3client/plugins/`
- macOS: `~/Library/Application Support/TS3Client/plugins/`

重启 TeamSpeak 3 客户端后在 设置 > 插件 中启用。

## 关键警告

**绝对不能在 `ts3plugin_onEditPlaybackVoiceDataEvent` 中调用 `printMessageToCurrentTab`！**
- 此回调在每个语音数据包时触发（每秒数百次）
- 在此回调中调用 UI 函数会导致 TeamSpeak 3 客户端崩溃
- 必须使用 `ts3plugin_onTalkStatusChangeEvent` 来监控语音状态变化

## 注意事项

- 插件 API 版本: 26
- Windows 平台使用 `wcharToUtf8` 进行字符编码转换
- 所有 TeamSpeak 回调函数必须实现（即使为空实现），否则插件加载会失败
- 使用 `ts3Functions.getClientDisplayName()` 而不是 `getClientVariableAsString()` 获取用户名更高效
