# -*- coding: utf-8 -*-
"""生成《Python 上位机开发笔记（嵌入式工程师版）》.docx"""
from docx import Document
from docx.shared import Pt, RGBColor, Inches
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.oxml.ns import qn

doc = Document()

# ---------- 全局中文字体 ----------
style = doc.styles["Normal"]
style.font.name = "Calibri"
style.font.size = Pt(10.5)
style.element.rPr.rFonts.set(qn("w:eastAsia"), "微软雅黑")


def set_cn(run, size=10.5, bold=False, color=None):
    run.font.name = "Calibri"
    run.font.size = Pt(size)
    run.font.bold = bold
    run._element.rPr.rFonts.set(qn("w:eastAsia"), "微软雅黑")
    if color:
        run.font.color.rgb = RGBColor(*color)


def h1(text):
    p = doc.add_heading(level=1)
    r = p.add_run(text)
    set_cn(r, 16, True, (0x1F, 0x4E, 0x79))
    return p


def h2(text):
    p = doc.add_heading(level=2)
    r = p.add_run(text)
    set_cn(r, 13, True, (0x2E, 0x74, 0xB5))
    return p


def h3(text):
    p = doc.add_heading(level=3)
    r = p.add_run(text)
    set_cn(r, 11.5, True)
    return p


def para(text, bold=False):
    p = doc.add_paragraph()
    r = p.add_run(text)
    set_cn(r, 10.5, bold)
    return p


def bullet(text):
    p = doc.add_paragraph(style="List Bullet")
    r = p.add_run(text)
    set_cn(r, 10.5)
    return p


def code(text):
    """等宽字体代码块"""
    p = doc.add_paragraph()
    p.paragraph_format.left_indent = Inches(0.2)
    p.paragraph_format.space_before = Pt(2)
    p.paragraph_format.space_after = Pt(2)
    r = p.add_run(text)
    r.font.name = "Consolas"
    r.font.size = Pt(9.5)
    r._element.rPr.rFonts.set(qn("w:eastAsia"), "微软雅黑")
    r.font.color.rgb = RGBColor(0x20, 0x20, 0x20)
    return p


def table(headers, rows, widths=None):
    t = doc.add_table(rows=1, cols=len(headers))
    t.style = "Light Grid Accent 1"
    for i, htext in enumerate(headers):
        c = t.rows[0].cells[i]
        c.text = ""
        r = c.paragraphs[0].add_run(htext)
        set_cn(r, 10, True)
    for row in rows:
        cells = t.add_row().cells
        for i, val in enumerate(row):
            cells[i].text = ""
            r = cells[i].paragraphs[0].add_run(str(val))
            set_cn(r, 10)
    return t


# ================= 标题页 =================
title = doc.add_paragraph()
title.alignment = WD_ALIGN_PARAGRAPH.CENTER
tr = title.add_run("Python 上位机开发笔记")
set_cn(tr, 26, True, (0x1F, 0x4E, 0x79))

sub = doc.add_paragraph()
sub.alignment = WD_ALIGN_PARAGRAPH.CENTER
sr = sub.add_run("（嵌入式工程师版）")
set_cn(sr, 14, False, (0x66, 0x66, 0x66))

note = doc.add_paragraph()
note.alignment = WD_ALIGN_PARAGRAPH.CENTER
nr = note.add_run("USB CDC 上位机 · 自研二进制协议 · 查询/设置/绘图")
set_cn(nr, 10.5, False, (0x88, 0x88, 0x88))

doc.add_paragraph()

# ================= 一、项目架构 =================
h1("一、项目架构")

h2("1.1 文件结构")
code("""motor_tool/
├── main.py         # 入口：发查询、收包、打印
├── protocol.py     # 协议：打包/解包/CRC8
├── debug_rx.py     # 诊断脚本：打印原始字节
└── 协议定义.txt     # 协议文档""")

h2("1.2 技术栈")
table(
    ["库", "用途", "备注"],
    [
        ["pyserial", "串口通信（唯一必需核心）", "import serial"],
        ["struct", "二进制帧编解码（内置）", "值 ↔ 字节"],
        ["numpy", "数据处理（后续绘图用）", "暂未用到"],
        ["pyqtgraph", "实时绘图（后续）", "暂未用到"],
    ],
)

h2("1.3 关键决策")
bullet("通信接口：USB CDC（本质是虚拟串口，pyserial 即可）")
bullet("协议帧：二进制，小端字节序")
bullet("第一版形态：命令行跑通协议，之后再上 GUI")

doc.add_page_break()

# ================= 二、协议层 =================
h1("二、协议层（protocol.py）")

h2("2.1 帧结构（小端）")
code("""[帧头 4B: 5A A5 5A A5] [长度 1B] [命令字 1B] [关键字 1B] [数据类型 1B] [数据 1/2/4B] [CRC8 1B]""")

para("长度 = 从命令字到数据末尾（不含 CRC）。")

h2("2.2 命令字与数据类型")
table(
    ["类型", "值", "说明"],
    [
        ["命令字 查询", "0x01", "下发，只带关键字"],
        ["命令字 回复查询", "0x81", "上报，带关键字+类型+数据"],
        ["命令字 设置", "0x02", "下发，带关键字+类型+数据"],
        ["命令字 回复设置", "0x82", "上报，类型为 BOOL，FALSE 时后跟错误码"],
        ["数据类型 BOOL", "0x00", "1 字节"],
        ["数据类型 U8", "0x01", "1 字节"],
        ["数据类型 U16", "0x02", "2 字节"],
        ["数据类型 U32", "0x03", "4 字节"],
        ["数据类型 FLOAT", "0x04", "4 字节（IEEE754）"],
    ],
)

h2("2.3 CRC8 算法")
para("多项式 0x07，初值 0x00，无反射，无输出异或，覆盖整帧（含帧头，直到 CRC 之前）。这是从测试向量反推出来的标准教科书版 CRC-8。")
code("""def crc8(data):
    crc = 0x00
    for b in data:
        crc ^= b
        for _ in range(8):
            if crc & 0x80:
                crc = ((crc << 1) ^ 0x07) & 0xFF
            else:
                crc = (crc << 1) & 0xFF
    return crc""")

h2("2.4 打包 / 解包")
code("""def build_query(keyword):                # 查询指令
    return _frame(CMD_QUERY, bytes([keyword]))

def build_set(keyword, dtype, value):    # 设置指令
    data = struct.pack(TYPE_FORMAT[dtype], value)
    return _frame(CMD_SET, bytes([keyword, dtype]) + data)

def parse_packet(buf):                   # 解包一帧，处理粘包/半包
    idx = buf.find(HEADER)
    if idx < 0:
        return None, b""
    buf = buf[idx:]
    length = buf[4]
    total = 4 + 1 + length + 1           # 帧头+长度+payload+CRC
    if len(buf) < total:
        return None, buf                 # 半包，继续等
    frame = buf[:total]
    if crc8(frame[:-1]) != frame[-1]:
        return None, buf[1:]             # CRC 错，丢 1 字节重新同步
    ...""")

doc.add_page_break()

# ================= 三、Python 类 vs C++ 类 =================
h1("三、Python 类 vs C++ 类")

h2("3.1 对照：创建即初始化")
code("""# C++
class Serial : public SerialBase {
public:
    Serial(const char* port, int baud, float timeout) { /* ... */ }
};
Serial ser("COM7", 115200, 0.2);

# Python
class Serial(SerialBase):
    def __init__(self, port=None, baudrate=9600, timeout=None):
        self.port = port
        self.open()      # 真正的开串口在这里

ser = Serial(port="COM7", baudrate=115200, timeout=0.2)""")

h2("3.2 关键差异")
table(
    ["C++", "Python", "说明"],
    [
        ["new / 栈对象", "直接调类名 Serial(...)", "没有 new，对象总在堆上"],
        ["构造函数可重载", "__init__ 只有一个", "靠默认参数解决"],
        ["先声明成员变量", "self.x = ... 现用现造", "无头文件、无 private"],
        ["隐式 this", "显式 self（方法第一个参数）", "调用时自动填"],
        ["private/public", "下划线约定 _x", "只是约定，拦不住"],
        ["class D : public B", "class D(B)", "super().__init__() 调父类"],
    ],
)

h2("3.3 创建即初始化的完整链条")
code("""Serial(...) → __init__ → super().__init__ → self.open() → 真正打开 COM 口""")
para("因此串口被占用 / 设备没插 / COM 号写错时，__init__ 里就会抛异常，需要 try/except 包住。")


# ================= 四、struct =================
h1("四、struct 模块（值 ↔ 字节）")

h2("4.1 两个核心函数")
code("""struct.pack('<H', 0x5678)          # 值 → 字节：b'xV'（小端）
struct.unpack('<H', b'xV')         # 字节 → 值：(22136,)""")

h2("4.2 格式对照表")
table(
    ["格式", "C 类型", "字节数"],
    [
        ["B", "uint8_t", "1"],
        ["b", "int8_t", "1"],
        ["H", "uint16_t", "2"],
        ["h", "int16_t", "2"],
        ["I", "uint32_t", "4"],
        ["i", "int32_t", "4"],
        ["Q", "uint64_t", "8"],
        ["f", "float", "4"],
        ["d", "double", "8"],
    ],
)
para("开头的 '<' 表示小端、'>' 表示大端（对应 C 的 htons/ntohl）。")

h2("4.3 三个坑")
bullet("坑 1：永远显式写 '<' 或 '>'，别用默认（默认是本机字节序，换平台会炸）。")
bullet("坑 2：unpack 永远返回元组，哪怕只有一个值，要加 [0] 取出来。")
bullet("坑 3：float 是 IEEE754 单精度，有精度误差（88.88 → 88.879997），别用 == 比较。")


# ================= 五、bytes =================
h1("五、bytes（字节数组）")

para("bytes 就是 C 的 uint8_t buf[]，只是不可修改（对应 const）。可改版是 bytearray。")

h2("5.1 三种创建方式")
code("""b"\\x5A\\xA5"                  # 字面量（只能 0x00~0xFF）
bytes([0x5A, 0xA5])          # 从 int 列表
bytes.fromhex("5A A5")       # 从 hex 字符串（调试最好用）""")

h2("5.2 索引 vs 切片（最关键的坑）")
table(
    ["写法", "返回", "对应 C"],
    [
        ["buf[0]", "int（90）", "buf[0] 取单个字节"],
        ["buf[0:2]", "bytes", "memcpy 取一段"],
        ["buf[-1]", "int（最后一个）", "buf[len-1]"],
        ["buf[:-1]", "bytes（去掉末尾）", "去掉 CRC 字节"],
    ],
)
para("一句话：buf[i] 给单个字节(int)，buf[i:j] 给一段(bytes)。")

h2("5.3 拼接、查找")
code("""buf += chunk          # 拼接（realloc + memcpy 的自动化）
buf.find(HEADER)     # 查找，找不到返回 -1（对应 strstr）
if buf:              # 非空判断""")

h2("5.4 不可修改，怎么改")
code("""buf[0] = 0x00        # 报错：does not support item assignment
ba = bytearray(buf)  # 转成可改版
ba[0] = 0x00         # 可以改
buf = bytes(ba)      # 转回 bytes""")

h2("5.5 数据流全链路")
code("""struct.pack(...) → bytes → ser.write(bytes)
ser.read(n)      → bytes → struct.unpack(...)
bytes.fromhex()  → bytes  （调试构造）
bytes.hex()      → 字符串  （打印查看）""")

doc.add_page_break()

# ================= 六、函数与返回值 =================
h1("六、函数与返回值")

h2("6.1 多返回值 = 元组 + 解包")
code("""def parse_packet(buf):
    return info, buf        # 实际是 return (info, buf) 一个元组

info, buf = parse_packet(b) # 左边自动拆包""")
para("对应 C 里用指针当输出参数，Python 用元组把这事做得又短又安全，不用传指针、不用判空。")

h2("6.2 动态类型")
para("变量没有类型，类型在值上。x = 5、x = \"abc\"、x = [1,2,3] 都能塞进同一个 x。")
para("代价：没有编译期检查，类型错误要到运行时才炸（如 baudrate=\"115200\" 字符串）。")


# ================= 七、串口通信要点 =================
h1("七、串口通信要点")

h2("7.1 数据是流，不是包")
para("串口（含 USB CDC）收到的数据是字节流，一帧可能分几次到，也可能一次来好几帧。必须写缓冲区 + 粘包/半包处理，不能假设一次 read() 就是一帧。这对应 MCU 中断里收 UART 的异步特性。")

h2("7.2 收包循环：用轮询，别用长阻塞读")
code("""# 可靠写法（短超时 + 轮询）
ser = serial.Serial(port, baud, timeout=0.2)  # 短超时
while time.time() < deadline:
    n = ser.in_waiting
    if n:
        buf += ser.read(n)
        info, buf = parse_packet(buf)
        if info: return info
    else:
        time.sleep(0.005)  # 没数据小睡，避免空转""")

h2("7.3 踩过的坑")
bullet("坑：ser.read(ser.in_waiting or 1) 在 in_waiting==0 时变成阻塞读 1 字节，配合长超时在 Windows CDC 上会卡住，导致 3ms 就到的数据读不到。")
bullet("原则：要么短超时轮询，要么开后台线程专门收，绝不在主流程长阻塞等数据。")


# ================= 八、核心心法 =================
h1("八、核心心法（嵌入式 → Python）")

table(
    ["C 思维", "Python 思维"],
    [
        ["管理内存 malloc/free", "垃圾回收 + 引用计数，自动 free"],
        ["声明类型 int x", "动态类型，类型在值上"],
        ["memcpy 序列化", "struct.pack / unpack"],
        ["htons / ntohl", "格式串开头写 '<' / '>'"],
        ["uint8_t buf[]", "bytes（只读）/ bytearray（可改）"],
        ["指针当输出参数", "返回元组 + 解包"],
        ["编译期报错", "运行时报错，需自己守类型边界"],
    ],
)

h2("一句话总结")
para("上位机用 Python（开发快、内存/类型自动管），下位机用 C（资源可控）。", bold=True)
para("Python 把最耗精力的两件事（内存管理、类型声明）自动化了，代价是少了编译期安全带——所以安全边界得靠写的人自己守。", bold=True)


# ================= 保存 =================
out = "Python上位机开发笔记.docx"
doc.save(out)
print("已生成:", out)
