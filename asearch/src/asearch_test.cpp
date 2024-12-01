/*
    Filename: fir_test.h
        FIR lab wirtten for WES/CSE237C class at UCSD.
        Testbench file
        Calls fir() function from fir.cpp
        Compares the output from fir() with out.gold.dat
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "asearch_kernel.h"
#include <iostream>
#include <fstream>
#include <string>

bool cmpLine(const string& str1, const string& str2);
void tracePath(result r, cell cellDetails[][COL], Pair dest);

int main()
{
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

    int gridIn[ROW * COL];
    Pair src = make_pair(8, 0);
    Pair dest = make_pair(0, 0);
    result r = result::PATH_NOT_FOUND;
    cell detailsOut[ROW * COL];

    for (int i = 0; i < ROW; i++)
    {
        for (int j = 0; j < COL; j++)
        {
            gridIn[i * COL + j] = grid[i][j];
            detailsOut[i * COL + j] = cell();
        }
    }

    std::cout << "Execution of the kernel" << std::endl;
    asearch(gridIn, src, dest, &r, detailsOut);

    cell details[ROW][COL];
    for (int i = 0; i < ROW; i++)
    {
        for (int j = 0; j < COL; j++)
        {
            details[i][j] = detailsOut[i * COL + j];
        }
    }

    tracePath(r, details, dest);

    std::cout << "Comparing observed against expected data" << std::endl;

    std::ifstream file_obs, file_exp;
    file_obs.open("out.dat");
    file_exp.open("out.gold.aStarSearch.dat");
    bool hasDataObs = file_obs.eof();
    bool hasDataExp = file_exp.eof();

    std::string obs, exp;
    for (unsigned int i = 1; hasDataObs || hasDataExp; i++)
    {
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

        std::getline(file_obs, obs);

        std::getline(file_exp, exp);

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

            output << "The Path is " << std::endl;

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
            for (int i = idx; i << idx >= 0; i--)
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