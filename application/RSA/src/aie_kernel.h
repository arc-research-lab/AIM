/**
* Copyright (C) 2019-2021 Xilinx, Inc
*
* Licensed under the Apache License, Version 2.0 (the "License"). You may
* not use this file except in compliance with the License. A copy of the
* License is located at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
* WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
* License for the specific language governing permissions and limitations
* under the License.
*/

#ifndef __KERNELS_H__
#define __KERNELS_H__

#include <adf/stream/types.h>
#include "aie_config.h"


// void aim_top_bottom(input_window_int32* __restrict in0, input_window_int32* __restrict  in1, output_stream_int32* __restrict  vec_out);
// void aim_top(input_window_int32* __restrict in0, input_window_int32* __restrict  in1, output_stream_acc80* __restrict  acc_out);

// void aim_mid(input_window_int32* __restrict in0, input_window_int32* __restrict  in1, input_stream_acc80* __restrict acc_in, output_stream_acc80* __restrict  acc_out);
// void aim_bottom(input_window_int32* __restrict in0, input_window_int32* __restrict  in1, input_stream_acc80* __restrict acc_in, output_stream_int32* __restrict  vec_out);

void aie_aim_bottom(input_window_int32* __restrict in0, input_window_int32* __restrict in1, input_stream_acc80* __restrict acc_in, output_stream_int32* __restrict vec_out);
void aie_aim_mid(input_window_int32* __restrict in0, input_window_int32* __restrict in1, input_stream_acc80* __restrict acc_in, output_stream_acc80* __restrict acc_out);
void aie_aim_top_bottom(input_window_int32* __restrict in0, input_window_int32* __restrict in1, output_stream_int32* __restrict vec_out);
void aie_aim_top(input_window_int32* __restrict in0, input_window_int32* __restrict in1, output_stream_acc80* __restrict acc_out);

#endif /**********__KERNELS_H__**********/
