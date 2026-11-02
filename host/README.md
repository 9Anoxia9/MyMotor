# MyMotor 上位机

MyMotor 电机驱动器的上位机程序，通过 USB CDC 串口与设备通信，支持：
参数查询/设置、实时绘图、示波器采集。

## 环境

- Python 3.9+
- Windows / Linux / macOS

## 安装依赖

```bash
pip install -r requirements.txt
```

Windows 下用启动器：

```bash
py -m pip install -r requirements.txt
```

## 运行

```bash
py main.py
```

程序会按 VID/PID 自动枚举目标设备，连接后输入命令即可。

## 命令

### 查询 / 设置

```
mydrive.axis0.motor.iu          # 查询
mydrive.axis0.motor.iu 0.5      # 设置
```

### 实时绘图 plot

```
plot[mydrive.axis0.motor.iu]                              # 单变量时间曲线
plot[mydrive.axis0.motor.iu, mydrive.axis0.motor.iv]      # 两条时间曲线
plot[xy: mydrive.a, mydrive.b]                            # XY 图（a 作 x，b 作 y）
```

### 示波器 osc

```
osc[mydrive.axis0.motor.iu]                               # 单通道时间波形
osc[mydrive.axis0.motor.iu, mydrive.axis0.motor.iv]       # 多通道叠加
osc[xy: mydrive.a, mydrive.b]                             # XY 图
```

`[]` 里含 `=` 的项是赋值，在开始前下发一次：

```
plot[mydrive.axis0.motor.iu = 0.5, mydrive.axis0.motor.iv]
```

输入命令时可用上下键翻历史。

## 文件说明

| 文件 | 作用 |
|------|------|
| `main.py` | 主程序：串口连接、命令解析 |
| `mydrive.py` | 设备对象模型 / 命名空间 |
| `plot.py` | 实时绘图 / 示波器 |
| `protocol.py` | 协议层：帧打包 / 解包 |
| `make_doc.py` | 生成 Word 文档 |
| `协议定义.txt` | 通信协议说明 |
