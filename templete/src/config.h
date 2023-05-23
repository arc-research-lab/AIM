#define INPUT_BITS 8192
#define DDR_BITS 512
#define PLIO_BITS 128

#define AIE_W 16
#define AIE_H 16

#define SENT_VALID ((INPUT_BITS / 31) / 4)
#define TAILS_A 8
#define TAILS_B 24

#define VEC_ELEM_BITS 32
#define VALID_BITS 31
#define VEC_SIZE 8

#define HOST_ADDER_BITS 73
#define LAST_VALID ((AIE_W * VALID_BITS) - ((AIE_W * VALID_BITS) / DDR_BITS) * DDR_BITS)

#define AIE_COL 18
#define AIE_ROW 17
#define AIE_NUM 306
#define OUTPUT_AIE 34
#define ROW_SIZE AIE_H
#define COL_SIZE AIE_W
#define TASK_INPUT_CHUNK 16
#define TASK_OUTPUT_CHUNK 32
#define AIE_OUTPUT_CHUNK 2
