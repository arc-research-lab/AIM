import sys
import re
import math
from jinja2 import Template
def write_code(code, path, enable):
    # pass
    if not enable:
        return
    with open(path, "w") as f:
        f.write(code)

def repeat_code(append, times, offset = 0):
    tmp = ""
    for i in range(times):
        tmp += append.format(itr = (i + offset))
    return tmp
        
def main(argv):
    INPUT_BITS = int(argv[1])
    PE = int(argv[2])
    AIE_NUM = int(argv[3])
    # BATCH = int(argv[4])
    DDR_BITS = int(argv[4])
    PLIO_BITS = int(argv[5])
    PL_FREQ = int(argv[6])

    enable = True

    COMPUTE_ELEMS = int(math.ceil(math.ceil(INPUT_BITS / 31) / 8.) * 8)
    PLIO_PORTS = int(AIE_NUM)
    SENT_VALID_ELE = int(INPUT_BITS / 31 / 4)

    ############################################################
    # config.h
    code = ""
    code += "#define INPUT_BITS " + str(INPUT_BITS) + "\n"
    # code += "#define PKT_RATIO " + str(PKT_RATIO) + "\n"
    # code += "#define BATCH " + str(BATCH) + "\n"
    code += "#define PE " + str(PE) + "\n"
    code += "#define AIE_NUM " + str(AIE_NUM) + "\n"
    code += "#define DDR_BITS " + str(DDR_BITS) + "\n"
    code += "#define PLIO_BITS " + str(PLIO_BITS) + "\n\n"

    code += "#define COMPUTE_ELEMS " + str(COMPUTE_ELEMS) + "\n\n"

    code += "#define TASK_INPUT_CHUNK (INPUT_BITS / DDR_BITS)\n"
    code += "#define TASK_OUTPUT_CHUNK (INPUT_BITS * 2 / DDR_BITS)\n"
    code += "#define VEC_ELEM_BITS 32\n"
    code += "#define VALID_BITS 31\n"
    code += "#define VEC_SIZE 8\n"
    code += "#define SENT_VALID_ELE " + str(SENT_VALID_ELE) + "\n"

    code += "#define INPUT_WINDOW_SIZE (COMPUTE_ELEMS * 4)\n"
    code += "#define OUTPUT_WINDOW_SIZE (COMPUTE_ELEMS * 2 * 3 * 4)\n"

    write_code(code, "./src/config.h", enable)

    ############################################################
    # aie_graph.cpp
    code = '#include "aie_graph.h"\n'
    code += 'using namespace adf;\n'
    for p in range(PE):
        code += repeat_code('PLIO* in0_' + str(p) + '_{itr} = new PLIO("in0_' + str(p) + '_{itr}", adf::plio_' + str(PLIO_BITS) + '_bits, "data/input0_{itr}.txt");\n', PLIO_PORTS)
        code += repeat_code('PLIO* in1_' + str(p) + '_{itr} = new PLIO("in1_' + str(p) + '_{itr}", adf::plio_' + str(PLIO_BITS) + '_bits, "data/input1_{itr}.txt");\n', PLIO_PORTS)
        code += repeat_code('PLIO* out_' + str(p) + '_{itr} = new PLIO("out_' + str(p) + '_{itr}", adf::plio_' + str(PLIO_BITS) + '_bits, "data/output_{itr}.txt");\n', PLIO_PORTS)
    code += 'simulation::platform<' + str(2 * PLIO_PORTS * PE) + ', ' + str(PLIO_PORTS * PE) + '> platform('
    for p in range(PE):
        code += repeat_code("in0_" + str(p) + "_{itr}, ", PLIO_PORTS)
    for p in range(PE):
        code += repeat_code("in1_" + str(p) + "_{itr}, ", PLIO_PORTS)
    for p in range(PE):
        code += repeat_code("out_" + str(p) + "_{itr}, ", PLIO_PORTS)
    code = code[:-2]
    code += ");\n\n"

    code += "simpleGraph addergraph;\n\n"

    net_idx = 0
    for i in range(PLIO_PORTS * PE):
        code += 'connect<> net' + str(net_idx) + '(platform.src[' + str(i) + '], addergraph.in0[' + str(i) + ']);\n'
        net_idx += 1
    code += "\n"

    for i in range(PLIO_PORTS * PE):
        code += 'connect<> net' + str(net_idx) + '(platform.src[' + str(i + PLIO_PORTS * PE) + '], addergraph.in1[' + str(i) + ']);\n'
        net_idx += 1
    code += "\n"

    for i in range(PLIO_PORTS * PE):
        code += 'connect<> net' + str(net_idx) + '(addergraph.out[' + str(i) + '], platform.sink[' + str(i) + ']);\n'
        net_idx += 1
    
    code += "\n"
    code += "#ifdef __AIESIM__\n"
    code += "int main(int argc, char** argv) {\n"
    code += "    addergraph.init();\n"
    code += "    addergraph.run(30);\n"
    code += "    addergraph.end();\n"
    code += "    return 0;\n"
    code += "}\n"
    code += "#endif\n"
    
    write_code(code, "./src/aie_graph.cpp", enable)

    ############################################################
    # system.cfg
    code = "[connectivity]\n"
    code += "nk=pl_kernel:" + str(PE) + ":"
    code += repeat_code("pl_kernel_{itr}.", PE)
    code = code[:-1]
    code += "\n"
    for p in range(PE):
        for i in range(PLIO_PORTS):
            code += "stream_connect=pl_kernel_" + str(p) + ".plio_a" + str(i) + ":ai_engine_0.in0_" + str(p) + "_" + str(i) + "\n"
            code += "stream_connect=pl_kernel_" + str(p) + ".plio_b" + str(i) + ":ai_engine_0.in1_" + str(p) + "_" + str(i) + "\n"
            code += "stream_connect=ai_engine_0.out_" + str(p) + "_" + str(i) + ":pl_kernel_" + str(p) + ".plio_out" + str(i) + "\n\n"

    write_code(code, "system.cfg", enable)

    ############################################################
    # pl_kernel.cpp
    
    with open('tools/pl_kernel.cpp.jinja') as f:
        code = Template(f.read())
    
        code = (code.render(PLIO_PORTS=int(AIE_NUM), AIE_NUM=AIE_NUM, INPUT_BITS=INPUT_BITS, DDR_BITS=DDR_BITS))

    write_code(code, "./src/pl_kernel.cpp", enable)

    ################################
    # host.cpp

    with open('tools/host.cpp.jinja') as f:
        code = Template(f.read())
    
        code = (code.render(PLIO_PORTS=int(AIE_NUM), AIE_NUM=AIE_NUM, INPUT_BITS=INPUT_BITS, DDR_BITS=DDR_BITS, PE=PE))

    write_code(code, "./src/host.cpp", enable)

    ##############################
    # Makefile
    with open("Makefile", "r") as f:
        code = f.read()
    code = re.sub(r"--pl-freq=\d+", "--pl-freq=" + str(PL_FREQ), code)
    with open("freq", "w") as f:
        f.write(str(PL_FREQ))
    code = re.sub(r"CLFLAGS \+= --clock.defaultFreqHz \d+", "CLFLAGS += --clock.defaultFreqHz " + str(PL_FREQ) + "000000", code)
    write_code(code, "Makefile", enable)

if __name__ == "__main__":
    main(sys.argv)