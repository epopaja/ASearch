#include "asearch_kernel.h"
#include <stdio.h>
#include <hls_math.h>
#include <hls_stream.h>

extern "C"
{
    void asearch(int gridIn[ROW * COL], Pair src, Pair dest, result* res, cell cellOut[ROW * COL])
    {
        #pragma HLS INTERFACE m_axi port=gridIn bundle=gmem
        #pragma HLS INTERFACE m_axi port=cellOut bundle=gmem
        #pragma HLS INTERFACE s_axilite port=src bundle=control
        #pragma HLS INTERFACE s_axilite port=dest bundle=control
        #pragma HLS INTERFACE s_axilite port=res bundle=control
        #pragma HLS INTERFACE s_axilite port=return bundle=control

        // Convert 1D input grid to 2D grid for easier processing
        int grid[ROW][COL];
        #pragma HLS ARRAY_PARTITION variable=grid complete dim=2
        for (int x = 0; x < ROW; x++)
        {
            #pragma HLS PIPELINE
            for (int y = 0; y < COL; y++)
            {
                grid[x][y] = gridIn[x * COL + y];
            }
        }

        // Initialize result to PATH_NOT_FOUND
        result r = PATH_NOT_FOUND;

        if (!isValid(src.first, src.second) || !isValid(dest.first, dest.second))
        {
            *res = INVALID_SOURCE;
            return;
        }

        if (!isUnBlocked(grid, src.first, src.second) || !isUnBlocked(grid, dest.first, dest.second))
        {
            *res = PATH_IS_BLOCKED;
            return;
        }

        if (isDestination(src.first, src.second, dest))
        {
            *res = ALREADY_AT_DESTINATION;
            return;
        }

        // Initialize closed list
        bool closedList[ROW][COL];
        #pragma HLS ARRAY_PARTITION variable=closedList complete dim=2
        for (int i = 0; i < ROW; i++)
        {
            #pragma HLS UNROLL
            for (int j = 0; j < COL; j++)
            {
                closedList[i][j] = false;
            }
        }

        // Initialize cell details
        cell cellDetails[ROW][COL];
        for (int i = 0; i < ROW; i++)
        {
            #pragma HLS UNROLL
            for (int j = 0; j < COL; j++)
            {
                #pragma HLS PIPELINE
                cellDetails[i][j].f = FLT_MAX;
                cellDetails[i][j].g = FLT_MAX;
                cellDetails[i][j].h = FLT_MAX;
                cellDetails[i][j].parent_i = -1;
                cellDetails[i][j].parent_j = -1;
            }
        }

        // Initialize source node
        int i = src.first, j = src.second;
        cellDetails[i][j].f = 0.0;
        cellDetails[i][j].g = 0.0;
        cellDetails[i][j].h = 0.0;
        cellDetails[i][j].parent_i = i;
        cellDetails[i][j].parent_j = j;

        // Initialize open list
        hls::stream<pPair> openList;
        pPair initial = make_pair(0.0, make_pair(i, j));
        openList.write(initial);
        bool foundDest = false;

        // Main A* loop
        while (!openList.empty() && !foundDest)
        {
            #pragma HLS PIPELINE II=1
            pPair current = openList.read();
            i = current.second.first;
            j = current.second.second;

            if (closedList[i][j])
                continue;
            closedList[i][j] = true;

            int dirI[8] = {-1, +1,  0,  0, -1, -1, +1, +1}; // Row changes
            int dirJ[8] = { 0,  0, +1, -1, +1, -1, +1, -1}; // Column changes

            // Process all neighbors
            for (int k = 0; k < 8; k++)
            {
                int newI = i + dirI[k];
                int newJ = j + dirJ[k];

                if (isValid(newI, newJ) && !closedList[newI][newJ] && isUnBlocked(grid, newI, newJ))
                {
                    if (isDestination(newI, newJ, dest))
                    {
                        cellDetails[newI][newJ].parent_i = i;
                        cellDetails[newI][newJ].parent_j = j;
                        foundDest = true;
                        break;
                    }

                    double newG = cellDetails[i][j].g + ((k < 4) ? 1.0 : 1.414);
                    double newH = calculateHValue(newI, newJ, dest);
                    double newF = newG + newH;

                    if (cellDetails[newI][newJ].f > newF)
                    {
                        cellDetails[newI][newJ].f = newF;
                        cellDetails[newI][newJ].g = newG;
                        cellDetails[newI][newJ].h = newH;
                        cellDetails[newI][newJ].parent_i = i;
                        cellDetails[newI][newJ].parent_j = j;
                        openList.write(make_pair(newF, make_pair(newI, newJ)));
                    }
                }
            }
        }

        // Set result and output cell details
        for (int x = 0; x < ROW; x++)
        {
            #pragma HLS PIPELINE
            for (int y = 0; y < COL; y++)
            {
                cellOut[x * COL + y] = cellDetails[x][y];
            }
        }

        *res = foundDest ? FOUND_PATH : PATH_NOT_FOUND;
    }
}
