// TerrainGenerator.cpp : This file contains the 'main' function. Program execution begins and ends there.
// A script that creates a terrain using the diamant square algorithm.

#include <iostream>
#include <tchar.h>
#include <string>
#include <winnt.rh>
#include <random> 
#include <time.h>
#include <SimpleImage.h>
#include <sstream>
#include <TextureGenerator.h>
#include "TerrainGenerator.h"
#include "DiamondSquare.h"
#include "SmoothArray.h"
#include "ImageGenerator.h"

using namespace std;
using namespace GEDUtils;

// Helper macro for array index.
#define IDX(x, y, w) ((x) + (y) * (w))

void printFormat();
int checkInput(int argc, _TCHAR* argv[]);

void GenerateTerrain(int smoothingIterations, int smoothAreaSize, bool seeded = false, int customSeed = 0);
float* randomTerrain();
void printTerrain(float* start, int resolution);
float* cutEdge(float* inputArr, int dimesion, float cutOff = 1);
vector<float> arrToVec(float* start, int resolution);

GEDUtils::SimpleImage createSimpleImage(float* start, int r);
GEDUtils::SimpleImage createSimpleColourImage(Vector3f* map, int resolution);
void safeImage(GEDUtils::SimpleImage simpleImage, wstring path);
float* downscaleArray(float* arr, int resolution);

int resolution;
wstring height;
wstring color;
wstring normal;


int _tmain(int argc, _TCHAR* argv[])
{
    // Expected Arguments: -r 1028 -o_height heightmanp o_color color -o_normal normal

    // Check if input is valid.
    cout << "Check input" << endl;
    if (checkInput(argc, argv) == 0)
        return 0;

    // Call generate terrain.
    // Change smoothin size and iterations here
    // as well as seed.
    GenerateTerrain(5, 5, true, 42);
}

void GenerateTerrain(int smoothingIterations, int smoothAreaSize, bool seeded, int customSeed)
{
    cout << endl;
    cout << "Generate terrain:\n" << endl;

    // Resolution adjustments for the diamant square algorithm.
    // Round to the next power of 2.
    int calculationResolution = pow(2, ceil(log(resolution) / log(2)));
    // Add one.
    calculationResolution += 1;
    resolution = calculationResolution;
    cout << endl;

    // Perform diamant square.
    cout << endl;
    float* terrain = diamondSquareTerrain(resolution, seeded, customSeed);

    cout << endl;
    // Perform smoothing.
    cout << "Smoothing terrain" << "\n\tSmoothin Iterations: " << smoothingIterations << "\n\tSmoothing Area: " << smoothAreaSize << endl;


    for (int i = 0; i < smoothingIterations; i++)
    {
        cout << "\t\tsmoothing iteration: " << i << endl;
        terrain = smoothArray(terrain, resolution, resolution, smoothAreaSize);
    }


    cout << endl;




    // Cut the edge that is too much.
    terrain = cutEdge(terrain, resolution, 1);

    float* downscaledTerrain = downscaleArray(terrain, resolution);
    SimpleImage downscaledTerrainImage = createSimpleImage(downscaledTerrain, resolution / 4);
    safeImage(downscaledTerrainImage, L"downscaledHeightmap.png");

    GEDUtils::SimpleImage heightImage = createSimpleImage(terrain, resolution);
    safeImage(heightImage, height);

    Vector3f* normalMap = generateNM(terrain, resolution);

    GEDUtils::SimpleImage normalImage = createSimpleColourImage(normalMap, resolution);
    safeImage(normalImage, normal);


    SimpleImage texureMap = generateColourMap(terrain, normalMap, resolution);
    safeImage(texureMap, color);


    //vector<float> terrainVector = arrToVec(terrain, resolution);
    //generateTexture(terrainVector, resolution, L"GeneratorColor.png", L"GeneratorNormal.png");

    //TextureGenerator texGen(L"..\\..\\..\\..\\external\\textures\\gras15.jpg", L"..\\..\\..\\..\\external\\textures\\ground02.jpg", L"..\\..\\..\\..\\external\\textures\\rock2.jpg", L"..\\..\\..\\..\\external\\textures\\rock5.jpg");

    // generate texture and normal map with vectorHeightmap
    //texGen.generateAndStoreImages(terrainVector, resolution, L"texturemap_compare.png", L"normalmap_compare.png");
    

    delete[] normalMap;
    delete[] terrain;
    delete[] downscaledTerrain;
}

/**
Prints the right format to the console as a response to wrong input.
**/
void printFormat()
{
    cout << "Format: -r <int> -o_height <path> o_color <path> -o_normal <path>" << "\n";
}

/**
Check input and set values to public fields.
Returns 0 in case some input was wrong.
**/
int checkInput(int argc, _TCHAR* argv[])
{
    // Check resolution.
    if (_tcscmp(argv[1], TEXT("-r")) != 0)
    {
        cout << "Missing resolution" << "\n";
        printFormat();
        return 0;
    }

    // Check heightmap path.
    if (_tcscmp(argv[3], TEXT("-o_height")) != 0)
    {
        cout << "Missing height" << "\n";
        printFormat();
        return 0;
    }

    // Check color-/ texturemap path.
    if (_tcscmp(argv[5], TEXT("-o_color")) != 0)
    {
        cout << "Missing color" << "\n";
        printFormat();
        return 0;
    }

    // Check normalmap path.
    if (_tcscmp(argv[7], TEXT("-o_normal")) != 0)
    {
        cout << "Missing normal" << "\n";
        printFormat();
        return 0;
    }

    resolution = _tstoi(argv[2]);

    // Check if resolution value is valid.
    if (resolution <= 0)
    {
        cout << "Resolution needs to be greater than 0\n";
        return 0;
    }

    wstring h(argv[4]);
    height = h;
    wstring c(argv[6]);
    color = c;
    wstring n(argv[8]);
    normal = n;

    // Print out the input fields on the console.
    cout << "Res:\t" << resolution << "\n";
    wcout << "Height:\t" << height << "\n";
    wcout << "Color:\t" << color << "\n";
    wcout << "Normal:\t" << normal << "\n";

    return 1;
}

/**
Creates a fully random terrain.
**/
float* randomTerrain()
{
    const int size = (int)(resolution * resolution);
    float* terrain = new float[size];

    default_random_engine generator(time(0));
    normal_distribution<float> distribution(0.5, 0.17);

    float ran = distribution(generator);
    for (int y = 0; y < resolution; y++)
    {
        for (int x = 0; x < resolution; x++)
        {
            ran = distribution(generator);
            while (ran < 0 || ran > 1)
            {
                ran = distribution(generator);
            }

            terrain[IDX(x, y, resolution)] = ran;
        }
    }

    return terrain;
}

/// <summary>Prints a square array on the console.</summary>
void printTerrain(float* start, int resolution)
{
    for (int y = 0; y < resolution; y++)
    {
        for (int x = 0; x < resolution; x++)
        {
            cout << start[IDX(x, y, resolution)] << ", ";
        }
        cout << "\n";
    }
}

/// <summary>Creates a smaller array from the source array with cutOff dimesions less.</summary>
float* cutEdge(float* inputArr, int dimesion, float cutOff)
{
    int newDimesion = dimesion - cutOff;
    int size = (newDimesion * newDimesion);
    float* cuttedArray = new float[size];

    for (int y = 0; y < newDimesion; y++)
    {
        for (int x = 0; x < newDimesion; x++)
        {
            cuttedArray[IDX(x, y, newDimesion)] = inputArr[IDX(x, y, dimesion)];
        }
    }
    resolution = newDimesion;
    delete[] inputArr;
    return cuttedArray;
}

float* downscaleArray(float* arr, int resolution)
{
    int scaledArrSize = resolution / 4;
    int scaledArrLength = scaledArrSize * scaledArrSize;
    float* scaledArr = new float[scaledArrLength] {};

    for (int y = 0; y < scaledArrSize; y++)
    {
        for (int x = 0; x < scaledArrSize; x++)
        {
            int _x = x * 4;
            int _y = y * 4;



            float avg = 0;
            float numCount = 0;

            for (int yy = 0; yy < 4; yy++)
                for (int xx = 0; xx < 4; xx++)
                    if (_x + xx < resolution - 1 && _y + yy < resolution - 1)
                    {
                        avg += arr[IDX(_x + xx, _y + yy, resolution)];
                        numCount++;
                    }
            avg /= numCount;

            scaledArr[IDX(x, y, scaledArrSize)] = avg;
        }
    }
    return scaledArr;
}

/// <summary>Takes an array an returns it as float vector</summary>
vector<float> arrToVec(float* start, int resolution)
{
    vector<float> final;

    for (int y = 0; y < resolution; y++)
    {
        for (int x = 0; x < resolution; x++)
        {
            final.push_back(start[IDX(x, y, resolution)]);
        }
    }

    return final;
}
