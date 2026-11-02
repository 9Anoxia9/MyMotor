"""协议层：帧的打包 / 解包

帧结构（小端）：
    [帧头 4B: 5A A5 5A A5] [长度 1B] [命令字 1B] [关键字 1B] [数据类型 1B] [数据 1/2/4B] [CRC8 1B]

长度 = 从命令字到数据末尾（不含 CRC)。
CRC8 = 多项式 0x07,初值 0x00,无反射,覆盖整帧（含帧头，直到 CRC 之前）。
"""
import struct

HEADER = b"\x5A\xA5\x5A\xA5"

# 命令字
CMD_QUERY = 0x01        # 查询（下发）
CMD_QUERY_REPLY = 0x81  # 回复查询（上报）
CMD_SET = 0x02          # 设置（下发）
CMD_SET_REPLY = 0x82    # 回复设置（上报）
CMD_OSC = 0x03          # 示波器（下发）
CMD_OSC_REPLY = 0x83    # 回复示波器（上报）

# 数据类型
TYPE_BOOL = 0x00
TYPE_U8 = 0x01
TYPE_U16 = 0x02
TYPE_U32 = 0x03
TYPE_FLOAT = 0x04

# 每种类型的数据字节数
TYPE_SIZE = {
    TYPE_BOOL: 1,
    TYPE_U8: 1,
    TYPE_U16: 2,
    TYPE_U32: 4,
    TYPE_FLOAT: 4,
}

# struct 打包格式，'<' 表示小端
TYPE_FORMAT = {
    TYPE_BOOL: "<B",
    TYPE_U8: "<B",
    TYPE_U16: "<H",
    TYPE_U32: "<I",
    TYPE_FLOAT: "<f",
}


def crc8(data):
    """CRC-8,poly 0x07,init 0x00,无反射,无输出异或。"""
    crc = 0x00
    for b in data:
        crc ^= b
        for _ in range(8):
            if crc & 0x80:
                crc = ((crc << 1) ^ 0x07) & 0xFF
            else:
                crc = (crc << 1) & 0xFF
    return crc


def _frame(cmd, rest):
    """组装完整帧。rest = 命令字之后的所有字节（关键字/类型/数据）。"""
    length = 1 + len(rest)                    # 长度 = 命令字 + 之后的字节
    head = HEADER + bytes([length])
    body = bytes([cmd]) + rest
    crc = crc8(head + body)
    return head + body + bytes([crc])


def build_query(keywords):
    """查询指令：只带关键字，一次可查一个或多个（最多 4 个）。

    keywords 可以是单个 int，或 int 列表/元组。"""
    if isinstance(keywords, int):
        keywords = [keywords]
    return _frame(CMD_QUERY, bytes(keywords))


def build_osc(keywords):
    """示波器指令：只带关键字，一次一个或多个通道。

    keywords 可以是单个 int，或 int 列表/元组。"""
    if isinstance(keywords, int):
        keywords = [keywords]
    return _frame(CMD_OSC, bytes(keywords))


def build_set(keyword, dtype, value):
    """设置指令：关键字 + 类型 + 数据。"""
    data = struct.pack(TYPE_FORMAT[dtype], value)
    return _frame(CMD_SET, bytes([keyword, dtype]) + data)


def _decode(dtype, data):
    """按类型把数据字节解码成 Python 值。"""
    fmt = TYPE_FORMAT[dtype]
    return struct.unpack(fmt, data[: TYPE_SIZE[dtype]])[0]


def parse_packet(buf):
    """从字节流里解析一帧。

    返回 (info, 剩余缓冲区):
      - info 为 None 表示数据还不完整或 CRC 错误，需继续收。
      - info 为 dict,字段:
          cmd —— 命令字
          查询回复(0x81): results = [ {keyword, dtype, value}, ... ] 列表
          设置回复(0x82): keyword / dtype / value(FALSE 时还有 error)
    """
    idx = buf.find(HEADER)
    if idx < 0:
        return None, b""
    buf = buf[idx:]
    if len(buf) < 6:                 # 帧头4 + 长度1 + 命令字1
        return None, buf
    length = buf[4]
    total = 4 + 1 + length + 1       # 帧头 + 长度 + payload(length 字节) + CRC
    if len(buf) < total:
        return None, buf             # 半包，继续等
    frame = buf[:total]
    if crc8(frame[:-1]) != frame[-1]:
        return None, buf[1:]         # CRC 错，丢弃 1 字节重新同步

    cmd = frame[5]
    payload = frame[6:5 + length]    # 关键字及之后（不含 CRC）

    # 回复示波器：标准帧后紧跟大数据包 [帧长度 2B][数据]
    if cmd == CMD_OSC_REPLY:
        rest = buf[total:]
        if len(rest) < 2:
            return None, buf                     # 数据包长度还没到
        data_len = struct.unpack("<H", rest[:2])[0]
        if len(rest) < 2 + data_len:
            return None, buf                     # 数据包未收全
        info = {"cmd": cmd, "keywords": list(payload), "data": rest[2:2 + data_len]}
        return info, rest[2 + data_len:]

    buf = buf[total:]
    info = {"cmd": cmd}

    if cmd == CMD_QUERY_REPLY:       # 回复查询：多个 [关键字+类型+数据] 拼接，按类型自定界
        results = []
        i = 0
        while i + 1 < len(payload):
            kw = payload[i]
            dtype = payload[i + 1]
            size = TYPE_SIZE[dtype]
            value = _decode(dtype, payload[i + 2 : i + 2 + size])
            results.append({"keyword": kw, "dtype": dtype, "value": value})
            i += 2 + size            # 关键字 1 + 类型 1 + 数据 size
        info["results"] = results

    elif cmd == CMD_SET_REPLY:       # 回复设置：关键字 + 类型(BOOL) + 数据
        info["keyword"] = payload[0]
        info["dtype"] = payload[1]
        ok = payload[2] == 0x01      # TRUE=0x01
        info["value"] = ok
        if not ok and len(payload) > 3:   # FALSE 时后跟错误码
            info["error"] = payload[3]

    return info, buf
