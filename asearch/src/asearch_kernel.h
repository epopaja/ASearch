/*
* Based off of https://www.geeksforgeeks.org/a-search-algorithm/
*/
#ifndef ASEARCH_H_
#define ASEARCH_H_

#define ROW 9
#define COL 10

#include <utility>
#include <cstring>
#include <iostream>
#include <math.h>
#include <stack>
#include <float.h>


using namespace std;

extern "C"
{
    enum result
    {
        PATH_NOT_FOUND = -1,
        FOUND_PATH = 0,
        INVALID_SOURCE = 1,
        INVALID_DESTINATION = 2,
        PATH_IS_BLOCKED = 3,
        ALREADY_AT_DESTINATION = 4,
    };

    typedef pair<int, int> Pair;

    typedef pair<double, pair<int, int>> pPair;

    struct cell
    {
        int parent_i, parent_j;

        double f, g, h;
    };

    void asearch(int grid[], Pair src, Pair dest, result* r, cell cellDetails[]);
}


bool isValid(int row, int col);

bool isUnBlocked(int grid[][COL], int row, int col);

bool isDestination(int row, int col, Pair dest);

double calculateHValue(int row, int col, Pair dest);

//void tracePath(result r, cell cellDetails[][COL], Pair dest);

void readGrid(const char* file, int grid[][COL]);

bool checkF(cell cellDetails[][COL], int i, int j, double f);

void init(pPair* list);

bool checkForEmpty(pPair* list);

void getNext(pPair* list, int* index);

void addPPair(pPair* list, const pPair& pair);

void removePPair(pPair* list, int index);

#endif
