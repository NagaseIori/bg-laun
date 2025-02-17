# bg-laun

一个后台启动工具。

主要用于 Windows 的自动计划任务。

## 功能

- **阻塞模式**：使用 `--blocking` 参数，启动子进程后等待其退出，任务计划停止时子进程也被终止。
- **非阻塞模式**：不加 `--blocking` 参数，启动子进程后立即退出，让子进程独立运行。

加上 `--console` 参数可在运行时分配控制台窗口用于调试。

## 使用方法

### 参数说明

- `--blocking`  
  阻塞模式，Launcher 会等子进程结束后才退出。

- `--console`  
  分配控制台窗口，显示标准输入/输出。

- `--help`  
  显示帮助信息。

其余参数将作为要启动的命令，例如：
```shell
bg-laun --blocking notepad.exe
bg-laun --console notepad.exe
bg-laun notepad.exe
```

## 编译方法

本项目采用 CMake 构建，需要 Windows 开发环境（如 Visual Studio）和 CMake 3.10+。

1. 克隆仓库：
   ```shell
   git clone <repo_url>
   ```
2. 创建构建目录并编译：
   ```shell
   mkdir build && cd build
   cmake -DCMAKE_BUILD_TYPE=Release ..
   cmake --build . --config Release
   ```
编译完成后，生成文件为 `bg-laun.exe`。

## 其它

- 默认编译为 GUI 应用（无控制台），如需查看日志请加 `--console` 参数。
- 启动的应用为后台应用，故不会显示任何窗口。如果非阻塞地启动后台应用，这一应用只能被强制终止（通过任务管理器等）。