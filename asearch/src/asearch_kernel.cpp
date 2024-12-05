/*
* Based off of https://www.geeksforgeeks.org/a-search-algorithm/
*/

#include "asearch_kernel.h"
#include <stdio.h>
#include <vector>
#include <hls_stream.h>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <cfloat>

extern "C"
{
    #define ROW 9  // Define ROW size
    #define COL 10  // Define COL size

    struct Pair {
        int first, second;
    };

    struct cell {
        float f, g, h;
        int parent_i, parent_j;

        cell() : f(FLT_MAX), g(FLT_MAX), h(FLT_MAX), parent_i(-1), parent_j(-1) {}
    };

    enum result {
        PATH_NOT_FOUND,
        INVALID_SOURCE,
        INVALID_DESTINATION,
        PATH_IS_BLOCKED,
        ALREADY_AT_DESTINATION,
        FOUND_PATH
    };

    // bool isValid(int row, int col);
    // bool isUnBlocked(int grid[ROW][COL], int row, int col);
    // bool isDestination(int row, int col, Pair dest);
    // float calculateHValue(int row, int col, Pair dest);
    // bool checkF(cell cellDetails[ROW][COL], int row, int col, float newF);
    // void addPPair(hls::stream<Pair>& openList, Pair p);
    // void getNext(hls::stream<Pair>& openList, Pair* next);
    // bool checkForEmpty(hls::stream<Pair>& openList);

    void asearch(int gridIn[], Pair src, Pair dest, result* res, cell cellOut[ROW * COL]) {
    #pragma HLS DATAFLOW
        int grid[ROW][COL];

        // Copy input data to 2D grid array
        for (int i = 0; i < ROW; i++) {
            for (int j = 0; j < COL; j++) {
    #pragma HLS PIPELINE II=1
                grid[i][j] = gridIn[i * COL + j];
            }
        }

        // Initial checks
        result r = PATH_NOT_FOUND;

        if (!isValid(src.first, src.second)) {
            *res = INVALID_SOURCE;
            return;
        }

        if (!isValid(dest.first, dest.second)) {
            *res = INVALID_DESTINATION;
            return;
        }

        if (!isUnBlocked(grid, src.first, src.second) || !isUnBlocked(grid, dest.first, dest.second)) {
            *res = PATH_IS_BLOCKED;
            return;
        }

        if (isDestination(src.first, src.second, dest)) {
            *res = ALREADY_AT_DESTINATION;
            return;
        }

        bool closedList[ROW][COL];
    #pragma HLS ARRAY_PARTITION variable=closedList complete dim=1
        memset(closedList, false, sizeof(closedList));

        cell cellDetails[ROW][COL];
    #pragma HLS ARRAY_PARTITION variable=cellDetails complete dim=1

        for (int i = 0; i < ROW; i++) {
            for (int j = 0; j < COL; j++) {
    #pragma HLS UNROLL factor=2
                cellDetails[i][j] = cell();
            }
        }

        int i = src.first;
        int j = src.second;
        cellDetails[i][j].f = 0.0;
        cellDetails[i][j].g = 0.0;
        cellDetails[i][j].h = 0.0;
        cellDetails[i][j].parent_i = i;
        cellDetails[i][j].parent_j = j;

        hls::stream<Pair> openList;
        Pair current;

        addPPair(openList, {0, {i, j}});
        bool foundDest = false;

        // Main A* Search Loop
        while (!checkForEmpty(openList) && !foundDest) {
    #pragma HLS PIPELINE
            getNext(openList, &current);
            i = current.second.first;
            j = current.second.second;

            closedList[i][j] = true;

            // Neighbor checking logic (North, South, East, West, diagonals)
            for (int d = 0; d < 8; d++) {
    #pragma HLS UNROLL
                int newI = i + ((d < 4) ? (d == 0 ? -1 : (d == 1 ? 1 : 0)) : (d == 4 || d == 5 ? -1 : 1));
                int newJ = j + ((d < 4) ? (d == 2 ? 1 : (d == 3 ? -1 : 0)) : (d == 4 || d == 6 ? 1 : -1));

                if (isValid(newI, newJ)) {
                    if (isDestination(newI, newJ, dest)) {
                        cellDetails[newI][newJ].parent_i = i;
                        cellDetails[newI][newJ].parent_j = j;
                        foundDest = true;
                        break;
                    } else if (!closedList[newI][newJ] && isUnBlocked(grid, newI, newJ)) {
                        float newG = cellDetails[i][j].g + ((d < 4) ? 1.0f : 1.414f);
                        float newH = calculateHValue(newI, newJ, dest);
                        float newF = newG + newH;

                        if (checkF(cellDetails, newI, newJ, newF)) {
                            addPPair(openList, {newF, {newI, newJ}});
                            cellDetails[newI][newJ].f = newF;
                            cellDetails[newI][newJ].g = newG;
                            cellDetails[newI][newJ].h = newH;
                            cellDetails[newI][newJ].parent_i = i;
                            cellDetails[newI][newJ].parent_j = j;
                        }
                    }
                }
            }
        }

        // Copy results
        for (int x = 0; x < ROW; x++) {
            for (int y = 0; y < COL; y++) {
    #pragma HLS PIPELINE II=1
                cellOut[x * COL + y] = cellDetails[x][y];
            }
        }

        *res = foundDest ? FOUND_PATH : PATH_NOT_FOUND;
    }


}

bool isValid(int row, int col)
{
    return (row >= 0) &&
        (row < ROW) &&
        (col >= 0) &&
        (col < COL);
}

bool isUnBlocked(int grid[][COL], int row, int col)
{
    return grid[row][col] == 1;
}

bool isDestination(int row, int col, Pair dest)
{
    return row == dest.first && col == dest.second;
}

double calculateHValue(int row, int col, Pair dest)
{
    double xDiff = row - dest.first;
    double yDiff = col - dest.second;

    double sum = (xDiff * xDiff) + (yDiff * yDiff);

    return sqrt(sum);
}

void readGrid(const char* file, int grid[][COL])
{
    FILE* pFile = fopen(file, "r");

    int val;
    for (int i = 0; i < ROW; i++)
    {
        for (int j = 0; j < COL; j++)
        {
            fscanf(pFile, "%i", &val);
            grid[i][j] = val;
        }
    }

    fclose(pFile);
}

bool checkF(cell cellDetails[][COL], int i, int j, double f)
{
    return cellDetails[i][j].f == FLT_MAX || cellDetails[i][j].f > f;
}

void init(pPair* list)
{
    for (int i = 0; i < COL * ROW; i++)
    {
        list[i] = make_pair(-1, make_pair(-1, -1));
    }
}

bool checkForEmpty(pPair* list)
{
    bool empty = true;

    for (int i = 0; empty && i < COL * ROW; i++)
    {
        if (list[i].first != -1)
        {
            empty = false;
            break;
        }
    }

    return empty;
}

void getNext(pPair* list, int* index)
{
    pPair rtnVal = make_pair(-1, make_pair(-1, -1));
    *index = 0;

    for (int i = 0; i < COL * ROW; i++)
    {
        if (rtnVal.first == -1 || // If no valid value has been grabbed
            (rtnVal.first > list[i].first && list[i].first != -1)// Or the list's value is an easier distance
            )
        {
            *index = i;
            rtnVal = list[i];
        }
    }
}

void addPPair(pPair* list, const pPair& pair)
{
    for (int i = 0; i < COL * ROW; i++)
    {
        if (list[i].first == -1)
        {
            list[i] = pair;
            break;
        }
    }
}

void removePPair(pPair* list, int index)
{
    if (0 <= index && index < COL * ROW)
    {
        list[index] = make_pair(-1, make_pair(-1, -1));
    }
}