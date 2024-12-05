/*
* Based off of https://www.geeksforgeeks.org/a-search-algorithm/
*/

#include "asearch_kernel.h"
#include <stdio.h>
#include <hls_stream.h>
#include <ap_int.h>
#include <string.h>
#include <float.h>

extern "C"
{
    void asearch(int gridIn[], Pair src, Pair dest, result* res, cell cellOut[]) {
        const int directions[8][2] = {
            {-1, 0},  // North
            {1, 0},   // South
            {0, 1},   // East
            {0, -1},  // West
            {-1, 1},  // North-East
            {-1, -1}, // North-West
            {1, 1},   // South-East
            {1, -1}   // South-West
        };
        // Local grid copy
        int grid[ROW][COL];
    #pragma HLS ARRAY_PARTITION variable=grid dim=2 complete
        for (int x = 0; x < ROW; x++) {
    #pragma HLS PIPELINE
            for (int y = 0; y < COL; y++) {
                grid[x][y] = gridIn[x * COL + y];
            }
        }

        // Local variables
        result r = PATH_NOT_FOUND;

        // Validation checks
        if (!isValid(src.first, src.second)) {
            *res = INVALID_SOURCE;
            return;
        }

        if (!isValid(dest.first, dest.second)) {
            *res = INVALID_DESTINATION;
            return;
        }

        if (!isUnBlocked(grid, src.first, src.second) ||
            !isUnBlocked(grid, dest.first, dest.second)) {
            *res = PATH_IS_BLOCKED;
            return;
        }

        if (isDestination(src.first, src.second, dest)) {
            *res = ALREADY_AT_DESTINATION;
            return;
        }

        // Initialize closed list
        bool closedList[ROW][COL];
    #pragma HLS ARRAY_PARTITION variable=closedList dim=2 complete
        for (int x = 0; x < ROW; x++) {
    #pragma HLS UNROLL
            for (int y = 0; y < COL; y++) {
                closedList[x][y] = false;
            }
        }

        // Cell details
        cell cellDetails[ROW][COL];
    #pragma HLS ARRAY_PARTITION variable=cellDetails dim=2 complete
        for (int i = 0; i < ROW; i++) {
    #pragma HLS UNROLL
            for (int j = 0; j < COL; j++) {
                cellDetails[i][j].f = FLT_MAX;
                cellDetails[i][j].g = FLT_MAX;
                cellDetails[i][j].h = FLT_MAX;
                cellDetails[i][j].parent_i = -1;
                cellDetails[i][j].parent_j = -1;
            }
        }

        // Set source details
        int i = src.first, j = src.second;
        cellDetails[i][j].f = 0.0;
        cellDetails[i][j].g = 0.0;
        cellDetails[i][j].h = 0.0;
        cellDetails[i][j].parent_i = i;
        cellDetails[i][j].parent_j = j;

        // Priority queue for open list
        pPair openList[ROW * COL];
        int openListSize = 0;
    #pragma HLS ARRAY_PARTITION variable=openList dim=1 complete
        addPPair(openList, make_pair(0.0, make_pair(i, j)), &openListSize);

        bool foundDest = false;

        // Main loop
        while (openListSize > 0 && !foundDest) {
    #pragma HLS PIPELINE
            // Extract the node with minimum f value
            pPair p = openList[0];
            removePPair(openList, &openListSize);

            i = p.second.first;
            j = p.second.second;
            closedList[i][j] = true;

            // Neighbor exploration (8 directions)
            for (int d = 0; d < 8; d++) {
    #pragma HLS UNROLL
                int newI = i + directions[d][0];
                int newJ = j + directions[d][1];
                double stepCost = (d < 4) ? 1.0 : 1.414;  // Diagonal cost

                if (isValid(newI, newJ) && !closedList[newI][newJ] &&
                    isUnBlocked(grid, newI, newJ)) {
                    if (isDestination(newI, newJ, dest)) {
                        cellDetails[newI][newJ].parent_i = i;
                        cellDetails[newI][newJ].parent_j = j;
                        foundDest = true;
                        break;
                    }

                    double newG = cellDetails[i][j].g + stepCost;
                    double newH = calculateHValue(newI, newJ, dest);
                    double newF = newG + newH;

                    if (checkF(cellDetails, newI, newJ, newF)) {
                        addPPair(openList, make_pair(newF, make_pair(newI, newJ)), &openListSize);
                        cellDetails[newI][newJ].f = newF;
                        cellDetails[newI][newJ].g = newG;
                        cellDetails[newI][newJ].h = newH;
                        cellDetails[newI][newJ].parent_i = i;
                        cellDetails[newI][newJ].parent_j = j;
                    }
                }
            }
        }

        // Output results
        for (int x = 0; x < ROW; x++) {
    #pragma HLS PIPELINE
            for (int y = 0; y < COL; y++) {
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