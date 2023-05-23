
import sys
import re
import math
from jinja2 import Template

def write_code(code, path, enable):
    if not enable:
        return
    with open(path, "w") as f:
        f.write(code)

def repeat_code(append, times, offset = 0):
    tmp = ""
    for i in range(times):
        tmp += append.format(itr = (i + offset))
    return tmp

def ceil(a, b):
    return int(math.ceil(float(a) / b) * b)

def main(argv):
    INPUT_BITS = int(argv[1])
    PARALLELISM = int(argv[2])
    AIE_W = int(argv[3])
    AIE_H = int(argv[4])
    DDR_BITS = int(argv[5])
    PLIO_BITS = int(argv[6])
    PL_FREQ = int(argv[7])

    enable = True

    elements = int(math.ceil(INPUT_BITS / 31.))
    # print(elements)
    AIE_COL = int(math.ceil(float(elements) / AIE_W + 1))
    AIE_ROW = int(math.ceil(float(elements) / AIE_H))
    OUTPUT_AIE = AIE_ROW + AIE_COL - 1
    AIE_NUM = AIE_COL * AIE_ROW
    TASK_INPUT_CHUNK = int(math.ceil(float(INPUT_BITS) / DDR_BITS))
    
    # print(float(elements) / AIE_H)/

    ############################################################
    # config.h
    code = ""
    code += "#define INPUT_BITS " + str(INPUT_BITS) + "\n"
    code += "#define DDR_BITS " + str(DDR_BITS) + "\n"
    code += "#define PLIO_BITS " + str(PLIO_BITS) + "\n\n"

    code += "#define AIE_W " + str(AIE_W) + "\n"
    code += "#define AIE_H " + str(AIE_H) + "\n\n"
    
    # code += "#define PARALLELISM " + str(PARALLELISM) + "\n"
    # code += "#define BATCH " + str(BATCH) + "\n\n"

    code += "#define SENT_VALID ((INPUT_BITS / 31) / 4)\n"
    code += "#define TAILS_A " + str(ceil(AIE_ROW * AIE_H - int(INPUT_BITS / 31), PLIO_BITS / 32)) + "\n"
    code += "#define TAILS_B " + str(ceil(AIE_COL * AIE_W - int(INPUT_BITS / 31), PLIO_BITS / 32)) + "\n\n"



    code += "#define VEC_ELEM_BITS 32\n"
    code += "#define VALID_BITS 31\n"
    code += "#define VEC_SIZE 8\n\n"

    code += "#define HOST_ADDER_BITS " + str(int(math.ceil(math.log(elements) / math.log(2)) + 64)) + "\n"
    # code += "#define OP_BITS (INPUT_BITS)\n"
    code += "#define LAST_VALID ((AIE_W * VALID_BITS) - ((AIE_W * VALID_BITS) / DDR_BITS) * DDR_BITS)\n\n"

    code += "#define AIE_COL " + str(AIE_COL) + "\n"
    code += "#define AIE_ROW " + str(AIE_ROW) + "\n"
    code += "#define AIE_NUM " + str(AIE_COL * AIE_ROW) + "\n"
    code += "#define OUTPUT_AIE " + str(OUTPUT_AIE) + "\n"

    code += "#define ROW_SIZE AIE_H\n"
    code += "#define COL_SIZE AIE_W\n"

    code += "#define RSA_DATA_CHUNK (INPUT_BITS / DDR_BITS)\n"
    code += "#define BATCH 16\n"

    code += "#define TASK_INPUT_CHUNK " + str(TASK_INPUT_CHUNK) + "\n"
    code += "#define TASK_OUTPUT_CHUNK " + str(int(math.ceil(INPUT_BITS * 2.0 / DDR_BITS)))+ "\n"
    code += "#define AIE_OUTPUT_CHUNK " + str(int(AIE_W * 31 / DDR_BITS) + 2) + "\n"

    # print(code)
    write_code(code, "./src/config.h", enable)

    ############################################################
    # aie_graph.cpp
    code = '#include "aie_graph.h"\n'
    code += 'using namespace adf;\n'
    
    for i in range(PARALLELISM):
        code += repeat_code('PLIO* p_' + str(i) + '_in0_{itr} = new PLIO("p_' + str(i) + '_in0_{itr}", adf::plio_' + str(PLIO_BITS) + '_bits, "data/input0_0.txt", 230);\n', AIE_ROW)
        code += "\n"
        code += repeat_code('PLIO* p_' + str(i) + '_in1_{itr} = new PLIO("p_' + str(i) + '_in1_{itr}", adf::plio_' + str(PLIO_BITS) + '_bits, "data/input1_1.txt", 230);\n', AIE_COL)
        code += "\n"
        code += repeat_code('PLIO* p_' + str(i) + '_out_{itr} = new PLIO("p_' + str(i) + '_out_{itr}", adf::plio_' + str(PLIO_BITS) + '_bits, "data/output_{itr}.txt", 230);\n', OUTPUT_AIE)
        code += "\n"


    code += 'simulation::platform<' + str(PARALLELISM * (AIE_ROW + AIE_COL)) + ', ' + str(PARALLELISM * OUTPUT_AIE) + '> platform('
    for i in range(PARALLELISM):
        code += repeat_code('p_' + str(i) + '_in0_{itr}, ', AIE_ROW)
    
    for i in range(PARALLELISM):
        code += repeat_code('p_' + str(i) + '_in1_{itr}, ', AIE_COL)

    for i in range(PARALLELISM):
        code += repeat_code('p_' + str(i) + '_out_{itr}, ', OUTPUT_AIE)

    code = code[:-2]
    code += ");\n\n"

    code += "simpleGraph addergraph;\n\n"

    netidx = 0

    for i in range(PARALLELISM):
        for j in range(AIE_ROW):
            for k in range(AIE_COL):
                code += "connect<> net" + str(netidx) + "(platform.src[" + str(i * AIE_ROW + j) + "], addergraph.p_" + str(i) + "_in0[" + str(k + j * AIE_COL) + "]);\n"
                netidx += 1
            code += "\n"
        code += "\n"
        for j in range(AIE_COL):
            for k in range(AIE_ROW):
                code += "connect<> net" + str(netidx) + "(platform.src[" + str(PARALLELISM * AIE_ROW + i * AIE_COL + j) + "], addergraph.p_" + str(i) + "_in1[" + str(j + k * AIE_COL) + "]);\n"
                netidx += 1
            code += "\n"
        # code += "\n"

        for j in range(OUTPUT_AIE):
            code += "connect<> net" + str(netidx) + "(addergraph.p_" + str(i) + "_out[" + str(j) + "], platform.sink[" + str(i * OUTPUT_AIE + j) + "]);\n"
            netidx += 1
        code += "\n"
    code += "\n"
    code += "#ifdef __AIESIM__\n"
    code += "int main(int argc, char** argv) {\n"
    code += "    addergraph.init();\n"
    code += "    addergraph.run(1);\n"
    code += "    addergraph.end();\n"
    code += "    return 0;\n"
    code += "}\n"
    code += "#endif\n"
    
    write_code(code, "./src/aie_graph.cpp", enable)

    #####################################################
    # aie_graph.h
    code = '''#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <adf.h>
#include "aie_kernel.h"
#include "aie_config.h"

using namespace adf;

class simpleGraph : public graph {
    private:
'''
    code += repeat_code("    kernel aie_aim_p{itr}[" + str(AIE_NUM) + "];\n", PARALLELISM)
    code += "    public:\n"
    code += repeat_code("    port<input> p_{itr}_in0[" + str(AIE_NUM) + "], p_{itr}_in1[" + str(AIE_NUM) + "];\n", PARALLELISM)
    code += repeat_code("    port<input> p_{itr}_out[" + str(AIE_NUM) + "];\n", PARALLELISM)
    code += "\n"
    code += "    simpleGraph() {\n"
    for p in range(PARALLELISM):
        # aie top bottom
        code += '''
        aie_aim_p''' + str(p) + '''[0] = kernel::create(aie_aim_top_bottom);
        connect<window<AIE_H * 4> >(p_''' + str(p) + '''_in0[0], aie_aim_p''' + str(p) + '''[0].in[0]);
        connect<window<AIE_W * 4 + AIE_H * 4> >(p_''' + str(p) + '''_in1[0], aie_aim_p''' + str(p) + '''[0].in[1]);
        source(aie_aim_p''' + str(p) + '''[0]) = "aie_aim_top_bottom.cc";
        runtime<ratio>(aie_aim_p''' + str(p) + '''[0]) = 1;

        aie_aim_p''' + str(p) + '''[''' + str(AIE_NUM - 1) + '''] = kernel::create(aie_aim_top_bottom);
        connect<window<AIE_H * 4> >(p_''' + str(p) + '''_in0[''' + str(AIE_NUM - 1) + '''], aie_aim_p''' + str(p) + '''[''' + str(AIE_NUM - 1) + '''].in[0]);
        connect<window<AIE_W * 4 + AIE_H * 4> >(p_''' + str(p) + '''_in1[''' + str(AIE_NUM - 1) + '''], aie_aim_p''' + str(p) + '''[''' + str(AIE_NUM - 1) + '''].in[1]);
        source(aie_aim_p''' + str(p) + '''[''' + str(AIE_NUM - 1) + ''']) = "aie_aim_top_bottom.cc";
        runtime<ratio>(aie_aim_p''' + str(p) + '''[''' + str(AIE_NUM - 1) + ''']) = 1;
        '''

        # aie top
        for i in range(AIE_COL - 1):
            code += '''
        aie_aim_p''' + str(p) + '''[''' + str(i + 1) + '''] = kernel::create(aie_aim_top);
        connect<window<AIE_H * 4> >(p_''' + str(p) + '''_in0[''' + str(i + 1) + '''], aie_aim_p''' + str(p) + '''[''' + str(i + 1) + '''].in[0]);
        connect<window<AIE_W * 4 + AIE_H * 4> >(p_''' + str(p) + '''_in1[''' + str(i + 1) + '''], aie_aim_p''' + str(p) + '''[''' + str(i + 1) + '''].in[1]);
        source(aie_aim_p''' + str(p) + '''[''' + str(i + 1) + ''']) = "aie_aim_top.cc";
        runtime<ratio>(aie_aim_p''' + str(p) + '''[''' + str(i + 1) + ''']) = 1;
        '''

        for i in range(AIE_ROW - 2):
            code += '''
        aie_aim_p''' + str(p) + '''[''' + str((i + 2) * AIE_COL - 1) + '''] = kernel::create(aie_aim_top);
        connect<window<AIE_H * 4> >(p_''' + str(p) + '''_in0[''' + str((i + 2) * AIE_COL - 1) + '''], aie_aim_p''' + str(p) + '''[''' + str((i + 2) * AIE_COL - 1) + '''].in[0]);
        connect<window<AIE_W * 4 + AIE_H * 4> >(p_''' + str(p) + '''_in1[''' + str((i + 2) * AIE_COL - 1) + '''], aie_aim_p''' + str(p) + '''[''' + str((i + 2) * AIE_COL - 1) + '''].in[1]);
        source(aie_aim_p''' + str(p) + '''[''' + str((i + 2) * AIE_COL - 1) + ''']) = "aie_aim_top.cc";
        runtime<ratio>(aie_aim_p''' + str(p) + '''[''' + str((i + 2) * AIE_COL - 1) + ''']) = 1;
        '''

        # aie mid
        for i in range(AIE_ROW - 2):
            for j in range(AIE_COL - 2):
                code += '''
        aie_aim_p''' + str(p) + '''[''' + str((i + 1) * AIE_COL + j + 1) + '''] = kernel::create(aie_aim_mid);
        connect<window<AIE_H * 4> >(p_''' + str(p) + '''_in0[''' + str((i + 1) * AIE_COL + j + 1) + '''], aie_aim_p''' + str(p) + '''[''' + str((i + 1) * AIE_COL + j + 1) + '''].in[0]);
        connect<window<AIE_W * 4 + AIE_H * 4> >(p_''' + str(p) + '''_in1[''' + str((i + 1) * AIE_COL + j + 1) + '''], aie_aim_p''' + str(p) + '''[''' + str((i + 1) * AIE_COL + j + 1) + '''].in[1]);
        source(aie_aim_p''' + str(p) + '''[''' + str((i + 1) * AIE_COL + j + 1) + ''']) = "aie_aim_mid.cc";
        runtime<ratio>(aie_aim_p''' + str(p) + '''[''' + str((i + 1) * AIE_COL + j + 1) + ''']) = 1;
        '''

        # aie bottom
        for i in range(AIE_ROW - 1):
            code += '''
        aie_aim_p''' + str(p) + '''[''' + str((i + 1) * AIE_COL) + '''] = kernel::create(aie_aim_bottom);
        connect<window<AIE_H * 4> >(p_''' + str(p) + '''_in0[''' + str((i + 1) * AIE_COL) + '''], aie_aim_p''' + str(p) + '''[''' + str((i + 1) * AIE_COL) + '''].in[0]);
        connect<window<AIE_W * 4 + AIE_H * 4> >(p_''' + str(p) + '''_in1[''' + str((i + 1) * AIE_COL) + '''], aie_aim_p''' + str(p) + '''[''' + str((i + 1) * AIE_COL) + '''].in[1]);
        source(aie_aim_p''' + str(p) + '''[''' + str((i + 1) * AIE_COL) + ''']) = "aie_aim_bottom.cc";
        runtime<ratio>(aie_aim_p''' + str(p) + '''[''' + str((i + 1) * AIE_COL) + ''']) = 1;
        '''

        for i in range(AIE_COL - 2):
            code += '''
        aie_aim_p''' + str(p) + '''[''' + str(i + 1 + (AIE_ROW - 1) * AIE_COL) + '''] = kernel::create(aie_aim_bottom);
        connect<window<AIE_H * 4> >(p_''' + str(p) + '''_in0[''' + str(i + 1 + (AIE_ROW - 1) * AIE_COL) + '''], aie_aim_p''' + str(p) + '''[''' + str(i + 1 + (AIE_ROW - 1) * AIE_COL) + '''].in[0]);
        connect<window<AIE_W * 4 + AIE_H * 4> >(p_''' + str(p) + '''_in1[''' + str(i + 1 + (AIE_ROW - 1) * AIE_COL) + '''], aie_aim_p''' + str(p) + '''[''' + str(i + 1 + (AIE_ROW - 1) * AIE_COL) + '''].in[1]);
        source(aie_aim_p''' + str(p) + '''[''' + str(i + 1 + (AIE_ROW - 1) * AIE_COL) + ''']) = "aie_aim_bottom.cc";
        runtime<ratio>(aie_aim_p''' + str(p) + '''[''' + str(i + 1 + (AIE_ROW - 1) * AIE_COL) + ''']) = 1;
        
'''

        # cascade
        for i in range(AIE_ROW - 1):
            for j in range(AIE_COL - 1):
                code += "        connect<cascade> (aie_aim_p" + str(p) + "[" + str(i * AIE_COL + j + 1) + "].out[0], aie_aim_p" + str(p) + "[" + str((i + 1) * AIE_COL + j) + "].in[2]);\n"
        # output
        for i in range(AIE_ROW):
            code += "        connect<stream>(aie_aim_p" + str(p) + "[" + str(i * AIE_COL) + "].out[0], p_" + str(p) + "_out[" + str(i) + "]);\n"

        for i in range(AIE_COL - 1):
            code += "        connect<stream>(aie_aim_p" + str(p) + "[" + str((AIE_ROW - 1) * AIE_COL + i + 1) + "].out[0], p_" + str(p) + "_out[" + str(i + AIE_ROW) + "]);\n"
    
    for p in range(PARALLELISM):
        for i in range(AIE_COL):
            loc = p * AIE_COL + i
            col = int(loc / 8)
            row = loc % 8
            idx = 0
            col_start = col * AIE_ROW + (0 if row % 2 == 0 else AIE_ROW - 1)
            # code += str(col_start) + "\n"
            if int(col_start + (1 if row % 2 == 0 else -1) * (min(i + 1, AIE_ROW) - 1)) >= 50:
                break
            for j in range(min(i + 1, AIE_ROW)):
                c = int(col_start + (1 if row % 2 == 0 else -1) * idx)
                idx += 1
                code += "        adf::location<kernel>(aie_aim_p" + str(p) + "[" + str(i + j * (AIE_COL - 1)) + "]) = adf::tile(" + str(c) + ", " + str(row) + ");\n"
            if int(col_start + (1 if row % 2 == 0 else -1) * (idx + AIE_ROW - i - 2)) >= 50:
                break
            for j in range(AIE_ROW - i - 1):
                c = int(col_start + (1 if row % 2 == 0 else -1) * idx)
                idx += 1
                code += "        adf::location<kernel>(aie_aim_p" + str(p) + "[" + str((i + 2) * AIE_COL - 1 + j * (AIE_COL - 1)) + "]) = adf::tile(" + str(c) + ", " + str(row) + ");\n"
        code += "\n"
    code += '''    };
};

#endif
'''
    write_code(code, "src/aie_graph.h", enable)
#     ############################################################
#     # system.cfg
    code = "[connectivity]\n"
    if PARALLELISM == 1:
        code += "nk=pl_kernel:1:pl_kernel_0\n"
    else:
        code += "nk=pl_kernel:" + str(PARALLELISM) + ":"
        code += repeat_code("pl_kernel_{itr}.", PARALLELISM)
        code = code[:-1]
        code += "\n"

    for i in range(PARALLELISM):
        code += repeat_code("stream_connect=pl_kernel_" + str(i) + ".p_0_a{itr}:ai_engine_0.p_" + str(i) + "_in0_{itr}\n", AIE_ROW)
        code += "\n"
        code += repeat_code("stream_connect=pl_kernel_" + str(i) + ".p_0_b{itr}:ai_engine_0.p_" + str(i) + "_in1_{itr}\n", AIE_COL)
        code += "\n"
        code += repeat_code("stream_connect=ai_engine_0.p_" + str(i) + "_out_{itr}:pl_kernel_" + str(i) + ".p_0_c{itr}\n", OUTPUT_AIE)
        code += "\n"
    write_code(code, "system.cfg", enable)

#     ############################################################
    # pl_kernel.cpp
    with open('tools/pl_kernel.cpp.jinja') as f:
        code = Template(f.read())
    r = 0
    shift = ""
    for i in range(64):
        shift += str(r)
        shift += ", "
        r += ((AIE_W * 31) - int((AIE_W * 31) / 512) * 512)
        if r >= 512:
            r -= 512
        # shift = 
    code = (code.render(AIE_ROW=AIE_ROW, AIE_COL=AIE_COL, OUTPUT_AIE=OUTPUT_AIE, INPUT_BITS=INPUT_BITS, DDR_BITS=DDR_BITS, AIE_H=AIE_H, AIE_W=AIE_W, shift2=shift[:-2]))
    write_code(code, "./src/pl_kernel.cpp", enable)

    ##############################
    # Makefile
    with open("Makefile", "r") as f:
        code = f.read()
    code = re.sub(r"--pl-freq=\d+", "--pl-freq=" + str(PL_FREQ), code)
    with open("freq", "w") as f:
        f.write(str(PL_FREQ))
    code = re.sub(r"CLFLAGS \+= --clock.defaultFreqHz \d+", "CLFLAGS += --clock.defaultFreqHz " + str(PL_FREQ) + "000000", code)
    write_code(code, "Makefile", enable)

#     ############################################################
#     # host.cpp
    m = ""
    mp = ""
    prv_e = ""
    rmod = ""
    rmod_x_1 = "" 
    try:
        with open("./tools/keys/" + str(INPUT_BITS) + "/m.txt") as f:
            m = f.read()
        with open("./tools/keys/" + str(INPUT_BITS) + "/mp.txt") as f:
            mp = f.read()
        with open("./tools/keys/" + str(INPUT_BITS) + "/prv_e.txt") as f:
            prv_e = f.read()
        with open("./tools/keys/" + str(INPUT_BITS) + "/rmod.txt") as f:
            rmod = f.read()
        with open("./tools/keys/" + str(INPUT_BITS) + "/rmod_x_1.txt") as f:
            rmod_x_1 = f.read()
    except:
        pass
    with open('tools/host.cpp.jinja') as f:
        code = Template(f.read())
    
        code = (code.render(RSA_IMPL=int(PARALLELISM), AIE_COL=AIE_COL, OUTPUT_AIE=OUTPUT_AIE, AIE_ROW=AIE_ROW, modulus=m, modulus_p=mp, rmod=rmod, rmod_x_1=rmod_x_1, prv_e=prv_e))
    

    # with open("./src/host.cpp", "r") as f:
    #     code = f.read()

    # krnl1 = "xrtRunHandle pl_kernl_rhdl1 = xrtKernelRun(pl_kernl_khdl1, in_bohdl0, in_bohdl1, out_bohdl, repeat, syn, block_ddr"
    # krnl1 += repeat_code(", nullptr", PARALLELISM * (AIE_COL + AIE_ROW + OUTPUT_AIE))
    # krnl1 += ");"
    # code = re.sub(r"xrtRunHandle pl_kernl_rhdl1 = xrtKernelRun\(pl_kernl_khdl1, in_bohdl0, in_bohdl1, out_bohdl, repeat, syn, block_ddr, .+", krnl1, code)

    write_code(code, "./src/host.cpp", enable)
    



if __name__ == "__main__":
    main(sys.argv)