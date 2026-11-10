"""FFT 分析脚本：抓相电流，检查电机本体的 5/7 次空间谐波。

用法（在 host 目录下运行）：
    python fft.py <转速(r/s)> <极对数> [帧数]

    例：
    python fft.py 20 7        # 20 r/s，7 对极，抓 8 帧（2048 点）
    python fft.py 20 7 32     # 抓 32 帧（8192 点），频率分辨率更细

前提：
    1. 设备已连接（脚本会自己找 VID/PID 连接）。
    2. 电机已经在某个稳定模式跑着（如 IF 开环、给定了 input_vel）。
    3. 需要 numpy（pyqtgraph 的依赖，一般已装）和 pyqtgraph。

输出：
    - 控制台打印基波 / 5 次 / 7 次谐波的频率、幅度、相对基波百分比。
    - 弹出一个频谱图窗口（横轴频率 Hz，纵轴幅度 A）。
"""

import sys

import numpy as np
import serial
import serial.tools.list_ports

import mydrive


TARGET_VID = 0xCAFE
TARGET_PID = 0x4001
FS = 8000.0               # OSC 采样率 8kHz（125us/点）
FRAME_SAMPLES = 256       # 每帧采样点数
KW_IU = 0x14              # 相电流 iu 的关键字


def find_port():
    ports = [p.device for p in serial.tools.list_ports.comports()
             if p.vid == TARGET_VID and p.pid == TARGET_PID]
    return ports[0] if ports else None


def connect():
    if mydrive.LINK is not None and mydrive.LINK.is_open:
        return
    dev = find_port()
    if dev is None:
        raise RuntimeError("未找到设备（VID/PID 不匹配），请确认 USB 已连接")
    mydrive.LINK = serial.Serial(dev, 115200, timeout=0.2)
    print("已连接", dev)


def capture(keyword, n_frames):
    """抓 n_frames 帧 OSC，拼接成一段连续样本。"""
    data = []
    mydrive.LOG_QUIET = True          # 关掉每帧的日志刷屏
    for _ in range(n_frames):
        channels = mydrive.osc_capture([keyword])
        if not channels:
            raise RuntimeError("抓取失败（设备没回复）")
        data.extend(channels[0])
    mydrive.LOG_QUIET = False
    return np.asarray(data, dtype=float)


def _peak_mag(freq, mag, f_target, search_hz):
    """在目标频率 ±search_hz 范围内找最大幅度，返回 (频率, 幅度)。"""
    df = freq[1] - freq[0]                    # 频率分辨率
    center = int(round(f_target / df))        # 目标频率最近的 bin
    half = max(1, int(search_hz / df))        # 搜索半径（bin 数）
    lo = max(0, center - half)
    hi = min(len(freq) - 1, center + half)
    idx = lo + int(np.argmax(mag[lo:hi + 1]))
    return freq[idx], mag[idx]


def fft_report(samples, speed, pole_pairs):
    """做 FFT，打印基波 / 5 次 / 7 次谐波，返回 (freq, mag) 供绘图。"""
    N = len(samples)
    samples = samples - samples.mean()              # 去直流偏置
    window = np.hanning(N)
    spec = np.fft.rfft(samples * window)
    freq = np.fft.rfftfreq(N, d=1.0 / FS)
    mag = np.abs(spec) * 2.0 / N                   # 归一化成幅度（约等于 A）

    f_elec = speed * pole_pairs                     # 电频率 Hz
    res = FS / N
    search = 2 * res                               # 峰搜索范围

    print("=" * 60)
    print("转速 %.1f r/s, 极对数 %d, 电频率 %.1f Hz" % (speed, pole_pairs, f_elec))
    print("采样 %d 点 @ %.0f Hz, 频率分辨率 %.2f Hz" % (N, FS, res))
    print("-" * 60)
    print("%-6s %-10s %-10s %-10s" % ("谐波", "频率(Hz)", "幅度(A)", "相对基波"))

    f0, m0 = _peak_mag(freq, mag, f_elec, search)
    print("%-6s %-10.1f %-10.4f %-10s" % ("基波", f0, m0, "100%"))

    for k, name in [(5, "5次"), (7, "7次")]:
        fk, mk = _peak_mag(freq, mag, k * f_elec, search)
        ratio = mk / m0 * 100 if m0 > 1e-9 else 0.0
        print("%-6s %-10.1f %-10.4f %-9.1f%%" % (name, fk, mk, ratio))

    print("-" * 60)
    print("判读：5 次 / 7 次处有明显幅度（相对基波几个 %% 以上）→ 电机本体有 5/7 次空间谐波。")
    return freq, mag


def plot_spectrum(freq, mag, marks):
    """用 pyqtgraph 画频谱，标记基波 / 5 次 / 7 次位置。"""
    import pyqtgraph as pg
    from PyQt5.QtWidgets import QApplication

    app = QApplication.instance() or QApplication([])
    win = pg.GraphicsLayoutWidget()
    win.setWindowTitle("相电流频谱")
    plot = win.addPlot()
    plot.setLabel("bottom", "频率", units="Hz")
    plot.setLabel("left", "幅度", units="A")
    plot.plot(freq, mag, pen="y")

    for f, name, color in marks:
        line = pg.InfiniteLine(pos=f, angle=90, pen=pg.mkPen(color, style=pg.QtCore.Qt.DashLine))
        plot.addItem(line)
        txt = pg.TextItem(name, color=color, anchor=(0, 1))
        txt.setPos(f, 0)
        plot.addItem(txt)

    win.show()
    app.exec_()


def main():
    speed = float(sys.argv[1])
    pole_pairs = int(sys.argv[2])
    n_frames = int(sys.argv[3]) if len(sys.argv) > 3 else 8

    connect()
    samples = capture(KW_IU, n_frames)
    freq, mag = fft_report(samples, speed, pole_pairs)

    f_elec = speed * pole_pairs
    marks = [(f_elec, "基波", "y"), (5 * f_elec, "5次", "r"), (7 * f_elec, "7次", "c")]
    plot_spectrum(freq, mag, marks)


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("用法: python fft.py <转速(r/s)> <极对数> [帧数]")
        print("例:   python fft.py 20 7")
        sys.exit(1)
    main()
