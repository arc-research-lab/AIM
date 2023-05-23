#include "aie_graph.h"
using namespace adf;
PLIO* p_0_in0_0 = new PLIO("p_0_in0_0", adf::plio_128_bits, "data/input0_0.txt", 230);
PLIO* p_0_in0_1 = new PLIO("p_0_in0_1", adf::plio_128_bits, "data/input0_0.txt", 230);

PLIO* p_0_in1_0 = new PLIO("p_0_in1_0", adf::plio_128_bits, "data/input1_1.txt", 230);
PLIO* p_0_in1_1 = new PLIO("p_0_in1_1", adf::plio_128_bits, "data/input1_1.txt", 230);
PLIO* p_0_in1_2 = new PLIO("p_0_in1_2", adf::plio_128_bits, "data/input1_1.txt", 230);

PLIO* p_0_out_0 = new PLIO("p_0_out_0", adf::plio_128_bits, "data/output_0.txt", 230);
PLIO* p_0_out_1 = new PLIO("p_0_out_1", adf::plio_128_bits, "data/output_1.txt", 230);
PLIO* p_0_out_2 = new PLIO("p_0_out_2", adf::plio_128_bits, "data/output_2.txt", 230);
PLIO* p_0_out_3 = new PLIO("p_0_out_3", adf::plio_128_bits, "data/output_3.txt", 230);

simulation::platform<5, 4> platform(p_0_in0_0, p_0_in0_1, p_0_in1_0, p_0_in1_1, p_0_in1_2, p_0_out_0, p_0_out_1, p_0_out_2, p_0_out_3);

simpleGraph addergraph;

connect<> net0(platform.src[0], addergraph.p_0_in0[0]);
connect<> net1(platform.src[0], addergraph.p_0_in0[1]);
connect<> net2(platform.src[0], addergraph.p_0_in0[2]);

connect<> net3(platform.src[1], addergraph.p_0_in0[3]);
connect<> net4(platform.src[1], addergraph.p_0_in0[4]);
connect<> net5(platform.src[1], addergraph.p_0_in0[5]);


connect<> net6(platform.src[2], addergraph.p_0_in1[0]);
connect<> net7(platform.src[2], addergraph.p_0_in1[3]);

connect<> net8(platform.src[3], addergraph.p_0_in1[1]);
connect<> net9(platform.src[3], addergraph.p_0_in1[4]);

connect<> net10(platform.src[4], addergraph.p_0_in1[2]);
connect<> net11(platform.src[4], addergraph.p_0_in1[5]);

connect<> net12(addergraph.p_0_out[0], platform.sink[0]);
connect<> net13(addergraph.p_0_out[1], platform.sink[1]);
connect<> net14(addergraph.p_0_out[2], platform.sink[2]);
connect<> net15(addergraph.p_0_out[3], platform.sink[3]);


#ifdef __AIESIM__
int main(int argc, char** argv) {
    addergraph.init();
    addergraph.run(1);
    addergraph.end();
    return 0;
}
#endif
