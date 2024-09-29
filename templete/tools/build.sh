#!/bin/bash
make all TARGET=hw | tee make.log
python tools/checkfreq.py
while [[ $? -eq 1 ]]; do
    make all TARGET=hw | tee make.log
    python tools/checkfreq.py
done