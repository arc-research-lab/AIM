#include <ap_int.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "config.h"

void merge_phase2(hls::stream<ap_uint<DDR_BITS>> &final_merge_out, hls::stream<ap_uint<DDR_BITS>> &serialize_out, const uint32_t batch, const uint32_t repeat){
    // generate
    uint16_t shift[64] = { 0, 496, 480, 464, 448, 432, 416, 400, 384, 368, 352, 336, 320, 304, 288, 272, 256, 240, 224, 208, 192, 176, 160, 144, 128, 112, 96, 80, 64, 48, 32, 16, 0, 496, 480, 464, 448, 432, 416, 400, 384, 368, 352, 336, 320, 304, 288, 272, 256, 240, 224, 208, 192, 176, 160, 144, 128, 112, 96, 80, 64, 48, 32, 16 };
    for(uint32_t rep = 0; rep < repeat; ++rep){
#pragma HLS loop_tripcount min=1 max=1 avg=1
        for(uint32_t j = 0; j < batch; ++j){
#pragma HLS loop_tripcount min=1 max=1 avg=1
            ap_uint<DDR_BITS * 2> fifo = 0;
            ap_uint<DDR_BITS> carry = 0;
            uint16_t idx = 0;
            int remains = 0;
            uint32_t output_idx = 0;
            ap_uint<DDR_BITS * 2> last = 0;
            for(uint32_t k = 0; k < OUTPUT_AIE; ++k){
                for(uint32_t t = 0; t < AIE_OUTPUT_CHUNK; ++t){
#pragma HLS pipeline II = 1
                    if(t < AIE_OUTPUT_CHUNK - 2){
                        ap_uint<DDR_BITS * 2> tmp = 0;
                        tmp(DDR_BITS - 1, 0) = serialize_out.read();
                        tmp += carry;
                        // fifo(remains + DDR_BITS - 1, remains) = tmp(DDR_BITS - 1, 0);
                        fifo += tmp << shift[idx];
                        carry = tmp(2 * DDR_BITS - 1, DDR_BITS);
                        if(output_idx < TASK_OUTPUT_CHUNK){
                            final_merge_out << fifo(DDR_BITS - 1, 0);
                            output_idx++;
                        }
                        fifo = (fifo >> DDR_BITS);
                    }else if(t == AIE_OUTPUT_CHUNK - 2){
                        last = 0;
                        last(DDR_BITS - 1, 0) = serialize_out.read();
                        last += carry;
                    }else if(t == AIE_OUTPUT_CHUNK - 1){
                        carry = serialize_out.read(); 
                        carry += last(LAST_VALID + DDR_BITS - 1, LAST_VALID);
                        // fifo(remains + LAST_VALID - 1, remains) = last(LAST_VALID - 1, 0);
                        fifo += last << shift[idx];
                        idx = (idx == 63 ? 0 : idx + 1);
                        remains += LAST_VALID;
                        if(remains >= DDR_BITS && output_idx < TASK_OUTPUT_CHUNK){
                            final_merge_out << fifo(DDR_BITS - 1, 0);
                            output_idx++;
                            remains -= DDR_BITS;
                            fifo = fifo >> DDR_BITS;
                        }
                    }
                }
            }
            if(output_idx < TASK_OUTPUT_CHUNK){
#pragma HLS loop_tripcount min=1 max=1 avg=1
                final_merge_out << fifo(DDR_BITS - 1, 0);
            }
        }
    }
}

void reorganize_a(hls::stream<ap_uint<PLIO_BITS> > &reorg_s, hls::stream<ap_uint<DDR_BITS> > &raw_data_s, const int batch, const uint32_t repeat){
    uint16_t shift[31] = {0, 16, 32, 48, 64, 80, 96, 112, 4, 20, 36, 52, 68, 84, 100, 116, 8, 24, 40, 56, 72, 88, 104, 120, 12, 28, 44, 60, 76, 92, 108};
    for(uint32_t rep = 0; rep < repeat; ++rep){
#pragma HLS loop_tripcount min=1 max=1 avg=1
        for(uint32_t bch = 0; bch < batch; ++bch){
#pragma HLS loop_tripcount min=1 max=1 avg=1
            uint16_t idx = 0;
            ap_uint<DDR_BITS + PLIO_BITS> fifo = 0;
            int remains = 0;
            uint32_t sent = 0;
            for(uint32_t i = 0; i < TASK_INPUT_CHUNK; ++i){
                for(uint32_t j = 0; j < 5; ++j){
#pragma HLS pipeline II = 1
                    if(j == 0){
                        ap_uint<DDR_BITS + PLIO_BITS> fifo_tmp = 0;
                        fifo_tmp(DDR_BITS - 1, 0) = raw_data_s.read();
                        fifo += fifo_tmp << shift[idx];
                        idx = (idx == 30 ? 0 : idx + 1);
                        // fifo(remains + DDR_BITS - 1, remains) = raw_data_s.read(); // buf[batch][i];
                        remains += DDR_BITS;
                    }
                    if(remains >= (PLIO_BITS / VEC_ELEM_BITS) * VALID_BITS && sent < SENT_VALID){
                        ap_uint<PLIO_BITS> tmp = 0;
                        for(uint32_t k = 0; k < (PLIO_BITS / VEC_ELEM_BITS); ++k){
                            tmp(k * VEC_ELEM_BITS + VALID_BITS - 1, k * VEC_ELEM_BITS) = fifo(k * VALID_BITS + VALID_BITS - 1, k * VALID_BITS);
                        }
                        sent++;
                        fifo = fifo >> ((PLIO_BITS / VEC_ELEM_BITS) * VALID_BITS);
                        remains -= (PLIO_BITS / VEC_ELEM_BITS) * VALID_BITS;
                        reorg_s << tmp;
                    }
                }
            }

            for(uint32_t i = 0; i < TAILS_A / (PLIO_BITS / VEC_ELEM_BITS); ++i){
        #pragma HLS pipeline II = 1
                ap_uint<PLIO_BITS> tmp = 0;
                for(uint32_t k = 0; k < (PLIO_BITS / VEC_ELEM_BITS); ++k){
                    tmp(k * VEC_ELEM_BITS + VALID_BITS - 1, k * VEC_ELEM_BITS) = fifo(k * VALID_BITS + VALID_BITS - 1, k * VALID_BITS);
                }
                fifo = fifo >> ((PLIO_BITS / VEC_ELEM_BITS) * VALID_BITS);
                reorg_s << tmp;
            }
        }
    }
}

void reorganize_b(hls::stream<ap_uint<PLIO_BITS> > &reorg_s, hls::stream<ap_uint<DDR_BITS> > &raw_data_s, const int batch, const uint32_t repeat){
    uint16_t shift[31] = {0, 16, 32, 48, 64, 80, 96, 112, 4, 20, 36, 52, 68, 84, 100, 116, 8, 24, 40, 56, 72, 88, 104, 120, 12, 28, 44, 60, 76, 92, 108};
    for(uint32_t rep = 0; rep < repeat; ++rep){
#pragma HLS loop_tripcount min=1 max=1 avg=1
        for(uint32_t bch = 0; bch < batch; ++bch){
#pragma HLS loop_tripcount min=1 max=1 avg=1
            uint16_t idx = 0;
            for(uint32_t i = 0; i < AIE_H / (PLIO_BITS / VEC_ELEM_BITS); ++i){
#pragma HLS pipeline II = 1
                ap_uint<PLIO_BITS> tmp = 0;
                reorg_s << tmp;
            }
            ap_uint<DDR_BITS + PLIO_BITS> fifo = 0;
            int remains = 0;
            uint32_t sent = 0;
            for(uint32_t i = 0; i < TASK_INPUT_CHUNK; ++i){
                for(uint32_t j = 0; j < 5; ++j){
#pragma HLS pipeline II = 1
                    if(j == 0){
                        ap_uint<DDR_BITS + PLIO_BITS> fifo_tmp = 0;
                        fifo_tmp(DDR_BITS - 1, 0) = raw_data_s.read();
                        fifo += fifo_tmp << shift[idx];
                        idx = (idx == 30 ? 0 : idx + 1);
                        // fifo(remains + DDR_BITS - 1, remains) = raw_data_s.read(); // buf[batch][i];
                        remains += DDR_BITS;
                    }
                    if(remains >= (PLIO_BITS / VEC_ELEM_BITS) * VALID_BITS && sent < SENT_VALID){
                        ap_uint<PLIO_BITS> tmp = 0;
                        for(uint32_t k = 0; k < (PLIO_BITS / VEC_ELEM_BITS); ++k){
                            tmp(k * VEC_ELEM_BITS + VALID_BITS - 1, k * VEC_ELEM_BITS) = fifo(k * VALID_BITS + VALID_BITS - 1, k * VALID_BITS);
                        }
                        sent++;
                        fifo = fifo >> ((PLIO_BITS / VEC_ELEM_BITS) * VALID_BITS);
                        remains -= (PLIO_BITS / VEC_ELEM_BITS) * VALID_BITS;
                        reorg_s << tmp;
                    }
                }
            }

            for(uint32_t i = 0; i < TAILS_B / (PLIO_BITS / VEC_ELEM_BITS); ++i){
#pragma HLS pipeline II = 1
                ap_uint<PLIO_BITS> tmp = 0;
                for(uint32_t k = 0; k < (PLIO_BITS / VEC_ELEM_BITS); ++k){
                    tmp(k * VEC_ELEM_BITS + VALID_BITS - 1, k * VEC_ELEM_BITS) = fifo(k * VALID_BITS + VALID_BITS - 1, k * VALID_BITS);
                }
                fifo = fifo >> ((PLIO_BITS / VEC_ELEM_BITS) * VALID_BITS);
                reorg_s << tmp;
            }
        }
    }
}



void receive_plio(hls::stream<ap_uint<DDR_BITS>> &merge0_out, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &plio, const uint32_t batch, const uint32_t repeat, const bool block_aie){
    uint16_t shift[64] = {0, 248, 496, 232, 480, 216, 464, 200, 448, 184, 432, 168, 416, 152, 400, 136, 384, 120, 368, 104, 352, 88, 336, 72, 320, 56, 304, 40, 288, 24, 272, 8, 256, 504, 240, 488, 224, 472, 208, 456, 192, 440, 176, 424, 160, 408, 144, 392, 128, 376, 112, 360, 96, 344, 80, 328, 64, 312, 48, 296, 32, 280, 16, 264};
    
    for(uint32_t rep = 0; rep < repeat; ++rep){
#pragma HLS loop_tripcount min=1 max=1 avg=1
        for(uint32_t bch = 0; bch < batch; ++bch){
#pragma HLS loop_tripcount min=1 max=1 avg=1
            uint16_t idx = 0;
            ap_uint<2 * VALID_BITS> carry = 0;
            ap_uint<DDR_BITS + VEC_SIZE * VALID_BITS> fifo = 0;
            int remains = 0;
            uint32_t output_idx = 0;
            for(uint32_t j = 0; j < AIE_W / VEC_SIZE; ++j){
                ap_uint<VALID_BITS * VEC_SIZE + 2 * VALID_BITS> res = 0;
                for(uint32_t v = 0; v < 3; ++v){
#pragma HLS pipeline II = 2
                    ap_uint<VALID_BITS * VEC_SIZE + 2 * VALID_BITS> p_vec = 0;
                    if(v == 0){
                        res += carry;
                    }
                    qdma_axis<PLIO_BITS, 0, 0, 0> tmp0;
                    qdma_axis<PLIO_BITS, 0, 0, 0> tmp1;
                    tmp0 = plio.read();
                    tmp1 = plio.read();
                    const uint32_t idx0 = 0 * (PLIO_BITS / VEC_ELEM_BITS) + 0;
                    p_vec((idx0 + 1) * VALID_BITS - 1, idx0 * VALID_BITS) = tmp0.data(0 * VEC_ELEM_BITS + VALID_BITS - 1, 0 * VEC_ELEM_BITS);

                    const uint32_t idx1 = 0 * (PLIO_BITS / VEC_ELEM_BITS) + 1;
                    p_vec((idx1 + 1) * VALID_BITS - 1, idx1 * VALID_BITS) = tmp0.data(1 * VEC_ELEM_BITS + VALID_BITS - 1, 1 * VEC_ELEM_BITS);

                    const uint32_t idx2 = 0 * (PLIO_BITS / VEC_ELEM_BITS) + 2;
                    p_vec((idx2 + 1) * VALID_BITS - 1, idx2 * VALID_BITS) = tmp0.data(2 * VEC_ELEM_BITS + VALID_BITS - 1, 2 * VEC_ELEM_BITS);

                    const uint32_t idx3 = 0 * (PLIO_BITS / VEC_ELEM_BITS) + 3;
                    p_vec((idx3 + 1) * VALID_BITS - 1, idx3 * VALID_BITS) = tmp0.data(3 * VEC_ELEM_BITS + VALID_BITS - 1, 3 * VEC_ELEM_BITS);

                    const uint32_t idx4 = 1 * (PLIO_BITS / VEC_ELEM_BITS) + 0;
                    p_vec((idx4 + 1) * VALID_BITS - 1, idx4 * VALID_BITS) = tmp1.data(0 * VEC_ELEM_BITS + VALID_BITS - 1, 0 * VEC_ELEM_BITS);

                    const uint32_t idx5 = 1 * (PLIO_BITS / VEC_ELEM_BITS) + 1;
                    p_vec((idx5 + 1) * VALID_BITS - 1, idx5 * VALID_BITS) = tmp1.data(1 * VEC_ELEM_BITS + VALID_BITS - 1, 1 * VEC_ELEM_BITS);

                    const uint32_t idx6 = 1 * (PLIO_BITS / VEC_ELEM_BITS) + 2;
                    p_vec((idx6 + 1) * VALID_BITS - 1, idx6 * VALID_BITS) = tmp1.data(2 * VEC_ELEM_BITS + VALID_BITS - 1, 2 * VEC_ELEM_BITS);

                    const uint32_t idx7 = 1 * (PLIO_BITS / VEC_ELEM_BITS) + 3;
                    p_vec((idx7 + 1) * VALID_BITS - 1, idx7 * VALID_BITS) = tmp1.data(3 * VEC_ELEM_BITS + VALID_BITS - 1, 3 * VEC_ELEM_BITS);

                    p_vec = p_vec << (v * VALID_BITS);

                    res += p_vec;
                    if(v == 2){
                        ap_uint<DDR_BITS + VEC_SIZE * VALID_BITS> fifo_tmp = 0;
                        fifo_tmp = res(VEC_SIZE * VALID_BITS - 1, 0);
                        fifo += fifo_tmp << shift[idx];
                        idx++;
                        remains += VEC_SIZE * VALID_BITS;
                        carry = res(VEC_SIZE * VALID_BITS + 2 * VALID_BITS - 1, VEC_SIZE * VALID_BITS);
                        if(remains >= DDR_BITS){
                            merge0_out << fifo(DDR_BITS - 1, 0);
                            fifo = (fifo >> DDR_BITS);
                            remains -= DDR_BITS;
                        }
                    }
                }
            }
            merge0_out << fifo(DDR_BITS - 1, 0);
            ap_uint<DDR_BITS> tmp = 0;
            tmp(2 * VALID_BITS - 1, 0) = carry;
            merge0_out << tmp;
        }
    }
}

void serialize_merge_stream(hls::stream<ap_uint<DDR_BITS>> &serialize_out, hls::stream<ap_uint<DDR_BITS>> &merge0_out, hls::stream<ap_uint<DDR_BITS>> &merge1_out, uint32_t size, uint32_t tails, uint32_t batch, uint32_t repeat){
    for(uint32_t rep = 0; rep < repeat; ++rep){
        for(uint32_t i = 0; i < batch; ++i){
            for(uint32_t j = 0; j < size; ++j){
#pragma HLS pipeline II = 1
                serialize_out << merge0_out.read();
            }
            for(uint32_t j = 0; j < tails; ++j){
#pragma HLS pipeline II = 1
                serialize_out << merge1_out.read();
            }
        }
    }
}


void read_ddr(ap_uint<DDR_BITS> *mem_in, hls::stream<ap_uint<DDR_BITS>> &ddr_in_s, const uint32_t batch, const uint32_t repeat, const bool block_ddr){
    ap_uint<DDR_BITS> buf_tmp[TASK_INPUT_CHUNK];
    if(block_ddr){
        for(uint32_t i = 0; i < TASK_INPUT_CHUNK; ++i){
            buf_tmp[i] = mem_in[i];
        }
    }
    for(uint32_t rep = 0; rep < repeat; ++rep){
        uint32_t idx = 0;
        for(uint32_t i = 0; i < batch; ++i){
            for(uint32_t j = 0; j < TASK_INPUT_CHUNK; ++j){
#pragma HLS pipeline II = 1
                ap_uint<DDR_BITS> tmp;
                if(!block_ddr){
                    tmp = mem_in[idx++];
                }else{
                    tmp = buf_tmp[j];
                }
                ddr_in_s << tmp;
            }
        }
    }
}

void write_ddr(ap_uint<DDR_BITS> *mem_out, hls::stream<ap_uint<DDR_BITS>> &out_s, const uint32_t batch, const uint32_t repeat, const bool block_ddr){
    for(uint32_t rep = 0; rep < repeat; ++rep){
        uint32_t idx = 0;
        for(uint32_t i = 0; i < batch; ++i){
            for(uint32_t j = 0; j < TASK_OUTPUT_CHUNK; ++j){
#pragma HLS pipeline II = 1
                ap_uint<DDR_BITS> tmp = out_s.read();
                if(!block_ddr){
                    mem_out[idx++] = tmp;
                }
            }
        }
    }
}

void split_input_a(hls::stream<ap_uint<PLIO_BITS>> &in_s, hls::stream<ap_uint<PLIO_BITS>> &in0_s, hls::stream<ap_uint<PLIO_BITS>> &in1_s, uint32_t size, uint32_t tails, const uint32_t batch, const uint32_t repeat){
    for(uint32_t rep = 0; rep < repeat; ++rep){
        for(uint32_t i = 0; i < batch; ++i){
            for(uint32_t j = 0; j < size; ++j){
#pragma HLS pipeline II = 1
                in0_s << in_s.read();
            }
            for(uint32_t j = 0; j < tails; ++j){
#pragma HLS pipeline II = 1
                in1_s << in_s.read();
            }
        }
    }
}

void send_plio(hls::stream<ap_uint<PLIO_BITS>> &in0_s, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &plio, uint32_t size, const uint32_t batch, const uint32_t repeat, bool block_aie){
    for(uint32_t rep = 0; rep < repeat; ++rep){
        for(uint32_t i = 0; i < batch; ++i){
            for(uint32_t j = 0; j < size; ++j){
#pragma HLS pipeline II = 1
                qdma_axis<PLIO_BITS, 0, 0, 0> tmp;
                tmp.data = in0_s.read();
                tmp.keep_all();
     
                plio.write(tmp);
                
            }
        }
    }
}


void split_input_b(hls::stream<ap_uint<PLIO_BITS>> &in_s, hls::stream<ap_uint<PLIO_BITS>> &in0_s, hls::stream<ap_uint<PLIO_BITS>> &in1_s, uint32_t size0, uint32_t size1, uint32_t size2, const uint32_t batch, const uint32_t repeat){
    for(uint32_t rep = 0; rep < repeat; ++rep){
        for(uint32_t i = 0; i < batch; ++i){
            for(uint32_t j = 0; j < size0; ++j){
#pragma HLS pipeline II = 1
                in0_s << in_s.read();
            }
            for(uint32_t j = 0; j < size1; ++j){
#pragma HLS pipeline II = 1
                ap_uint<PLIO_BITS> tmp = in_s.read();
                in0_s << tmp;
                in1_s << tmp;
            }
            for(uint32_t j = 0; j < size2; ++j){
#pragma HLS pipeline II = 1
                in1_s << in_s.read();
            }
        }
    }
}


extern "C" {
void pl_kernel(ap_uint<DDR_BITS> *mem_in_a, ap_uint<DDR_BITS> *mem_in_b, ap_uint<DDR_BITS> *mem_out, uint32_t batch, uint32_t repeat, const bool block_ddr, bool block_aie \
            , hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_a0, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_a1, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_a2, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_a3, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_a4, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_a5, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_a6, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_a7, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_a8, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_a9, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_a10, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_a11, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_a12, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_a13, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_a14, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_a15, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_a16 \
            , hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_b0, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_b1, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_b2, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_b3, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_b4, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_b5, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_b6, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_b7, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_b8, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_b9, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_b10, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_b11, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_b12, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_b13, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_b14, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_b15, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_b16, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_b17 \
            , hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c0, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c1, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c2, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c3, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c4, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c5, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c6, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c7, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c8, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c9, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c10, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c11, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c12, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c13, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c14, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c15, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c16, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c17, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c18, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c19, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c20, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c21, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c22, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c23, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c24, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c25, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c26, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c27, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c28, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c29, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c30, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c31, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c32, hls::stream<qdma_axis<PLIO_BITS, 0, 0, 0> > &p_0_c33){
#pragma HLS interface m_axi offset=slave bundle=gmem0 port=mem_in_a max_read_burst_length=16 num_read_outstanding=16
#pragma HLS interface m_axi offset=slave bundle=gmem1 port=mem_in_b max_read_burst_length=16 num_read_outstanding=16
#pragma HLS interface m_axi offset=slave bundle=gmem2 port=mem_out max_read_burst_length=16 num_read_outstanding=16

#pragma HLS interface s_axilite bundle=control port=mem_in_a
#pragma HLS interface s_axilite bundle=control port=mem_in_b
#pragma HLS interface s_axilite bundle=control port=mem_out

#pragma HLS interface s_axilite bundle=control port=batch
#pragma HLS interface s_axilite bundle=control port=repeat
#pragma HLS interface s_axilite bundle=control port=block_ddr
#pragma HLS interface s_axilite bundle=control port=block_aie

#pragma HLS interface s_axilite bundle=control port=return

for(uint32_t rep = 0; rep < repeat; ++rep){
    hls::stream<ap_uint<DDR_BITS>> raw_in_a_s;
    hls::stream<ap_uint<DDR_BITS>> raw_in_b_s;
    hls::stream<ap_uint<PLIO_BITS>> reorganized_a_s;
    hls::stream<ap_uint<PLIO_BITS>> reorganized_b_s;

#pragma HLS STREAM variable = raw_in_a_s depth = 32
#pragma HLS STREAM variable = raw_in_b_s depth = 32
#pragma HLS STREAM variable = reorganized_a_s depth = 32
#pragma HLS STREAM variable = reorganized_b_s depth = 32


    hls::stream<ap_uint<PLIO_BITS>> pl_a0_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_a1_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_a2_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_a3_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_a4_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_a5_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_a6_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_a7_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_a8_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_a9_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_a10_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_a11_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_a12_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_a13_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_a14_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_a15_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_a16_s;

#pragma HLS STREAM variable = pl_a0_s depth = 4
#pragma HLS STREAM variable = pl_a1_s depth = 4
#pragma HLS STREAM variable = pl_a2_s depth = 4
#pragma HLS STREAM variable = pl_a3_s depth = 4
#pragma HLS STREAM variable = pl_a4_s depth = 4
#pragma HLS STREAM variable = pl_a5_s depth = 4
#pragma HLS STREAM variable = pl_a6_s depth = 4
#pragma HLS STREAM variable = pl_a7_s depth = 4
#pragma HLS STREAM variable = pl_a8_s depth = 4
#pragma HLS STREAM variable = pl_a9_s depth = 4
#pragma HLS STREAM variable = pl_a10_s depth = 4
#pragma HLS STREAM variable = pl_a11_s depth = 4
#pragma HLS STREAM variable = pl_a12_s depth = 4
#pragma HLS STREAM variable = pl_a13_s depth = 4
#pragma HLS STREAM variable = pl_a14_s depth = 4
#pragma HLS STREAM variable = pl_a15_s depth = 4
#pragma HLS STREAM variable = pl_a16_s depth = 4

    hls::stream<ap_uint<PLIO_BITS>> pl_b0_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_b1_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_b2_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_b3_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_b4_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_b5_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_b6_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_b7_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_b8_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_b9_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_b10_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_b11_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_b12_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_b13_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_b14_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_b15_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_b16_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_b17_s;

#pragma HLS STREAM variable = pl_b0_s depth = 4
#pragma HLS STREAM variable = pl_b1_s depth = 4
#pragma HLS STREAM variable = pl_b2_s depth = 4
#pragma HLS STREAM variable = pl_b3_s depth = 4
#pragma HLS STREAM variable = pl_b4_s depth = 4
#pragma HLS STREAM variable = pl_b5_s depth = 4
#pragma HLS STREAM variable = pl_b6_s depth = 4
#pragma HLS STREAM variable = pl_b7_s depth = 4
#pragma HLS STREAM variable = pl_b8_s depth = 4
#pragma HLS STREAM variable = pl_b9_s depth = 4
#pragma HLS STREAM variable = pl_b10_s depth = 4
#pragma HLS STREAM variable = pl_b11_s depth = 4
#pragma HLS STREAM variable = pl_b12_s depth = 4
#pragma HLS STREAM variable = pl_b13_s depth = 4
#pragma HLS STREAM variable = pl_b14_s depth = 4
#pragma HLS STREAM variable = pl_b15_s depth = 4
#pragma HLS STREAM variable = pl_b16_s depth = 4
#pragma HLS STREAM variable = pl_b17_s depth = 4


    hls::stream<ap_uint<PLIO_BITS>> pl_pa0_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pa1_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pa2_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pa3_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pa4_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pa5_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pa6_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pa7_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pa8_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pa9_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pa10_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pa11_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pa12_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pa13_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pa14_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pa15_s;



    hls::stream<ap_uint<PLIO_BITS>> pl_pb0_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pb1_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pb2_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pb3_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pb4_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pb5_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pb6_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pb7_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pb8_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pb9_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pb10_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pb11_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pb12_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pb13_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pb14_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pb15_s;
    hls::stream<ap_uint<PLIO_BITS>> pl_pb16_s;



    hls::stream<ap_uint<DDR_BITS>> merge0_c0;
    hls::stream<ap_uint<DDR_BITS>> merge0_c1;
    hls::stream<ap_uint<DDR_BITS>> merge0_c2;
    hls::stream<ap_uint<DDR_BITS>> merge0_c3;
    hls::stream<ap_uint<DDR_BITS>> merge0_c4;
    hls::stream<ap_uint<DDR_BITS>> merge0_c5;
    hls::stream<ap_uint<DDR_BITS>> merge0_c6;
    hls::stream<ap_uint<DDR_BITS>> merge0_c7;
    hls::stream<ap_uint<DDR_BITS>> merge0_c8;
    hls::stream<ap_uint<DDR_BITS>> merge0_c9;
    hls::stream<ap_uint<DDR_BITS>> merge0_c10;
    hls::stream<ap_uint<DDR_BITS>> merge0_c11;
    hls::stream<ap_uint<DDR_BITS>> merge0_c12;
    hls::stream<ap_uint<DDR_BITS>> merge0_c13;
    hls::stream<ap_uint<DDR_BITS>> merge0_c14;
    hls::stream<ap_uint<DDR_BITS>> merge0_c15;
    hls::stream<ap_uint<DDR_BITS>> merge0_c16;
    hls::stream<ap_uint<DDR_BITS>> merge0_c17;
    hls::stream<ap_uint<DDR_BITS>> merge0_c18;
    hls::stream<ap_uint<DDR_BITS>> merge0_c19;
    hls::stream<ap_uint<DDR_BITS>> merge0_c20;
    hls::stream<ap_uint<DDR_BITS>> merge0_c21;
    hls::stream<ap_uint<DDR_BITS>> merge0_c22;
    hls::stream<ap_uint<DDR_BITS>> merge0_c23;
    hls::stream<ap_uint<DDR_BITS>> merge0_c24;
    hls::stream<ap_uint<DDR_BITS>> merge0_c25;
    hls::stream<ap_uint<DDR_BITS>> merge0_c26;
    hls::stream<ap_uint<DDR_BITS>> merge0_c27;
    hls::stream<ap_uint<DDR_BITS>> merge0_c28;
    hls::stream<ap_uint<DDR_BITS>> merge0_c29;
    hls::stream<ap_uint<DDR_BITS>> merge0_c30;
    hls::stream<ap_uint<DDR_BITS>> merge0_c31;
    hls::stream<ap_uint<DDR_BITS>> merge0_c32;
    hls::stream<ap_uint<DDR_BITS>> merge0_c33;

#pragma HLS STREAM variable = merge0_c0 depth = 4
#pragma HLS STREAM variable = merge0_c1 depth = 4
#pragma HLS STREAM variable = merge0_c2 depth = 4
#pragma HLS STREAM variable = merge0_c3 depth = 4
#pragma HLS STREAM variable = merge0_c4 depth = 4
#pragma HLS STREAM variable = merge0_c5 depth = 4
#pragma HLS STREAM variable = merge0_c6 depth = 4
#pragma HLS STREAM variable = merge0_c7 depth = 4
#pragma HLS STREAM variable = merge0_c8 depth = 4
#pragma HLS STREAM variable = merge0_c9 depth = 4
#pragma HLS STREAM variable = merge0_c10 depth = 4
#pragma HLS STREAM variable = merge0_c11 depth = 4
#pragma HLS STREAM variable = merge0_c12 depth = 4
#pragma HLS STREAM variable = merge0_c13 depth = 4
#pragma HLS STREAM variable = merge0_c14 depth = 4
#pragma HLS STREAM variable = merge0_c15 depth = 4
#pragma HLS STREAM variable = merge0_c16 depth = 4
#pragma HLS STREAM variable = merge0_c17 depth = 4
#pragma HLS STREAM variable = merge0_c18 depth = 4
#pragma HLS STREAM variable = merge0_c19 depth = 4
#pragma HLS STREAM variable = merge0_c20 depth = 4
#pragma HLS STREAM variable = merge0_c21 depth = 4
#pragma HLS STREAM variable = merge0_c22 depth = 4
#pragma HLS STREAM variable = merge0_c23 depth = 4
#pragma HLS STREAM variable = merge0_c24 depth = 4
#pragma HLS STREAM variable = merge0_c25 depth = 4
#pragma HLS STREAM variable = merge0_c26 depth = 4
#pragma HLS STREAM variable = merge0_c27 depth = 4
#pragma HLS STREAM variable = merge0_c28 depth = 4
#pragma HLS STREAM variable = merge0_c29 depth = 4
#pragma HLS STREAM variable = merge0_c30 depth = 4
#pragma HLS STREAM variable = merge0_c31 depth = 4
#pragma HLS STREAM variable = merge0_c32 depth = 4
#pragma HLS STREAM variable = merge0_c33 depth = 4


    hls::stream<ap_uint<DDR_BITS>> merge0_pc0;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc1;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc2;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc3;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc4;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc5;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc6;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc7;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc8;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc9;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc10;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc11;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc12;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc13;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc14;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc15;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc16;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc17;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc18;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc19;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc20;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc21;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc22;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc23;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc24;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc25;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc26;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc27;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc28;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc29;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc30;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc31;
    hls::stream<ap_uint<DDR_BITS>> merge0_pc32;


    hls::stream<ap_uint<DDR_BITS>> serialized_out;
    hls::stream<ap_uint<DDR_BITS>> final_merge_out;
#pragma HLS STREAM variable = serialized_out depth = 32
#pragma HLS STREAM variable = final_merge_out depth = 32

   
#pragma HLS dataflow
    
    read_ddr(mem_in_a, raw_in_a_s, batch, 1, block_ddr);
    read_ddr(mem_in_b, raw_in_b_s, batch, 1, block_ddr);
    reorganize_a(reorganized_a_s, raw_in_a_s, batch, 1);
    reorganize_b(reorganized_b_s, raw_in_b_s, batch, 1);

    split_input_a(reorganized_a_s, pl_a0_s, pl_pa0_s, AIE_H / (PLIO_BITS / 32), AIE_H / (PLIO_BITS / 32) * 16, batch, 1);
    
    split_input_a(pl_pa0_s, pl_a1_s, pl_pa1_s, AIE_H / (PLIO_BITS / 32), AIE_H / (PLIO_BITS / 32) * 15, batch, 1);
    
    split_input_a(pl_pa1_s, pl_a2_s, pl_pa2_s, AIE_H / (PLIO_BITS / 32), AIE_H / (PLIO_BITS / 32) * 14, batch, 1);
    
    split_input_a(pl_pa2_s, pl_a3_s, pl_pa3_s, AIE_H / (PLIO_BITS / 32), AIE_H / (PLIO_BITS / 32) * 13, batch, 1);
    
    split_input_a(pl_pa3_s, pl_a4_s, pl_pa4_s, AIE_H / (PLIO_BITS / 32), AIE_H / (PLIO_BITS / 32) * 12, batch, 1);
    
    split_input_a(pl_pa4_s, pl_a5_s, pl_pa5_s, AIE_H / (PLIO_BITS / 32), AIE_H / (PLIO_BITS / 32) * 11, batch, 1);
    
    split_input_a(pl_pa5_s, pl_a6_s, pl_pa6_s, AIE_H / (PLIO_BITS / 32), AIE_H / (PLIO_BITS / 32) * 10, batch, 1);
    
    split_input_a(pl_pa6_s, pl_a7_s, pl_pa7_s, AIE_H / (PLIO_BITS / 32), AIE_H / (PLIO_BITS / 32) * 9, batch, 1);
    
    split_input_a(pl_pa7_s, pl_a8_s, pl_pa8_s, AIE_H / (PLIO_BITS / 32), AIE_H / (PLIO_BITS / 32) * 8, batch, 1);
    
    split_input_a(pl_pa8_s, pl_a9_s, pl_pa9_s, AIE_H / (PLIO_BITS / 32), AIE_H / (PLIO_BITS / 32) * 7, batch, 1);
    
    split_input_a(pl_pa9_s, pl_a10_s, pl_pa10_s, AIE_H / (PLIO_BITS / 32), AIE_H / (PLIO_BITS / 32) * 6, batch, 1);
    
    split_input_a(pl_pa10_s, pl_a11_s, pl_pa11_s, AIE_H / (PLIO_BITS / 32), AIE_H / (PLIO_BITS / 32) * 5, batch, 1);
    
    split_input_a(pl_pa11_s, pl_a12_s, pl_pa12_s, AIE_H / (PLIO_BITS / 32), AIE_H / (PLIO_BITS / 32) * 4, batch, 1);
    
    split_input_a(pl_pa12_s, pl_a13_s, pl_pa13_s, AIE_H / (PLIO_BITS / 32), AIE_H / (PLIO_BITS / 32) * 3, batch, 1);
    
    split_input_a(pl_pa13_s, pl_a14_s, pl_pa14_s, AIE_H / (PLIO_BITS / 32), AIE_H / (PLIO_BITS / 32) * 2, batch, 1);
    
    split_input_a(pl_pa14_s, pl_a15_s, pl_a16_s, AIE_H / (PLIO_BITS / 32), AIE_H / (PLIO_BITS / 32), batch, 1);



    send_plio(pl_a0_s, p_0_a0, AIE_H / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_a1_s, p_0_a1, AIE_H / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_a2_s, p_0_a2, AIE_H / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_a3_s, p_0_a3, AIE_H / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_a4_s, p_0_a4, AIE_H / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_a5_s, p_0_a5, AIE_H / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_a6_s, p_0_a6, AIE_H / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_a7_s, p_0_a7, AIE_H / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_a8_s, p_0_a8, AIE_H / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_a9_s, p_0_a9, AIE_H / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_a10_s, p_0_a10, AIE_H / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_a11_s, p_0_a11, AIE_H / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_a12_s, p_0_a12, AIE_H / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_a13_s, p_0_a13, AIE_H / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_a14_s, p_0_a14, AIE_H / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_a15_s, p_0_a15, AIE_H / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_a16_s, p_0_a16, AIE_H / (PLIO_BITS / 32), batch, 1, block_aie);


    split_input_b(reorganized_b_s, pl_b0_s, pl_pb0_s, AIE_H / (PLIO_BITS / 32), AIE_W / (PLIO_BITS / 32), AIE_W * 17 / (PLIO_BITS / 32), batch, 1);

    split_input_b(pl_pb0_s, pl_b1_s, pl_pb1_s, AIE_H / (PLIO_BITS / 32), AIE_W / (PLIO_BITS / 32), AIE_W * 16 / (PLIO_BITS / 32), batch, 1);

    split_input_b(pl_pb1_s, pl_b2_s, pl_pb2_s, AIE_H / (PLIO_BITS / 32), AIE_W / (PLIO_BITS / 32), AIE_W * 15 / (PLIO_BITS / 32), batch, 1);

    split_input_b(pl_pb2_s, pl_b3_s, pl_pb3_s, AIE_H / (PLIO_BITS / 32), AIE_W / (PLIO_BITS / 32), AIE_W * 14 / (PLIO_BITS / 32), batch, 1);

    split_input_b(pl_pb3_s, pl_b4_s, pl_pb4_s, AIE_H / (PLIO_BITS / 32), AIE_W / (PLIO_BITS / 32), AIE_W * 13 / (PLIO_BITS / 32), batch, 1);

    split_input_b(pl_pb4_s, pl_b5_s, pl_pb5_s, AIE_H / (PLIO_BITS / 32), AIE_W / (PLIO_BITS / 32), AIE_W * 12 / (PLIO_BITS / 32), batch, 1);

    split_input_b(pl_pb5_s, pl_b6_s, pl_pb6_s, AIE_H / (PLIO_BITS / 32), AIE_W / (PLIO_BITS / 32), AIE_W * 11 / (PLIO_BITS / 32), batch, 1);

    split_input_b(pl_pb6_s, pl_b7_s, pl_pb7_s, AIE_H / (PLIO_BITS / 32), AIE_W / (PLIO_BITS / 32), AIE_W * 10 / (PLIO_BITS / 32), batch, 1);

    split_input_b(pl_pb7_s, pl_b8_s, pl_pb8_s, AIE_H / (PLIO_BITS / 32), AIE_W / (PLIO_BITS / 32), AIE_W * 9 / (PLIO_BITS / 32), batch, 1);

    split_input_b(pl_pb8_s, pl_b9_s, pl_pb9_s, AIE_H / (PLIO_BITS / 32), AIE_W / (PLIO_BITS / 32), AIE_W * 8 / (PLIO_BITS / 32), batch, 1);

    split_input_b(pl_pb9_s, pl_b10_s, pl_pb10_s, AIE_H / (PLIO_BITS / 32), AIE_W / (PLIO_BITS / 32), AIE_W * 7 / (PLIO_BITS / 32), batch, 1);

    split_input_b(pl_pb10_s, pl_b11_s, pl_pb11_s, AIE_H / (PLIO_BITS / 32), AIE_W / (PLIO_BITS / 32), AIE_W * 6 / (PLIO_BITS / 32), batch, 1);

    split_input_b(pl_pb11_s, pl_b12_s, pl_pb12_s, AIE_H / (PLIO_BITS / 32), AIE_W / (PLIO_BITS / 32), AIE_W * 5 / (PLIO_BITS / 32), batch, 1);

    split_input_b(pl_pb12_s, pl_b13_s, pl_pb13_s, AIE_H / (PLIO_BITS / 32), AIE_W / (PLIO_BITS / 32), AIE_W * 4 / (PLIO_BITS / 32), batch, 1);

    split_input_b(pl_pb13_s, pl_b14_s, pl_pb14_s, AIE_H / (PLIO_BITS / 32), AIE_W / (PLIO_BITS / 32), AIE_W * 3 / (PLIO_BITS / 32), batch, 1);

    split_input_b(pl_pb14_s, pl_b15_s, pl_pb15_s, AIE_H / (PLIO_BITS / 32), AIE_W / (PLIO_BITS / 32), AIE_W * 2 / (PLIO_BITS / 32), batch, 1);

    split_input_b(pl_pb15_s, pl_b16_s, pl_b17_s, AIE_H / (PLIO_BITS / 32), AIE_W / (PLIO_BITS / 32), AIE_W / (PLIO_BITS / 32), batch, 1);


    send_plio(pl_b0_s, p_0_b0, (AIE_W + AIE_H) / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_b1_s, p_0_b1, (AIE_W + AIE_H) / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_b2_s, p_0_b2, (AIE_W + AIE_H) / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_b3_s, p_0_b3, (AIE_W + AIE_H) / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_b4_s, p_0_b4, (AIE_W + AIE_H) / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_b5_s, p_0_b5, (AIE_W + AIE_H) / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_b6_s, p_0_b6, (AIE_W + AIE_H) / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_b7_s, p_0_b7, (AIE_W + AIE_H) / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_b8_s, p_0_b8, (AIE_W + AIE_H) / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_b9_s, p_0_b9, (AIE_W + AIE_H) / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_b10_s, p_0_b10, (AIE_W + AIE_H) / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_b11_s, p_0_b11, (AIE_W + AIE_H) / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_b12_s, p_0_b12, (AIE_W + AIE_H) / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_b13_s, p_0_b13, (AIE_W + AIE_H) / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_b14_s, p_0_b14, (AIE_W + AIE_H) / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_b15_s, p_0_b15, (AIE_W + AIE_H) / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_b16_s, p_0_b16, (AIE_W + AIE_H) / (PLIO_BITS / 32), batch, 1, block_aie);

    send_plio(pl_b17_s, p_0_b17, (AIE_W + AIE_H) / (PLIO_BITS / 32), batch, 1, block_aie);



    receive_plio(merge0_c0, p_0_c0, batch, 1, block_aie);

    receive_plio(merge0_c1, p_0_c1, batch, 1, block_aie);

    receive_plio(merge0_c2, p_0_c2, batch, 1, block_aie);

    receive_plio(merge0_c3, p_0_c3, batch, 1, block_aie);

    receive_plio(merge0_c4, p_0_c4, batch, 1, block_aie);

    receive_plio(merge0_c5, p_0_c5, batch, 1, block_aie);

    receive_plio(merge0_c6, p_0_c6, batch, 1, block_aie);

    receive_plio(merge0_c7, p_0_c7, batch, 1, block_aie);

    receive_plio(merge0_c8, p_0_c8, batch, 1, block_aie);

    receive_plio(merge0_c9, p_0_c9, batch, 1, block_aie);

    receive_plio(merge0_c10, p_0_c10, batch, 1, block_aie);

    receive_plio(merge0_c11, p_0_c11, batch, 1, block_aie);

    receive_plio(merge0_c12, p_0_c12, batch, 1, block_aie);

    receive_plio(merge0_c13, p_0_c13, batch, 1, block_aie);

    receive_plio(merge0_c14, p_0_c14, batch, 1, block_aie);

    receive_plio(merge0_c15, p_0_c15, batch, 1, block_aie);

    receive_plio(merge0_c16, p_0_c16, batch, 1, block_aie);

    receive_plio(merge0_c17, p_0_c17, batch, 1, block_aie);

    receive_plio(merge0_c18, p_0_c18, batch, 1, block_aie);

    receive_plio(merge0_c19, p_0_c19, batch, 1, block_aie);

    receive_plio(merge0_c20, p_0_c20, batch, 1, block_aie);

    receive_plio(merge0_c21, p_0_c21, batch, 1, block_aie);

    receive_plio(merge0_c22, p_0_c22, batch, 1, block_aie);

    receive_plio(merge0_c23, p_0_c23, batch, 1, block_aie);

    receive_plio(merge0_c24, p_0_c24, batch, 1, block_aie);

    receive_plio(merge0_c25, p_0_c25, batch, 1, block_aie);

    receive_plio(merge0_c26, p_0_c26, batch, 1, block_aie);

    receive_plio(merge0_c27, p_0_c27, batch, 1, block_aie);

    receive_plio(merge0_c28, p_0_c28, batch, 1, block_aie);

    receive_plio(merge0_c29, p_0_c29, batch, 1, block_aie);

    receive_plio(merge0_c30, p_0_c30, batch, 1, block_aie);

    receive_plio(merge0_c31, p_0_c31, batch, 1, block_aie);

    receive_plio(merge0_c32, p_0_c32, batch, 1, block_aie);

    receive_plio(merge0_c33, p_0_c33, batch, 1, block_aie);


    serialize_merge_stream(serialized_out, merge0_c0, merge0_pc0, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 33, batch, 1);


    serialize_merge_stream(merge0_pc0, merge0_c1, merge0_pc1, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 32, batch, 1);

    serialize_merge_stream(merge0_pc1, merge0_c2, merge0_pc2, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 31, batch, 1);

    serialize_merge_stream(merge0_pc2, merge0_c3, merge0_pc3, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 30, batch, 1);

    serialize_merge_stream(merge0_pc3, merge0_c4, merge0_pc4, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 29, batch, 1);

    serialize_merge_stream(merge0_pc4, merge0_c5, merge0_pc5, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 28, batch, 1);

    serialize_merge_stream(merge0_pc5, merge0_c6, merge0_pc6, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 27, batch, 1);

    serialize_merge_stream(merge0_pc6, merge0_c7, merge0_pc7, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 26, batch, 1);

    serialize_merge_stream(merge0_pc7, merge0_c8, merge0_pc8, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 25, batch, 1);

    serialize_merge_stream(merge0_pc8, merge0_c9, merge0_pc9, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 24, batch, 1);

    serialize_merge_stream(merge0_pc9, merge0_c10, merge0_pc10, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 23, batch, 1);

    serialize_merge_stream(merge0_pc10, merge0_c11, merge0_pc11, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 22, batch, 1);

    serialize_merge_stream(merge0_pc11, merge0_c12, merge0_pc12, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 21, batch, 1);

    serialize_merge_stream(merge0_pc12, merge0_c13, merge0_pc13, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 20, batch, 1);

    serialize_merge_stream(merge0_pc13, merge0_c14, merge0_pc14, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 19, batch, 1);

    serialize_merge_stream(merge0_pc14, merge0_c15, merge0_pc15, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 18, batch, 1);

    serialize_merge_stream(merge0_pc15, merge0_c16, merge0_pc16, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 17, batch, 1);

    serialize_merge_stream(merge0_pc16, merge0_c17, merge0_pc17, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 16, batch, 1);

    serialize_merge_stream(merge0_pc17, merge0_c18, merge0_pc18, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 15, batch, 1);

    serialize_merge_stream(merge0_pc18, merge0_c19, merge0_pc19, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 14, batch, 1);

    serialize_merge_stream(merge0_pc19, merge0_c20, merge0_pc20, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 13, batch, 1);

    serialize_merge_stream(merge0_pc20, merge0_c21, merge0_pc21, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 12, batch, 1);

    serialize_merge_stream(merge0_pc21, merge0_c22, merge0_pc22, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 11, batch, 1);

    serialize_merge_stream(merge0_pc22, merge0_c23, merge0_pc23, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 10, batch, 1);

    serialize_merge_stream(merge0_pc23, merge0_c24, merge0_pc24, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 9, batch, 1);

    serialize_merge_stream(merge0_pc24, merge0_c25, merge0_pc25, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 8, batch, 1);

    serialize_merge_stream(merge0_pc25, merge0_c26, merge0_pc26, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 7, batch, 1);

    serialize_merge_stream(merge0_pc26, merge0_c27, merge0_pc27, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 6, batch, 1);

    serialize_merge_stream(merge0_pc27, merge0_c28, merge0_pc28, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 5, batch, 1);

    serialize_merge_stream(merge0_pc28, merge0_c29, merge0_pc29, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 4, batch, 1);

    serialize_merge_stream(merge0_pc29, merge0_c30, merge0_pc30, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 3, batch, 1);

    serialize_merge_stream(merge0_pc30, merge0_c31, merge0_pc31, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK * 2, batch, 1);

    serialize_merge_stream(merge0_pc31, merge0_c32, merge0_c33, AIE_OUTPUT_CHUNK, AIE_OUTPUT_CHUNK, batch, 1);

    merge_phase2(final_merge_out, serialized_out, batch, 1);

    write_ddr(mem_out, final_merge_out, batch, 1, block_ddr);
}

}
}