"""实时绘图：从设备读取参数并滚动显示。

用法（main.py 里输入）：
    plot[mydrive.a]                单变量，时间模式
    plot[mydrive.a, mydrive.b]     a、b 两条时间曲线（都为 y，时间为 x）
    plot[xy: mydrive.a, mydrive.b]  XY 模式（a 作 x，b 作 y）
    osc[mydrive.a]                 单通道时间波形
    osc[mydrive.a, mydrive.b]      a、b 两条时间曲线
    osc[xy: mydrive.a, mydrive.b]  双通道 XY 图（a 作 x，b 作 y）
    含 '=' 的项是赋值，开始前下发一次，如 plot[a=1, b]、osc[a=1, b]
"""
import sys
import time

import pyqtgraph as pg
from PyQt5.QtWidgets import QApplication
from PyQt5.QtCore import QTimer

import mydrive

# QApplication 全进程只建一次，跨多次绘图复用
_app = None

# plot 模式（慢速轮询）滚动窗口总时长（秒）
PLOT_WINDOW = 5.0

# 示波器模式滚动窗口总时长（秒）：跨帧累计，超过该时长的历史波形被丢弃
OSC_WINDOW = 0.132

# 示波器 XY 模式滚动保留的最大点数（无时间轴，按点数限长）
OSC_XY_MAX_POINTS = 4096

# 连续多少秒没收到数据就自动关窗（设备失联）
DISCONNECT_TIMEOUT = 3.0


def _link_ok():
    """设备链路是否还活着（串口未关闭）。"""
    return mydrive.LINK is not None and mydrive.LINK.is_open


def _keyword_of(path):
    """把 "mydrive.a" 解析成该参数的关键字。

    从「类」出发导航：类访问触发 __get__ 时 obj 为 None，描述符返回自身，
    因此这里只取 .keyword,不会真正下发查询。
    """
    parts = path.rsplit('.',1)              #从右往左按'.'拆分1次
    if parts[0] not in mydrive.NAMESPACE:
        raise ValueError("不存在对象: " + parts[0])
    obj = mydrive.NAMESPACE[parts[0]]
    desc = getattr(type(obj), parts[1])     # 类访问 → 返回描述符，不查询
    return obj.kw_base + desc.keyword


def _split_specs(param):
    """把 'a=1, b' 拆成 (sets, paths)。

    含 '=' 的项视为赋值（形如 'mydrive.a=1'），其余为绘图/示波路径。
    返回 (sets, paths)：sets = [(path, value), ...]，paths = [path, ...]
    """
    sets = []
    paths = []
    for s in param.split(','):
        s = s.strip()
        if not s:
            continue
        if '=' in s:
            path, val = s.split('=', 1)
            sets.append((path.strip(), val.strip()))
        else:
            paths.append(s)
    return sets, paths


def _parse_param(param):
    """解析 'xy: a=1, b' → (xy, sets, paths)。

    前缀 'xy:' 表示 XY 模式（恰好 2 个路径），否则为时间模式（多变量都作 y）。
    """
    param = param.strip()
    xy = False
    if param.lower().startswith("xy:"):
        xy = True
        param = param[3:].strip()
    sets, paths = _split_specs(param)
    return xy, sets, paths


def _resolve_set(path, value):
    """把 'mydrive.a' 解析成 (obj, leaf, value)，用于 setattr 赋值。

    校验对象/属性是否存在，并按 dtype 提前把值转成 float/int，
    坏值在进入绘图循环前就报错。
    """
    prefix, leaf = path.rsplit('.', 1)
    if prefix not in mydrive.NAMESPACE:
        raise ValueError("不存在对象: " + prefix)
    obj = mydrive.NAMESPACE[prefix]
    desc = getattr(type(obj), leaf, None)
    if desc is None or not hasattr(desc, 'keyword'):
        raise ValueError("不存在属性: " + leaf)
    try:
        val = float(value) if desc.dtype == mydrive.p.TYPE_FLOAT else int(value)
    except (TypeError, ValueError):
        raise ValueError("无法解析赋值: %s=%s" % (path, value))
    return obj, leaf, val


def run_plot(param):
    """实时绘图（慢速轮询）。

    'xy:' 前缀 = XY 模式，否则时间模式（所有变量都作 y，时间为 x）。
    含 '=' 的项是赋值，开始时下发一次。
    """
    global _app

    # 1. 解析参数
    xy, sets, paths = _parse_param(param)
    if xy and len(paths) != 2:
        raise ValueError("XY 模式需要恰好 2 个变量")
    set_items = [_resolve_set(p, v) for p, v in sets]   # 进入循环前校验好
    keywords = [_keyword_of(s) for s in paths]

    # 2. 绘图期间静默串口日志，退出后恢复
    mydrive.LOG_QUIET = True
    if mydrive.LINK:
        mydrive.LINK.reset_input_buffer()   # 进绘图前清空残留，避免干扰

    # 赋值项只在开始时下发一次（无需重复发送）
    try:
        for obj, leaf, val in set_items:
            setattr(obj, leaf, val)
    except Exception as e:
        print("赋值失败:", e)

    if _app is None:
        _app = QApplication(sys.argv)

    # 3. 建窗口、曲线
    win = pg.GraphicsLayoutWidget()
    win.setWindowTitle("实时绘图")
    plot = win.addPlot()

    colors = ["y", "r", "c", "g", "m", "w"]

    if xy:
        # XY 模式：第 1 个作 x 轴，第 2 个作 y 轴
        plot.setLabel("bottom", paths[0].rsplit('.', 1)[-1])
        plot.setLabel("left", paths[1].rsplit('.', 1)[-1])
        xy_curve = plot.plot(pen="y")
        data_x = []
        data_y = []
    else:
        # 时间模式：所有变量都作 y，时间为 x
        plot.setLabel("bottom", "时间", units="s")
        curves = [plot.plot(pen=colors[i % len(colors)]) for i in range(len(keywords))]
        data_t = []
        data_chs = [[] for _ in keywords]
        start = time.time()

    last_data = time.time()    # 距上次成功收包的时间戳，用于超时关窗

    def update():
        nonlocal last_data
        if not _link_ok():
            win.close()
            return
        if time.time() - last_data > DISCONNECT_TIMEOUT:
            print("连续 %.0f 秒无数据，关闭绘图窗口" % DISCONNECT_TIMEOUT)
            win.close()
            return
        try:
            vals = mydrive.query_multi(keywords)
        except Exception as e:
            print("设备断开，关闭绘图窗口:", e)
            win.close()
            return
        if not vals:
            return
        last_data = time.time()
        if xy:
            if len(vals) < 2:
                return                    # 失步：回复不足两个值，跳过这一拍
            data_x.append(vals[0])
            data_y.append(vals[1])
            if len(data_x) > 500:
                data_x.pop(0)
                data_y.pop(0)
            xy_curve.setData(data_x, data_y)
        else:
            t_now = time.time() - start
            data_t.append(t_now)
            for ch_buf, v in zip(data_chs, vals):
                ch_buf.append(v)
            while data_t and data_t[0] < t_now - PLOT_WINDOW:
                data_t.pop(0)
                for ch_buf in data_chs:
                    ch_buf.pop(0)
            for crv, ch_buf in zip(curves, data_chs):
                crv.setData(data_t, ch_buf)

    timer = QTimer()
    timer.timeout.connect(update)
    timer.start(20)

    win.show()
    _app.exec_()                    # 阻塞，关窗后返回
    if mydrive.LINK:
        try:
            mydrive.LINK.reset_input_buffer()   # 丢掉最后一次查询的残留回复
        except Exception:
            pass                     # 拔线后 reset 会抛异常，忽略，交给主循环重连
    mydrive.LOG_QUIET = False       # 恢复串口日志


def run_osc(param):
    """示波器模式：连续采集并滚动显示。

    默认时间模式：所有通道都作 y，时间为 x（多通道叠加）。
    'xy:' 前缀：XY 模式（恰好 2 个通道，第 1 个作 x，第 2 个作 y，利萨如图）。
    含 '=' 的项是赋值，开始时下发一次。
    """
    global _app

    xy, sets, paths = _parse_param(param)
    if xy and len(paths) != 2:
        raise ValueError("XY 模式需要恰好 2 个变量")
    set_items = [_resolve_set(p, v) for p, v in sets]
    keywords = [_keyword_of(s) for s in paths]

    mydrive.LOG_QUIET = True
    if mydrive.LINK:
        mydrive.LINK.reset_input_buffer()   # 进示波器前清空残留

    # 赋值项只在开始时下发一次（无需重复发送）
    try:
        for obj, leaf, val in set_items:
            setattr(obj, leaf, val)
    except Exception as e:
        print("赋值失败:", e)

    if _app is None:
        _app = QApplication(sys.argv)

    win = pg.GraphicsLayoutWidget()
    win.setWindowTitle("示波器")
    plot = win.addPlot()

    DT = 125e-6          # 采样间隔：两点间 125us

    colors = ["y", "r", "c", "g", "m", "w"]

    if xy:
        # XY 模式：第 1 个通道作 x 轴，第 2 个通道作 y 轴（利萨如图）
        plot.setLabel("bottom", paths[0].rsplit('.', 1)[-1])
        plot.setLabel("left", paths[1].rsplit('.', 1)[-1])
        xy_curve = plot.plot(pen="y")
        data_x = []
        data_y = []
    else:
        plot.setLabel("bottom", "时间")
        plot.getAxis("bottom").enableAutoSIPrefix(True)   # 毫秒级自动加 m 前缀
        curves = [plot.plot(pen=colors[i % len(colors)]) for i in range(len(keywords))]
        # 滚动缓冲：跨帧累计波形，时间轴为相对 start 的真实时间（秒）
        start = time.time()
        data_t = []
        data_chs = [[] for _ in keywords]

    last_data = time.time()    # 距上次成功收包的时间戳，用于超时关窗

    def update():
        nonlocal last_data
        if not _link_ok():
            win.close()
            return
        if time.time() - last_data > DISCONNECT_TIMEOUT:
            print("连续 %.0f 秒无数据，关闭示波器窗口" % DISCONNECT_TIMEOUT)
            win.close()
            return
        try:
            channels = mydrive.osc_capture(keywords)
        except Exception as e:
            print("设备断开，关闭示波器窗口:", e)
            win.close()
            return
        if not channels:
            return
        last_data = time.time()

        if xy:
            if len(channels) < 2:
                return
            data_x.extend(channels[0])
            data_y.extend(channels[1])
            # XY 无时间轴，按点数限长，保留最近 OSC_XY_MAX_POINTS 个点
            excess = len(data_x) - OSC_XY_MAX_POINTS
            if excess > 0:
                del data_x[:excess]
                del data_y[:excess]
            xy_curve.setData(data_x, data_y)
        else:
            n = len(channels[0])
            t_now = time.time() - start
            # 帧内各点真实时间：最新点对齐到 t_now，往前按 DT 排（保持升序）
            data_t.extend(t_now - (n - 1 - i) * DT for i in range(n))
            for ch_buf, ch in zip(data_chs, channels):
                ch_buf.extend(ch)
            # 丢弃超出滚动窗口的历史（按时间阈值一次性裁掉，避免逐点 pop）
            cutoff = t_now - OSC_WINDOW
            i = 0
            while i < len(data_t) and data_t[i] < cutoff:
                i += 1
            if i:
                del data_t[:i]
                for ch_buf in data_chs:
                    del ch_buf[:i]
            for crv, ch_buf in zip(curves, data_chs):
                crv.setData(data_t, ch_buf)

    timer = QTimer()
    timer.timeout.connect(update)
    timer.start(1)                          # 尽快连续采集（每拍约 60ms）

    win.show()
    _app.exec_()                            # 阻塞，关窗后返回
    if mydrive.LINK:
        try:
            mydrive.LINK.reset_input_buffer()
        except Exception:
            pass                     # 拔线后 reset 会抛异常，忽略，交给主循环重连
    mydrive.LOG_QUIET = False
