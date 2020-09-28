#include "DiamondSquare.h"
#include <time.h>
#include <random>
#include <iostream>


// Helper macro for array index.
#define IDX(x, y, w) ((x) + (y) * (w))

std::default_random_engine generator(time(0));
std::normal_distribution<float> zeroToOneDistribution(0.5, 0.25);
std::normal_distribution<float> aroundZeroDistribution(0, 0.1);
float roughness = 1.3;


/// <summary>Diamantstep for the diamant square algorithm </summary>
/// <param name="x">coordinates of the point the diamant step is used on</param>
/// <param name="y">coordinates of the point the diamant step is used on</param>
/// <param name="s">current step size (size between points for the diamant step in this iteration)</param>
/// <param name="start">array pointer</param>
/// <param name="offset">a predifined offset added ontop of the point after die diamant calculation</param>
void diamond(int x, int y, int s, int r, float* start, float offset)
{
    float sum = 0;
    float count = 0;
    for (int xt = -s; xt <= s; xt += s)
    {
        for (int yt = -s; yt <= s; yt += s)
        {
            if (xt == 0 || yt == 0 || x + xt < 0 || y + yt < 0 || x + xt >= r || y + yt >= r)
                continue;
            int index = IDX(x + xt, y + yt, r);
            sum += start[index];
            count++;
        }
    }
    sum /= count;
    sum += offset;

    sum = (sum < 0) ? 0 : (sum > 1) ? 1 : sum;

    start[IDX(x, y, r)] = sum;
}

/// <summary>Square step for the diamant square algorithm </summary>
/// <param name="x">coordinates of the point the square step is used on</param>
/// <param name="y">coordinates of the point the square step is used on</param>
/// <param name="s">current step size (size between points for the diamant step in this iteration)</param>
/// <param name="start">array pointer</param>
/// <param name="offset">a predifined offset added ontop of the point after die square calculation</param>
void square(int x, int y, int s, int r, float* start, float offset)
{
    float sum = 0;
    float count = 0;
    for (int xt = -s; xt <= s; xt += s)
    {
        for (int yt = -s; yt <= s; yt += s)
        {
            if ((xt != 0 && yt != 0) || (xt == 0 && yt == 0) || x + xt < 0 || y + yt < 0 || x + xt >= r || y + yt >= r)
                continue;
            int index = IDX(x + xt, y + yt, r);
            sum += start[index];
            count++;
        }
    }
    sum /= count;
    sum += offset;

    sum = (sum < 0) ? 0 : (sum > 1) ? 1 : sum;

    start[IDX(x, y, r)] = sum;
}

/// <summary>Returns a random number that is within [0, 1] </summary>
float getRandom(float scale = 1)
{
    float ran = zeroToOneDistribution(generator) * roughness;

    while (ran < 0 || ran > 1)
    {
        ran = zeroToOneDistribution(generator) * roughness;
    }

    return ran * scale;
}

/// <summary>Returns a random number that is within [-1, 1] </summary>
float getRandomAroundZero(float scale)
{
    float ran = aroundZeroDistribution(generator) * roughness;

    while (ran < -1 || ran > 1)
    {
        ran = aroundZeroDistribution(generator) * roughness;
    }

    return ran * scale;
}

/// <summary>Performs the diamant square algorithm</summary>
float* diamondSquareTerrain(int resolution, bool seeded, int customSeed)
{
    int seed = (seeded) ? customSeed : time(0);
    float sigma = 0.25;

    std::default_random_engine newGenerator(seed);
    generator = newGenerator;

    std::normal_distribution<float> newZeroToOneDistribution(0.5, sigma);
    zeroToOneDistribution = newZeroToOneDistribution;
    std::normal_distribution<float> newAroundZeroDistribution(0, sigma);
    aroundZeroDistribution = newAroundZeroDistribution;

    std::cout << "Diamant Square:\n\tSeed: " << seed << "\n\tSigma: " << sigma << std::endl;

    double r = resolution;

    // Init terrain
    float* terrain = new float[(int)(r * r)];

    // Set corners.
    // Up Left
    terrain[0] = getRandom();
    // Up Right
    int index = IDX(0, (int)r - 1, (int)r);
    terrain[index] = getRandom();
    // Down Left
    index = IDX((int)r - 1, 0, (int)r);
    terrain[index] = getRandom();
    // Down Right
    index = IDX((int)r - 1, (int)r - 1, (int)r);
    terrain[index] = getRandom();

    // Until s = 1
    for (int s = r / 2; s >= 1; s /= 2)
    {

        // Perform diamant operations.
        for (int y = s; y < r; y += s * 2)
        {
            for (int x = s; x < r - 1; x += s * 2)
            {
                diamond(x, y, s, r, terrain, getRandomAroundZero(s / r));
                // std::cout << "Diamt(" << x << ", " << y << ")\n";
            }
        }
        int xStart = 0;

        // Perform square operations.
        for (int y = 0; y < r; y += s)
        {
            // Every other row start one s earlier.
            for (int x = s - (xStart % 2) * s; x < r; x += s * 2)
            {
                square(x, y, s, r, terrain, getRandomAroundZero(s / r));
                // std::cout << "Square(" << x << ", " << y << ")\n";
            }
            xStart++;
        }
    }

    return terrain;
}