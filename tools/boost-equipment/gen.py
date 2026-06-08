#!/usr/bin/env python3
"""Driver: import all /tmp/d_*.py data modules, then validate or generate."""
import glob, importlib, os, sys
sys.path.insert(0, "/tmp")
import sets

for f in sorted(glob.glob("/tmp/d_*.py")):
    importlib.import_module(os.path.basename(f)[:-3])

cmd = sys.argv[1] if len(sys.argv) > 1 else "validate"
if cmd == "validate":
    errs = sets.validate()
    if errs:
        print("\n".join(errs))
        print(f"\n{len(errs)} ERRORS")
        sys.exit(1)
    n = sum(1 for c in sets.DATA.values() for t in c.values() for sp in t if sp)
    print(f"all {n} sets valid ({len(sets.DATA)} classes)")
elif cmd == "generate":
    print(sets.generate())
