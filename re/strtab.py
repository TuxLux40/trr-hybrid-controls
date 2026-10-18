# Find the pointer table that holds the OPT_* / PM_* name strings in a tomb?.dll and print indices of interest.
import sys, struct, pefile
pe = pefile.PE(sys.argv[1]); base = pe.OPTIONAL_HEADER.ImageBase
data = pe.get_memory_mapped_image()
def cstr(va):
    o = va - base
    if not 0 < o < len(data): return None
    e = data.find(b"\0", o, o + 80)
    s = data[o:e] if e > 0 else b""
    return s.decode("latin1") if s and all(32 <= c < 127 for c in s) else None
# scan for runs of >=50 consecutive qword pointers to printable strings
res = []; o = 0; n = len(data) - 8
run_start = None; run = 0
for o in range(0, n, 8):
    p = struct.unpack_from("<Q", data, o)[0]
    if base < p < base + len(data) and cstr(p):
        if run == 0: run_start = o
        run += 1
    else:
        if run >= 50: res.append((run_start, run))
        run = 0
for st, ln in res:
    names = [cstr(struct.unpack_from("<Q", data, st + 8*i)[0]) for i in range(ln)]
    hit = [(i, x) for i, x in enumerate(names) if x.startswith(("OPT_CONTROL", "OPT_MODERN", "OPT_TANK", "PM_ROLL"))]
    print(hex(base + st), ln, hit[:8])
