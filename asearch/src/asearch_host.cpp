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

#include "cmdlineparser.h"
#include "asearch_kernel.h"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdlib.h> // for system()
#include <string>
#include <iterator>

 // XRT includes
#include "experimental/xrt_bo.h"
#include "experimental/xrt_device.h"
#include "experimental/xrt_kernel.h"

#define DATA_SIZE 4096

bool cmpLine(const string& str1, const string& str2);

int main(int argc, char** argv)
{
    // Command Line Parser
    sda::utils::CmdLineParser parser;

    // Switches
    //**************//"<Full Arg>",  "<Short Arg>", "<Description>", "<Default>"
    parser.addSwitch("--xclbin_file", "-x", "input binary file string", "");
    parser.addSwitch("--device_id", "-d", "device index", "0");
    parser.parse(argc, argv);

    // Read settings
    std::string binaryFile = parser.value("xclbin_file");
    int device_index = stoi(parser.value("device_id"));

    if (argc < 3)
    {
        parser.printHelp();
        return EXIT_FAILURE;
    }

    std::cout << "Open the device" << device_index << std::endl;
    auto device = xrt::device(device_index);
    std::cout << "Load the xclbin " << binaryFile << std::endl;
    auto uuid = device.load_xclbin(binaryFile);

    auto krnl = xrt::kernel(device, uuid, "asearch");

    std::cout << "Allocate Small Buffer For Input in Device Memory\n";
    auto bo_out = xrt::bo(device, 128, krnl.group_id(0));

    // Map the contents of the buffer object into host memory
    auto bo_out_map = bo_out.map<int*>();

    int grid[COL][ROW];
    Pair src = make_pair(8, 0);
    Pair dest = make_pair(0, 0);
    result r = result::PATH_NOT_FOUND;
    cell details[ROW][COL];
    readGrid("input.dat", grid);

    std::cout << "Execution of the kernel\n";
    auto run = krnl(grid, src, dest, &r, details);
    run.wait();


    // Comparing results with the golden output.
    printf("Comparing observed against expected data \n");

    std::ifstream file_obs, file_exp;
    file_obs.open("out.dat");
    file_exp.open("out.gold.aStarSearch.dat");
    std::istream_iterator<std::string> iterObs(file_obs);
    std::istream_iterator<std::string> iterExp(file_exp);
    std::istream_iterator<std::string> end;
    do
    {
        if (iterObs == end ||
            iterExp == end)
        {
            std::cout << "*******************************************" << std::endl;
            std::cout << "FAIL: Output DOES NOT match the golden output" << std::endl;
            std::cout << "*******************************************" << std::endl;
            return 1;
        }

        if (!cmpLine(*iterObs, *iterExp)) // compare the data
        {
            std::cout << "*******************************************" << std::endl;
            std::cout << "FAIL: Output DOES NOT match the golden output" << std::endl;
            std::cout << "*******************************************" << std::endl;
            return 1;
        }

        // Move to next line
        iterObs++;
        iterExp++;

    } while (hasDataObs && hasDataExp);

    std::cout << "*******************************************" << std::endl;
    std::cout << "PASS: The output matches the golden output" << std::endl;
    std::cout << "*******************************************" << std::endl;

    return 0;
}

bool cmpLine(const string& str1, const string& str2)
{
    bool match = true;
    int i;

    for (i = 0; match && i < str1.length() && i < str2.length(); i++)
    {
        match &= str1.at(i) == str2.at(i);
    }

    if (match)
    {
        match = str1.length() == str2.length();
    }

    return match;
}