# force.py <pid> <value> <seconds>: hold the host options byte (0x140585f64) at <value> for N seconds, then restore the original.
import sys, time
pid, val, secs = sys.argv[1], int(sys.argv[2], 16), float(sys.argv[3])
A = 0x140585f64
m = open(f"/proc/{pid}/mem", "r+b", 0)
m.seek(A); orig = m.read(1)[0]
print(f"orig={orig:#04x} forcing {val:#04x} for {secs}s", flush=True)
end = time.time() + secs
try:
    while time.time() < end:
        m.seek(A); cur = m.read(1)[0]
        if cur != val: m.seek(A); m.write(bytes([(cur & ~2) | (val & 2)]))
        time.sleep(0.02)
finally:
    m.seek(A); m.write(bytes([orig])); print("restored", hex(orig), flush=True)
