#ifndef ANNEALING_H_INCLUDED
#define ANNEALING_H_INCLUDED

#include <iostream>

#include <string>
#include <cmath>
#include "randomAndShuffles.h"

enum Direction {LEFT = -1, RIGHT = 1};

struct Path
{
    int* path = nullptr;
    int sizePath = 0;
    float energy = 0.0f;
};

void printPath(Path& path)
{
    using std::cout;
    using std::endl;
    cout << "Path: ";
    for (int i = 0; i < path.sizePath; ++i)
        cout << path.path[i] << ", ";
    cout << endl << "Distance: " << path.energy << endl;
}

void deletePath(Path& path)
{
    delete[] path.path;
}

int* copyTable(int* tab, int n)
{
    int* result = new int[n];
    for (int i = 0; i < n; ++i)
        result[i] = tab[i];
    return result;
}

Path copyPath(const Path& path)
{
    Path newPath = path;
    newPath.path = copyTable(path.path, path.sizePath);
    return newPath;
}

void calculateSwapEnergy(Path& newPath, float** matrix, int i1, int i2)
{
    // get selected vertices to swap
    int v1 = newPath.path[i1];
    int v2 = newPath.path[i2];
    // get their connected vertices
    int v1L, v1R, v2L, v2R;
    if (i1 == 0)
        v1L = newPath.path[newPath.sizePath-1];
    else
        v1L = newPath.path[i1+LEFT];
    v1R = newPath.path[i1+RIGHT];

    v2L = newPath.path[i2+LEFT];
    if (i2 == newPath.sizePath-1)
        v2R = newPath.path[0];
    else
        v2R = newPath.path[i2+RIGHT];

    // delete distances of nonexisting connections
    newPath.energy-= matrix[v1][v1L] + matrix[v1][v1R] +
                     matrix[v2][v2L] + matrix[v2][v2R];

    // swap two vertices
    std::swap(newPath.path[i1], newPath.path[i2]);
    // now i1->v2 and i2->v1, so i1+RIGHT=v2R
    // get their connected vertices
    if (i1 == 0)
        v2L = newPath.path[newPath.sizePath-1];
    else
        v2L = newPath.path[i1+LEFT];
    v2R = newPath.path[i1+RIGHT];

    v1L = newPath.path[i2+LEFT];
    if (i2 == newPath.sizePath-1)
        v1R = newPath.path[0];
    else
        v1R = newPath.path[i2+RIGHT];

    // add distances of new connections
    newPath.energy+= matrix[v1][v1L] + matrix[v1][v1R] +
                     matrix[v2][v2L] + matrix[v2][v2R];
}

void calculateReverseEnergy(Path& newPath, float** matrix, int i1, int i2)
{
    // get selected vertices to swap
    int v1 = newPath.path[i1];
    int v2 = newPath.path[i2];
    // get their connected vertices
    int v1R = newPath.path[i1+RIGHT];
    int v2L = newPath.path[i2+LEFT];
    // calculate new distance
    newPath.energy+= matrix[v1][v2L] + matrix[v2][v1R] -
                     matrix[v1][v1R] - matrix[v2][v2L];
}

void reverseSetPath(Path& path, int i1, int i2)
{
    while(++i1 < --i2)
        std::swap(path.path[i1], path.path[i2]);
}

Path calculatePath(Path& oldPath, float** matrix, int i1, int i2, bool swap)
{
    Path newPath = copyPath(oldPath);

    if (swap)
        calculateSwapEnergy(newPath, matrix, i1, i2);
    else
        calculateReverseEnergy(newPath, matrix, i1, i2);

    return newPath;
}

Path initializeNewPath(float** matrix, int matrixSize)
{
    Path currentPath;
    currentPath.sizePath = matrixSize;

    currentPath.path = new int[matrixSize];
    // make path equal to [0,1,2,3,4,...]
    for (int i = 0; i < matrixSize; ++i)
        currentPath.path[i] = i;

    // shuffle, take random initial state
    shuffle(currentPath.path, matrixSize);

    // calculate energy of initial state
    for (int i = 0; i < matrixSize-1; ++i)
        currentPath.energy+= matrix[currentPath.path[i]][currentPath.path[i+1]];
    currentPath.energy+= matrix[currentPath.path[0]][currentPath.path[matrixSize-1]];

    return currentPath;
}

void getTwoRandomIndices(int& i1, int& i2, int size)
{
    i1 = randomBetween(0, size-1);
    i2 = randomBetween(0, size-1);
    while (i2 == i1)
        i2 = randomBetween(0, size-1);
    // s1 is to the left of s2
    // so swapping and reversing takes fewers steps
    if (i2 < i1) std::swap(i2, i1);
}

// returns true if path was changed, false otherwise
bool getNewPath(Path& currentPath, float** matrix, float temperature)
{
    // get two random indices for swapping or reversing
    int i1, i2;
    getTwoRandomIndices(i1, i2, currentPath.sizePath);
    // depending on the temperature choose a method of calculating neighbouring state
    float r = randomProbability();
    float p = exp(-1/temperature);
    // higher probability for swapping for higher temperature
    bool swap = (p > r);

    // if reverse is chosen, and random indices are i.e. 2 and 4
    // then between them is only one vertice to be reversed, so no reverse happens
    // same for 2 and 3, then there are no vertices between to be reversed
    // so distance between random indices must be 3 or greater.
    while ((i2 - i1 < 3) && !swap)
        getTwoRandomIndices(i1, i2, currentPath.sizePath);

    Path newPath;
    // get new neighboring state
    newPath = calculatePath(currentPath, matrix, i1, i2, swap);

    if (newPath.energy >= currentPath.energy)
    {
        float dEnergy = currentPath.energy - newPath.energy;
        p = exp(dEnergy/temperature);
        if (p < r)
            return false;
    }

    // since reverse does not need to physically reverse vertices to calculate distance
    // if the new reversed path is to be used it needs to be physically reversed
    if (!swap)
        reverseSetPath(newPath, i1, i2);

    // delete old path and set it to new path
    deletePath(currentPath);
    currentPath = newPath;
    return true;
}

#endif // ANNEALING_H_INCLUDED
