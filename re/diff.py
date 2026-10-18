# diff.py a.pkl b.pkl c.pkl: report bytes equal in a and b (stable) but different in c
import sys, pickle
a, b, c = (pickle.load(open(p, "rb")) for p in sys.argv[1:4])
bases = a["dllbases"]
def name(addr):
    if 0x140000000 <= addr < 0x140800000: return f"host {addr:#x}"
    for n, base in bases.items():
        if base <= addr < base + 0x1000000: return f"dll{n} +{addr-base:#x} (ghidra {0x180000000+addr-base:#x})"
    return hex(addr)
hits = []
for s, (tag, da) in a["regs"].items():
    db = b["regs"][s][1]; dc = c["regs"][s][1]
    for i in range(len(da)):
        if da[i] == db[i] and da[i] != dc[i]:
            hits.append((s + i, da[i], dc[i]))
print(len(hits), "stable-then-changed bytes")
for addr, x, y in hits[:200]: print(name(addr), x, "->", y)
