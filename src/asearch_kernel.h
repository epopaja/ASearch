/*
        Filename: asearch_kernel.h
                Header file

*/
#ifndef ASEARCH_H_
#define ASEARCH_H_

#define ROW 9
#define COL 10

#include <bits/stdc++.h>

enum result
{
    PATH_NOT_FOUND = -1,
    FOUND_PATH = 0,
    INVALID_SOURCE = 1,
    INVALID_DESTINATION = 2,
    PATH_IS_BLOCKED = 3,
    ALREADY_AT_DESTINATION = 4,
}

typedef std::pair<int, int> Pair;

typedef std::pair<double, pair<int, int>> pPair;

struct cell
{
    int parent_i, parent_j;

    double f, g, h;
};

extern "C"
{
    bool isValid(int row, int col);

    bool isUnBlocked(int grid[][COL], int row, int col);

    bool isDestination(int row, int col, Pair dest);

    double calculateHValue(int row, int col, Pair dest);

    void tracePath(result r, cell cellDetails[][COL], Pair dest);

    int** readGrid(const char* file);

    void aStarSearch(int grid[][COL], Pair src, Pair dest);
}

#endif
