#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <adf.h>
#include "aie_kernel.h"
#include "aie_config.h"

using namespace adf;

class simpleGraph : public graph {
    private:
    kernel aie_aim_p0[6];
    public:
    port<input> p_0_in0[6], p_0_in1[6];
    port<input> p_0_out[6];

    simpleGraph() {

        aie_aim_p0[0] = kernel::create(aie_aim_top_bottom);
        connect<window<AIE_H * 4> >(p_0_in0[0], aie_aim_p0[0].in[0]);
        connect<window<AIE_W * 4 + AIE_H * 4> >(p_0_in1[0], aie_aim_p0[0].in[1]);
        source(aie_aim_p0[0]) = "aie_aim_top_bottom.cc";
        runtime<ratio>(aie_aim_p0[0]) = 1;

        aie_aim_p0[5] = kernel::create(aie_aim_top_bottom);
        connect<window<AIE_H * 4> >(p_0_in0[5], aie_aim_p0[5].in[0]);
        connect<window<AIE_W * 4 + AIE_H * 4> >(p_0_in1[5], aie_aim_p0[5].in[1]);
        source(aie_aim_p0[5]) = "aie_aim_top_bottom.cc";
        runtime<ratio>(aie_aim_p0[5]) = 1;
        
        aie_aim_p0[1] = kernel::create(aie_aim_top);
        connect<window<AIE_H * 4> >(p_0_in0[1], aie_aim_p0[1].in[0]);
        connect<window<AIE_W * 4 + AIE_H * 4> >(p_0_in1[1], aie_aim_p0[1].in[1]);
        source(aie_aim_p0[1]) = "aie_aim_top.cc";
        runtime<ratio>(aie_aim_p0[1]) = 1;
        
        aie_aim_p0[2] = kernel::create(aie_aim_top);
        connect<window<AIE_H * 4> >(p_0_in0[2], aie_aim_p0[2].in[0]);
        connect<window<AIE_W * 4 + AIE_H * 4> >(p_0_in1[2], aie_aim_p0[2].in[1]);
        source(aie_aim_p0[2]) = "aie_aim_top.cc";
        runtime<ratio>(aie_aim_p0[2]) = 1;
        
        aie_aim_p0[3] = kernel::create(aie_aim_bottom);
        connect<window<AIE_H * 4> >(p_0_in0[3], aie_aim_p0[3].in[0]);
        connect<window<AIE_W * 4 + AIE_H * 4> >(p_0_in1[3], aie_aim_p0[3].in[1]);
        source(aie_aim_p0[3]) = "aie_aim_bottom.cc";
        runtime<ratio>(aie_aim_p0[3]) = 1;
        
        aie_aim_p0[4] = kernel::create(aie_aim_bottom);
        connect<window<AIE_H * 4> >(p_0_in0[4], aie_aim_p0[4].in[0]);
        connect<window<AIE_W * 4 + AIE_H * 4> >(p_0_in1[4], aie_aim_p0[4].in[1]);
        source(aie_aim_p0[4]) = "aie_aim_bottom.cc";
        runtime<ratio>(aie_aim_p0[4]) = 1;
        
        connect<cascade> (aie_aim_p0[1].out[0], aie_aim_p0[3].in[2]);
        connect<cascade> (aie_aim_p0[2].out[0], aie_aim_p0[4].in[2]);
        connect<stream>(aie_aim_p0[0].out[0], p_0_out[0]);
        connect<stream>(aie_aim_p0[3].out[0], p_0_out[1]);
        connect<stream>(aie_aim_p0[4].out[0], p_0_out[2]);
        connect<stream>(aie_aim_p0[5].out[0], p_0_out[3]);
        adf::location<kernel>(aie_aim_p0[0]) = adf::tile(0, 0);
        adf::location<kernel>(aie_aim_p0[5]) = adf::tile(1, 0);
        adf::location<kernel>(aie_aim_p0[1]) = adf::tile(1, 1);
        adf::location<kernel>(aie_aim_p0[3]) = adf::tile(0, 1);
        adf::location<kernel>(aie_aim_p0[2]) = adf::tile(0, 2);
        adf::location<kernel>(aie_aim_p0[4]) = adf::tile(1, 2);

    };
};

#endif
