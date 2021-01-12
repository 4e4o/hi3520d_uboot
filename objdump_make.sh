#!/bin/bash

shopt -s globstar
for f in ./wrt_build/**; do
	f2="${f/wrt_build/wrt_build_buggy}"
	arm-linux-gnueabi-objdump -d "$f" > "${f}.dis"
	arm-linux-gnueabi-objdump -d "$f2" > "${f2}.dis"
    	echo "$f"
done


