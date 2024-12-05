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
    enum result {
        PATH_NOT_FOUND = -1,
        FOUND_PATH = 0,
        INVALID_SOURCE = 1,
        INVALID_DESTINATION = 2,
        PATH_IS_BLOCKED = 3,
        ALREADY_AT_DESTINATION = 4,
    };

    typedef ap_int<32> int32;
    typedef ap_fixed<32, 16> float32;

    typedef struct {
        int32 parent_i;
        int32 parent_j;
        float32 f, g, h;
    } cell_t;

    typedef struct {
        float32 f;
        int32 i, j;
    } pPair_t;

    // Optimized A* Search Function
    void asearch(int32 gridIn[], int32 src_i, int32 src_j, int32 dest_i, int32 dest_j, 
                 result* res, cell_t cellOut[], int ROW, int COL) {

    #pragma HLS INTERFACE m_axi depth=1024 port=gridIn bundle=gmem
    #pragma HLS INTERFACE m_axi depth=1024 port=cellOut bundle=gmem
    #pragma HLS INTERFACE s_axilite port=src_i
    #pragma HLS INTERFACE s_axilite port=src_j
    #pragma HLS INTERFACE s_axilite port=dest_i
    #pragma HLS INTERFACE s_axilite port=dest_j
    #pragma HLS INTERFACE s_axilite port=res
    #pragma HLS INTERFACE s_axilite port=ROW
    #pragma HLS INTERFACE s_axilite port=COL
    #pragma HLS INTERFACE s_axilite port=return

        // Local grid
        int32 grid[1024][1024];
    #pragma HLS ARRAY_PARTITION variable=grid cyclic factor=4 dim=2

        // Load grid into local memory
        for (int i = 0; i < ROW; i++) {
    #pragma HLS LOOP_TRIPCOUNT min=1 max=1024
            for (int j = 0; j < COL; j++) {
    #pragma HLS PIPELINE II=1
                grid[i][j] = gridIn[i * COL + j];
            }
        }

        // Check for invalid input or immediate termination cases
        if (src_i < 0 || src_i >= ROW || src_j < 0 || src_j >= COL ||
            dest_i < 0 || dest_i >= ROW || dest_j < 0 || dest_j >= COL) {
            *res = INVALID_SOURCE;
            return;
        }

        // A simple case when the source and destination are the same
        if (src_i == dest_i && src_j == dest_j) {
            *res = ALREADY_AT_DESTINATION;
            return;
        }

        // Initialization
        bool closedList[1024][1024] = {false};
    #pragma HLS ARRAY_PARTITION variable=closedList cyclic factor=4 dim=2

        cell_t cellDetails[1024][1024];
    #pragma HLS ARRAY_PARTITION variable=cellDetails cyclic factor=4 dim=2

        for (int i = 0; i < ROW; i++) {
    #pragma HLS LOOP_TRIPCOUNT min=1 max=1024
            for (int j = 0; j < COL; j++) {
    #pragma HLS PIPELINE II=1
                cellDetails[i][j].f = FLT_MAX;
                cellDetails[i][j].g = FLT_MAX;
                cellDetails[i][j].h = FLT_MAX;
                cellDetails[i][j].parent_i = -1;
                cellDetails[i][j].parent_j = -1;
            }
        }

        // Setting up the source
        int i = src_i, j = src_j;
        cellDetails[i][j].f = 0.0;
        cellDetails[i][j].g = 0.0;
        cellDetails[i][j].h = 0.0;
        cellDetails[i][j].parent_i = i;
        cellDetails[i][j].parent_j = j;

        // Main loop
        bool foundDest = false;
        while (!foundDest) {
            // Check neighbors
            for (int dir = 0; dir < 8; dir++) {
    #pragma HLS UNROLL
                int ni = i + (dir == 0 ? -1 : dir == 1 ? 1 : 0);
                int nj = j + (dir == 2 ? -1 : dir == 3 ? 1 : 0);
                if (ni >= 0 && ni < ROW && nj >= 0 && nj < COL &&
                    !closedList[ni][nj] && grid[ni][nj] != 0) {
                    float32 g = cellDetails[i][j].g + 1.0;
                    float32 h = calculateHValue(ni, nj, dest_i, dest_j);  // Implement heuristic function
                    float32 f = g + h;
                    if (f < cellDetails[ni][nj].f) {
                        cellDetails[ni][nj].f = f;
                        cellDetails[ni][nj].g = g;
                        cellDetails[ni][nj].h = h;
                        cellDetails[ni][nj].parent_i = i;
                        cellDetails[ni][nj].parent_j = j;

                        if (ni == dest_i && nj == dest_j) {
                            foundDest = true;
                            break;
                        }
                    }
                }
            }
            if (foundDest) break;
            closedList[i][j] = true;  // Mark current as closed
        }

        // Copy cellDetails back to output
        for (int i = 0; i < ROW; i++) {
    #pragma HLS LOOP_TRIPCOUNT min=1 max=1024
            for (int j = 0; j < COL; j++) {
    #pragma HLS PIPELINE II=1
                cellOut[i * COL + j] = cellDetails[i][j];
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