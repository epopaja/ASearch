# General Setup
1. Log into build server
2. Run following commands
   - `source /tools/Xilinx/Vitis/2023.1/settings64.sh`
   - `source /opt/xilinx/xrt/setup.sh`
3. Navigate to the asearch folder in the repository
  
# Software Emulation
1. Build file by running `make all TARGET=sw_emu PLATFORM=/opt/xilinx/platforms/xilinx_u280_gen3x16_xdma_1_202211_1/xilinx_u280_gen3x16_xdma_1_202211_1.xpfm`.  
**Make sure the build succededs.**
2. Run `export XCL_EMULATION_MODE=sw_emu`
3. Run `./asearch_xrt -x build_dir.sw_emu.xilinx_u280_gen3x16_xdma_1_202211_1/asearch.xclbin`
4. Observe output for success message and check the out.dat file to make sure it matches the gold file.

# Hardware Emulation
1. Build file by running `make all TARGET=hw_emu PLATFORM=/opt/xilinx/platforms/xilinx_u280_gen3x16_xdma_1_202211_1/xilinx_u280_gen3x16_xdma_1_202211_1.xpfm`.  
**Make sure the build succededs.**
2. Run `export XCL_EMULATION_MODE=hw_emu`
3. Run `./asearch_xrt -x build_dir.hw_emu.xilinx_u280_gen3x16_xdma_1_202211_1/asearch.xclbin`
4. Observe output for success message and check the out.dat file to make sure it matches the gold file.


# Hardware
1. Run `tmux new -t build`
2. Run the steps in general setup.
3. Build file by running `make all TARGET=hw PLATFORM=/opt/xilinx/platforms/xilinx_u280_gen3x16_xdma_1_202211_1/xilinx_u280_gen3x16_xdma_1_202211_1.xpfm`.
4. Perform `ctrl + b, d` and wait ~2 hours for build to complete.
5. You can check to see if the process has completed by running the command `tmux attach`  
**Make sure the build succededs.**
6. **TO BE UPDATED**
