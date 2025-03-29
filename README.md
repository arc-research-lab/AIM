# Team
Principal Investigator: Prof. Peipei Zhou, https://peipeizhou-eecs.github.io/

Ph.D. Students: Zhuoping Yang (Student Lead) and Jinming Zhuang

Faculty Collaborators: Professors Cunxi Yu (University of Maryland), Alex Jones (Syracuse University)

Student Collaborator: Jiaqi Yin

[PDF](https://peipeizhou-eecs.github.io/publication/2023iccad/2023iccad.pdf)

## 🚀 Thank You for Using AIM!!
### Your support and growing engagement inspire us to continually improve and enhance AIM
- **Downloads since 22 Feb 2025:** <!--CLONES-->174<!--/CLONES-->
- **Views: since 22 Feb 2025:** <!--VIEWS-->641<!--/VIEWS-->
<img src="./assets/AIM_traffic_plot.png" width="600" />


# AIM: Accelerating **A**rbitrary-precision **I**nteger **M**ultiplication on Heterogeneous Reconfigurable Computing Platform Versal ACAP (ICCAD'23) 

## ACM/IEEE Reference Format
Z. Yang, J. Zhuang, J. Yin, C. Yu, A. K. Jones and P. Zhou, "AIM: Accelerating Arbitrary-Precision Integer Multiplication on Heterogeneous Reconfigurable Computing Platform Versal ACAP," 2023 IEEE/ACM International Conference on Computer Aided Design (ICCAD), San Francisco, CA, USA, 2023, pp. 1-9, [doi: 10.1109/ICCAD57390.2023.10323754](https://doi.org/10.1109/ICCAD57390.2023.10323754).




## Overview
In this repo, we propose AIM architecture and demonstrate the effectiveness of the heterogeneous platform (FPGA + vector units) for arbitrary-precision integer multiplication. The decomposed multiplications are assigned to the AIE array (vector units), which has high computing power, and the carry propagation is offloaded to the programmable logic to take advantage of its traits for fine-grained data processing. 
![image](./images/AIM%20arch.png)

We carry out experiments on the proposed AIM architecture and compare it with SOTA CPU (Intel Xeon 6346) and GPU (Nvidia A5000). [GMP](https://gmplib.org/) and [CGBN](https://github.com/NVlabs/CGBN) are used for CPU and GPU in the experiments. The results show AIM acheives up to 12.63x and 2.13x energy efficiency gains over CPU and GPU respectively.
![image](./images/Compare.png)

We also demonstrate the applicability of AIM architecture on three applications. Please find [LIM](./application/IntegerMultiplication/), [RSA](./application/RSA/), and [Mandelbrot](./application/Mandelbrot/) in this repo.

## Installation
AIM framework is verified on AMD Versal VCK190 using Vitis tool 2021.1 on Ubuntu 20.04. Users need to apply the AIE license from AMD and then setup the development environment as follows:

**Step 1**
Setup the Vitis tool. Users first need to download and install **Vitis 2021.1** tools from https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/vitis/archive-vitis.html 

**Step 2**
Setup the platform package (BSP) by downloading **VCK190 Base 2021.1** from https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/embedded-platforms/archive-vitis-embedded.html
The files in BSP has the hardware information and should be accessible for the Vitis tools.


**Step 3**
Setup the Petalinux cross compilation toolchain. Users need to download **Versal common image** from https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/embedded-platforms/archive-vitis-embedded.html

Then, execute the **sdk.sh** in the **PetaLinux 2021.1 Installer**

**Step 4**
Install dependencies required for the compilation:
> sudo
apt install make ocl-icd-libopencl1 opencl-headers
ocl-icd-opencl-dev graphviz

**Step 5**
Install [Python3.8.10](https://www.python.org/downloads/release/python-3810/) and dependencies:
> pip install jinja2 configparser

## Usage - Large Integer Multiplication (LIM)
Users can use the [AIM template](templete/) to generate their own projects that leverage AIM to compute arbitrary integer multiplication.

As can be found in [AIM.conf](templete/tools/AIM.conf), the configuration file has three parts. The first part **[Config]** describe the application configuration and target PL frequency. Users can specify the bitwidth of multiplication and it does not have to be the power of 2. Besides, users can first set a high initial frequency, and AIM will automatically decrease the frequency if the timing violation happens.
The second part **[Constraints]** describe how many hardware resources can be used for arbitrary integer multiplication. The third part **[Profiling]** is our profiling results that is used for modeling the hardware resource utilization and performce prediction.

To generate the source code, users can execute the [python script](templete/tools/AIM.py). It will read the config file, generate the large integer multiplier, and report the estimated performance in 32bit multiplication GOPS.
> python tools/AIM.py

Before compiling the project, users need to ensure the following variables in the [Makefile](templete/Makefile) are set correctly. 

```Makefile
PLATFORM=/${YOUR_PATH}/xilinx_vck190_base_202110_1.xpfm
SYSROOT=/${YOUR_PATH}/2021.1/sysroots/cortexa72-cortexa53-xilinx-linux
EDGE_COMMON_SW=/${YOUR_PATH}/xilinx-versal-common-v2021.1
HLS_INCLUDE=/${YOUR_PATH}/Vitis_HLS/2021.1/include/
```

Then, users need to setup the environment using the following commands:
```bash
source /${YOUR_PATH}/Vitis/2021.1/settings64.sh  
source /${YOUR\_PATH}/xrt/setup.sh  
unset LD_LIBRARY_PATH  
source /${YOUR\_PATH}/environment-setup-cortexa72-cortexa53-xilinx-linux
```

To compile the project, uses can use the [**build.sh**](templete/tools/build.sh). If the target frequency cannot be achieved, it will check the worst negative slack and recompile the project with a decreased frequency.

