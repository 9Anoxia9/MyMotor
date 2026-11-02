"""上位机主程序：发送查询并解析设备上报。"""
import os
import time
import serial
import serial.tools.list_ports

from prompt_toolkit import PromptSession
from prompt_toolkit.history import FileHistory

import mydrive
import plot

# 命令行历史文件：记录输入过的命令，支持上下键切换（跨运行保留）
HIST_FILE = os.path.join(os.path.expanduser("~"), ".mydrive_history")

# 带历史的输入会话：上下键翻历史、光标/复制粘贴等编辑体验，跨终端稳定
session = PromptSession(history=FileHistory(HIST_FILE))

BAUD = 115200        # USB CDC 会忽略，但 API 要求传一个 int
TIMEOUT = 0.2        # 读超时(秒)，用短超时配合轮询，见 send_and_wait

# 目标设备识别：USB CDC 自研设备，靠 VID/PID 定位（换 COM 号也能找到）
TARGET_VID = 0xCAFE
TARGET_PID = 0x4001
PREFERRED_PORT = "COM7"    # 多个匹配时优先


def find_port():
    """枚举匹配目标 VID/PID 的串口，优先返回 PREFERRED_PORT。"""
    ports = [p.device for p in serial.tools.list_ports.comports()
             if p.vid == TARGET_VID and p.pid == TARGET_PID]
    if not ports:
        return None
    if PREFERRED_PORT in ports:
        return PREFERRED_PORT
    return ports[0]


def link_alive():
    """设备是否在线（写探测，能可靠检测掉电/拔线）。

    in_waiting 读探测在设备掉电但 USB 句柄仍有效时不报错，会误判在线；
    改用 probe 发一个无害查询，靠写操作超时/失败来真实判断。
    """
    return mydrive.LINK is not None and mydrive.probe(mydrive.LINK)


def reconnect():
    """查找并打开目标设备，成功返回 True；失败置 LINK 为 None 并返回 False。"""
    if mydrive.LINK is not None:
        try:
            mydrive.LINK.close()      # 释放失效句柄，避免泄漏
        except Exception:
            pass
        mydrive.LINK = None

    dev = find_port()
    if dev is None:
        return False
    try:
        mydrive.LINK = serial.Serial(port=dev, baudrate=BAUD, timeout=TIMEOUT)
        print("已连接", dev)
        return True
    except Exception as e:
        print("打开端口失败:", e)
        mydrive.LINK = None
        return False


def main():
    while True:
        # 未连接（首次 / 拔线后）→ 尝试（重）连接
        if not link_alive():
            if not reconnect():
                print("未检测到设备，3 秒后重试...（Ctrl+C 退出）")
                time.sleep(1)
                continue

        try:
            line = session.prompt(">>>")
            if line in ("quit", "exit", "q"):
                break

            if line.startswith("plot["):
                p_str = line[line.index('[') + 1:line.index(']')]
                plot.run_plot(p_str)
                continue

            if line.startswith("osc["):
                p_str = line[line.index('[') + 1:line.index(']')]
                plot.run_osc(p_str)
                continue

            var = line.split()      # 将字符串空格分割
            if len(var) >= 3:
                print("error: 最多2个参数")
                continue

            var_name = var[0]
            var_val = var[1] if len(var) > 1 else None

            prefix,leaf = var_name.rsplit('.',1)    # 将字符串'.'分割

            if prefix not in mydrive.NAMESPACE:
                print("不存在对象:", prefix)
                continue
            obj = mydrive.NAMESPACE[prefix]

            if not hasattr(type(obj),leaf):
                print("不存在属性:",leaf)
                continue

            if var_val is None:
                getattr(obj, leaf)
            else:
                setattr(obj, leaf, var_val)

        except Exception as e:
            print("出错:", e)
            continue

    if mydrive.LINK is not None:
        try:
            mydrive.LINK.close()
        except Exception:
            pass
    print("已关闭")


if __name__ == "__main__":
    main()
