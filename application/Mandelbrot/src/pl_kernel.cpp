#include "config.h"
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define INDICATOR_NONE 0
#define INDICATOR_EXIT 1

#define INDICATOR_FINISHED_ONE_TASK 2
#define INDICATOR_CALCULATE_A_X_B 3
#define INDICATOR_CALCULATE_A_SQUARE 4
#define INDICATOR_CALCULATE_B_SQUARE 5

// void receive_acc80(hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0>>& task0_out, hls::stream<ap_uint<VALID_BITS * VEC_SIZE>> &p_res_s, uint32_t batch, uint32_t repeat){
//     for(uint32_t rep = 0; rep < repeat; ++rep){
// #pragma HLS loop_tripcount min=1 max=1 avg=1
//         for(uint32_t i = 0; i < batch; ++i){
// #pragma HLS loop_tripcount min=100 max=100 avg=100
//             ap_uint<2 * VALID_BITS> carry = 0;
//             for(uint32_t j = 0; j < COMPUTE_ELEMS * 2 / VEC_SIZE; ++j){
//                 ap_uint<VALID_BITS * VEC_SIZE + 2 * VALID_BITS> p_res = 0;
                
//                 for(uint32_t v = 0; v < 3; ++v){
//                     if(v == 0){
//                         p_res += carry;
//                     }
//                     ap_uint<VALID_BITS * VEC_SIZE + 2 * VALID_BITS> p_res_v = 0;

//                     qdma_axis<PLIO_BITS, 0, 0, 0> tmp1 = task0_out.read();
//                     const uint32_t idx0 = 0 * (PLIO_BITS / VEC_ELEM_BITS) + 0;
//                     p_res_v((idx0 + 1) * VALID_BITS - 1, idx0 * VALID_BITS) = tmp1.data(0 * VEC_ELEM_BITS + VALID_BITS - 1, 0 * VEC_ELEM_BITS);
//                     const uint32_t idx1 = 0 * (PLIO_BITS / VEC_ELEM_BITS) + 1;
//                     p_res_v((idx1 + 1) * VALID_BITS - 1, idx1 * VALID_BITS) = tmp1.data(1 * VEC_ELEM_BITS + VALID_BITS - 1, 1 * VEC_ELEM_BITS);
//                     const uint32_t idx2 = 0 * (PLIO_BITS / VEC_ELEM_BITS) + 2;
//                     p_res_v((idx2 + 1) * VALID_BITS - 1, idx2 * VALID_BITS) = tmp1.data(2 * VEC_ELEM_BITS + VALID_BITS - 1, 2 * VEC_ELEM_BITS);
//                     const uint32_t idx3 = 0 * (PLIO_BITS / VEC_ELEM_BITS) + 3;
//                     p_res_v((idx3 + 1) * VALID_BITS - 1, idx3 * VALID_BITS) = tmp1.data(3 * VEC_ELEM_BITS + VALID_BITS - 1, 3 * VEC_ELEM_BITS);

//                     qdma_axis<PLIO_BITS, 0, 0, 0> tmp2 = task0_out.read();
//                     const uint32_t idx4 = 1 * (PLIO_BITS / VEC_ELEM_BITS) + 0;
//                     p_res_v((idx4 + 1) * VALID_BITS - 1, idx4 * VALID_BITS) = tmp2.data(0 * VEC_ELEM_BITS + VALID_BITS - 1, 0 * VEC_ELEM_BITS);
//                     const uint32_t idx5 = 1 * (PLIO_BITS / VEC_ELEM_BITS) + 1;
//                     p_res_v((idx5 + 1) * VALID_BITS - 1, idx5 * VALID_BITS) = tmp2.data(1 * VEC_ELEM_BITS + VALID_BITS - 1, 1 * VEC_ELEM_BITS);
//                     const uint32_t idx6 = 1 * (PLIO_BITS / VEC_ELEM_BITS) + 2;
//                     p_res_v((idx6 + 1) * VALID_BITS - 1, idx6 * VALID_BITS) = tmp2.data(2 * VEC_ELEM_BITS + VALID_BITS - 1, 2 * VEC_ELEM_BITS);
//                     const uint32_t idx7 = 1 * (PLIO_BITS / VEC_ELEM_BITS) + 3;
//                     p_res_v((idx7 + 1) * VALID_BITS - 1, idx7 * VALID_BITS) = tmp2.data(3 * VEC_ELEM_BITS + VALID_BITS - 1, 3 * VEC_ELEM_BITS);

//                     p_res_v << (v * VALID_BITS);

// //                     for(uint32_t k = 0; k < VEC_SIZE / (PLIO_BITS / VEC_ELEM_BITS); ++k){
// // #pragma HLS pipeline II = 1
// //                         qdma_axis<PLIO_BITS, 0, 0, 0> tmp = task0_out.read();
// //                         for(uint32_t t = 0; t < (PLIO_BITS / VEC_ELEM_BITS); ++t){
// // #pragma HLS unroll
// //                             uint32_t idx = k * (PLIO_BITS / VEC_ELEM_BITS) + t + v;
// //                             p_res_v((idx + 1) * VALID_BITS - 1, idx * VALID_BITS) = tmp.data(t * VEC_ELEM_BITS + VALID_BITS - 1, t * VEC_ELEM_BITS);
// //                         }
// //                     }
//                     p_res += p_res_v;
//                     if(v == 2){
//                         carry = p_res(VALID_BITS * VEC_SIZE + 2 * VALID_BITS - 1, VALID_BITS * VEC_SIZE);
//                         p_res_s << ((ap_uint<VALID_BITS * VEC_SIZE>) p_res(VALID_BITS * VEC_SIZE - 1, 0));
//                     }
//                 }
//             }
//         }
//     }
// }

void receive_acc80(hls::stream<uint32_t> &aim_indicator1_s, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0>>& task0_out, hls::stream<ap_uint<VALID_BITS * VEC_SIZE>> &p_res_s, uint32_t batch, uint32_t repeat){
//     for(uint32_t rep = 0; rep < repeat; ++rep){
// #pragma HLS loop_tripcount min=1 max=1 avg=1
//         for(uint32_t i = 0; i < batch; ++i){
// #pragma HLS loop_tripcount min=100 max=100 avg=100
        uint32_t indicator = aim_indicator1_s.read();
        while(indicator != 0){
            ap_uint<2 * VALID_BITS> carry = 0;
            for(uint32_t j = 0; j < COMPUTE_ELEMS * 2 / VEC_SIZE; ++j){
                ap_uint<VALID_BITS * VEC_SIZE + 2 * VALID_BITS> p_res = 0;
                for(uint32_t v = 0; v < 3; ++v){
#pragma HLS pipeline II = 2
                    if(v == 0){
                        p_res += carry;
                    }
                    ap_uint<VALID_BITS * VEC_SIZE + 2 * VALID_BITS> p_res_v = 0;

                    qdma_axis<PLIO_BITS, 0, 0, 0> tmp1 = task0_out.read();
                    const uint32_t idx0 = 0 * (PLIO_BITS / VEC_ELEM_BITS) + 0;
                    p_res_v((idx0 + 1) * VALID_BITS - 1, idx0 * VALID_BITS) = tmp1.data(0 * VEC_ELEM_BITS + VALID_BITS - 1, 0 * VEC_ELEM_BITS);
                    const uint32_t idx1 = 0 * (PLIO_BITS / VEC_ELEM_BITS) + 1;
                    p_res_v((idx1 + 1) * VALID_BITS - 1, idx1 * VALID_BITS) = tmp1.data(1 * VEC_ELEM_BITS + VALID_BITS - 1, 1 * VEC_ELEM_BITS);
                    const uint32_t idx2 = 0 * (PLIO_BITS / VEC_ELEM_BITS) + 2;
                    p_res_v((idx2 + 1) * VALID_BITS - 1, idx2 * VALID_BITS) = tmp1.data(2 * VEC_ELEM_BITS + VALID_BITS - 1, 2 * VEC_ELEM_BITS);
                    const uint32_t idx3 = 0 * (PLIO_BITS / VEC_ELEM_BITS) + 3;
                    p_res_v((idx3 + 1) * VALID_BITS - 1, idx3 * VALID_BITS) = tmp1.data(3 * VEC_ELEM_BITS + VALID_BITS - 1, 3 * VEC_ELEM_BITS);

                    qdma_axis<PLIO_BITS, 0, 0, 0> tmp2 = task0_out.read();
                    const uint32_t idx4 = 1 * (PLIO_BITS / VEC_ELEM_BITS) + 0;
                    p_res_v((idx4 + 1) * VALID_BITS - 1, idx4 * VALID_BITS) = tmp2.data(0 * VEC_ELEM_BITS + VALID_BITS - 1, 0 * VEC_ELEM_BITS);
                    const uint32_t idx5 = 1 * (PLIO_BITS / VEC_ELEM_BITS) + 1;
                    p_res_v((idx5 + 1) * VALID_BITS - 1, idx5 * VALID_BITS) = tmp2.data(1 * VEC_ELEM_BITS + VALID_BITS - 1, 1 * VEC_ELEM_BITS);
                    const uint32_t idx6 = 1 * (PLIO_BITS / VEC_ELEM_BITS) + 2;
                    p_res_v((idx6 + 1) * VALID_BITS - 1, idx6 * VALID_BITS) = tmp2.data(2 * VEC_ELEM_BITS + VALID_BITS - 1, 2 * VEC_ELEM_BITS);
                    const uint32_t idx7 = 1 * (PLIO_BITS / VEC_ELEM_BITS) + 3;
                    p_res_v((idx7 + 1) * VALID_BITS - 1, idx7 * VALID_BITS) = tmp2.data(3 * VEC_ELEM_BITS + VALID_BITS - 1, 3 * VEC_ELEM_BITS);

                    p_res_v = p_res_v << (v * VALID_BITS);

                    p_res += p_res_v;
                    if(v == 2){
                        carry = p_res(VALID_BITS * VEC_SIZE + 2 * VALID_BITS - 1, VALID_BITS * VEC_SIZE);
                        p_res_s << ((ap_uint<VALID_BITS * VEC_SIZE>) p_res(VALID_BITS * VEC_SIZE - 1, 0));
                    }
                }
            }
            indicator = aim_indicator1_s.read();
        }
    // }
}

// void reorganize_send(hls::stream<ap_uint<DDR_BITS> > &s, uint32_t append_zeros, uint32_t batch, uint32_t repeat, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> >& plio, bool block_ddr)
// {
//     for(uint32_t rep = 0; rep < repeat; ++rep) {
// #pragma HLS loop_tripcount min=1 max=1 avg=1
//         for (uint32_t bch = 0; bch < batch; ++bch) {
// #pragma HLS loop_tripcount min=100 max=100 avg=100
//             ap_uint<DDR_BITS + PLIO_BITS> fifo = 0;
//             int sent_elem = 0;
//             int remains = 0;
//             for (uint32_t j = 0; j < TASK_INPUT_CHUNK; ++j) {
//                 // while (remains >= VALID_BITS * (PLIO_BITS / VEC_ELEM_BITS)) {
//                 for (uint32_t k = 0; k < 5; ++k){
// #pragma HLS loop_tripcount min=4 max=5 avg=4
// #pragma HLS pipeline II = 1
//                     if(k == 0){
//                         if(!block_ddr){
//                             fifo(remains + DDR_BITS - 1, remains) = s.read(); // buf_a[bch][i][j];
//                         }
//                         remains += DDR_BITS;
//                     }else if ( k == 4 && remains < VALID_BITS * (PLIO_BITS / VEC_ELEM_BITS)){
//                         continue;
//                     }
//                     ap_uint<PLIO_BITS> tmp = 0;
//                     tmp(0 * VEC_ELEM_BITS + VALID_BITS - 1, 0 * VEC_ELEM_BITS) = fifo((0 + 1) * VALID_BITS - 1, 0 * VALID_BITS);
//                     tmp(1 * VEC_ELEM_BITS + VALID_BITS - 1, 1 * VEC_ELEM_BITS) = fifo((1 + 1) * VALID_BITS - 1, 1 * VALID_BITS);
//                     tmp(2 * VEC_ELEM_BITS + VALID_BITS - 1, 2 * VEC_ELEM_BITS) = fifo((2 + 1) * VALID_BITS - 1, 2 * VALID_BITS);
//                     tmp(3 * VEC_ELEM_BITS + VALID_BITS - 1, 3 * VEC_ELEM_BITS) = fifo((3 + 1) * VALID_BITS - 1, 3 * VALID_BITS);

//                     fifo = fifo >> (VALID_BITS * (PLIO_BITS / VEC_ELEM_BITS));
//                     remains -= (VALID_BITS * (PLIO_BITS / VEC_ELEM_BITS));
//                     sent_elem++;
//                     qdma_axis<PLIO_BITS, 0, 0, 0> x;
//                     x.data = tmp;
//                     x.keep_all();
//                     plio.write(x);
//                 }
//             }
            
//             for (uint32_t j = 0; j < ((COMPUTE_ELEMS + 8) / (PLIO_BITS / VEC_ELEM_BITS)) - SENT_VALID_ELE; ++j) {
// #pragma HLS pipeline II = 1
//                 ap_uint<PLIO_BITS> tmp = 0;
//                 for (uint32_t k = 0; k < (PLIO_BITS / VEC_ELEM_BITS); ++k) {
// #pragma HLS unroll
//                     tmp(k * VEC_ELEM_BITS + VALID_BITS - 1, k * VEC_ELEM_BITS) = fifo((k + 1) * VALID_BITS - 1, k * VALID_BITS);
//                 }
//                 sent_elem++;
//                 fifo = fifo >> (VALID_BITS * (PLIO_BITS / VEC_ELEM_BITS));
//                 qdma_axis<PLIO_BITS, 0, 0, 0> x;
//                 x.data = tmp;
//                 x.keep_all();
//                 plio.write(x);
//             }
//         }
//     }
// }

void reorganize_send(hls::stream<uint32_t> &aim_indicator0_s, hls::stream<uint32_t> &aim_indicator1_s, hls::stream<uint32_t> &aim_indicator2_s, hls::stream<ap_uint<DDR_BITS> > &a_s, hls::stream<ap_uint<DDR_BITS> > &b_s, uint32_t batch, uint32_t repeat, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> >& plio_a, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> >& plio_b, bool block_ddr){
//     for(uint32_t rep = 0; rep < repeat; ++rep) {
// #pragma HLS loop_tripcount min=1 max=1 avg=1
//         for (uint32_t bch = 0; bch < batch; ++bch) {
// #pragma HLS loop_tripcount min=100 max=100 avg=100
        uint32_t aim_indicator = aim_indicator0_s.read();
        aim_indicator1_s << aim_indicator;
        aim_indicator2_s << aim_indicator;
        while(aim_indicator != 0){
            ap_uint<DDR_BITS + PLIO_BITS> fifo_a = 0;
            ap_uint<DDR_BITS + PLIO_BITS> fifo_b = 0;
            int sent_elem = 0;
            int remains = 0;
            for (uint32_t j = 0; j < TASK_INPUT_CHUNK; ++j) {
                // while (remains >= VALID_BITS * (PLIO_BITS / VEC_ELEM_BITS)) {
                for (uint32_t k = 0; k < 5; ++k){
#pragma HLS loop_tripcount min=4 max=5 avg=4
#pragma HLS pipeline II = 1
                    if(k == 0 && !block_ddr){
                            // ap_uint<DDR_BITS + PLIO_BITS> tmp_fifo_a = fifo_a << (PLIO_BITS - remains);
                            // ap_uint<DDR_BITS + PLIO_BITS> tmp_fifo_b = fifo_b << (PLIO_BITS - remains);
                            // ap_uint<DDR_BITS + PLIO_BITS> tmp_fifo_a = a_s.read(); // buf_a[bch][i][j];
                            // ap_uint<DDR_BITS + PLIO_BITS> tmp_fifo_b = b_s.read(); // buf_a[bch][i][j];
                            // fifo_a = tmp_fifo_a >> (PLIO_BITS - remains);
                            // fifo_b = tmp_fifo_b >> (PLIO_BITS - remains);

                        fifo_a(remains + DDR_BITS - 1, remains) = a_s.read();
                        fifo_b(remains + DDR_BITS - 1, remains) = b_s.read();

                        // fifo_a += tmp_fifo_a << remains;
                        // fifo_b += tmp_fifo_b << remains;
                        remains += DDR_BITS;
                    }else if(k == 0 && block_ddr){
                        remains += DDR_BITS;
                    }else if ( k == 4 && remains < VALID_BITS * (PLIO_BITS / VEC_ELEM_BITS)){
                        continue;
                    }
                    ap_uint<PLIO_BITS> tmp_a = 0;
                    ap_uint<PLIO_BITS> tmp_b = 0;

                    tmp_a(0 * VEC_ELEM_BITS + VALID_BITS - 1, 0 * VEC_ELEM_BITS) = fifo_a((0 + 1) * VALID_BITS - 1, 0 * VALID_BITS);
                    tmp_a(1 * VEC_ELEM_BITS + VALID_BITS - 1, 1 * VEC_ELEM_BITS) = fifo_a((1 + 1) * VALID_BITS - 1, 1 * VALID_BITS);
                    tmp_a(2 * VEC_ELEM_BITS + VALID_BITS - 1, 2 * VEC_ELEM_BITS) = fifo_a((2 + 1) * VALID_BITS - 1, 2 * VALID_BITS);
                    tmp_a(3 * VEC_ELEM_BITS + VALID_BITS - 1, 3 * VEC_ELEM_BITS) = fifo_a((3 + 1) * VALID_BITS - 1, 3 * VALID_BITS);

                    tmp_b(0 * VEC_ELEM_BITS + VALID_BITS - 1, 0 * VEC_ELEM_BITS) = fifo_b((0 + 1) * VALID_BITS - 1, 0 * VALID_BITS);
                    tmp_b(1 * VEC_ELEM_BITS + VALID_BITS - 1, 1 * VEC_ELEM_BITS) = fifo_b((1 + 1) * VALID_BITS - 1, 1 * VALID_BITS);
                    tmp_b(2 * VEC_ELEM_BITS + VALID_BITS - 1, 2 * VEC_ELEM_BITS) = fifo_b((2 + 1) * VALID_BITS - 1, 2 * VALID_BITS);
                    tmp_b(3 * VEC_ELEM_BITS + VALID_BITS - 1, 3 * VEC_ELEM_BITS) = fifo_b((3 + 1) * VALID_BITS - 1, 3 * VALID_BITS);

                    fifo_a = fifo_a >> (VALID_BITS * (PLIO_BITS / VEC_ELEM_BITS));
                    fifo_b = fifo_b >> (VALID_BITS * (PLIO_BITS / VEC_ELEM_BITS));

                    remains -= (VALID_BITS * (PLIO_BITS / VEC_ELEM_BITS));
                    sent_elem++;
                    qdma_axis<PLIO_BITS, 0, 0, 0> x_a;
                    x_a.data = tmp_a;
                    x_a.keep_all();
                    plio_a.write(x_a);

                    qdma_axis<PLIO_BITS, 0, 0, 0> x_b;
                    x_b.data = tmp_b;
                    x_b.keep_all();
                    plio_b.write(x_b);
                }
            }
            
            for (uint32_t j = 0; j < ((COMPUTE_ELEMS + 8) / (PLIO_BITS / VEC_ELEM_BITS)) - SENT_VALID_ELE; ++j) {
#pragma HLS pipeline II = 1
                ap_uint<PLIO_BITS> tmp_a = 0;
                ap_uint<PLIO_BITS> tmp_b = 0;
                for (uint32_t k = 0; k < (PLIO_BITS / VEC_ELEM_BITS); ++k) {
#pragma HLS unroll
                    tmp_a(k * VEC_ELEM_BITS + VALID_BITS - 1, k * VEC_ELEM_BITS) = fifo_a((k + 1) * VALID_BITS - 1, k * VALID_BITS);
                    tmp_b(k * VEC_ELEM_BITS + VALID_BITS - 1, k * VEC_ELEM_BITS) = fifo_b((k + 1) * VALID_BITS - 1, k * VALID_BITS);
                }
                sent_elem++;
                fifo_a = fifo_a >> (VALID_BITS * (PLIO_BITS / VEC_ELEM_BITS));
                fifo_b = fifo_b >> (VALID_BITS * (PLIO_BITS / VEC_ELEM_BITS));
                if(j < ((COMPUTE_ELEMS) / (PLIO_BITS / VEC_ELEM_BITS)) - SENT_VALID_ELE){
                    qdma_axis<PLIO_BITS, 0, 0, 0> x_a;
                    x_a.data = tmp_a;
                    x_a.keep_all();
                    plio_a.write(x_a);
                }
                qdma_axis<PLIO_BITS, 0, 0, 0> x_b;
                x_b.data = tmp_b;
                x_b.keep_all();
                plio_b.write(x_b);
            }

            aim_indicator = aim_indicator0_s.read();
            aim_indicator1_s << aim_indicator;
            aim_indicator2_s << aim_indicator;
        }
    // }
}

void read_ddr(ap_uint<DDR_BITS> *mem_a, hls::stream<ap_uint<DDR_BITS> > & a_s, uint32_t batch, uint32_t repeat, bool block_ddr){
    if(block_ddr) return;
    for(uint32_t rep = 0; rep < repeat; ++rep){
        uint32_t idx = 0;
        for (uint32_t i = 0; i < AIE_NUM; ++i) {
            for (uint32_t k = 0; k < batch; ++k) {
                for (uint32_t l = 0; l < TASK_INPUT_CHUNK; ++l) {
#pragma HLS pipeline II = 1
                    a_s << mem_a[idx++];
                }
            }
        }
    }
}

void write_ddr(ap_uint<DDR_BITS> *mem_out, hls::stream<ap_uint<DDR_BITS> > & c_s, uint32_t batch, uint32_t repeat, bool block_ddr){
    if(block_ddr) return;
    for(uint32_t rep = 0; rep < repeat; ++rep){
        uint32_t idx = 0;
        for (uint32_t i = 0; i < AIE_NUM; ++i) {
            for (uint32_t k = 0; k < batch; ++k) {
                for (uint32_t l = 0; l < TASK_OUTPUT_CHUNK; ++l) {
#pragma HLS pipeline II = 1
                    mem_out[idx++] = c_s.read();
                }
            }
        }
    }
}

void split_stream(hls::stream<ap_uint<DDR_BITS> > & s_in, hls::stream<ap_uint<DDR_BITS> > & s_out0, hls::stream<ap_uint<DDR_BITS> > & s_out1, uint32_t batch, uint32_t tails, uint32_t repeat, bool block_ddr){
    if(block_ddr) return;
    for(uint32_t rep = 0; rep < repeat; ++rep){
        for (uint32_t k = 0; k < batch; ++k) {
            for (uint32_t l = 0; l < TASK_INPUT_CHUNK; ++l) {
#pragma HLS pipeline II = 1
                s_out0 << s_in.read();
            }
            for(uint32_t j = 0; j < tails; ++j){
#pragma HLS pipeline II = 1
                s_out1 << s_in.read();
            }
        }
    }
}

void merge_stream(hls::stream<ap_uint<DDR_BITS> > & s_in0, hls::stream<ap_uint<DDR_BITS> > & s_in1, hls::stream<ap_uint<DDR_BITS> > & s_out, uint32_t batch, uint32_t tails, uint32_t repeat, bool block_ddr){
    if(block_ddr) return;
    for(uint32_t rep = 0; rep < repeat; ++rep){
        for (uint32_t k = 0; k < batch; ++k) {
            for (uint32_t l = 0; l < TASK_OUTPUT_CHUNK; ++l) {
#pragma HLS pipeline II = 1
                s_out << s_in0.read();
            }
            for(uint32_t j = 0; j < tails; ++j){
#pragma HLS pipeline II = 1
                s_out << s_in1.read();
            }
        }
    }
}

void crop_out(hls::stream<uint32_t> &aim_indicator2_s, hls::stream<ap_uint<VALID_BITS * VEC_SIZE>> &p_res_s, hls::stream<ap_uint<DDR_BITS> > &out_c, uint32_t batch, uint32_t repeat, bool block_ddr){
    // for(uint32_t rep = 0; rep < repeat; ++rep){
    //     for(uint32_t i = 0; i < batch; ++i){
        uint32_t indicator = aim_indicator2_s.read();
        while(indicator != 0){
            uint32_t output_idx = 0;
            ap_uint<DDR_BITS + VEC_SIZE * VALID_BITS> fifo_res = 0;
            int remains = 0;
            for(uint32_t j = 0; j < COMPUTE_ELEMS * 2 / VEC_SIZE; ++j){
#pragma HLS pipeline II = 1
                fifo_res(remains + VALID_BITS * VEC_SIZE - 1, remains) = p_res_s.read();
                remains += (output_idx < TASK_OUTPUT_CHUNK) ? (VALID_BITS * VEC_SIZE) : 0;
                if(remains >= DDR_BITS && output_idx < TASK_OUTPUT_CHUNK){
                    if(!block_ddr){
                        out_c << (ap_uint<DDR_BITS>) fifo_res(DDR_BITS - 1, 0);
                    }
                    fifo_res = fifo_res >> DDR_BITS;
                    remains -= DDR_BITS;
                    output_idx++;
                }
            }
            indicator = aim_indicator2_s.read();
        }
    // }
}




void mandelbrot_gen_real(const uint32_t width, const uint32_t height, \
                ap_uint<DDR_BITS> x0[INPUT_BITS / DDR_BITS], bool sign_x0, ap_uint<DDR_BITS> step0[INPUT_BITS / DDR_BITS], \
                hls::stream<ap_uint<DDR_BITS>> & real0_s, hls::stream<bool> & real_sign_s, \
                hls::stream<uint32_t> & addr_s){
    ap_uint<DDR_BITS> x0_buf[INPUT_BITS / DDR_BITS];
    ap_uint<DDR_BITS> real_buf[INPUT_BITS / DDR_BITS];
    ap_uint<DDR_BITS> step_buf[INPUT_BITS / DDR_BITS];
    uint16_t real_large = 0;
    uint16_t step0_large = 0;
    for(uint32_t i = 0; i < INPUT_BITS / DDR_BITS; ++i){
#pragma HLS pipeline II = 1
        x0_buf[i] = x0[i];
        step_buf[i] = step0[i];
        
        if(x0_buf[i] > step_buf[i]){
            real_large = i + 1;
        }else if(x0_buf[i] < step_buf[i]){
            step0_large = i + 1;
        }
    }
    bool x0_greater_step0 = (real_large >= step0_large);
    bool real_greater = x0_greater_step0;
    bool real_positive = sign_x0;
    
    bool next_real_sign = 0;
    for(uint32_t i = 0; i < height; ++i){
        for(uint32_t j = 0; j < width; ++j){
            uint16_t carry = 0;
            real_large = 0;
            step0_large = 0;
            for(uint32_t k = 0; k < INPUT_BITS / DDR_BITS; ++k){
#pragma HLS pipeline II = 1
                if(j == 0){
                    real_greater = x0_greater_step0;
                    real_positive = sign_x0;
                    real_buf[k] = x0_buf[k];
                }
                ap_uint<DDR_BITS + 1> res = 0;
                
                if(real_positive){
                    res = real_buf[k] + step_buf[k];
                    res += carry;
                    carry = res[DDR_BITS] == 1 ? 1 : 0;
                    next_real_sign = real_positive;
                }else if(real_greater){
                    res[DDR_BITS] = 1;
                    res(DDR_BITS - 1, 0) = real_buf[k];
                    res -= step_buf[k];
                    res -= carry;
                    carry = res[DDR_BITS] == 1 ? 0 : 1;
                    next_real_sign = real_positive;
                }else{
                    res[DDR_BITS] = 1;
                    res(DDR_BITS - 1, 0) = step_buf[k];
                    res -= real_buf[k];
                    res -= carry;
                    carry = res[DDR_BITS] == 1 ? 0 : 1;
                    next_real_sign = !real_positive;
                }

                if(res(DDR_BITS - 1, 0) > step_buf[k]){
                    real_large = k + 1;
                }else if(res(DDR_BITS - 1, 0) < step_buf[k]){
                    step0_large = k + 1;
                }
                
                real0_s << res(DDR_BITS - 1, 0);
                real_buf[k] = res(DDR_BITS - 1, 0);

                if(k == (INPUT_BITS / DDR_BITS - 1)){
                    
                    real_greater = (real_large > step0_large);
                    real_sign_s << next_real_sign;
                    real_positive = next_real_sign;
                    addr_s << (i * width + j);
                }
            }
        }
    }
}

void mandelbrot_gen0_image(const uint32_t height, \
                ap_uint<DDR_BITS> y0[INPUT_BITS / DDR_BITS], bool sign_y0, ap_uint<DDR_BITS> step1[INPUT_BITS / DDR_BITS], \
                hls::stream<ap_uint<DDR_BITS>> & image0_s, hls::stream<bool> & image_sign_s){
    ap_uint<DDR_BITS> image_buf[INPUT_BITS / DDR_BITS];
    ap_uint<DDR_BITS> step_buf[INPUT_BITS / DDR_BITS];
    uint16_t image_large = 0;
    uint16_t step1_large = 0;
    for(uint32_t i = 0; i < INPUT_BITS / DDR_BITS; ++i){
#pragma HLS pipeline II = 1
        image_buf[i] = y0[i];
        
        step_buf[i] = step1[i];
        if(image_buf[i] > step_buf[i]){
            image_large = i + 1;
        }else if(image_buf[i] < step_buf[i]){
            step1_large = i + 1;
        }
    }
    bool y0_greater_step1 = (image_large >= step1_large);
    bool image_greater = y0_greater_step1;
    bool image_positive = sign_y0;
    
    bool next_image_sign = 0;

    for(uint32_t i = 0; i < height; ++i){
        uint16_t carry = 0;
        image_large = 0;
        step1_large = 0;
        for(uint32_t k = 0; k < INPUT_BITS / DDR_BITS; ++k){
#pragma HLS pipeline II = 1
            ap_uint<DDR_BITS + 1> res = 0;
            if(image_positive){
                res(DDR_BITS - 1, 0) = image_buf[k];
                res += step_buf[k];
                res += carry;
                carry = res[DDR_BITS] == 1 ? 1 : 0;
                next_image_sign = image_positive;
            }else if(image_greater){
                res[DDR_BITS] = 1;
                res(DDR_BITS - 1, 0) = image_buf[k];
                res -= step_buf[k];
                res -= carry;
                carry = res[DDR_BITS] == 0 ? 1 : 0;
                next_image_sign = image_positive;
            }else{
                res[DDR_BITS] = 1;
                res(DDR_BITS - 1, 0) = step_buf[k];
                res -= image_buf[k];
                res -= carry;
                carry = res[DDR_BITS] == 0 ? 1 : 0;
                next_image_sign = !image_positive;
            }

            if(res(DDR_BITS - 1, 0) > step_buf[k]){
                image_large = k + 1;
            }else if(res(DDR_BITS - 1, 0) < step_buf[k]){
                step1_large = k + 1;
            }
            
            image0_s << res(DDR_BITS - 1, 0);
            image_buf[k] = res(DDR_BITS - 1, 0);
            if(k == (INPUT_BITS / DDR_BITS - 1)){
                image_greater = (image_large > step1_large);
                image_sign_s << next_image_sign;
                image_positive = next_image_sign;
            }
        }
    }

}


void mandelbrot_gen1_image(const uint32_t width, const uint32_t height, hls::stream<ap_uint<DDR_BITS>> & image_in_s, hls::stream<bool> & image_sign_in_s, hls::stream<ap_uint<DDR_BITS>> & image_out_s, hls::stream<bool> & image_sign_out_s){
    
    ap_uint<DDR_BITS> tmp_buf[INPUT_BITS / DDR_BITS];
    bool image_sign = 0;
    for(uint32_t i = 0; i < height; ++i){
        for(uint32_t j = 0; j < width; ++j){
            for(uint32_t k = 0; k < INPUT_BITS / DDR_BITS; ++k){
#pragma HLS pipeline II = 1
                if(j == 0 && k == 0){
                    tmp_buf[k] = image_in_s.read();
                    image_sign = image_sign_in_s.read();
                }else if(j == 0){
                    tmp_buf[k] = image_in_s.read();
                }
                image_out_s << tmp_buf[k];
                if(k == INPUT_BITS / DDR_BITS - 1){
                    image_sign_out_s << image_sign;
                }
            }
        }
    }
}

void select_sender(const uint32_t total_pixels, \
        hls::stream<ap_uint<DDR_BITS>> & orig_real_s, hls::stream<bool> & orig_real_sign_s, hls::stream<ap_uint<DDR_BITS>> & orig_image_s, hls::stream<bool> & orig_image_sign_s, hls::stream<uint32_t> & orig_addr_s,/* x0 y0 */ \
        hls::stream<ap_uint<DDR_BITS>> & a_x_b_real_s, hls::stream<ap_uint<DDR_BITS>> & a_x_b_image_s, /* calculate a * b */ \
        hls::stream<ap_uint<DDR_BITS>> & next_real_s, hls::stream<bool> & next_real_sign_s, hls::stream<ap_uint<DDR_BITS>> & next_image_s, hls::stream<bool> & next_image_sign_s, /*  */ \
        hls::stream<ap_uint<DDR_BITS>> & in_a0_s, hls::stream<bool> & in_a0_sign_s, hls::stream<ap_uint<DDR_BITS>> & in_b0_s, hls::stream<bool> & in_b0_sign_s, hls::stream<uint32_t> & in_addr_s, hls::stream<uint32_t> & in_itr_s, /* back up a0 & b0 every iteration */ \
        hls::stream<ap_uint<DDR_BITS>> & mul_in0_s, hls::stream<ap_uint<DDR_BITS>> & mul_in1_s, /* to AIM multiplier */ \
        hls::stream<ap_uint<DDR_BITS>> & operand0_s, hls::stream<bool> & operand0_sign_s, hls::stream<ap_uint<DDR_BITS>> & operand1_s, hls::stream<bool> & operand1_sign_s, /* backup operands */ \
        hls::stream<ap_uint<DDR_BITS>> & out_a0_s, hls::stream<bool> & out_a0_sign_s, hls::stream<ap_uint<DDR_BITS>> & out_b0_s, hls::stream<bool> & out_b0_sign_s, hls::stream<uint32_t> & out_addr_s, hls::stream<uint32_t> & out_itr_s, /* back up a0 & b0 every iteration */ \
        hls::stream<uint32_t> & recv_indicator_s, hls::stream<bool> &finish_notify_s, \
        hls::stream<uint32_t> & aim_indicator0_s){
    for(uint32_t i = 0; i < total_pixels; ){
        if(!finish_notify_s.empty()){
            finish_notify_s.read();
            ++i;
        }else if(!a_x_b_real_s.empty()){
            aim_indicator0_s << 1;
            for(uint32_t j = 0; j < INPUT_BITS / DDR_BITS; ++j){
#pragma HLS pipeline II = 1
                mul_in0_s << a_x_b_real_s.read();
                mul_in1_s << a_x_b_image_s.read();
                if(INPUT_BITS / DDR_BITS - 1 == j){
                    recv_indicator_s << (INDICATOR_CALCULATE_A_X_B);
                }
            }
        }else if(!next_real_s.empty()){
            aim_indicator0_s << 1;
            for(uint32_t j = 0; j < INPUT_BITS / DDR_BITS; ++j){
#pragma HLS pipeline II = 1
                ap_uint<DDR_BITS> real = next_real_s.read();
                mul_in0_s << real;
                mul_in1_s << real;
                operand0_s << real;
                out_a0_s << in_a0_s.read();
                if(INPUT_BITS / DDR_BITS - 1 == j){
                    operand0_sign_s << next_real_sign_s.read();
                    out_a0_sign_s << in_a0_sign_s.read();
                    uint32_t itr = in_itr_s.read();
                    itr = itr + 1;
                    out_itr_s << itr;
                    out_addr_s << in_addr_s.read();
                    recv_indicator_s << (INDICATOR_CALCULATE_A_SQUARE);
                }
            }
            aim_indicator0_s << 1;
            for(uint32_t j = 0; j < INPUT_BITS / DDR_BITS; ++j){
#pragma HLS pipeline II = 1
                ap_uint<DDR_BITS> image = next_image_s.read();
                mul_in0_s << image;
                mul_in1_s << image;
                operand1_s << image;
                out_b0_s << in_b0_s.read();
                if(INPUT_BITS / DDR_BITS - 1 == j){
                    operand1_sign_s << next_image_sign_s.read();
                    out_b0_sign_s << in_b0_sign_s.read();
                    recv_indicator_s << (INDICATOR_CALCULATE_B_SQUARE);
                }
            }
        }else if(!orig_real_s.empty() && !operand0_s.full() && !out_a0_s.full() && !recv_indicator_s.full()){
            aim_indicator0_s << 1;
            for(uint32_t j = 0; j < INPUT_BITS / DDR_BITS; ++j){
#pragma HLS pipeline II = 1
                ap_uint<DDR_BITS> real = orig_real_s.read();
                mul_in0_s << real;
                mul_in1_s << real;
                operand0_s << real;
                out_a0_s << real;
                if(INPUT_BITS / DDR_BITS - 1 == j){
                    bool real_sign = orig_real_sign_s.read();
                    operand0_sign_s << real_sign;
                    out_a0_sign_s << real_sign;
                    out_addr_s << orig_addr_s.read();
                    out_itr_s << 0;
                    recv_indicator_s << (INDICATOR_CALCULATE_A_SQUARE);
                }
            }
            aim_indicator0_s << 1;
            for(uint32_t j = 0; j < INPUT_BITS / DDR_BITS; ++j){
#pragma HLS pipeline II = 1
                ap_uint<DDR_BITS> image = orig_image_s.read();
                mul_in0_s << image;
                mul_in1_s << image;
                operand1_s << image;
                out_b0_s << image;
                if(INPUT_BITS / DDR_BITS - 1 == j){
                    bool image_sign = orig_image_sign_s.read();
                    operand1_sign_s << image_sign;
                    out_b0_sign_s << image_sign;
                    recv_indicator_s << (INDICATOR_CALCULATE_B_SQUARE);
                }
            }
        }
    }
    aim_indicator0_s << 0;
}

void recv_split(hls::stream<uint32_t> & recv_indicator_s, /*  */ \
                hls::stream<bool> & finish_notify_s, const uint32_t total_pixels, \
                hls::stream<ap_uint<DDR_BITS>> & mul_res_s, \
                hls::stream<ap_uint<DDR_BITS>> & a2_s, hls::stream<ap_uint<DDR_BITS>> & b2_s, hls::stream<ap_uint<DDR_BITS>> & a_x_b_s \
){
    for(uint32_t i = 0; i < total_pixels;){
        if(!finish_notify_s.empty()){
            finish_notify_s.read();
            ++i;
        }else if(!mul_res_s.empty()){
            uint32_t indicator = INDICATOR_NONE;
            for(uint32_t j = 0; j < INPUT_BITS * 2 / DDR_BITS; ++j){
#pragma HLS pipeline II = 1
                if(j == 0){
                    indicator = recv_indicator_s.read();
                }
                ap_uint<DDR_BITS> res = mul_res_s.read();
                if(INDICATOR_CALCULATE_A_SQUARE == indicator){
                    a2_s << res;
                }else if(INDICATOR_CALCULATE_B_SQUARE == indicator){
                    b2_s << res;
                }else if(INDICATOR_CALCULATE_A_X_B){
                    a_x_b_s << res;
                }
            }
        }
    }
}

// a^2 + b^2 & a^2 - b^2
void square_add_sub(hls::stream<bool> & finish_notify_s, const uint32_t total_pixels, \
                    hls::stream<ap_uint<DDR_BITS>> & a2_s, hls::stream<ap_uint<DDR_BITS>> & b2_s, \
                    hls::stream<ap_uint<DDR_BITS>> & a2_add_b2_s, hls::stream<ap_uint<DDR_BITS>> & a2_sub_b2_s, hls::stream<ap_uint<DDR_BITS>> & b2_sub_a2_s, hls::stream<bool> & a2_sub_b2_sign_s){
    for(uint32_t i = 0; i < total_pixels;){
        if(!finish_notify_s.empty()){
            finish_notify_s.read();
            ++i;
        }else if(!a2_s.empty()){
            uint32_t add_carry = 0;
            uint32_t sub_carry0 = 0;
            uint32_t sub_carry1 = 0;
            uint32_t a2_large = 0;
            uint32_t b2_large = 0;
            for(uint32_t j = 0; j < INPUT_BITS * 2 / DDR_BITS; ++j){
#pragma HLS pipeline II = 1
                ap_uint<DDR_BITS> a2 = a2_s.read();
                ap_uint<DDR_BITS> b2 = b2_s.read();
                ap_uint<DDR_BITS + 1> add_res = 0;
                ap_uint<DDR_BITS + 1> sub_res0 = 0;
                ap_uint<DDR_BITS + 1> sub_res1 = 0;

                sub_res0[DDR_BITS] = 1;
                sub_res1[DDR_BITS] = 1;

                add_res(DDR_BITS - 1, 0) = a2(DDR_BITS - 1, 0);
                sub_res0(DDR_BITS - 1, 0) = a2(DDR_BITS - 1, 0);
                sub_res1(DDR_BITS - 1, 0) = b2(DDR_BITS - 1, 0);

                add_res += b2;
                add_res += add_carry;

                sub_res0 -= b2;
                sub_res0 -= sub_carry0;

                sub_res1 -= a2;
                sub_res1 -= sub_carry1;

                a2_add_b2_s << add_res(DDR_BITS - 1, 0);
                a2_sub_b2_s << sub_res0(DDR_BITS - 1, 0);
                b2_sub_a2_s << sub_res1(DDR_BITS - 1, 0);

                add_carry = add_res[DDR_BITS];
                sub_carry0 = 1 - sub_res0[DDR_BITS];
                sub_carry1 = 1 - sub_res1[DDR_BITS];


                if(a2 > b2){
                    a2_large = j + 1;
                }else if(b2 > a2){
                    b2_large = j + 1;
                }

                if((INPUT_BITS * 2 / DDR_BITS - 1 == j) && (a2_large > b2_large)){
                    a2_sub_b2_sign_s << true;
                }else if(INPUT_BITS * 2 / DDR_BITS - 1 == j){
                    a2_sub_b2_sign_s << false;
                }
            }
        }
    }
}

void sel_sub(hls::stream<bool> & discard_notify_s, const uint32_t total_pixels, \
            hls::stream<ap_uint<DDR_BITS>> & a2_sub_b2_s, hls::stream<ap_uint<DDR_BITS>> & b2_sub_a2_s, hls::stream<bool> & a2_sub_b2_sign_s, \
            hls::stream<ap_uint<DDR_BITS>> & output_s, hls::stream<bool> & output_sign_s){
    for(uint32_t i = 0; i < total_pixels;){
        bool discard = discard_notify_s.read();
        bool sign = a2_sub_b2_sign_s.read();
        if(discard){
            ++i;
        }else{
            output_sign_s << sign;
        }
        for(uint32_t j = 0; j < INPUT_BITS * 2 / DDR_BITS; ++j){
#pragma HLS pipeline II = 1
            ap_uint<DDR_BITS> a2_sub_b2 = a2_sub_b2_s.read();
            ap_uint<DDR_BITS> b2_sub_a2 = b2_sub_a2_s.read();
            if(sign && !discard){
                output_s << a2_sub_b2;
            }else if(!discard){
                output_s << b2_sub_a2;
            }
        }
    }
}

void shift_module(hls::stream<bool> & finish_notify_s, const uint32_t total_pixels, \
                    hls::stream<ap_uint<DDR_BITS>> & input_s, hls::stream<ap_uint<DDR_BITS>> & output_s, const uint32_t chunk_pos, const uint32_t bit_pos){
    for(uint32_t i = 0; i < total_pixels;){
        if(!finish_notify_s.empty()){
            finish_notify_s.read();
            ++i;
        }else if(!input_s.empty()){
            ap_uint<DDR_BITS> chunk = 0;
            for(uint32_t j = 0; j < INPUT_BITS * 2 / DDR_BITS; ++j){
#pragma HLS pipeline II = 1
                if(j < chunk_pos){
                    input_s.read();
                }else if((bit_pos != 0) && (j == chunk_pos)){
                    ap_uint<DDR_BITS> tmp = input_s.read();
                    // chunk += (tmp >> bit_pos); 
                    chunk(DDR_BITS - bit_pos - 1, 0) = tmp(DDR_BITS - 1, bit_pos);
                }else if((bit_pos != 0) && (j <= chunk_pos + INPUT_BITS / DDR_BITS)){
                    ap_uint<DDR_BITS> tmp = input_s.read();
                    // chunk += (tmp << bit_pos); 
                    chunk(DDR_BITS - 1, DDR_BITS - bit_pos) = tmp(bit_pos - 1, 0); 
                    output_s << chunk;
                    // chunk = 0;
                    chunk(DDR_BITS - bit_pos - 1, 0) = tmp(DDR_BITS - 1, bit_pos); // (tmp >> bit_pos); 
                }else if((bit_pos == 0) && (j < chunk_pos + INPUT_BITS / DDR_BITS)){
                    output_s << input_s.read();
                }else{
                    input_s.read();
                }
            }
        }
    }
}

void compare_a2_add_b2(ap_uint<DDR_BITS> * thresh, const uint32_t max_itr, const uint32_t total_pixels, hls::stream<ap_uint<DDR_BITS>> & input_s, hls::stream<bool> & finished_s, \
                        hls::stream<uint32_t> & curr_itr_s, hls::stream<uint32_t> & curr_addr_s, \
                        hls::stream<uint32_t> & next_itr_s, hls::stream<uint32_t> & next_addr_s, \
                        hls::stream<uint32_t> & res_itr_s, hls::stream<uint32_t> & res_addr_s, \
                        hls::stream<bool> & sub_discard_notify_s, hls::stream<bool> & const_discard_notify_s, hls::stream<bool> & feed_a_x_b_discard_notify_s){
    ap_uint<DDR_BITS> thresh_buf[INPUT_BITS / DDR_BITS * 2];
    for(uint32_t i = 0; i < INPUT_BITS / DDR_BITS * 2; ++i){
#pragma HLS pipeline II = 1
        thresh_buf[i] = thresh[i];
    }
    for(uint32_t i = 0; i < total_pixels;){

        uint32_t itr = curr_itr_s.read();
        uint32_t addr = curr_addr_s.read();

        uint32_t sum_large = 0;
        uint32_t thresh_large = 0;
        for(uint32_t j = 0; j < INPUT_BITS / DDR_BITS * 2; ++j){
#pragma HLS pipeline II = 1
            ap_uint<DDR_BITS> sum = input_s.read();
            if(sum > thresh_buf[j]){
                sum_large = j + 1;
            }else if(sum < thresh_buf[j]){
                thresh_large = j + 1;
            }
        }

        if((sum_large > thresh_large) || (itr >= max_itr)){
            finished_s << true;
            sub_discard_notify_s << true;
            const_discard_notify_s << true;
            feed_a_x_b_discard_notify_s << true;
            ++i;
            res_itr_s << itr;
            res_addr_s << addr;
        }else{
            next_itr_s << itr;
            next_addr_s << addr;
            sub_discard_notify_s << false;
            const_discard_notify_s << false;
            feed_a_x_b_discard_notify_s << false;
        }

    }
}

void finish_notifiers(const uint32_t total_pixels, hls::stream<bool> & in_s, \
                    hls::stream<bool> & out0_s, hls::stream<bool> & out1_s, hls::stream<bool> & out2_s, \
                    hls::stream<bool> & out3_s, hls::stream<bool> & out4_s, hls::stream<bool> & out5_s, \
                    hls::stream<bool> & out6_s, hls::stream<bool> & out7_s, hls::stream<bool> & out8_s){
    for(uint32_t i = 0; i < total_pixels; ++i){
#pragma HLS pipeline II = 1
        in_s.read();
        out0_s << 1;
        out1_s << 1;
        out2_s << 1;
        out3_s << 1;
        out4_s << 1;
        out5_s << 1;
        out6_s << 1;
        out7_s << 1;
        out8_s << 1;
    }
}

void sign_add(hls::stream<bool> & finish_notify_s, const uint32_t total_pixels, \
                hls::stream<ap_uint<DDR_BITS>> & input_s, hls::stream<bool> & input_sign_s, \
                hls::stream<ap_uint<DDR_BITS>> & const_s, hls::stream<bool> & const_sign_s,
                hls::stream<ap_uint<DDR_BITS>> & out0_s, hls::stream<ap_uint<DDR_BITS>> & out1_s, hls::stream<ap_uint<DDR_BITS>> & out2_s, hls::stream<uint32_t> & out_sel_s, hls::stream<bool> & out_sign_s){
    for(uint32_t i = 0; i < total_pixels; ){
        if(!finish_notify_s.empty()){
            finish_notify_s.read();
            ++i;
        }else if(!input_s.empty()){
            uint32_t input_large = 0;
            uint32_t const_large = 0;
            uint32_t add_carry = 0;
            uint32_t sub_carry0 = 0;
            uint32_t sub_carry1 = 0;

            for(uint32_t j = 0; j < INPUT_BITS / DDR_BITS; ++j){
#pragma HLS pipeline II = 1
                ap_uint<DDR_BITS> op0 = input_s.read();
                ap_uint<DDR_BITS> op1 = const_s.read();

                ap_uint<DDR_BITS + 1> val0 = 0;
                ap_uint<DDR_BITS + 1> val1 = 0;
                ap_uint<DDR_BITS + 1> val2 = 0;

                val0 = op0;
                val1 = op0;
                val2 = op1;

                val1[DDR_BITS] = 1;
                val2[DDR_BITS] = 1;

                val0 += op1;
                val0 += add_carry;

                val1 -= op1;
                val1 -= sub_carry0;

                val2 -= op0;
                val2 -= sub_carry1;

                out0_s << val0(DDR_BITS - 1, 0);
                out1_s << val1(DDR_BITS - 1, 0);
                out2_s << val2(DDR_BITS - 1, 0);

                add_carry = val0[DDR_BITS];
                sub_carry0 = 1 - val1[DDR_BITS];
                sub_carry1 = 1 - val2[DDR_BITS];


                if(op0 > op1){
                    input_large = j + 1;
                }else if(op0 < op1){
                    const_large = j + 1;
                }

                if((INPUT_BITS / DDR_BITS - 1 == j)){
                    bool input_sign = input_sign_s.read();
                    bool const_sign = const_sign_s.read();

                    if(input_sign == const_sign){
                        out_sel_s << 0;
                        out_sign_s << input_sign;
                    }else if(input_large > const_large){
                        out_sel_s << 1;
                        out_sign_s << input_sign;
                    }else{
                        out_sel_s << 2;
                        out_sign_s << (const_sign);
                    }
                }
            }
        }
    }
}


void sign_add_sel(hls::stream<bool> & finish_notify_s, const uint32_t total_pixels, \
                hls::stream<ap_uint<DDR_BITS>> & v0_s, hls::stream<ap_uint<DDR_BITS>> & v1_s, hls::stream<ap_uint<DDR_BITS>> & v2_s, hls::stream<uint32_t> & out_sel_s,
                hls::stream<ap_uint<DDR_BITS>> & output_s){
    for(uint32_t i = 0; i < total_pixels;){
        if(!finish_notify_s.empty()){
            finish_notify_s.read();
            ++i;
        }else if(!out_sel_s.empty()){
            uint32_t sel = 0;
            for(uint32_t j = 0; j < INPUT_BITS / DDR_BITS; ++j){
#pragma HLS pipeline II = 1
                if(0 == j){
                    sel = out_sel_s.read();
                }
                ap_uint<DDR_BITS> val0 = v0_s.read();
                ap_uint<DDR_BITS> val1 = v1_s.read();
                ap_uint<DDR_BITS> val2 = v2_s.read();

                if(0 == sel){
                    output_s << val0;
                }else if(1 == sel){
                    output_s << val1;
                }else{
                    output_s << val2;
                }

            }
        }
    }
}

void feed_back_a_x_b(const uint32_t total_pixels, hls::stream<bool> & discard_notify_s,\
                hls::stream<ap_uint<DDR_BITS>> & in_real_s, hls::stream<bool> & in_real_sign_s, hls::stream<ap_uint<DDR_BITS>> & in_image_s, hls::stream<bool> & in_image_sign_s, \
                hls::stream<ap_uint<DDR_BITS>> & back_real_s, hls::stream<ap_uint<DDR_BITS>> & back_image_s, hls::stream<bool> & back_a_x_b_sign_s){
    for(uint32_t i = 0; i < total_pixels;){
        bool discard = discard_notify_s.read();
        if(discard){
            ++i;
            for(uint32_t j = 0; j < INPUT_BITS / DDR_BITS; ++j){
#pragma HLS pipeline II = 1
                if(0 == j){
                    in_real_sign_s.read();
                    in_image_sign_s.read();
                }
                in_real_s.read();
                in_image_s.read();
            }
        }else{
            for(uint32_t j = 0; j < INPUT_BITS / DDR_BITS; ++j){
#pragma HLS pipeline II = 1
                if(0 == j){
                    bool real_sign = in_real_sign_s.read();
                    bool image_sign = in_image_sign_s.read();
                    back_a_x_b_sign_s << (real_sign == image_sign);
                }
                back_real_s << in_real_s.read();
                back_image_s << in_image_s.read();
            }
        }
    }
}

void discard_a0_b0(const uint32_t total_pixels, hls::stream<bool> & discard_notify_s, \
                    hls::stream<ap_uint<DDR_BITS>> & in_a0_s, hls::stream<bool> & in_a0_sign_s, hls::stream<ap_uint<DDR_BITS>> & in_b0_s, hls::stream<bool> & in_b0_sign_s, \
                    hls::stream<ap_uint<DDR_BITS>> & out_a0_s, hls::stream<bool> & out_a0_sign_s, hls::stream<ap_uint<DDR_BITS>> & out_b0_s, hls::stream<bool> & out_b0_sign_s, \
                    hls::stream<ap_uint<DDR_BITS>> & next_a0_s, hls::stream<bool> & next_a0_sign_s, hls::stream<ap_uint<DDR_BITS>> & next_b0_s, hls::stream<bool> & next_b0_sign_s){
    for(uint32_t i = 0; i < total_pixels; ){
        bool discard = discard_notify_s.read();
        bool a0_sign = in_a0_sign_s.read();
        bool b0_sign = in_b0_sign_s.read();
        
        for(uint32_t j = 0; j < INPUT_BITS / DDR_BITS; ++j){
#pragma HLS pipeline II = 1
            if(!discard && j == 0){
                out_a0_sign_s << a0_sign;
                next_a0_sign_s << a0_sign;

                out_b0_sign_s << b0_sign;
                next_b0_sign_s << b0_sign;
            }else if(0 == j){
                ++i;
            }
            ap_uint<DDR_BITS> a0 = in_a0_s.read();
            ap_uint<DDR_BITS> b0 = in_b0_s.read();
            if(!discard){
                out_a0_s << a0;
                next_a0_s << a0;

                out_b0_s << b0;
                next_b0_s << b0;
            }
        }
    }
}

void iterate(const uint32_t total_pixels, hls::stream<bool> & finish_notify_s, \
            hls::stream<uint32_t> & in_itr_s, hls::stream<uint32_t> & in_addr_s, \
            hls::stream<ap_uint<DDR_BITS>> & in_real_s, hls::stream<bool> & in_real_sign_s, hls::stream<ap_uint<DDR_BITS>> & in_image_s, hls::stream<bool> & in_image_sign_s, \
            hls::stream<ap_uint<DDR_BITS>> & out_real_s, hls::stream<bool> & out_real_sign_s, hls::stream<ap_uint<DDR_BITS>> & out_image_s, hls::stream<bool> & out_image_sign_s, \
            hls::stream<uint32_t> & out_itr_s, hls::stream<uint32_t> & out_addr_s){
    for(uint32_t i = 0; i < total_pixels;){
        if(!finish_notify_s.empty()){
            finish_notify_s.read();
            ++i;
        }else if(!in_real_s.empty() && !in_image_s.empty()){
            for(uint32_t j = 0; j < INPUT_BITS / DDR_BITS; ++j){
#pragma HLS pipeline II = 1
                if(0 == j){
                    out_itr_s << in_itr_s.read();
                    out_addr_s << in_addr_s.read();
                    out_real_sign_s << in_real_sign_s.read();
                }
                out_real_s << in_real_s.read();
            }
            for(uint32_t j = 0; j < INPUT_BITS / DDR_BITS; ++j){
#pragma HLS pipeline II = 1
                if(0 == j){
                    out_image_sign_s << in_image_sign_s.read();
                }
                out_image_s << in_image_s.read();
            }
        }
    }   
}

void write_image(uint32_t *mem_out, hls::stream<uint32_t> &out_addr_s, hls::stream<uint32_t> &out_itr_count_s, const uint32_t total_pixels){
    for(uint32_t i = 0; i < total_pixels; ++i){
#pragma HLS pipeline II = 1
        uint32_t addr = out_addr_s.read();
        mem_out[addr] = out_itr_count_s.read();
    }
}

extern "C" {

    void pl_kernel(ap_uint<DDR_BITS> *threshold, ap_uint<DDR_BITS> *x0, bool sign_x0, ap_uint<DDR_BITS> *y0, bool sign_y0, ap_uint<DDR_BITS> *step0, ap_uint<DDR_BITS> *step1, uint32_t *mem_out, uint32_t width, uint32_t height, uint32_t norm_chunk_real, uint32_t norm_bits_real, uint32_t norm_chunk_image, uint32_t norm_bits_image, uint32_t max_iteration, uint32_t repeat, bool block_ddr, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &plio_a0, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &plio_b0, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &plio_out0){
#pragma HLS interface m_axi offset=slave bundle=gmem0 port=threshold max_read_burst_length=32 num_read_outstanding=32
#pragma HLS interface m_axi offset=slave bundle=gmem1 port=x0 max_read_burst_length=32 num_read_outstanding=32
#pragma HLS interface m_axi offset=slave bundle=gmem2 port=y0 max_read_burst_length=32 num_read_outstanding=32
#pragma HLS interface m_axi offset=slave bundle=gmem3 port=step0 max_read_burst_length=32 num_read_outstanding=32
#pragma HLS interface m_axi offset=slave bundle=gmem4 port=step1 max_read_burst_length=32 num_read_outstanding=32
#pragma HLS interface m_axi offset=slave bundle=gmem5 port=mem_out max_read_burst_length=32 num_read_outstanding=32

#pragma HLS interface s_axilite bundle=control port=threshold
#pragma HLS interface s_axilite bundle=control port=x0
#pragma HLS interface s_axilite bundle=control port=y0
#pragma HLS interface s_axilite bundle=control port=sign_x0
#pragma HLS interface s_axilite bundle=control port=sign_y0
#pragma HLS interface s_axilite bundle=control port=mem_out
#pragma HLS interface s_axilite bundle=control port=width
#pragma HLS interface s_axilite bundle=control port=height

#pragma HLS interface s_axilite bundle=control port=step0
#pragma HLS interface s_axilite bundle=control port=step1
#pragma HLS interface s_axilite bundle=control port=max_iteration

#pragma HLS interface s_axilite bundle=control port=norm_chunk_real
#pragma HLS interface s_axilite bundle=control port=norm_bits_real
#pragma HLS interface s_axilite bundle=control port=norm_chunk_image
#pragma HLS interface s_axilite bundle=control port=norm_bits_image

#pragma HLS interface s_axilite bundle=control port=repeat
#pragma HLS interface s_axilite bundle=control port=block_ddr

#pragma HLS interface s_axilite bundle=control port=return
// for(uint32_t rep = 0; rep < repeat; ++rep){
    hls::stream<ap_uint<DDR_BITS>> orig_real_s;
    hls::stream<ap_uint<DDR_BITS>> orig_image_s0;
    hls::stream<ap_uint<DDR_BITS>> orig_image_s1;
    hls::stream<bool> orig_real_sign_s;
    hls::stream<bool> orig_image_sign_s0;
    hls::stream<bool> orig_image_sign_s1;
    hls::stream<uint32_t> orig_addr_s;
#pragma HLS STREAM variable = orig_real_s depth = 128
#pragma HLS STREAM variable = orig_image_s0 depth = 128
#pragma HLS STREAM variable = orig_real_sign_s depth = 32
#pragma HLS STREAM variable = orig_image_sign_s0 depth = 32
#pragma HLS STREAM variable = orig_addr_s depth = 32

    hls::stream<ap_uint<DDR_BITS>> mul_in0_s;
    hls::stream<ap_uint<DDR_BITS>> mul_in1_s;
    hls::stream<ap_uint<DDR_BITS>> mul_res_s;
#pragma HLS STREAM variable = mul_in0_s depth = 128
#pragma HLS STREAM variable = mul_in1_s depth = 128
#pragma HLS STREAM variable = mul_res_s depth = 128

    hls::stream<uint32_t> aim_indicator0_s;
    hls::stream<uint32_t> aim_indicator1_s;
    hls::stream<uint32_t> aim_indicator2_s;
#pragma HLS STREAM variable = aim_indicator0_s depth = 128
#pragma HLS STREAM variable = aim_indicator1_s depth = 128
#pragma HLS STREAM variable = aim_indicator2_s depth = 128
    hls::stream<ap_uint<VALID_BITS * VEC_SIZE>> p_res_s0;
#pragma HLS STREAM variable = p_res_s0 depth = 128

    hls::stream<ap_uint<DDR_BITS>> backup_real_s;
    hls::stream<ap_uint<DDR_BITS>> backup_image_s;
#pragma HLS STREAM variable = backup_real_s depth = 128
#pragma HLS STREAM variable = backup_image_s depth = 128
    hls::stream<bool> backup_real_sign_s;
    hls::stream<bool> backup_image_sign_s;
#pragma HLS STREAM variable = backup_real_sign_s depth = 64
#pragma HLS STREAM variable = backup_image_sign_s depth = 64

    hls::stream<ap_uint<DDR_BITS>> back2send_real_s;
    hls::stream<ap_uint<DDR_BITS>> back2send_image_s;
#pragma HLS STREAM variable = back2send_real_s depth = 128
#pragma HLS STREAM variable = back2send_image_s depth = 128

    hls::stream<bool> a_x_b_sign_s;
    hls::stream<ap_uint<DDR_BITS>> a_x_b_s0;
    hls::stream<ap_uint<DDR_BITS>> a_x_b_s1;
#pragma HLS STREAM variable = a_x_b_s0 depth = 128
#pragma HLS STREAM variable = a_x_b_s1 depth = 128

    hls::stream<ap_uint<DDR_BITS>> a2_s;
    hls::stream<ap_uint<DDR_BITS>> b2_s;
    hls::stream<ap_uint<DDR_BITS>> a2_add_b2_s;
#pragma HLS STREAM variable = a2_s depth = 128
#pragma HLS STREAM variable = b2_s depth = 128
#pragma HLS STREAM variable = a2_add_b2_s depth = 128

    hls::stream<uint32_t> send_out_itr_s;
    hls::stream<uint32_t> send_out_addr_s;
#pragma HLS STREAM variable = send_out_itr_s depth = 64
#pragma HLS STREAM variable = send_out_addr_s depth = 64

    hls::stream<uint32_t> res_out_itr_s;
    hls::stream<uint32_t> res_out_addr_s;

    hls::stream<uint32_t> next_itr_s; 
    hls::stream<uint32_t> next_addr_s;


    hls::stream<ap_uint<DDR_BITS>> a2_sub_b2_s;
    hls::stream<ap_uint<DDR_BITS>> b2_sub_a2_s;
#pragma HLS STREAM variable = a2_sub_b2_s depth = 128
#pragma HLS STREAM variable = b2_sub_a2_s depth = 128
    hls::stream<bool> a2_sub_b2_sign_s;

    hls::stream<ap_uint<DDR_BITS>> a2_minus_b2_s0;
    hls::stream<ap_uint<DDR_BITS>> a2_minus_b2_s1;
#pragma HLS STREAM variable = a2_minus_b2_s0 depth = 128
#pragma HLS STREAM variable = a2_minus_b2_s1 depth = 128
    hls::stream<bool> a2_minus_b2_sign_s;

    hls::stream<ap_uint<DDR_BITS>> sender_a0_s;
    hls::stream<ap_uint<DDR_BITS>> sender_b0_s;
#pragma HLS STREAM variable = sender_a0_s depth = 128
#pragma HLS STREAM variable = sender_b0_s depth = 128
    hls::stream<bool> sender_a0_sign_s;
    hls::stream<bool> sender_b0_sign_s;
#pragma HLS STREAM variable = sender_a0_sign_s depth = 64
#pragma HLS STREAM variable = sender_b0_sign_s depth = 64

    hls::stream<ap_uint<DDR_BITS>> discard_a0_s;
    hls::stream<ap_uint<DDR_BITS>> discard_b0_s;
#pragma HLS STREAM variable = discard_a0_s depth = 128
#pragma HLS STREAM variable = discard_b0_s depth = 128
    hls::stream<bool> discard_a0_sign_s;
    hls::stream<bool> discard_b0_sign_s;
#pragma HLS STREAM variable = discard_a0_sign_s depth = 64
#pragma HLS STREAM variable = discard_b0_sign_s depth = 64

    hls::stream<ap_uint<DDR_BITS>> next_a0_s;
    hls::stream<ap_uint<DDR_BITS>> next_b0_s;
#pragma HLS STREAM variable = next_a0_s depth = 128
#pragma HLS STREAM variable = next_b0_s depth = 128
    hls::stream<bool> next_a0_sign_s;
    hls::stream<bool> next_b0_sign_s;
#pragma HLS STREAM variable = next_a0_sign_s depth = 16
#pragma HLS STREAM variable = next_b0_sign_s depth = 16
    hls::stream<uint32_t> recv_indicator_s;
#pragma HLS STREAM variable = recv_indicator_s depth = 16

    hls::stream<ap_uint<DDR_BITS>> a_x_b_add_b_s0;
    hls::stream<ap_uint<DDR_BITS>> a_x_b_add_b_s1;
    hls::stream<ap_uint<DDR_BITS>> a_x_b_add_b_s2;
#pragma HLS STREAM variable = a_x_b_add_b_s0 depth = 128
#pragma HLS STREAM variable = a_x_b_add_b_s1 depth = 128
#pragma HLS STREAM variable = a_x_b_add_b_s2 depth = 128
    hls::stream<uint32_t> sel_a_x_b_add_b_s;
    hls::stream<ap_uint<DDR_BITS>> next_image_s;
#pragma HLS STREAM variable = next_image_s depth = 128
    hls::stream<bool> next_image_sign_s;
#pragma HLS STREAM variable = next_image_sign_s depth = 64
    hls::stream<ap_uint<DDR_BITS>> a2_minus_b2_add_a_s0;
    hls::stream<ap_uint<DDR_BITS>> a2_minus_b2_add_a_s1;
    hls::stream<ap_uint<DDR_BITS>> a2_minus_b2_add_a_s2;
#pragma HLS STREAM variable = a2_minus_b2_add_a_s0 depth = 128
#pragma HLS STREAM variable = a2_minus_b2_add_a_s1 depth = 128
#pragma HLS STREAM variable = a2_minus_b2_add_a_s2 depth = 128
    hls::stream<uint32_t> sel_a2_minus_b2_add_a_s;
#pragma HLS STREAM variable = sel_a2_minus_b2_add_a_s depth = 64
    hls::stream<ap_uint<DDR_BITS>> next_real_s;
#pragma HLS STREAM variable = next_real_s depth = 64
    hls::stream<bool> next_real_sign_s;
#pragma HLS STREAM variable = next_real_sign_s depth = 64

    hls::stream<bool> sub_discard_notify_s;
    hls::stream<bool> const_discard_notify_s;
    hls::stream<bool> feed_a_x_b_discard_notify_s;

    hls::stream<bool> finish_src_s;
    hls::stream<bool> finish_0_s;
    hls::stream<bool> finish_1_s;
    hls::stream<bool> finish_2_s;
    hls::stream<bool> finish_3_s;
    hls::stream<bool> finish_4_s;
    hls::stream<bool> finish_5_s;
    hls::stream<bool> finish_6_s;
    hls::stream<bool> finish_7_s;
    hls::stream<bool> finish_8_s;

    uint32_t total_pixels = width * height;
#pragma HLS dataflow

            mandelbrot_gen_real(width, height, \
                x0, sign_x0, step0, \
                orig_real_s, orig_real_sign_s, \
                orig_addr_s);

            mandelbrot_gen0_image(height, \
                y0, sign_y0, step1, \
                orig_image_s0, orig_image_sign_s0);
            
            mandelbrot_gen1_image(width, height, orig_image_s0, orig_image_sign_s0, orig_image_s1, orig_image_sign_s1);

            select_sender(total_pixels, \
                orig_real_s, orig_real_sign_s, orig_image_s1, orig_image_sign_s1, orig_addr_s,/* x0 y0 */ \
                back2send_real_s, back2send_image_s, /* calculate a * b */ \
                next_real_s, next_real_sign_s, next_image_s, next_image_sign_s, /*  */ \
                next_a0_s, next_a0_sign_s, next_b0_s, next_b0_sign_s, next_addr_s, next_itr_s, /* back up a0 & b0 every iteration */ \
                mul_in0_s, mul_in1_s, /* to AIM multiplier */ \
                backup_real_s, backup_real_sign_s, backup_image_s, backup_image_sign_s, /* backup operands */ \
                sender_a0_s, sender_a0_sign_s, sender_b0_s, sender_b0_sign_s, send_out_addr_s, send_out_itr_s, /* back up a0 & b0 every iteration */ \
                recv_indicator_s, finish_0_s, aim_indicator0_s);
            
            recv_split(recv_indicator_s, /*  */ \
                finish_1_s, total_pixels,
                mul_res_s, 
                a2_s, b2_s, a_x_b_s0);

            square_add_sub(finish_2_s, total_pixels, \
                a2_s, b2_s, \
                a2_add_b2_s, a2_sub_b2_s, b2_sub_a2_s, a2_sub_b2_sign_s);

            compare_a2_add_b2(threshold, max_iteration, total_pixels, a2_add_b2_s, finish_src_s, \
                send_out_itr_s, send_out_addr_s, \
                next_itr_s, next_addr_s, \
                res_out_itr_s, res_out_addr_s, \
                sub_discard_notify_s, const_discard_notify_s, feed_a_x_b_discard_notify_s);

            finish_notifiers(total_pixels, finish_src_s, \
                    finish_0_s, finish_1_s, finish_2_s, \
                    finish_3_s, finish_4_s, finish_5_s, \
                    finish_6_s, finish_7_s, finish_8_s);

            sel_sub(sub_discard_notify_s, total_pixels, \
            a2_sub_b2_s, b2_sub_a2_s, a2_sub_b2_sign_s, \
            a2_minus_b2_s0, a2_minus_b2_sign_s);

            feed_back_a_x_b(total_pixels, feed_a_x_b_discard_notify_s,\
                backup_real_s, backup_real_sign_s, backup_image_s, backup_image_sign_s, \
                back2send_real_s, back2send_image_s, a_x_b_sign_s);

            discard_a0_b0(total_pixels, const_discard_notify_s, \
                    sender_a0_s, sender_a0_sign_s, sender_b0_s, sender_b0_sign_s, \
                    discard_a0_s, discard_a0_sign_s, discard_b0_s, discard_b0_sign_s, \
                    next_a0_s, next_a0_sign_s, next_b0_s, next_b0_sign_s);
// a * b
            shift_module(finish_3_s, total_pixels, \
                a_x_b_s0, a_x_b_s1, norm_chunk_image, norm_bits_image);

            sign_add(finish_4_s, total_pixels, \
                a_x_b_s1, a_x_b_sign_s, \
                discard_b0_s, discard_b0_sign_s,
                a_x_b_add_b_s0, a_x_b_add_b_s1, a_x_b_add_b_s2, sel_a_x_b_add_b_s, next_image_sign_s);

            sign_add_sel(finish_5_s, total_pixels, \
                a_x_b_add_b_s0, a_x_b_add_b_s1, a_x_b_add_b_s2, sel_a_x_b_add_b_s,
                next_image_s);

// a2 - b2
            shift_module(finish_6_s, total_pixels, \
                a2_minus_b2_s0, a2_minus_b2_s1, norm_chunk_real, norm_bits_real);

            sign_add(finish_7_s, total_pixels, \
                a2_minus_b2_s1, a2_minus_b2_sign_s, \
                discard_a0_s, discard_a0_sign_s,
                a2_minus_b2_add_a_s0, a2_minus_b2_add_a_s1, a2_minus_b2_add_a_s2, sel_a2_minus_b2_add_a_s, next_real_sign_s);

            sign_add_sel(finish_8_s, total_pixels, \
                a2_minus_b2_add_a_s0, a2_minus_b2_add_a_s1, a2_minus_b2_add_a_s2, sel_a2_minus_b2_add_a_s,
                next_real_s);
            write_image(mem_out, res_out_addr_s, res_out_itr_s, total_pixels);

            reorganize_send(aim_indicator0_s, aim_indicator1_s, aim_indicator2_s, mul_in0_s, mul_in1_s, 1, 1, plio_a0, plio_b0, block_ddr);
            receive_acc80(aim_indicator1_s, plio_out0, p_res_s0, 1, 1);
            crop_out(aim_indicator2_s, p_res_s0, mul_res_s, 1, 1, block_ddr);

            

// } 
    }

} // extern "C"
