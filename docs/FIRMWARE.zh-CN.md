# AutoJam 固件与工程说明

## 安装到设备

适用目标为预装 Fingersonic App1 更新器的 **Fingersonic Guitar，ESP32-S3、16 MB Flash、OPI PSRAM**。AutoJam 占用 App0。已经装好的设备可以直接看[新手入门](QUICKSTART.zh-CN.md)。

1. 在可上网时先下载发布页的 **AutoJam.ino.bin**，保存到电脑。发布页还提供 SHA-256 校验文件。[下载固件](https://github.com/nasonliu/AutoJam/releases/latest)
2. 进入设备的 App1 更新器。如果已在 AutoJam，长按摇杆约一秒进入系统设置，上下拨摇杆找到 **UPLOADER**，按摇杆选择，再按一次确认。也可以依照开发套件说明，断开充电，在真正关机后按住旋钮冷启动。[上游更新与恢复指引](https://github.com/lastness09/fingersonic-guitar-devkit/tree/main/recovery)
3. 电脑连接设备发出的 **Fingersonic** 热点，出厂密码为 **88888888**。这时电脑可能暂时离线，刚才下载好的文件可以直接用。在固件所在目录执行：

   ```sh
   curl --fail -F 'file=@AutoJam.ino.bin' http://192.168.4.1/upload
   ```

4. 等待上传结束和设备重启。屏幕经过 AutoJam 开屏后进入演奏主页；电脑可以连回平时使用的网络。此后设备可以独立演奏。

**选择文件时只选 App0 应用镜像 AutoJam.ino.bin。** 通用 ESP32 的整片擦除和合并镜像烧录会影响出厂引导及 App1 恢复分区。需要恢复原厂乐器功能时，请使用[厂商恢复说明](https://github.com/lastness09/fingersonic-guitar-devkit/tree/main/recovery)列出的工具与镜像。

## 从源码编译

克隆后保持工程目录名为 AutoJam，Arduino 草图入口与目录同名。下面是已验证的构建配置。使用 Arduino CLI 1.4.1、Arduino ESP32 Core 3.3.2。Adafruit 依赖单独安装。

```sh
git clone https://github.com/nasonliu/AutoJam.git
cd AutoJam
index=https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core update-index --additional-urls "$index"
arduino-cli core install esp32:esp32@3.3.2 --additional-urls "$index"
arduino-cli lib install 'Adafruit BusIO@1.17.4' 'Adafruit MPR121@1.1.3' 'Adafruit GFX Library@2.0.0' 'Adafruit ST7735 and ST7789 Library@1.11.0'
board='esp32:esp32:esp32s3:USBMode=default,CDCOnBoot=cdc,CPUFreq=240,FlashMode=qio,FlashSize=16M,PartitionScheme=custom,DebugLevel=none,PSRAM=opi,LoopCore=1,EventsCore=1'
arduino-cli compile --jobs 3 --fqbn "$board" --build-path build .
```

Windows 安装相同依赖后可运行 `./build.ps1`。输出文件为 `build/AutoJam.ino.bin`，应小于等于 2 MiB；仓库内的 partitions.csv 定义了 App0 和 App1 的位置。

## 音乐与交互

- **AUTO** 按风格和 P1/P2 自动走和弦。录入动机后先呈现一到两小节的原节奏，再结合当前和弦、前句尾音、音区和曲风生成回答。曲风会影响发音长短、重复、留白和收尾。
- **LOOP** 由摇杆选择下一小节的和弦。按弦、松弦按最近的 1/16 网格量化；跨线仍按住时，录入延伸至第二小节。引擎保存近期演奏片段，匹配节奏和音高走向寻找接句。手敲的鼓、军鼓和镲也可提供旋律节奏。
- **调性与音色** 通过演奏设置的 SCALE、KEY、LEAD、LD LVL 修改。选择 G 大调时可用 Pop、MAJOR、KEY G：P1 显示 G–D–Em–C，八方向和弦与生成声部随之移调。

| 文件 | 职责 |
|---|---|
| AutoJam.ino | Arduino 启动入口 |
| Jam.cpp / Jam.h | 和弦进行、调式、动机录入、量化与乐句生成 |
| GrooveCore.cpp / .h | 32 kHz 音频渲染、鼓与 FM 声部、音量及播放时序 |
| GrooveRuntime.cpp / .h | 触摸、摇杆、编码器、设备任务、I²S 与电源状态 |
| GrooveDisplay.cpp、RingLedOutput.cpp | 开屏、演奏界面、轨道图与环形灯 |
| config.h、partitions.csv | 板级引脚及分区 |
| tests/ | 音乐逻辑回归、DSP 测试与设备界面截图渲染 |

## 在电脑上验证

从仓库根目录运行以下命令。第一组只需要 C++17 编译器；第二组需要 Python、Pillow 和 Adafruit GFX 中的 glcdfont.c。

```sh
mkdir -p build
c++ -std=c++17 -O2 tests/JamTests.cpp GrooveCore.cpp Jam.cpp src/DaisySP/Control/*.cpp src/DaisySP/Synthesis/*.cpp -o build/jam-tests
./build/jam-tests
python3 -m pip install -r tests/requirements.txt
python3 tests/test.py --gfx /path/to/directory/containing/glcdfont.c
```

音乐回归覆盖 14 种风格、移调、跨小节延音和 AUTO/Loop 动机发展。显示测试在 build/ 中生成 PNG，可以检查开屏、录入提示和八方向和弦图。最终触控手感、扬声器和灯环的物理朝向以实机为准。
