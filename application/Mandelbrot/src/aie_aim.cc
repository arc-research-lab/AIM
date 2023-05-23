

#include "aie_config.h"

void aie_aim(input_value_window* __restrict  in0, input_value_window* __restrict  in1, output_value_window* __restrict  out){

    v8acc80 acc_tmp = null_v8acc80();
    window_decr(out, 8 * 3);
    window_decr(in0, 8);

    for(uint32_t i = 0; i < COMPUTE_ELEMS / 8; ++i)
    chess_prepare_for_pipelining
    chess_loop_range(COMPUTE_ELEMS / 8, COMPUTE_ELEMS / 8)
    {
        v8acc80 res_upd = null_v8acc80();
        v16int32 b0, b_next;
        v8int32 a0, a_next;
  
        window_incr(in0, i * 8 + 8);
        window_decr(in1, i * 8 + 8);

        window_writeincr(out, srs(acc_tmp, 0));
        window_writeincr(out, srs(acc_tmp, 31));
        window_writeincr(out, srs(acc_tmp, 62));

        a0 = window_read_v8(in0);
        b0 = upd_w(b0, 1, window_readincr_v8(in1));
        b0 = upd_w(b0, 0, window_read_v8(in1));


        for(uint32_t j = 0; j <= i; ++j)
        chess_prepare_for_pipelining
        chess_loop_range(1, COMPUTE_ELEMS / 8)
        {
            window_decr(in0, 8);
            a_next = window_read_v8(in0);
            b_next = upd_w(b_next, 1, window_readincr_v8(in1));
            b_next = upd_w(b_next, 0, window_read_v8(in1));
            res_upd = lmac8(res_upd, b0,  0, 0x76543210, a0, 0, 0);
            res_upd = lmac8(res_upd, b0, -1, 0x76543210, a0, 1, 0);
            res_upd = lmac8(res_upd, b0, -2, 0x76543210, a0, 2, 0);
            res_upd = lmac8(res_upd, b0, -3, 0x76543210, a0, 3, 0);
            res_upd = lmac8(res_upd, b0, -4, 0x76543210, a0, 4, 0);
            res_upd = lmac8(res_upd, b0, -5, 0x76543210, a0, 5, 0);
            res_upd = lmac8(res_upd, b0, -6, 0x76543210, a0, 6, 0);
            res_upd = lmac8(res_upd, b0, -7, 0x76543210, a0, 7, 0);
            a0 = a_next;
            b0 = b_next;
        }
        
        acc_tmp = res_upd;
    }

    window_incr(in1, 8);
    window_decr(in0, 16);


    for(uint32_t i = 0; i < COMPUTE_ELEMS / 8; ++i)
    chess_prepare_for_pipelining
    chess_loop_range(COMPUTE_ELEMS / 8, COMPUTE_ELEMS / 8)
    {
        v8acc80 res_upd = null_v8acc80();
        v16int32 b0, b_next;
        v8int32 a0, a_next;
        window_incr(in0, (2 - i) * 8);
        window_incr(in1, (i) * 8);

        b0 = upd_w(b0, 1, window_readincr_v8(in1));
        b0 = upd_w(b0, 0, window_read_v8(in1));
        a0 = window_read_v8(in0);
        window_decr(in0, 8);
        window_writeincr(out, srs(acc_tmp, 0));
        window_writeincr(out, srs(acc_tmp, 31));
        window_writeincr(out, srs(acc_tmp, 62));

        for(uint32_t j = 0; j < COMPUTE_ELEMS / 8 - i; ++j)
        chess_prepare_for_pipelining
        chess_loop_range(1, COMPUTE_ELEMS / 8)
        {
            
            b_next = upd_w(b_next, 1, window_readincr_v8(in1));
            a_next = window_read_v8(in0);
            window_decr(in0, 8);

            b_next = upd_w(b_next, 0, window_read_v8(in1));
            

            res_upd = lmac8(res_upd, b0,  0, 0x76543210, a0, 0, 0);
            res_upd = lmac8(res_upd, b0, -1, 0x76543210, a0, 1, 0);
            res_upd = lmac8(res_upd, b0, -2, 0x76543210, a0, 2, 0);
            res_upd = lmac8(res_upd, b0, -3, 0x76543210, a0, 3, 0);
            res_upd = lmac8(res_upd, b0, -4, 0x76543210, a0, 4, 0);
            res_upd = lmac8(res_upd, b0, -5, 0x76543210, a0, 5, 0);
            res_upd = lmac8(res_upd, b0, -6, 0x76543210, a0, 6, 0);
            res_upd = lmac8(res_upd, b0, -7, 0x76543210, a0, 7, 0);
            b0 = b_next;
            a0 = a_next;

        }
        acc_tmp = res_upd;
    }
    window_writeincr(out, srs(acc_tmp, 0));
    window_writeincr(out, srs(acc_tmp, 31));
    window_writeincr(out, srs(acc_tmp, 62));

}