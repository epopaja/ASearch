#include "asearch_kernel.h"
#include <hls_math.h> // For hardware-compatible math functions

extern "C" {
    void asearch(int gridIn[ROW * COL], Pair src, Pair dest, result* res, cell cellOut[ROW * COL]) {
#pragma HLS INTERFACE m_axi port=gridIn offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=cellOut offset=slave bundle=gmem
#pragma HLS INTERFACE s_axilite port=src bundle=control
#pragma HLS INTERFACE s_axilite port=dest bundle=control
#pragma HLS INTERFACE s_axilite port=res bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

        // Local variables
        int grid[ROW][COL];
#pragma HLS ARRAY_PARTITION variable=grid complete dim=2
        cell cellDetails[ROW][COL];
#pragma HLS ARRAY_PARTITION variable=cellDetails complete dim=2
        bool closedList[ROW][COL];
#pragma HLS ARRAY_PARTITION variable=closedList complete dim=2

        // Initialize grid and cellDetails
        initGridAndDetails(grid, gridIn, cellDetails, closedList);

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

        // Open list
        pPair openList[ROW * COL];
#pragma HLS ARRAY_PARTITION variable=openList complete dim=1
        int index = 0;
        init(openList);

        int i = src.first, j = src.second;
        cellDetails[i][j] = {0.0, 0.0, 0.0, i, j};
        addPPair(openList, make_pair(0.0, make_pair(i, j)));

        bool foundDest = false;
        while (!checkForEmpty(openList) && !foundDest) {
#pragma HLS PIPELINE II=1
            getNext(openList, &index);
            pPair p = openList[index];
            removePPair(openList, index);

            i = p.second.first;
            j = p.second.second;
            closedList[i][j] = true;

            // Check all neighbors
            processNeighbors(grid, cellDetails, openList, closedList, dest, i, j, &foundDest);
        }

        *res = foundDest ? FOUND_PATH : PATH_NOT_FOUND;

        // Copy cell details to output
        copyCellDetails(cellDetails, cellOut);
    }

    void initGridAndDetails(int grid[ROW][COL], const int gridIn[ROW * COL], cell cellDetails[ROW][COL], bool closedList[ROW][COL]) {
#pragma HLS INLINE
        for (int x = 0; x < ROW; x++) {
            for (int y = 0; y < COL; y++) {
#pragma HLS UNROLL
                grid[x][y] = gridIn[x * COL + y];
                cellDetails[x][y] = {FLT_MAX, FLT_MAX, FLT_MAX, -1, -1};
                closedList[x][y] = false;
            }
        }
    }

    void processNeighbors(int grid[ROW][COL], cell cellDetails[ROW][COL], pPair openList[ROW * COL], bool closedList[ROW][COL], Pair dest, int i, int j, bool* foundDest) {
#pragma HLS INLINE
        static const int dRow[] = {-1, 1, 0, 0, -1, -1, 1, 1};
        static const int dCol[] = {0, 0, 1, -1, 1, -1, 1, -1};
        static const double cost[] = {1.0, 1.0, 1.0, 1.0, 1.414, 1.414, 1.414, 1.414};

        for (int dir = 0; dir < 8; dir++) {
#pragma HLS UNROLL
            int newI = i + dRow[dir];
            int newJ = j + dCol[dir];

            if (isValid(newI, newJ)) {
                if (isDestination(newI, newJ, dest)) {
                    cellDetails[newI][newJ].parent_i = i;
                    cellDetails[newI][newJ].parent_j = j;
                    *foundDest = true;
                    return;
                } else if (!closedList[newI][newJ] && isUnBlocked(grid, newI, newJ)) {
                    double newG = cellDetails[i][j].g + cost[dir];
                    double newH = calculateHValue(newI, newJ, dest);
                    double newF = newG + newH;

                    if (checkF(cellDetails, newI, newJ, newF)) {
                        addPPair(openList, make_pair(newF, make_pair(newI, newJ)));
                        cellDetails[newI][newJ] = {newF, newG, newH, i, j};
                    }
                }
            }
        }
    }

    void copyCellDetails(const cell cellDetails[ROW][COL], cell cellOut[ROW * COL]) {
#pragma HLS INLINE
        for (int x = 0; x < ROW; x++) {
            for (int y = 0; y < COL; y++) {
#pragma HLS PIPELINE
                cellOut[x * COL + y] = cellDetails[x][y];
            }
        }
    }
}
