/*
* Based off of https://www.geeksforgeeks.org/a-search-algorithm/
*/

#include "asearch_kernel.h"
#include <stdio.h>

extern "C"
{
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

    void tracePath(result r, cell cellDetails[][COL], Pair dest)
    {
        FILE* pFile;
        int row = dest.first;
        int col = dest.second;

        stack<Pair> Path;

        pFile = fopen("output.dat", "w");

        switch (r)
        {
            case FOUND_PATH:
            {
                fprintf(pFile, "The destination cell is found\r\n\r\n");

                fprintf(pFile, "The path is \r\n");

                while (!(cellDetails[row][col].parent_i == row &&
                    cellDetails[row][col].parent_j == col))
                {
                    Path.push(make_pair(row, col));
                    int tempRow = cellDetails[row][col].parent_i;
                    int tempCol = cellDetails[row][col].parent_j;
                    row = tempRow;
                    col = tempCol;
                }

                Path.push(make_pair(row, col));
                while (!Path.empty())
                {
                    pair<int, int> p = Path.top();
                    Path.pop();
                    fprintf(pFile, "(%d,%d)\r\n", p.first, p.second);
                }
            }
            break;

            case INVALID_SOURCE:
                fprintf(pFile, "Source is invalid");
                break;

            case INVALID_DESTINATION:
                fprintf(pFile, "Destination is invalid");
                break;

            case PATH_IS_BLOCKED:
                fprintf(pFile, "Source or the destination is blocked");
                break;

            case ALREADY_AT_DESTINATION:
                fprintf(pFile, "Already at destination");
                break;
        }

        fclose(pFile);
    }

    void readGrid(const char* file, int grid[ROW][COl])
    {
        FILE* pFile = fopen(file, "r");

        int val;
        for (int i = 0; i < ROW; i++)
        {
            for (int j = 0; j < COL; j++)
            {
                fscanf(pFile, "%i", &val);
                rtnVal[i][j] = val
            }
        }

        fclose(pFile);
    }

    void aStarSearch(int grid[][COL], Pair src, Pair dest)
    {
        result r = PATH_NOT_FOUND;

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

        // Create and populate closed list
        bool closedList[ROW][COL];
        memset(closedList, false, sizeof(closedList));

        cell cellDetails[ROW][COL];
        int i, j, newI, newJ;

        for (i = 0; i < ROW; i++)
        {
            for (j = 0; j < COL; j++)
            {
                cellDetails[i][j].f = FLT_MAX;
                cellDetails[i][j].g = FLT_MAX;
                cellDetails[i][j].h = FLT_MAX;
                cellDetails[i][j].parent_i = -1;
                cellDetails[i][j].parent_j = -1;
            }
        }

        // Set starting node
        i = src.first;
        j = src.second;
        cellDetails[i][j].f = 0.0;
        cellDetails[i][j].g = 0.0;
        cellDetails[i][j].h = 0.0;
        cellDetails[i][j].parent_i = i;
        cellDetails[i][j].parent_j = j;

        set<pPair> openList;

        openList.insert(make_pair(0.0, make_pair(i, j)));
        bool foundDest = false;

        while (!openList.empty() && !foundDest)
        {
            pPair p = *openList.begin();

            openList.erase(openList.begin());

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
                        openList.insert(make_pair(newF,
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
                        openList.insert(make_pair(newF,
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
                        openList.insert(make_pair(newF, make_pair(newI, newJ)));

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
                        openList.insert(make_pair(newF, make_pair(newI, newJ)));

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
                        cellDetails[newI][newJ].f = newF;
                        cellDetails[newI][newJ].g = newG;
                        cellDetails[newI][newJ].h = newH;
                        cellDetails[newI][newJ].parent_i = i;
                        cellDetails[newI][newJ].parent_j = j;
                    }
                }
            }
        }

        r = foundDest ? FOUND_PATH : PATH_NOT_FOUND;

        tracePath(r, cellDetails, dest);
    }

    bool checkF(cell cellDetails[ROW][COL], int i, int j, double f)
    {
        return cellDetails[i][j] == FLT_MAX || cellDetails[i][j].f > f;
    }
}
