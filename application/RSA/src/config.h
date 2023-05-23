#define INPUT_BITS 8192
#define DDR_BITS 512
#define PLIO_BITS 128

#define AIE_W 136
#define AIE_H 136

#define SENT_VALID ((INPUT_BITS / 31) / 4)
#define TAILS_A 8
#define TAILS_B 144

#define VEC_ELEM_BITS 32
#define VALID_BITS 31
#define VEC_SIZE 8

#define HOST_ADDER_BITS 73
#define LAST_VALID ((AIE_W * VALID_BITS) - ((AIE_W * VALID_BITS) / DDR_BITS) * DDR_BITS)

#define AIE_COL 3
#define AIE_ROW 2
#define AIE_NUM 6
#define OUTPUT_AIE 4
#define ROW_SIZE AIE_H
#define COL_SIZE AIE_W
#define RSA_DATA_CHUNK (INPUT_BITS / DDR_BITS)
#define BATCH 16
#define TASK_INPUT_CHUNK 16
#define TASK_OUTPUT_CHUNK 32
#define AIE_OUTPUT_CHUNK 10
