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
    void asearch(int gridIn[], Pair src, Pair dest, result* res, cell cellOut[])
    {
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

        int grid[ROW][COL];
        for (int x = 0; x < ROW; x++)
        {
            for (int y = 0; y < COL; y++)
            {
                grid[x][y] = gridIn[x * COL + y];
            }
        }

        result r = PATH_NOT_FOUND;

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

        bool closedList[ROW][COL] = {false};

        cell cellDetails[ROW][COL];
        for (int i = 0; i < ROW; i++)
        {
            for (int j = 0; j < COL; j++)
            {
                cellDetails[i][j] = { -1, -1, FLT_MAX, FLT_MAX, FLT_MAX };
            }
        }

        int i = src.first, j = src.second;
        cellDetails[i][j].f = 0.0;
        cellDetails[i][j].g = 0.0;
        cellDetails[i][j].h = 0.0;
        cellDetails[i][j].parent_i = i;
        cellDetails[i][j].parent_j = j;

        pPair openList[ROW * COL];
        for (int k = 0; k < ROW * COL; k++)
        {
            openList[k] = make_pair(-1, make_pair(-1, -1));
        }

        addPPair(openList, make_pair(0.0, make_pair(i, j)));
        bool foundDest = false;

        while (true)
        {
            int index = -1;
            for (int k = 0; k < ROW * COL; k++)
            {
                if (openList[k].first != -1)
                {
                    index = k;
                    break;
                }
            }

            if (index == -1) // Open list is empty
                break;

            pPair p = openList[index];
            removePPair(openList, index);

            i = p.second.first;
            j = p.second.second;
            closedList[i][j] = true;

            for (int d = 0; d < 8; d++)
            {
                int newI = i + directions[d][0];
                int newJ = j + directions[d][1];

                if (isValid(newI, newJ))
                {
                    if (isDestination(newI, newJ, dest))
                    {
                        cellDetails[newI][newJ].parent_i = i;
                        cellDetails[newI][newJ].parent_j = j;
                        foundDest = true;
                        break;
                    }
                    else if (!closedList[newI][newJ] && isUnBlocked(grid, newI, newJ))
                    {
                        double newG = cellDetails[i][j].g + ((d < 4) ? 1.0 : 1.414);
                        double newH = calculateHValue(newI, newJ, dest);
                        double newF = newG + newH;

                        if (newF < cellDetails[newI][newJ].f)
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

            if (foundDest)
                break;
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