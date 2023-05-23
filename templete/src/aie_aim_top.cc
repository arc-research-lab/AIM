#include "aie_config.h"


void aie_aim_top(input_window_int32* __restrict in0, input_window_int32* __restrict in1, output_stream_acc80* __restrict acc_out){
    v16int32 b0 = null_v16int32();
    v8acc80 col_res = null_v8acc80();
    v8int32 a0 = null_v8int32();
    
    window_decr(in0, 8);
    a0 = window_read_v8(in0);
    window_decr(in0, 8);

    for(uint32_t col = 0; col < COL_SIZE / 8; ++col)
    chess_prepare_for_pipelining
    chess_loop_range(COL_SIZE  / 8, COL_SIZE  / 8)
    {
        b0 = upd_w(b0, 1, window_readincr_v8(in1));
        b0 = upd_w(b0, 0, window_read_v8(in1));
        col_res = null_v8acc80();

        for(uint32_t row = 0; row < ROW_SIZE / 8; ++row)
        chess_prepare_for_pipelining
        chess_loop_range(ROW_SIZE / 8, ROW_SIZE / 8)
        {
            v16int32 b_next;
            b_next = upd_w(b_next, 1, window_readincr_v8(in1));
            b_next = upd_w(b_next, 0, window_read_v8(in1));
            v8int32 a_next = window_read_v8(in0);
            window_decr(in0, 8);

            col_res = lmac8(col_res, b0,  0, 0x76543210, a0, 0, 0);
            col_res = lmac8(col_res, b0, -1, 0x76543210, a0, 1, 0);
            col_res = lmac8(col_res, b0, -2, 0x76543210, a0, 2, 0);
            col_res = lmac8(col_res, b0, -3, 0x76543210, a0, 3, 0);
            col_res = lmac8(col_res, b0, -4, 0x76543210, a0, 4, 0);
            col_res = lmac8(col_res, b0, -5, 0x76543210, a0, 5, 0);
            col_res = lmac8(col_res, b0, -6, 0x76543210, a0, 6, 0);
            col_res = lmac8(col_res, b0, -7, 0x76543210, a0, 7, 0);

            a0 = a_next;
            b0 = b_next;
        }

        window_decr(in1, ROW_SIZE);
        writeincr_v4(acc_out, ext_lo(col_res));
        writeincr_v4(acc_out, ext_hi(col_res));

    }
}