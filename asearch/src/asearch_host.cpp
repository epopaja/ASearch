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

bool cmpLine(const string& str1, const string& str2);
void tracePath(result r, cell cellDetails[][COL], Pair dest);

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

    int grid[ROW][COL] =
    {
        {1,0,1,1,1,1,0,1,1,1},
        {1,1,1,0,1,1,1,0,1,1},
        {1,1,1,0,1,1,0,1,0,1},
        {0,0,1,0,1,0,0,0,0,1},
        {1,1,1,0,1,1,1,0,1,0},
        {1,0,1,1,1,1,0,1,0,0},
        {1,0,1,1,1,1,0,1,1,1},
        {1,1,1,0,0,0,1,0,0,1}
    };
    auto gridIn = xrt::bo(device, ROW * COL * sizeof(int), krnl.group_id(0));
    auto resultOut = xrt::bo(device, sizeof(result), krnl.group_id(3));
    auto detailsOut = xrt::bo(device, ROW * COL * sizeof(cell), krnl.group_id(4));

    auto gridIn_map = gridIn.map<int*>();
    auto detailsOut_map = detailsOut.map<cell*>();
    Pair src = make_pair(8, 0);
    Pair dest = make_pair(0, 0);
    result* r = resultOut.map<result*>();
    *r = result::PATH_NOT_FOUND;
    for (int i = 0; i < ROW; i++)
    {
        for (int j = 0; j < COL; j++)
        {
            gridIn_map[i * COL + j] = grid[i][j];
            detailsOut_map[i * COL + j] = cell();
        }
    }

    std::cout << "Synchronize Data In" << std::endl;
    gridIn.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    resultOut.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    detailsOut.sync(XCL_BO_SYNC_BO_TO_DEVICE);

    std::cout << "Execution of the kernel" << std::endl;
    auto run = krnl(gridIn, src, dest, resultOut, detailsOut);
    run.wait();

    std::cout << "Synchronize Data Out" << std::endl;
    resultOut.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    detailsOut.sync(XCL_BO_SYNC_BO_FROM_DEVICE);

    // Transpose to multi dimension array
    cell details[ROW][COL];
    for (int i = 0; i < ROW; i++)
    {
        for (int j = 0; j < COL; j++)
        {
            details[i][j] = detailsOut_map[i * COL + j];
        }
    }

    tracePath(*r, details, dest);

    // Comparing results with the golden output.
    std::cout << "Comparing observed against expected data" << std::endl;

    std::ifstream file_obs, file_exp;
    file_obs.open("out.dat");
    file_exp.open("out.gold.aStarSearch.dat");
    bool hasDataObs = true, hasDataExp = true;

    for (unsigned int i = 1; hasDataObs && hasDataExp; i++)
    {
        std::string obs, exp;
        std::getline(file_obs, obs);

        std::getline(file_exp, exp);

        if (hasDataExp && !hasDataObs) // They don't agree on number of line in output
        {
            std::cout << "*******************************************" << std::endl;
            std::cout << "FAIL: Output DOES NOT match the golden output" << std::endl;
            std::cout << "Expected has data and Observed doesn't @ " << i << std::endl;
            std::cout << "*******************************************" << std::endl;
            return 2;
        }

        if (!hasDataExp && hasDataObs) // They don't agree on number of line in output
        {
            std::cout << "*******************************************" << std::endl;
            std::cout << "FAIL: Output DOES NOT match the golden output" << std::endl;
            std::cout << "Observed has data and Expected doesn't @ " << i << std::endl;
            std::cout << "*******************************************" << std::endl;
            return 3;
        }

        if (!cmpLine(obs, exp)) // compare the data
        {
            std::cout << "*******************************************" << std::endl;
            std::cout << "FAIL: Output DOES NOT match the golden output" << std::endl;
            std::cout << "EXP: \"" << exp << "\"" << std::endl;
            std::cout << "OBS: \"" << obs << "\"" << std::endl;
            std::cout << "*******************************************" << std::endl;
            return 1;
        }

        hasDataObs = file_obs.eof();
        hasDataExp = file_exp.eof();
    } while (hasDataObs && hasDataExp);

    std::cout << "*******************************************" << std::endl;
    std::cout << "PASS: The output matches the golden output" << std::endl;
    std::cout << "*******************************************" << std::endl;

    return 0;
}

bool cmpLine(const string& str1, const string& str2)
{
    bool match = true;

    for (std::size_t i = 0; match && i < str1.length() && i < str2.length(); i++)
    {
        match &= str1.at(i) == str2.at(i);
    }

    if (match)
    {
        match = str1.length() == str2.length();
    }

    return match;
}

void tracePath(result r, cell cellDetails[][COL], Pair dest)
{
    std::cout << "Result: " << r << endl;
    //    std::cout << "Dest: " << dest << endl;
    std::ofstream output;
    int row = dest.first;
    int col = dest.second;

    output.open("out.dat", std::ofstream::trunc);
    Pair Path[ROW * COL];

    switch (r)
    {
        case FOUND_PATH:
        {
            output << "The destination cell is found" << std::endl << std::endl;

            output << "The path is " << std::endl;

            int idx = 0;
            while (!(cellDetails[row][col].parent_i == row &&
                cellDetails[row][col].parent_j == col))
            {
                Path[idx] = make_pair(row, col);
                idx++;
                int tempRow = cellDetails[row][col].parent_i;
                int tempCol = cellDetails[row][col].parent_j;
                row = tempRow;
                col = tempCol;
            }


            Path[idx] = make_pair(row, col);
            for (int i = idx; i >= 0; i--)
            {
                Pair p = Path[i];
                output << "(" << p.first << "," << p.second << ")" << endl;
            }
        }
        break;

        case INVALID_SOURCE:
            output << "Source is invalid";
            break;

        case INVALID_DESTINATION:
            output << "Destination is invalid";
            break;

        case PATH_IS_BLOCKED:
            output << "Source or the destination is blocked";
            break;

        case ALREADY_AT_DESTINATION:
            output << "Already at destination";
            break;

        case PATH_NOT_FOUND:
            output << "Path was not found";
            break;
    }

    output.flush();
    output.close();
}
