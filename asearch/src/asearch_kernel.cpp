#include "asearch_kernel.h"
#include <stdio.h>
#include <hls_math.h> // Use HLS math functions for better synthesis compatibility

extern "C" {
    void asearch(int gridIn[], Pair src, Pair dest, result* res, cell cellOut[]) {
        // Copy grid input to local grid array
        int grid[ROW][COL];
        #pragma HLS ARRAY_PARTITION variable=grid complete dim=2
        for (int x = 0; x < ROW; x++) {
            #pragma HLS UNROLL factor=4
            for (int y = 0; y < COL; y++) {
                grid[x][y] = gridIn[x * COL + y];
            }
        }

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

        // Closed list and cell details initialization
        bool closedList[ROW][COL];
        cell cellDetails[ROW][COL];
        #pragma HLS ARRAY_PARTITION variable=closedList complete dim=2
        #pragma HLS ARRAY_PARTITION variable=cellDetails complete dim=2

        for (int i = 0; i < ROW; i++) {
            #pragma HLS UNROLL factor=4
            for (int j = 0; j < COL; j++) {
                closedList[i][j] = false;
                cellDetails[i][j].f = FLT_MAX;
                cellDetails[i][j].g = FLT_MAX;
                cellDetails[i][j].h = FLT_MAX;
                cellDetails[i][j].parent_i = -1;
                cellDetails[i][j].parent_j = -1;
            }
        }

        // Set up source node
        int i = src.first, j = src.second;
        cellDetails[i][j].f = 0.0;
        cellDetails[i][j].g = 0.0;
        cellDetails[i][j].h = 0.0;
        cellDetails[i][j].parent_i = i;
        cellDetails[i][j].parent_j = j;

        // Priority queue simulation
        pPair openList[ROW * COL];
        #pragma HLS ARRAY_PARTITION variable=openList complete
        init(openList);
        addPPair(openList, make_pair(0.0, make_pair(i, j)));

        bool foundDest = false;

        // Main A* Search loop
        while (!checkForEmpty(openList) && !foundDest) {
            getNext(openList, &index);
            pPair p = openList[index];
            removePPair(openList, index);

            i = p.second.first;
            j = p.second.second;
            closedList[i][j] = true;

            // Direction arrays for 8 possible moves
            const int dirI[8] = {-1, +1,  0,  0, -1, -1, +1, +1};
            const int dirJ[8] = { 0,  0, +1, -1, +1, -1, +1, -1};

            for (int dir = 0; dir < 8; dir++) {
                #pragma HLS UNROLL factor=2
                int newI = i + dirI[dir];
                int newJ = j + dirJ[dir];

                if (isValid(newI, newJ)) {
                    if (isDestination(newI, newJ, dest)) {
                        cellDetails[newI][newJ].parent_i = i;
                        cellDetails[newI][newJ].parent_j = j;
                        foundDest = true;
                        break;
                    }
                    else if (!closedList[newI][newJ] && isUnBlocked(grid, newI, newJ)) {
                        double stepCost = (dir >= 4) ? 1.414 : 1.0; // Diagonal cost
                        double newG = cellDetails[i][j].g + stepCost;
                        double newH = calculateHValue(newI, newJ, dest);
                        double newF = newG + newH;

                        if (checkF(cellDetails, newI, newJ, newF)) {
                            addPPair(openList, make_pair(newF, make_pair(newI, newJ)));
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

        // Output results
        for (int x = 0; x < ROW; x++) {
            #pragma HLS UNROLL factor=4
            for (int y = 0; y < COL; y++) {
                cellOut[x * COL + y] = cellDetails[x][y];
            }
        }

        *res = foundDest ? FOUND_PATH : PATH_NOT_FOUND;
    }
}
