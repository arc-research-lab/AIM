#include "./aie_graph.h"
#include "./config.h"

#define DATA_SIZE (INPUT_BITS/32)
#define N_ITER 1

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fstream>
#include <opencv2/opencv.hpp>
#define AP_INT_MAX_W 16384
#include <ap_int.h>

// This is used for the PL Kernels
#include "xrt/xrt.h"
#include "xrt/experimental/xrt_kernel.h"

// Using the ADF API that call XRT API
#include "adf/adf_api/XRTConfig.h"
simpleGraph addergraph;
#define NORM_BITS 6
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

bool sign_add(ap_uint<INPUT_BITS> a, bool a_positive, ap_uint<INPUT_BITS> b, bool b_positive, ap_uint<INPUT_BITS> &res){
    if(a_positive == b_positive){
        res = a + b;
        return a_positive;
    }else if(a_positive && a >= b){
        res = a - b;
        return true;
    }else if(a_positive){
        res = b - a;
        return false;
    }else if(b_positive && b >= a){
        res = b - a;
        return true;
    }else{
        res = a - b;
        return false;
    }
}

uint32_t iterate(ap_uint<INPUT_BITS> x0, bool x0_positive, ap_uint<INPUT_BITS> y0, bool y0_positive, ap_uint<INPUT_BITS * 2> threadhold, uint32_t norm_bits, uint32_t max_itr){
    ap_uint<INPUT_BITS> real = x0;
    ap_uint<INPUT_BITS> image = y0;
    // ap_fixed<INPUT_BITS, INPUT_BITS - NORM_BITS> a0 = 0;
    // ap_fixed<INPUT_BITS, INPUT_BITS - NORM_BITS> b0 = 0;
    // a0(INPUT_BITS - 1, 0) = x0(INPUT_BITS - 1, 0);
    // b0(INPUT_BITS - 1, 0) = y0(INPUT_BITS - 1, 0);
    // std::cout << a0 << std::endl;
    // std::cout << b0 << std::endl;
    bool real_positive = x0_positive;
    bool image_positive = y0_positive;

    for(uint32_t i = 0; i <= max_itr; ++i){
        ap_uint<INPUT_BITS * 2> real_square = 0;
        real_square += real;
        real_square *= real;

        ap_uint<INPUT_BITS * 2> image_square = 0;
        image_square += image;
        image_square *= image;
        ap_uint<INPUT_BITS * 2> sum = (image_square + real_square);
        ap_fixed<INPUT_BITS * 2, INPUT_BITS * 2 - NORM_BITS * 2> tmp = 0;
        tmp(INPUT_BITS * 2 - 1, 0) = threadhold(INPUT_BITS * 2 - 1, 0);

        // ap_fixed<INPUT_BITS, INPUT_BITS - NORM_BITS> tmp = 0;
        // tmp(INPUT_BITS - 1, 0) = real(INPUT_BITS - 1, 0);
        // std::cout << std::hex << sum << std::endl;
        if(sum > threadhold){
            
            return i;
        }

        ap_uint<INPUT_BITS * 2> tmp_real;
        bool real_tmp;
        if(real_square >= image_square){
            tmp_real = real_square - image_square;
            real_tmp = true;
        }else {
            tmp_real = image_square - real_square;
            real_tmp = false;
        }

        tmp_real >>= NORM_BITS;

        ap_uint<INPUT_BITS * 2> tmp_image = real;
        tmp_image *= image;

        tmp_image >>= (NORM_BITS - 1);

        image_positive = sign_add(tmp_image(INPUT_BITS - 1, 0), (real_positive == image_positive), y0, y0_positive, image);
        real_positive = sign_add(tmp_real(INPUT_BITS - 1, 0), real_tmp, x0, x0_positive, real);

    }
    return max_itr;
}

void sw_cmp(uint32_t width, uint32_t height, ap_uint<INPUT_BITS> x0, ap_uint<INPUT_BITS> y0, ap_uint<INPUT_BITS * 2> threadhold, uint32_t norm_bits, uint32_t step0, uint32_t step1, uint32_t max_itr){
    cv::Mat img = cv::Mat::zeros(height, width, CV_8UC3);
    ap_uint<INPUT_BITS > image = 0;
    image(INPUT_BITS - 1, 0) = y0(INPUT_BITS - 1, 0);
    // std::cout << d << std::endl;
    bool real_positive = 1;
    bool image_positive = 1;

    for(uint32_t i = 0; i < height; ++i){
        ap_uint<INPUT_BITS > real = x0;
        real_positive = true;
        for(uint32_t j = 0; j < width; ++j){

            ap_fixed<INPUT_BITS, INPUT_BITS - NORM_BITS> a0 = 0;
            ap_fixed<INPUT_BITS, INPUT_BITS - NORM_BITS> b0 = 0;
            a0(INPUT_BITS - 1, 0) = real(INPUT_BITS - 1, 0);
            b0(INPUT_BITS - 1, 0) = image(INPUT_BITS - 1, 0);

            uint32_t itr = iterate(real, real_positive, image, image_positive, threadhold, norm_bits, max_itr);
            img.at<cv::Vec3b>(i, j)[0] = itr & 0x3 * 255 / 3;
            img.at<cv::Vec3b>(i, j)[1] = (itr >> 2) & 0x3 * 255 / 3;
            img.at<cv::Vec3b>(i, j)[2] = (itr >> 4) & 0x3 * 255 / 3;

            if(real_positive && real >= step0){
                real -= step0;
            }else if(real_positive){
                real = step0 - real;
                real_positive = false;
            }else if(!real_positive){
                real += step0;
            }
        }

        if(image_positive && image >= step1){
            image -= step1;
        }else if(image_positive){
            image = step1 - image;
            image_positive = false;
        }else if(!image_positive){
            image += step1;
        }
        
    }
    
    imwrite("sw_mandelbrot.png", img);
}


int main(int argc, char** argv) {
    int repeat = 1;
    int enable_vrf = 1;
    bool block_ddr = false;
    uint32_t batch = 1;

    uint32_t width = 512;
    uint32_t height = 512;
    uint32_t max_itr = 64;

    uint32_t norm_bit = NORM_BITS;
    // uint32_t thresh = norm_bit * 2 + 2;
    
    if(argc == 6){
        width = atoi(argv[2]);
        height = atoi(argv[3]);
        max_itr = atoi(argv[4]); 
        norm_bit = atoi(argv[5]); 
        // thresh = atoi(argv[6]); 
    }




    // std::cout << "width: " << width << std::endl;
    // std::cout << "height: " << height << std::endl;
    // std::cout << "max_itr: " << max_itr << std::endl;
    // std::cout << "norm_bit: " << norm_bit << std::endl;
    // std::cout << "thresh: " << thresh << std::endl;

    // std::cout << "norm_chunk_real: " << norm_chunk_real << std::endl;
    // std::cout << "norm_bits_real: " << norm_bits_real << std::endl;
    // std::cout << "norm_chunk_image: " << norm_chunk_image << std::endl;
    // std::cout << "norm_bits_image: " << norm_bits_image << std::endl;


    auto dhdl = xrtDeviceOpen(0); // Open Device the local device
    if (dhdl == nullptr) throw std::runtime_error("No valid device handle found. Make sure using right xclOpen index.");
    auto xclbin = load_xclbin(dhdl, "krnl_adder.xclbin");
    auto top = reinterpret_cast<const axlf*>(xclbin.data());
    adf::registerXRT(dhdl, top->m_header.uuid);

    printf("graph init. This does nothing because CDO in boot PDI already configures AIE.\n");
    addergraph.init();

    printf("graph run\n");
    addergraph.run(-1);

    srand(time(0));


    uint32_t norm_chunk_real = norm_bit / DDR_BITS;
    uint32_t norm_bits_real = norm_bit % DDR_BITS;
    uint32_t norm_chunk_image = (norm_bit - 1) / DDR_BITS;
    uint32_t norm_bits_image = (norm_bit - 1) % DDR_BITS;
    ap_uint<INPUT_BITS * 2> threadhold = 0;
    threadhold[2 * norm_bit + 2] = 1;
    unsigned int * thresh_buff = (unsigned int *) malloc(INPUT_BITS * 2 / 8);
    unsigned int * x0_buff = (unsigned int *) malloc(INPUT_BITS / 8);
    unsigned int * y0_buff = (unsigned int *) malloc(INPUT_BITS / 8);

    unsigned int * step0_buff = (unsigned int *) malloc(INPUT_BITS / 8);
    unsigned int * step1_buff = (unsigned int *) malloc(INPUT_BITS / 8);

    

    xrtBufferHandle threashold_hdl = xrtBOAlloc(dhdl, INPUT_BITS * 2 / 8, 0, 0);
    xrtBufferHandle x0_hdl = xrtBOAlloc(dhdl, INPUT_BITS / 8, 0, 0);
    xrtBufferHandle y0_hdl = xrtBOAlloc(dhdl, INPUT_BITS / 8, 0, 0);
    xrtBufferHandle step0_hdl = xrtBOAlloc(dhdl, INPUT_BITS / 8, 0, 0);
    xrtBufferHandle step1_hdl = xrtBOAlloc(dhdl, INPUT_BITS / 8, 0, 0);
    


    auto threashold_mapped = reinterpret_cast<uint32_t*>(xrtBOMap(threashold_hdl));
    auto x0_mapped = reinterpret_cast<uint32_t*>(xrtBOMap(x0_hdl));
    auto y0_mapped = reinterpret_cast<uint32_t*>(xrtBOMap(y0_hdl));
    auto step0_mapped = reinterpret_cast<uint32_t*>(xrtBOMap(step0_hdl));
    auto step1_mapped = reinterpret_cast<uint32_t*>(xrtBOMap(step1_hdl));
    

    bool x0_sign = false, y0_sign = false;
    char c = 'z';
    // cv::Mat img_color = cv::imread("img.png");
    // ap_uint<INPUT_BITS * 2> threadhold = 0;

    ap_fixed<INPUT_BITS, INPUT_BITS - NORM_BITS> x0_tmp = -2;
    ap_fixed<INPUT_BITS, INPUT_BITS - NORM_BITS> y0_tmp = -2;
    ap_fixed<INPUT_BITS, INPUT_BITS - NORM_BITS> x1_tmp = 2;
    ap_fixed<INPUT_BITS, INPUT_BITS - NORM_BITS> y1_tmp = 2;

    // ap_fixed<INPUT_BITS, INPUT_BITS - NORM_BITS> step0_tmp = 5.56393525043092351340956321976e-17;
    // ap_fixed<INPUT_BITS, INPUT_BITS - NORM_BITS> step1_tmp = 5.55752518677835249146627857619e-17;

    ap_uint<INPUT_BITS> x0 = 0;
    ap_uint<INPUT_BITS> y0 = 0;

    // x0(INPUT_BITS - 1, 0) = x0_tmp(INPUT_BITS - 1, 0);
    // y0(INPUT_BITS - 1, 0) = y0_tmp(INPUT_BITS - 1, 0);

    ap_uint<INPUT_BITS> step0 = 0;
    ap_uint<INPUT_BITS> step1 = 0;

    // step0(INPUT_BITS - 1, 0) = step0_tmp(INPUT_BITS - 1, 0);
    // step1(INPUT_BITS - 1, 0) = step1_tmp(INPUT_BITS - 1, 0);

    uint32_t thresh = norm_bit * 2 + 2;

    step0[norm_bit - 6] = 1;
    step1[norm_bit - 6] = 1;
    threadhold[thresh] = 1;
    x0[norm_bit + 1] = 1;
    y0[norm_bit + 1] = 1;

// while(c != 'q')
// {
//     cv::imshow("img.png", img_color);
//     c = cv::waitKey(0);
//     if(c == 'i'){
//         norm_bit++;
//         threadhold = 0;
//         x0 <<= 1;
//         y0 <<= 1;
//         threadhold[norm_bit * 2 + 2] = 1;
//     }else if(c == 'o'){
//         norm_bit--;
//         threadhold = 0;
//         x0 >>= 1;
//         y0 >>= 1;
//         threadhold[norm_bit * 2 + 2] = 1;
//     }else if(c == 'r'){
//         // run again
//     }else if(c == 'w'){
//         ap_uint<INPUT_BITS> tmp = height / 4 * step1;
//         // tmp[norm_bit - 2] = 1;
//         y0_sign = sign_add(y0, y0_sign, tmp, true, y0);
//     }else if(c == 's'){
//         ap_uint<INPUT_BITS> tmp = height / 4 * step1;
//         // tmp[norm_bit - 2] = 1;
//         y0_sign = sign_add(y0, y0_sign, tmp, false, y0);
//     }else if(c == 'd'){
//         ap_uint<INPUT_BITS> tmp = width / 4 * step0;
//         // tmp[norm_bit - 2] = 1;
//         x0_sign = sign_add(x0, x0_sign, tmp, false, x0);
//     }else if(c == 'a'){
//         ap_uint<INPUT_BITS> tmp = width / 4  * step0;
//         // tmp[norm_bit - 2] = 1;
//         x0_sign = sign_add(x0, x0_sign, tmp, true, x0);
//     }else if(c == 'u'){
//         x0 <<= 1;
//         y0 <<= 1;
//         threadhold <<= 1;
//         norm_bit++;
//         step0 <<= 1;
//         step1 <<= 1;
//         threadhold[norm_bit * 2 + 2] = 1;
//     }else if(c == 'p'){
//         x0 >>= 1;
//         y0 >>= 1;
//         threadhold >>= 1;
//         norm_bit--;
//         step0 >>= 1;
//         step1 >>= 1;
//         threadhold[norm_bit * 2 + 2] = 1;
//     }else if(c == 'n'){
//         max_itr += 8;
//     }else if(c == 'm'){
//         max_itr -= 8;
//     }else if(c == 'x'){
//         threadhold >>= 1;
//         norm_bit--;
//         width >>= 1;
//         height >>= 1;
//         x0 >>= 1;
//         y0 >>= 1;

//         threadhold[norm_bit * 2 + 2] = 1;
//     }else if(c == 'c'){
//         threadhold <<= 1;
//         norm_bit++;
//         width <<= 1;
//         height <<= 1;
//         x0 <<= 1;
//         y0 <<= 1;
//         threadhold[norm_bit * 2 + 2] = 1;
//     }else{
//         continue;
//     }
    xrtBufferHandle out_bohdl_0 = xrtBOAlloc(dhdl, (width * height) * sizeof(int), 0, 0);
    auto out_bomapped_0 = reinterpret_cast<uint32_t*>(xrtBOMap(out_bohdl_0));

    norm_chunk_real = norm_bit / DDR_BITS;
    norm_bits_real = norm_bit % DDR_BITS;
    norm_chunk_image = (norm_bit - 1) / DDR_BITS;
    norm_bits_image = (norm_bit - 1) % DDR_BITS;

    // step0[0] = 1;
    // step1[0] = 1;
    // threadhold[thresh] = 1;
    // x0[norm_bit + 1] = 1;
    // y0[norm_bit + 1] = 1;


    for(uint32_t i = 0; i < INPUT_BITS * 2 / 32; ++i){
        thresh_buff[i] = threadhold((i + 1) * 32 - 1, i * 32);
    }

    for(uint32_t i = 0; i < INPUT_BITS / 32; ++i){
        x0_buff[i] = x0((i + 1) * 32 - 1, i * 32);
        y0_buff[i] = y0((i + 1) * 32 - 1, i * 32);
        step0_buff[i] = step0((i + 1) * 32 - 1, i * 32);
        step1_buff[i] = step1((i + 1) * 32 - 1, i * 32);
    }

    memcpy(threashold_mapped, thresh_buff, INPUT_BITS * 2 / 8);
    memcpy(x0_mapped, x0_buff, INPUT_BITS / 8);
    memcpy(y0_mapped, y0_buff, INPUT_BITS / 8);
    memcpy(step0_mapped, step0_buff, INPUT_BITS / 8);
    memcpy(step1_mapped, step1_buff, INPUT_BITS / 8);
    memset(out_bomapped_0, 0xABCDEF00, (width * height) * sizeof(int));

    xrtKernelHandle pl_kernel_khdl1_0 = xrtPLKernelOpen(dhdl, top->m_header.uuid, "pl_kernel:{pl_kernel_0}");

    double kernel_time_in_sec = 0;
    std::chrono::duration<double> kernel_time(0);
    auto kernel_start = std::chrono::high_resolution_clock::now();

    xrtRunHandle pl_kernel_rhdl1_0 = xrtKernelRun(pl_kernel_khdl1_0, threashold_hdl, x0_hdl, x0_sign, y0_hdl, y0_sign, step0_hdl, step1_hdl, out_bohdl_0, 1, 1, norm_chunk_real, norm_bits_real, norm_chunk_image, norm_bits_image, 1, 1, 0, nullptr, nullptr, nullptr);

    auto state_0 = xrtRunWait(pl_kernel_rhdl1_0);
    xrtRunClose(pl_kernel_rhdl1_0);
    xrtKernelClose(pl_kernel_khdl1_0);

    auto kernel_end = std::chrono::high_resolution_clock::now();
    kernel_time = std::chrono::duration<double>(kernel_end - kernel_start);
    kernel_time_in_sec = kernel_time.count();

    uint32_t mul = 0;
    cv::Mat img = cv::Mat::zeros(height, width, CV_8UC1);
    for(uint32_t i = 0; i < height; ++i){
        for(uint32_t j = 0; j < width; ++j){
            mul += out_bomapped_0[i * width + j] * 3 + 2;
            img.at<uchar>(i, j) = (out_bomapped_0[i * width + j] * 255. / max_itr);
        }
    }

    
    // cv::applyColorMap(img, img_color, cv::COLORMAP_JET);
    
    std::cout << std::hex << "x0: " << (x0_sign ? "" : "- ") << x0 << std::endl;
    std::cout << std::hex << "y0: " << (y0_sign ? "" : "- ") << y0 << std::endl;
    std::cout << std::hex << "x1: " << (x0_sign ? 1 : -1) * x0 + step0 * width << std::endl;
    std::cout << std::hex << "y1: " << (y0_sign ? 1 : -1) * y0 + step1 * height << std::endl;

    std::cout << std::dec << "max iteration: " << max_itr << std::endl;
    std::cout << std::dec << "norm bits: " << norm_bit << std::endl;
    std::cout << std::dec << "#multiplications: " << mul << "\n";
    std::cout << std::dec << "finish in " << kernel_time_in_sec << " sec\n";

    std::cout << std::hex << x0 << " ";
    std::cout << std::hex << y0 << " ";
    std::cout << std::hex << (x0_sign ? 1 : -1) * x0 + step0 * width << " ";
    std::cout << std::hex << (y0_sign ? 1 : -1) * y0 + step1 * height << " ";
    std::cout << std::dec << max_itr << std::endl;
    xrtBOFree(out_bohdl_0);
    
// }
    // imwrite("img.png", img_color);
    xrtBOFree(threashold_hdl);
    xrtBOFree(x0_hdl);
    xrtBOFree(y0_hdl);
    xrtBOFree(step0_hdl);
    xrtBOFree(step1_hdl);
   

    xrtDeviceClose(dhdl);
    return 0;

}

// int main(int argc, char** argv) {
//     cv::Mat image = cv::imread("img.png");
//     cv::imshow("image", image);
//     cv::waitKey(0);
//     return 0;
// }