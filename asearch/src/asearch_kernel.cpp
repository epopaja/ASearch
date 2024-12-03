/*
* Based off of https://www.geeksforgeeks.org/a-search-algorithm/
*/

#include "asearch_kernel.h"
#include <stdio.h>

extern "C"
{
    void asearch(int gridIn[], Pair src, Pair dest, result* res, cell cellOut[])
    {
        #pragma HLS INTERFACE m_axi port=gridIn offset=slave bundle=gmem
        #pragma HLS INTERFACE m_axi port=cellOut offset=slave bundle=gmem
        #pragma HLS INTERFACE s_axilite port=src bundle=control
        #pragma HLS INTERFACE s_axilite port=dest bundle=control
        #pragma HLS INTERFACE s_axilite port=res bundle=control
        #pragma HLS INTERFACE s_axilite port=return bundle=control

        int grid[ROW][COL];
        #pragma HLS ARRAY_PARTITION variable=grid dim=2 complete

        // Flatten input grid for easier data loading
        for (int x = 0; x < ROW; x++) {
            #pragma HLS UNROLL factor=2
            for (int y = 0; y < COL; y++) {
                #pragma HLS PIPELINE II=1
                grid[x][y] = gridIn[x * COL + y];
            }
        }

        result r = PATH_NOT_FOUND;

        if (!isValid(src.first, src.second)) {
            r = INVALID_SOURCE;
        }

        if (!isValid(dest.first, dest.second)) {
            r = INVALID_DESTINATION;
        }

        if (!isUnBlocked(grid, src.first, src.second) ||
            !isUnBlocked(grid, dest.first, dest.second)) {
            r = PATH_IS_BLOCKED;
        }

        if (isDestination(src.first, src.second, dest)) {
            r = ALREADY_AT_DESTINATION;
        }

        bool closedList[ROW][COL];
        #pragma HLS ARRAY_PARTITION variable=closedList dim=2 complete

        cell cellDetails[ROW][COL];
        #pragma HLS ARRAY_PARTITION variable=cellDetails dim=2 complete

        for (int i = 0; i < ROW; i++) {
            #pragma HLS UNROLL factor=2
            for (int j = 0; j < COL; j++) {
                #pragma HLS PIPELINE II=1
                closedList[i][j] = false;
                cellDetails[i][j] = cell();
                cellDetails[i][j].f = FLT_MAX;
                cellDetails[i][j].g = FLT_MAX;
                cellDetails[i][j].h = FLT_MAX;
                cellDetails[i][j].parent_i = -1;
                cellDetails[i][j].parent_j = -1;
            }
        }

        // Initialize start node
        int i = src.first, j = src.second;
        cellDetails[i][j].f = 0.0;
        cellDetails[i][j].g = 0.0;
        cellDetails[i][j].h = 0.0;
        cellDetails[i][j].parent_i = i;
        cellDetails[i][j].parent_j = j;

        pPair openList[ROW * COL];
        #pragma HLS ARRAY_PARTITION variable=openList dim=1 complete

        init(openList);
        addPPair(openList, make_pair(0.0, make_pair(i, j)));
        bool foundDest = false;

        while (!checkForEmpty(openList) && !foundDest) {
            int index;
            getNext(openList, &index);
            pPair p = openList[index];
            removePPair(openList, index);

            i = p.second.first;
            j = p.second.second;
            closedList[i][j] = true;

            for (int dir = 0; dir < 8; dir++) {
                #pragma HLS UNROLL factor=2
                #pragma HLS PIPELINE II=1
                int newI = i + dirOffsets[dir][0];
                int newJ = j + dirOffsets[dir][1];

                if (isValid(newI, newJ)) {
                    if (isDestination(newI, newJ, dest)) {
                        cellDetails[newI][newJ].parent_i = i;
                        cellDetails[newI][newJ].parent_j = j;
                        foundDest = true;
                        break;
                    } else if (!closedList[newI][newJ] &&
                               isUnBlocked(grid, newI, newJ)) {
                        double newG = cellDetails[i][j].g + ((dir < 4) ? 1.0 : 1.414);
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

        for (int x = 0; x < ROW; x++) {
            #pragma HLS UNROLL factor=2
            for (int y = 0; y < COL; y++) {
                #pragma HLS PIPELINE II=1
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