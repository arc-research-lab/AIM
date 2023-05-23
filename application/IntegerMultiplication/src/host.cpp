#include "./aie_graph.h"
#include "./config.h"



#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fstream>
#include <memory>
#define AP_INT_MAX_W 32768
#include <ap_int.h>

// This is used for the PL Kernels
#include "xrt/xrt.h"
#include "xrt/experimental/xrt_kernel.h"

// Using the ADF API that call XRT API
#include "adf/adf_api/XRTConfig.h"
simpleGraph addergraph;

int ceil(int val, int f){
    return (val / f) * f == val ? val : (val / f + 1) * f;
}



static std::vector<char> load_xclbin(xrtDeviceHandle device, const std::string& fnm) {
    if (fnm.empty()) throw std::runtime_error("No xclbin specified");

    std::ifstream stream(fnm);
    stream.seekg(0, stream.end);
    size_t size = stream.tellg();
    stream.seekg(0, stream.beg);

    std::vector<char> header(size);
    stream.read(header.data(), size);

    auto top = reinterpret_cast<const axlf*>(header.data());
    if (xrtDeviceLoadXclbin(device, top)) throw std::runtime_error("Xclbin loading failed");

    return header;
}


void sw_cmp(const bool enable, unsigned int * data_a, unsigned int * data_b, unsigned int * data_c, unsigned int offset){
    if(!enable) return;
    ap_uint<HOST_ADDER_BITS - 32> carry = 0;
    for(uint32_t i = 0; i < INPUT_BITS / 32; ++i){
        ap_uint<HOST_ADDER_BITS> res = 0;
        for(uint32_t j = 0; j <= i; ++j){
            ap_uint<HOST_ADDER_BITS> a0 = data_a[offset * (TASK_INPUT_CHUNK * 512 / 32) + j];
            ap_uint<HOST_ADDER_BITS> b0 = data_b[offset * (TASK_INPUT_CHUNK * 512 / 32) + i - j];
            res += a0 * b0;
        }
        res += carry;
        carry = res(HOST_ADDER_BITS - 1, 32);
        data_c[offset * (TASK_OUTPUT_CHUNK * 512 / 32) + i] = (unsigned int) res(31, 0);
    }

    for(uint32_t i = 0; i < INPUT_BITS / 32; ++i){
        ap_uint<HOST_ADDER_BITS> res = 0;
        for(uint32_t j = 1; j < INPUT_BITS / 32 - i; ++j){
            ap_uint<HOST_ADDER_BITS> a0 = data_a[offset * (TASK_INPUT_CHUNK * 512 / 32) + j + i];
            ap_uint<HOST_ADDER_BITS> b0 = data_b[offset * (TASK_INPUT_CHUNK * 512 / 32) + INPUT_BITS / 32 - j];
            res += a0 * b0;
        }
        res += carry;
        carry = res(HOST_ADDER_BITS - 1, 32);
        data_c[offset * (TASK_OUTPUT_CHUNK * 512 / 32) + INPUT_BITS / 32 + i] = (unsigned int) res(31, 0);
    }
}

int main(int argc, char** argv) {

    
    int repeat = 2;
    int enable_vrf = 1;
    int batch = 1;
    bool block_ddr = false;

    if(argc == 6){
        batch = atoi(argv[2]);
        repeat = atoi(argv[3]);
        enable_vrf = atoi(argv[4]);
        block_ddr = (atoi(argv[5]) == 1);
    }

    
    // std::cout << "syn: " << syn << std::endl;
    std::cout << "repeat: " << repeat << " times" << std::endl;
    std::cout << "software verify?  " << (enable_vrf == 1 ? "yes" : "no") << std::endl;
    std::cout << "remove ddr?  " << (block_ddr == 1 ? "yes" : "no") << std::endl;

    //////////////////////////////////////////
    // Open xclbin
    //////////////////////////////////////////
    auto dhdl = xrtDeviceOpen(0); // Open Device the local device
    if (dhdl == nullptr) throw std::runtime_error("No valid device handle found. Make sure using right xclOpen index.");
    auto xclbin = load_xclbin(dhdl, "krnl_adder.xclbin");
    auto top = reinterpret_cast<const axlf*>(xclbin.data());
    adf::registerXRT(dhdl, top->m_header.uuid);
    
    srand(time(0));



    unsigned int *data_in_a_0 = (unsigned int *) calloc(1, batch * TASK_INPUT_CHUNK * 512 / 8);
    unsigned int *data_in_b_0 = (unsigned int *) calloc(1, batch * TASK_INPUT_CHUNK * 512 / 8);
    unsigned int *data_c_sw_0 = (unsigned int *) calloc(1, batch * TASK_OUTPUT_CHUNK * 512 / 8);
    unsigned int *data_c_hw_0 = (unsigned int *) calloc(1, batch * TASK_OUTPUT_CHUNK * 512 / 8);

    for(uint32_t i = 0; i < batch; ++i){
            uint32_t offset = i * INPUT_BITS / 32;
            for(uint32_t k = 0; k < INPUT_BITS / 32; ++k){
                data_in_a_0[offset + k] = rand() * 3;
                data_in_b_0[offset + k] = rand() * 3;
            }
            
            sw_cmp(enable_vrf, data_in_a_0, data_in_b_0, data_c_sw_0, (i));
            
    }
    xrtBufferHandle in_bohdl0_0 = xrtBOAlloc(dhdl, batch * TASK_INPUT_CHUNK * 512 / 8, 0, 0);
    auto in_bomapped0_0 = reinterpret_cast<uint32_t*>(xrtBOMap(in_bohdl0_0));
    memcpy(in_bomapped0_0, data_in_a_0, batch * TASK_INPUT_CHUNK * 512 / 8);


    xrtBufferHandle in_bohdl1_0 = xrtBOAlloc(dhdl, batch * TASK_INPUT_CHUNK * 512 / 8, 0, 0);
    auto in_bomapped1_0 = reinterpret_cast<uint32_t*>(xrtBOMap(in_bohdl1_0));
    memcpy(in_bomapped1_0, data_in_b_0, batch * TASK_INPUT_CHUNK * 512 / 8);

    xrtBufferHandle out_bohdl_0 = xrtBOAlloc(dhdl, batch * TASK_OUTPUT_CHUNK * 512 / 8, 0, 0);
    auto out_bomapped_0 = reinterpret_cast<uint32_t*>(xrtBOMap(out_bohdl_0));
    memset(out_bomapped_0, 0xABCDEF00, batch * TASK_OUTPUT_CHUNK * 512 / 8);

    xrtBufferHandle out_bohdl2_0 = xrtBOAlloc(dhdl, 4 * 8, 0, 0);
    auto out_bomapped2_0 = reinterpret_cast<uint64_t*>(xrtBOMap(out_bohdl2_0));
    memset(out_bomapped2_0, 0xABCDEF00, 4 * 8);


    std::cout << ("graph init. This does nothing because CDO in boot PDI already configures AIE.\n");
    addergraph.init();

    std::cout << ("graph run\n");
    addergraph.run(-1);

    

    xrtKernelHandle pl_kernl_khdl1_0 = xrtPLKernelOpen(dhdl, top->m_header.uuid, "pl_kernel:{pl_kernel_0}");
    


    double kernel_time_in_sec = 0;
    std::chrono::duration<double> kernel_time(0);
    auto kernel_start = std::chrono::high_resolution_clock::now();
    

    xrtRunHandle pl_kernl_rhdl1_0 = xrtKernelRun(pl_kernl_khdl1_0, in_bohdl0_0, in_bohdl1_0, out_bohdl_0, batch, repeat, block_ddr, false, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

    

    auto state_0 = xrtRunWait(pl_kernl_rhdl1_0);


    auto kernel_end = std::chrono::high_resolution_clock::now();
    kernel_time = std::chrono::duration<double>(kernel_end - kernel_start);
    kernel_time_in_sec = kernel_time.count();


    std::cout << "pl_0 completed with status(" << state_0 << ")\n";
    xrtRunClose(pl_kernl_rhdl1_0);
    xrtKernelClose(pl_kernl_khdl1_0);
  

    

    int errorCount = 0;

    
    if(enable_vrf == 1)
    {

        for(uint32_t i = 0; i < batch; ++i){
            uint32_t offset = i * (TASK_OUTPUT_CHUNK * 512 / 32);
            for(uint32_t k = 0; k < INPUT_BITS * 2 / 32; ++k){
                if(out_bomapped_0[offset + k] != data_c_sw_0[offset + k]){
                    std::cout << "BATCH(" << i << ") @(" << k << ") " << out_bomapped_0[offset + k] << " ~ " << data_c_sw_0[offset + k] << std::endl;
                    errorCount++;
                }
            }
        }
 
        

        if (errorCount)
            std::cout << "Test(" << batch * INPUT_BITS * 1 / 32 * 2 << ") failed with " << errorCount << " errors\n";
        else
            std::cout << ("TEST PASSED!!\n");
    }
 
    std::cout << "Releasing remaining XRT objects...\n";

    xrtBOFree(in_bohdl0_0);
    xrtBOFree(in_bohdl1_0);
    xrtBOFree(out_bohdl_0);
    

    free(data_in_a_0);
    free(data_in_b_0);
    free(data_c_sw_0);
    free(data_c_hw_0);
     

    xrtDeviceClose(dhdl);
    std::ofstream ofs;
    ofs.open("res.log");
    ofs << INPUT_BITS << "-bits, " << "BATCH:" << batch << ", #AIE:" << AIE_NUM << std::endl;
    ofs << kernel_time_in_sec << " sec" << std::endl;
    ofs.close();

    std::cout << "finish in " << kernel_time_in_sec << " sec\n";
    std::cout << INPUT_BITS << "-bits: " << std::chrono::duration_cast<std::chrono::nanoseconds>(kernel_end - kernel_start).count() / (double)repeat << " ns\n";

    return errorCount;

}