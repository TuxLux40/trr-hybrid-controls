# watch.py <pid> <addr>...: print every change of the given host-global bytes (Wine process)
import sys, time
pid = sys.argv[1]; addrs = [int(a, 16) for a in sys.argv[2:]]
m = open(f"/proc/{pid}/mem", "rb", 0); last = None
while True:
    cur = []
    for a in addrs: m.seek(a); cur.append(m.read(1)[0])
    if cur != last:
        print(time.strftime("%H:%M:%S"), " ".join(f"{a:#x}={v:#04x}" for a, v in zip(addrs, cur)), flush=True); last = cur
    time.sleep(0.1)
