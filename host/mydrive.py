import struct
import protocol as p
import time

LINK = None
LOG_QUIET = False

class RemoteProperty:

    def __init__(self,keyword,dtype):
        self.keyword = keyword;    #实例的成员,每个实例各自一份
        self.dtype = dtype         #实例的成员,每个实例各自一份

    def __kw__(self,obj):
        return obj.kw_base + self.keyword

    def __get__(self,obj,owner):
        if obj is None:
            return self
        info = send_and_wait(LINK, p.build_query(self.__kw__(obj)))
        if info and info.get("results"):
            return info["results"][0]["value"]
        return None

    #set仅能返回None,所以收发逻辑必须在此处完成
    def __set__(self,obj,val):
        if self.dtype == p.TYPE_FLOAT:
            val = float(val)
        else:
            val = int(val)
        send_and_wait(LINK, p.build_set(self.__kw__(obj),self.dtype,val))



class Motor:

    def __init__(self,axis_id):
        self.kw_base = axis_id << 7 #轴0->0x00，轴1->0x80
    
    iu = RemoteProperty(0x05,p.TYPE_FLOAT)
    iv = RemoteProperty(0x06,p.TYPE_FLOAT)
    iw = RemoteProperty(0x07,p.TYPE_FLOAT)
    id_set = RemoteProperty(0x08,p.TYPE_FLOAT)
    iq_set = RemoteProperty(0x09,p.TYPE_FLOAT)
    id_feedback = RemoteProperty(0x0A,p.TYPE_FLOAT)
    iq_feedback = RemoteProperty(0x0B,p.TYPE_FLOAT)
    ia_feedback = RemoteProperty(0x0C,p.TYPE_FLOAT)
    ib_feedback = RemoteProperty(0x0D,p.TYPE_FLOAT)
    integral_d = RemoteProperty(0x0E,p.TYPE_FLOAT)
    integral_q = RemoteProperty(0x0F,p.TYPE_FLOAT)
    mod_d = RemoteProperty(0x010,p.TYPE_FLOAT)
    mod_q = RemoteProperty(0x011,p.TYPE_FLOAT)

class Encoder:

    def __init__(self,axis_id):
        self.kw_base = axis_id << 7 #轴0->0x00，轴1->0x80

    phase = RemoteProperty(0x12,p.TYPE_FLOAT)
    pos_est = RemoteProperty(0x13,p.TYPE_FLOAT)
    vel_est = RemoteProperty(0x14,p.TYPE_FLOAT)
    vel_ins = RemoteProperty(0x15,p.TYPE_FLOAT)

class Control:
    def __init__(self,axis_id):
            self.kw_base = axis_id << 7 #轴0->0x00，轴1->0x80
    
    input_pos = RemoteProperty(0x30,p.TYPE_FLOAT)
    input_vel = RemoteProperty(0x31,p.TYPE_FLOAT)
    input_torque = RemoteProperty(0x32,p.TYPE_FLOAT)


class VFOpenloop:

    def __init__(self,axis_id):
        self.kw_base = axis_id << 7 #轴0->0x00，轴1->0x80

    _vel_input = RemoteProperty(0x20,p.TYPE_FLOAT)
    _vel_ramp = RemoteProperty(0x21,p.TYPE_FLOAT)

class IFOpenloop:

    def __init__(self,axis_id):
        self.kw_base = axis_id << 7 #轴0->0x00，轴1->0x80

    vel_input = RemoteProperty(0x22,p.TYPE_FLOAT)
    vel_ramp = RemoteProperty(0x23,p.TYPE_FLOAT)
    current_input = RemoteProperty(0x24,p.TYPE_FLOAT)
    current_ramp = RemoteProperty(0x25,p.TYPE_FLOAT)


class Axis:

    def __init__(self,id):
        self.kw_base = id;
        self.motor = Motor(id);
        self.encoder = Encoder(id);
        self.control = Control(id);
        self.VF_openloop = VFOpenloop(id);
        self.IF_openloop = IFOpenloop(id);

    status = RemoteProperty(0x01,p.TYPE_U8);

class MyDrive:
    axis0 = Axis(0)
    axis1 = Axis(1)
    AXES = (0, 1)          # 轴 id 列表：加轴时在这里补一句即可


_mydrive = MyDrive()


# 命名空间的本质是字符串映射对象：key=容器对象路径前缀，value=实例。
# 叶子变量（iu/iv/iw 等描述符）挂在类上，靠 hasattr(type(obj),leaf) 动态查，不必登记。
def _build_namespace(drive):
    ns = {"mydrive": drive}
    for aid in drive.AXES:
        axis = getattr(drive, "axis%d" % aid)
        ns["mydrive.axis%d" % aid] = axis
        ns["mydrive.axis%d.motor" % aid] = axis.motor
        ns["mydrive.axis%d.encoder" % aid] = axis.encoder
        ns["mydrive.axis%d.control" % aid] = axis.control
        ns["mydrive.axis%d.VF_openloop" % aid] = axis.VF_openloop
        ns["mydrive.axis%d.IF_openloop" % aid] = axis.IF_openloop
    return ns


NAMESPACE = _build_namespace(_mydrive)

def send_and_wait(ser, frame, timeout=1.0):
    """发送一帧，等待并返回解析后的第一帧回复（dict 或 None）。

    用轮询方式收包：有数据就读，没数据就小睡一下，绝不阻塞，
    这是 debug_rx.py 验证过可靠的方式。
    """
    ser.write(frame)
    buf = b""
    info = None                     # 循环前初始化，避免无数据时 UnboundLocalError
    deadline = time.time() + timeout
    while time.time() < deadline:
        n = ser.in_waiting
        if n:
            buf += ser.read(n)
            info, buf = p.parse_packet(buf)
            if info is not None:
                break
        else:
            time.sleep(0.005)   # 没数据时小睡，避免空转

    if info is not None:
        if not LOG_QUIET:
            print("收到回复: 命令字 0x%02X" % info["cmd"])
            if "results" in info:                 # 查询回复（可能多个）
                for r in info["results"]:
                    print("  关键字 0x%02X = %r" % (r["keyword"], r["value"]))
            elif "keywords" in info:              # 示波器回复
                print("  通道 %d 个, 数据 %d 字节" % (len(info["keywords"]), len(info["data"])))
            else:                                 # 设置回复
                print("  关键字 0x%02X = %r" % (info["keyword"], info["value"]))
    else:
        if not LOG_QUIET:
            print("超时，无回复")
    return info


def query_multi(keywords):
    """一次查询多个关键字（最多 4 个），返回与 keywords 顺序一致的 value 列表。

    用于绘图：一帧同时取多个参数，保证它们是同一时刻采到的。
    """
    info = send_and_wait(LINK, p.build_query(list(keywords)))
    if info and info.get("results"):
        return [r["value"] for r in info["results"]]
    return None


def osc_capture(keywords):
    """下发示波器指令，返回各通道浮点样本列表（顺序与 keywords 一致）。

    回复数据为各通道 FLOAT 样本依次拼接，每通道样本数相等。
    样本数 = 数据字节数 / (通道数 * 4)。
    """
    keywords = list(keywords)
    info = send_and_wait(LINK, p.build_osc(keywords), 3.0)
    if not info or info.get("cmd") != p.CMD_OSC_REPLY:
        return None
    data = info["data"]
    n_ch = len(keywords)
    if n_ch == 0:
        return []
    n_samples = len(data) // (n_ch * 4)
    channels = []
    for i in range(n_ch):
        seg = data[i * n_samples * 4 : (i + 1) * n_samples * 4]
        channels.append(list(struct.unpack("<%df" % n_samples, seg)))
    return channels


def probe(ser, timeout=0.3):
    """静默探测设备是否在线：发一个无害查询，在线返回 True。

    用写操作探测（而非读 in_waiting）：设备掉电但 USB 句柄仍有效时，
    读操作不报错，只有写操作才会超时/失败，能真实反映设备状态。
    """
    global LOG_QUIET
    old = LOG_QUIET
    LOG_QUIET = True
    try:
        obj = NAMESPACE["mydrive.axis0.motor"]
        kw = obj.kw_base + Motor.iu.keyword   # 实际关键字 = 轴基址 + 相对关键字（类访问，不查询）
        info = send_and_wait(ser, p.build_query([kw]), timeout=timeout)
        return info is not None
    except Exception:
        return False
    finally:
        LOG_QUIET = old

