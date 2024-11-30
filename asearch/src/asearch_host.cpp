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

    Pair src = make_pair(8, 0);
    Pair dest = make_pair(0, 0);
    result r = result::PATH_NOT_FOUND;
    cell details[ROW][COL];

    std::cout << "Execution of the kernel" << std::endl;
    auto run = krnl(grid, src, dest, &r, details);
    run.wait();

    tracePath(r, details, dest);

    // Comparing results with the golden output.
std::cout << "Comparing observed against expected data" << std::endl;

    std::ifstream file_obs, file_exp;
    file_obs.open("out.dat");
    file_exp.open("out.gold.aStarSearch.dat");
    bool hasDataObs, hasDataExp;
    std::istream_iterator<std::string> iterObs(file_obs);
    std::istream_iterator<std::string> iterExp(file_exp);
    std::istream_iterator<std::string> end;
    do
    {
        hasDataExp = iterExp != end;
        hasDataObs = iterObs != end;
        if (hasDataExp && !hasDataObs) // They don't agree on number of line in output
        {
            std::cout << "*******************************************" << std::endl;
            std::cout << "FAIL: Output DOES NOT match the golden output" << std::endl;
            std::cout << "Expected has data and Observed doesn't" << std::endl;
            std::cout << "*******************************************" << std::endl;
            return 2;
        }

        if (!hasDataExp && hasDataObs) // They don't agree on number of line in output
        {
            std::cout << "*******************************************" << std::endl;
            std::cout << "FAIL: Output DOES NOT match the golden output" << std::endl;
            std::cout << "Expected has data and Observed doesn't" << std::endl;
            std::cout << "*******************************************" << std::endl;
            return 3;
        }

        if (!cmpLine(*iterObs, *iterExp)) // compare the data
        {
            std::cout << "*******************************************" << std::endl;
            std::cout << "FAIL: Output DOES NOT match the golden output" << std::endl;
            std::cout << "EXP: \"" << *iterExp << "\"" << std::endl;
            std::cout << "OBS: \"" << *iterObs << "\"" << std::endl;
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
    std::cout << "Result: " << r  << endl;
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
            idx++;
            for (int i = 0; i < idx; i++)
            {
                Pair p = Path[idx];
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
