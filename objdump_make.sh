#!/bin/bash

shopt -s globstar
for f in ./sdk_build/**; do
	f2="${f/sdk_build/wrt_build}"
	arm-linux-gnueabi-objdump -d "$f" > "${f}.dis"
	arm-linux-gnueabi-objdump -d "$f2" > "${f2}.dis"
    	echo "$f"
done


