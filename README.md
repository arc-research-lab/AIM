# AIM_ICCAD23

Title: **AIM**: Accelerating **A**rbitrary-precision **I**nteger **M**ultiplication on Heterogeneous Reconfigurable Computing Platform Versal ACAP

## Overview
In this repo, we propose AIM architecture and demonstrate the effectiveness of the heterogeneous platform (FPGA + vector units) for arbitrary-precision integer multiplication. The decomposed multiplications are assigned to the AIE array (vector units), which has high computating power, and the carry propogation is offloaded to the programmable logic to take advantage of its fine-grained traits. 
![image](./images/versal_arc.png)

We carry out experiments on the proposed AIM architecture and compare it with SOTA CPU (Intel Xeon 6346) and GPU (Nvidia A5000). [GMP](https://gmplib.org/) and [CGBN](https://github.com/NVlabs/CGBN) are used for CPU and GPU in the experiments. The results show AIM acheives up to 12.63x and 2.13x energy efficiency gains over CPU and GPU respectively.

We also demonstrate the applicability of AIM architecture on three applications. Please find [LIM](./application/IntegerMultiplication/), [RSA](./application/RSA/), and [Mandelbrot](./application/Mandelbrot/) in this repo.
