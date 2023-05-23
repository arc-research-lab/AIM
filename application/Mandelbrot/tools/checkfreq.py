import re
import linecache
import os


timing_rpt_path = r"./_x/reports/link/imp/impl_1_xilinx_vck190_base_wrapper_timing_summary_routed.rpt"

if not (os.path.exists(timing_rpt_path)):
    exit(2)

WNS = float(linecache.getline(timing_rpt_path, 140).split()[0])
makefile_path = r"./Makefile"
freq = float(linecache.getline(makefile_path, 119).split()[3])
cycle = 1e9 / freq
new_cycle = cycle - WNS + 0.001

new_freq = int((1e9 / new_cycle) / 1e6)
freq_diff = (new_freq - int((freq / 1e6)))
if freq_diff >= 0 and freq_diff < 10:
    exit(0)
elif WNS >= 0:
    exit(0)
else:
    code = ""
    with open("Makefile", "r") as f:
        code = f.read()
    code = re.sub(r"--pl-freq=\d+", "--pl-freq=" + str(new_freq), code)
    code = re.sub(r"CLFLAGS \+= --clock.defaultFreqHz \d+", "CLFLAGS += --clock.defaultFreqHz " + str(new_freq) + "000000", code)
    with open("Makefile", "w") as f:
        f.write(code)
    with open("freq", "w") as f:
        f.write(str(new_freq))
    exit(1)