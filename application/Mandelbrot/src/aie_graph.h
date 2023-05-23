
#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <adf.h>
#include "aie_kernel.h"
#include "aie_config.h"

using namespace adf;

class simpleGraph : public graph {
   private:
    kernel aie_aim0[AIE_NUM * PE];
    // pktsplit<PKT_RATIO> sp_a[AIE_NUM / PKT_RATIO];
    // pktsplit<PKT_RATIO> sp_b[AIE_NUM / PKT_RATIO];
    // pktmerge<PKT_RATIO> mg_out[AIE_NUM / PKT_RATIO];
   public:
    
    port<input> in0[AIE_NUM * PE], in1[AIE_NUM * PE];
    port<output> out[AIE_NUM * PE];
    
    simpleGraph() {
        for(uint32_t i = 0; i < AIE_NUM * PE; ++i){
            // sp_a[i] = pktsplit<PKT_RATIO>::create();
            // sp_b[i] = pktsplit<PKT_RATIO>::create();
            // mg_out[i] = pktmerge<PKT_RATIO>::create();

            // connect<pktstream>(in0[i], sp_a[i].in[0]);
            // connect<pktstream>(in1[i], sp_b[i].in[0]);
            // connect<pktstream>(mg_out[i].out[0], out[i]);

            // for(int j = 0; j < PKT_RATIO; ++j){
                aie_aim0[i] = kernel::create(aie_aim);

                connect<window<INPUT_WINDOW_SIZE> >(in0[i], aie_aim0[i].in[0]);
                connect<window<INPUT_WINDOW_SIZE + 8 * 4> >(in1[i], aie_aim0[i].in[1]);
                connect<window<OUTPUT_WINDOW_SIZE>>((aie_aim0[i].out[0]), out[i]);

                source(aie_aim0[i]) = "aie_aim.cc";
                runtime<ratio>(aie_aim0[i]) = 1;

            // }
        }

    };
};

#endif /**********__GRAPH_H__**********/
