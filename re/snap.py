# Snapshot host globals + the game DLL's rw data of a running Wine game: snap.py <pid> <out.pkl>
import sys, re, pickle
pid, out = sys.argv[1], sys.argv[2]
rows = []
for l in open(f"/proc/{pid}/maps"):
    m = re.match(r"([0-9a-f]+)-([0-9a-f]+) (\S+) \S+ \S+ \S+\s*(.*)", l)
    rows.append((int(m[1], 16), int(m[2], 16), m[3], m[4]))
dlls = {r[3][-5]: r[0] for r in rows if re.search(r"/[456]/tomb[456]\.dll$", r[3]) and r[2] == "r--p" and r[1] - r[0] == 0x1000}
mem = open(f"/proc/{pid}/mem", "rb", 0)
want = [(0x140169000, 0x140800000, "host")]
for n, base in dlls.items():  # rw-p mappings right after each dll header, within a 16 MiB image
    for r in rows:
        if r[2] == "rw-p" and base < r[0] < base + 0x1000000: want.append((r[0], r[1], "dll" + n))
snap = {"dllbases": dlls, "regs": {}}
for s, e, tag in want:
    chunks = []
    for rs, re_, perm, _ in rows:
        a, b = max(s, rs), min(e, re_)
        if a < b and perm.startswith("rw"):
            mem.seek(a); chunks.append((a, mem.read(b - a)))
    for a, d in chunks: snap["regs"][a] = (tag, d)
pickle.dump(snap, open(out, "wb"))
print("dll bases", {k: hex(v) for k, v in dlls.items()}, "regions", len(snap["regs"]), sum(len(d) for _, d in snap["regs"].values()) >> 10, "KiB")
