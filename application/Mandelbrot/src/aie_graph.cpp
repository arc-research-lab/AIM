#include "aie_graph.h"
using namespace adf;
PLIO* in0_0_0 = new PLIO("in0_0_0", adf::plio_128_bits, "data/input0_0.txt");
PLIO* in1_0_0 = new PLIO("in1_0_0", adf::plio_128_bits, "data/input1_0.txt");
PLIO* out_0_0 = new PLIO("out_0_0", adf::plio_128_bits, "data/output_0.txt");
simulation::platform<2, 1> platform(in0_0_0, in1_0_0, out_0_0);

simpleGraph addergraph;

connect<> net0(platform.src[0], addergraph.in0[0]);

connect<> net1(platform.src[1], addergraph.in1[0]);

connect<> net2(addergraph.out[0], platform.sink[0]);

#ifdef __AIESIM__
int main(int argc, char** argv) {
    addergraph.init();
    addergraph.run(30);
    addergraph.end();
    return 0;
}
#endif
