#include "asearch_kernel.h"
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>

extern "C"
{
    void asearch(int gridIn[], Pair src, Pair dest, result* res, cell cellOut[])
    {
        int grid[ROW][COL];

        // Unroll the loop for grid initialization
        for (int x = 0; x < ROW; x++)
        {
            for (int y = 0; y < COL; y += 4) // Process 4 elements per iteration
            {
                grid[x][y] = gridIn[x * COL + y];
                if (y + 1 < COL) grid[x][y + 1] = gridIn[x * COL + y + 1];
                if (y + 2 < COL) grid[x][y + 2] = gridIn[x * COL + y + 2];
                if (y + 3 < COL) grid[x][y + 3] = gridIn[x * COL + y + 3];
            }
        }

        result r = PATH_NOT_FOUND;

        // Early checks
        if (!isValid(src.first, src.second))
        {
            *res = INVALID_SOURCE;
            return;
        }

        if (!isValid(dest.first, dest.second))
        {
            *res = INVALID_DESTINATION;
            return;
        }

        if (!isUnBlocked(grid, src.first, src.second) ||
            !isUnBlocked(grid, dest.first, dest.second))
        {
            *res = PATH_IS_BLOCKED;
            return;
        }

        if (isDestination(src.first, src.second, dest))
        {
            *res = ALREADY_AT_DESTINATION;
            return;
        }

        bool closedList[ROW][COL];
        memset(closedList, false, sizeof(closedList));

        cell cellDetails[ROW][COL];

        // Initialize cell details with loop unrolling
        for (int i = 0; i < ROW; i++)
        {
            for (int j = 0; j < COL; j += 2) // Unrolling by factor of 2
            {
                cellDetails[i][j] = cell();
                if (j + 1 < COL)
                    cellDetails[i][j + 1] = cell();
            }
        }

        int i = src.first, j = src.second;
        cellDetails[i][j].f = 0.0;
        cellDetails[i][j].g = 0.0;
        cellDetails[i][j].h = 0.0;
        cellDetails[i][j].parent_i = i;
        cellDetails[i][j].parent_j = j;

        pPair openList[ROW * COL];
        int index = 0;
        init(openList);
        addPPair(openList, make_pair(0.0, make_pair(i, j)));

        bool foundDest = false;

        // Main loop
        while (!checkForEmpty(openList) && !foundDest)
        {
            getNext(openList, &index);
            pPair p = openList[index];
            removePPair(openList, index);

            i = p.second.first;
            j = p.second.second;
            closedList[i][j] = true;

            // Check all possible movements (8 directions)
            const int dRow[] = {-1, 1, 0, 0, -1, -1, 1, 1};
            const int dCol[] = {0, 0, 1, -1, 1, -1, 1, -1};
            const double cost[] = {1.0, 1.0, 1.0, 1.0, 1.414, 1.414, 1.414, 1.414};

            for (int dir = 0; dir < 8; dir += 2) // Unroll loop by factor of 2
            {
                int newI = i + dRow[dir];
                int newJ = j + dCol[dir];
                if (isValid(newI, newJ) && !closedList[newI][newJ] &&
                    isUnBlocked(grid, newI, newJ))
                {
                    double newG = cellDetails[i][j].g + cost[dir];
                    double newH = calculateHValue(newI, newJ, dest);
                    double newF = newG + newH;

                    if (checkF(cellDetails, newI, newJ, newF))
                    {
                        addPPair(openList, make_pair(newF, make_pair(newI, newJ)));
                        cellDetails[newI][newJ].f = newF;
                        cellDetails[newI][newJ].g = newG;
                        cellDetails[newI][newJ].h = newH;
                        cellDetails[newI][newJ].parent_i = i;
                        cellDetails[newI][newJ].parent_j = j;
                    }
                }

                // Unrolled second direction
                if (dir + 1 < 8)
                {
                    newI = i + dRow[dir + 1];
                    newJ = j + dCol[dir + 1];
                    if (isValid(newI, newJ) && !closedList[newI][newJ] &&
                        isUnBlocked(grid, newI, newJ))
                    {
                        double newG = cellDetails[i][j].g + cost[dir + 1];
                        double newH = calculateHValue(newI, newJ, dest);
                        double newF = newG + newH;

                        if (checkF(cellDetails, newI, newJ, newF))
                        {
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

            if (isDestination(i, j, dest))
            {
                foundDest = true;
                break;
            }
        }

        for (int x = 0; x < ROW; x++)
        {
            for (int y = 0; y < COL; y++)
            {
                cellOut[x * COL + y] = cellDetails[x][y];
            }
        }

        *res = foundDest ? FOUND_PATH : PATH_NOT_FOUND;
    }
}
