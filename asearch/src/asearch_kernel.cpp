/*
* Based off of https://www.geeksforgeeks.org/a-search-algorithm/
*/

#include "asearch_kernel.h"
#include <stdio.h>

extern "C"
{
    void asearch(int gridIn[], Pair src, Pair dest, result* res, cell cellOut[])
    {
        int grid[ROW][COL];
        for (int x = 0; x < ROW; x++)
        {
            for (int y = 0; y < COL; y++)
            {
                grid[x][y] = gridIn[x * COL + y];
            }
        }

        result r = PATH_NOT_FOUND;

        printf("Pre-Check\n");

        if (!isValid(src.first, src.second))
        {
            r = INVALID_SOURCE;
        }

        if (!isValid(dest.first, dest.second))
        {
            r = INVALID_DESTINATION;
        }

        if (!isUnBlocked(grid, src.first, src.second) ||
            !isUnBlocked(grid, dest.first, dest.second))
        {
            r = PATH_IS_BLOCKED;
        }

        if (isDestination(src.first, src.second, dest))
        {
            r = ALREADY_AT_DESTINATION;
        }

        printf("Doing initialization\n");
        printf("Grid:\n");
        for (int x = 0; x < ROW; x++)
        {
            for (int y = 0; y < COL; y++)
            {
                printf("%d ", grid[x][y]);
            }
            printf("\n");
        }

        // Create and populate closed list
        bool closedList[ROW][COL];
        memset(closedList, false, sizeof(closedList));

        int i, j, newI, newJ;

        cell cellDetails[ROW][COL];
        for (i = 0; i < ROW; i++)
        {
            for (j = 0; j < COL; j++)
            {
                cellDetails[i][j] = cell();
                cellDetails[i][j].f = FLT_MAX;
                cellDetails[i][j].g = FLT_MAX;
                cellDetails[i][j].h = FLT_MAX;
                cellDetails[i][j].parent_i = -1;
                cellDetails[i][j].parent_j = -1;
            }
        }

        printf("Selecting first node\n");
        // Set starting node
        i = src.first;
        j = src.second;
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

        printf("Loop Starting\n");
        while (!checkForEmpty(openList) && !foundDest)
        {
            getNext(openList, &index);
            pPair p = openList[index];

            removePPair(openList, index);

            i = p.second.first;
            j = p.second.second;
            closedList[i][j] = true;

            /*
            Cell-->Popped Cell (i,   j)
            N -->  North       (i-1, j)
            S -->  South       (i+1, j)
            E -->  East        (i,   j+1)
            W -->  West        (i,   j-1)
            N.E--> North-East  (i-1, j+1)
            N.W--> North-West  (i-1, j-1)
            S.E--> South-East  (i+1, j+1)
            S.W--> South-West  (i+1, j-1)
            */

            double newG, newH, newF;

            // Check North
            newI = i - 1;
            newJ = j;
            if (isValid(newI, newJ))
            {
                if (isDestination(newI, newJ, dest))
                {
                    cellDetails[newI][newJ].parent_i = i;
                    cellDetails[newI][newJ].parent_j = j;
                    foundDest = true;
                    break;
                }
                else if (!closedList[newI][newJ] &&
                    isUnBlocked(grid, newI, newJ))
                {
                    newG = cellDetails[i][j].g + 1.0;
                    newH = calculateHValue(newI, newJ, dest);
                    newF = newG + newH;

                    if (checkF(cellDetails, newI, newJ, newF))
                    {
                        addPPair(openList, make_pair(newF,
                            make_pair(newI, newJ)));

                        cellDetails[newI][newJ].f = newF;
                        cellDetails[newI][newJ].g = newG;
                        cellDetails[newI][newJ].h = newH;
                        cellDetails[newI][newJ].parent_i = i;
                        cellDetails[newI][newJ].parent_j = j;
                    }
                }
            }

            // Check South
            newI = i + 1;
            newJ = j;
            if (isValid(newI, newJ))
            {
                if (isDestination(newI, newJ, dest))
                {
                    cellDetails[newI][newJ].parent_i = i;
                    cellDetails[newI][newJ].parent_j = j;
                    foundDest = true;
                    break;
                }
                else if (!closedList[newI][newJ] &&
                    isUnBlocked(grid, newI, newJ))
                {
                    newG = cellDetails[newI][newJ].g + 1.0;
                    newH = calculateHValue(newI, newJ, dest);
                    newF = newG + newH;

                    if (checkF(cellDetails, newI, newJ, newF))
                    {
                        addPPair(openList, make_pair(newF,
                            make_pair(newI, newJ)));
                        cellDetails[newI][newJ].f = newF;
                        cellDetails[newI][newJ].g = newG;
                        cellDetails[newI][newJ].h = newH;
                        cellDetails[newI][newJ].parent_i = i;
                        cellDetails[newI][newJ].parent_j = j;
                    }
                }
            }

            // Check East
            newI = i;
            newJ = j + 1;
            if (isValid(newI, newJ))
            {
                if (isDestination(newI, newJ, dest))
                {
                    cellDetails[newI][newJ].parent_i = i;
                    cellDetails[newI][newJ].parent_j = j;
                    foundDest = true;
                    break;
                }
                else if (!closedList[newI][newJ] &&
                    isUnBlocked(grid, newI, newJ))
                {
                    newG = cellDetails[i][j].g + 1.0;
                    newH = calculateHValue(newI, newJ, dest);
                    newF = newG + newH;

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

            // Check West
            newI = i;
            newJ = j - 1;
            if (isValid(newI, newJ))
            {
                if (isDestination(newI, newJ, dest))
                {
                    cellDetails[newI][newJ].parent_i = i;
                    cellDetails[newI][newJ].parent_j = j;
                    foundDest = true;
                    break;
                }
                else if (!closedList[newI][newJ] &&
                    isUnBlocked(grid, newI, newJ))
                {
                    newG = cellDetails[i][j].g + 1.0;
                    newH = calculateHValue(newI, newJ, dest);
                    newF = newG + newH;

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

            // Check North-East
            newI = i - 1;
            newJ = j + 1;
            if (isValid(newI, newJ))
            {
                if (isDestination(newI, newJ, dest))
                {
                    cellDetails[newI][newJ].parent_i = i;
                    cellDetails[newI][newJ].parent_j = j;
                    foundDest = true;
                    break;
                }
                else if (!closedList[newI][newJ] &&
                    isUnBlocked(grid, newI, newJ))
                {
                    newG = cellDetails[newI][newJ].g + 1.414;
                    newH = calculateHValue(newI, newJ, dest);
                    newF = newG + newH;

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

            // Check North-West
            newI = i - 1;
            newJ = j - 1;
            if (isValid(newI, newJ))
            {
                if (isDestination(newI, newJ, dest))
                {
                    cellDetails[newI][newJ].parent_i = i;
                    cellDetails[newI][newJ].parent_j = j;
                    foundDest = true;
                    break;
                }
                else if (!closedList[newI][newJ] &&
                    isUnBlocked(grid, newI, newJ))
                {
                    newG = cellDetails[newI][newJ].g + 1.414;
                    newH = calculateHValue(newI, newJ, dest);
                    newF = newG + newH;

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

            // Check South-East
            newI = i + 1;
            newJ = j + 1;
            if (isValid(newI, newJ))
            {
                if (isDestination(newI, newJ, dest))
                {
                    cellDetails[newI][newJ].parent_i = i;
                    cellDetails[newI][newJ].parent_j = j;
                    foundDest = true;
                    break;
                }
                else if (!closedList[newI][newJ] &&
                    isUnBlocked(grid, newI, newJ))
                {
                    newG = cellDetails[newI][newJ].g + 1.414;
                    newH = calculateHValue(newI, newJ, dest);
                    newF = newG + newH;

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

            // Check South-West
            newI = i + 1;
            newJ = j - 1;
            if (isValid(newI, newJ))
            {
                if (isDestination(newI, newJ, dest))
                {
                    cellDetails[newI][newJ].parent_i = i;
                    cellDetails[newI][newJ].parent_j = j;
                    foundDest = true;
                    break;
                }
                else if (!closedList[newI][newJ] &&
                    isUnBlocked(grid, newI, newJ))
                {
                    newG = cellDetails[newI][newJ].g + 1.414;
                    newH = calculateHValue(newI, newJ, dest);
                    newF = newG + newH;

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
        printf("Loop Done\n");

        printf("Closed Loop\n");
        for (int x = 0; x < ROW; x++)
        {
            for (int y = 0; y < COL; y++)
            {
                printf("closedList[%d][%d] = %d\n", x, y, closedList[x][y]);
            }
        }

        for (int x = 0; x < ROW; x++)
        {
            for (int y = 0; y < COL; y++)
            {
                cellOut[x * COL + y] = cellDetails[x][y];
            }
        }

        r = foundDest ? FOUND_PATH : PATH_NOT_FOUND;
        *res = r;
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
