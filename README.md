# TeamSpeak 3 Voice Monitor Plugin

一个简单的 TeamSpeak 3 插件，用于监听频道内用户的语音活动并实时显示发言者昵称。

## 功能特性

- 实时监听频道内所有用户的语音活动
- 当用户发言时，在聊天窗口显示 `[监听中] 用户 [昵称] 正在发言...`
- 支持 Windows、Linux、macOS 平台

## 项目结构

```
voice-monitor-plugin/
├── CMakeLists.txt          # CMake 构建配置
├── build.bat               # Windows 构建脚本
├── build.sh                # Linux/macOS 构建脚本
├── README.md               # 本文件
├── src/
│   ├── plugin.c            # 插件源代码
│   └── plugin.h            # 插件头文件
└── build/                  # 构建输出目录
```

## 前置要求

### Windows
- CMake 3.10+
- MinGW-w64 或 Visual Studio

### Linux
- CMake 3.10+
- GCC/G++

### macOS
- CMake 3.10+
- Xcode Command Line Tools

## 构建步骤

### Windows

```batch
# 方法 1: 使用构建脚本
build.bat

# 方法 2: 手动构建
cmake -B build -G "MinGW Makefiles" .
cmake --build build --config Release
```

### Linux/macOS

```bash
# 方法 1: 使用构建脚本
chmod +x build.sh
./build.sh

# 方法 2: 手动构建
cmake -B build .
cmake --build build --config Release
```

## 安装插件

1. 构建完成后，在 `build/` 目录下会生成插件文件：
   - Windows: `VoiceMonitorPlugin_win64.dll`
   - Linux: `VoiceMonitorPlugin_linux_amd64.so`
   - macOS: `VoiceMonitorPlugin_mac.dylib`

2. 将插件文件复制到 TeamSpeak 3 的 plugins 目录：
   - Windows: `%APPDATA%\TS3Client\plugins\`
   - Linux: `~/.ts3client/plugins/`
   - macOS: `~/Library/Application Support/TS3Client/plugins/`

3. 重启 TeamSpeak 3 客户端

4. 在 `设置` > `插件` 中启用 "Voice Monitor" 插件

## 使用方法

插件启用后，当频道内有用户发言时，会在当前聊天窗口自动显示：

```
[监听中] 用户 [某用户昵称] 正在发言...
```

## 技术说明

### 核心回调函数

插件使用 `ts3plugin_onEditPlaybackVoiceDataEvent` 回调函数来监听语音数据：

```c
void ts3plugin_onEditPlaybackVoiceDataEvent(
    uint64 serverConnectionHandlerID,  // 服务器连接句柄
    anyID clientID,                     // 发言用户 ID
    short* samples,                     // 音频采样数据
    int sampleCount,                    // 采样数量
    int channels                        // 声道数
)
```

### SDK 函数调用

- `ts3Functions.getClientVariableAsString()` - 获取用户昵称
- `ts3Functions.printMessageToCurrentTab()` - 打印消息到聊天窗口

## 注意事项

1. 此插件仅用于学习和测试目的
2. 在生产环境中使用时，请考虑添加适当的错误处理和性能优化
3. 频繁调用 `printMessageToCurrentTab` 可能会影响性能，建议添加节流机制

## 许可证

MIT License

## 相关资源

- [TeamSpeak 3 Plugin SDK](https://www.teamspeak.com/downloads#sdk)
- [TeamSpeak Community Forums](https://community.teamspeak.com)
- [TeamSpeak 3 Client Documentation](https://community.teamspeak.com/t/teamspeak-3-plugin-sdk/)
