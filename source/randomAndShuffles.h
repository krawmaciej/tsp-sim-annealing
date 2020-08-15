#ifndef RANDOMANDSHUFFLES_H_INCLUDED
#define RANDOMANDSHUFFLES_H_INCLUDED

#include <random>
#include <chrono>

unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
std::default_random_engine generator(seed);

float randomProbability()
{
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    return distribution(generator);
}

int randomBetween(int a, int b)
{
    std::uniform_int_distribution<int> distribution(a, b);
    return distribution(generator);
}

void shuffle(int* tab, int n)
{
    for (int i = n-1; i > 0; --i)
    {
        int j = randomBetween(0, i);
        std::swap(tab[i], tab[j]);
    }
}

#endif // RANDOMANDSHUFFLES_H_INCLUDED
