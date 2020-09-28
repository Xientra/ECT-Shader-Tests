#include "ImageGenerator.h"
#include <SimpleImage.h>
#include <string>
#include <iostream>
#include <TextureGenerator.h>
#include <tchar.h>

using namespace GEDUtils;
using namespace std;

#define IDX(x, y, w) ((x) + (y) * (w))

float move(float number)
{
    float ret;
    ret = (number + 1) / 2;
    return ret;
}

/// <summary>Wrapper function to create a SimpleImage from an array</summary>
Vector3f* generateNM(float* heightmap, int resolution)
{
    Vector3f* ret = new Vector3f[resolution * resolution];

    for (int x = 0; x < resolution; x++)
    {
        for (int y = 0; y < resolution; y++)
        {
            int div = 2;

            float xPrev;
            //yNext = heightmap[IDX((x - 1 >= 0) ? x - 1 : x, y, resolution)]
            if (x - 1 >= 0)
            {
                xPrev = heightmap[IDX(x - 1, y, resolution)];
            } else
            {
                xPrev = heightmap[IDX(x, y, resolution)];
                div = 1;
            }

            float xNext;
            if (x + 1 < resolution)
            {
                xNext = heightmap[IDX(x + 1, y, resolution)];
            } else
            {
                xNext = heightmap[IDX(x, y, resolution)];
                div = 1;
            }

            xNext *= resolution;
            xPrev *= resolution;

            float xDiv = (xNext - xPrev) / div;

            div = 2;

            float yPrev;
            if (y - 1 >= 0)
            {
                yPrev = heightmap[IDX(x, y - 1, resolution)];
            } else
            {
                yPrev = heightmap[IDX(x, y, resolution)];
                div = 1;
            }

            float yNext;
            if (y + 1 < resolution)
            {
                yNext = heightmap[IDX(x, y + 1, resolution)];
            } else
            {
                yNext = heightmap[IDX(x, y, resolution)];
                div = 1;
            }

            yNext *= resolution;
            yPrev *= resolution;

            float yDiv = (yNext - yPrev) / div;

            //std::cout << "divs: " << yDiv << "   " << xDiv << "\n";

            Vector3f current{ -xDiv, -yDiv, 1 };

            current.normalize();

            current.x = move(current.x);
            current.y = move(current.y);
            current.z = move(current.z);

            ret[IDX(x, y, resolution)] = current;

            //std::cout << current.x << "   " << current.y << "   " << current.z << "\n";
        }
    }
    return ret;
}


void calcAlphas(float height, float slope, float& alpha1, float& alpha2, float& alpha3)
{
    slope *= 0.8;
    alpha1 = (1 - height) * slope;
    alpha2 = height;
    alpha3 = height * slope;
}

// <summary>Calculates the weights for color blending given an slope and height.</summary>
void calculateWeights(float height, float slope, float& a0, float& a1, float& a2, float& a3)
{
    float s1 = (slope - 0.75) * 7;
    s1 = (s1 < 0) ? 0 : s1;
    float s2 = (1 - s1);

    // Caps for blending.
    // Blending happends between top and bottom.
    // Outside of it the weights will be 1 for the top texture or the bottom texture.
    // The same applies for the slope.
    
    //float top = 0.70f;
    //float bottom = 0.25f;
    //float topSlope = 0.999f;
    //float bottomSlope = 0.995f;
    

    float top = 0.8f;
    float bottom = 0.5f;
    float topSlope = 0.999f;
    float bottomSlope = 0.995f;

    // Scale the slope on an wider scale.
    slope -= 0.9999;
    slope = (slope < 0) ? 0 : slope;
    slope *= 10000;

    // Calculate slope scalars.
    s1 = (slope < bottomSlope) ? 1 : (slope > topSlope) ? 0 : 1 - (slope - bottomSlope) * (1 / (topSlope - bottomSlope));
    s2 = 1 - s1;

    a3 = (height < bottom) ? 1 : (height > top) ? 0 : 1 - (height - bottom) * (1 / (top - bottom));
    a1 = a3 * s1;
    a3 *= s2;

    a2 = 1 - a3;
    a0 = a2 * s1;
    a2 *= s2;
}

SimpleImage generateColourMap(float* heightMap, Vector3f* normalMap, int resolution)
{
    GEDUtils::SimpleImage grass = GEDUtils::SimpleImage(L"..\\..\\..\\..\\external\\textures\\gras15.jpg");
    GEDUtils::SimpleImage dirt = GEDUtils::SimpleImage(L"..\\..\\..\\..\\external\\textures\\mud02.jpg");
    GEDUtils::SimpleImage pebbles = GEDUtils::SimpleImage(L"..\\..\\..\\..\\external\\textures\\pebble03.jpg");
    GEDUtils::SimpleImage rock = GEDUtils::SimpleImage(L"..\\..\\..\\..\\external\\textures\\rock4.jpg");

    Vector3f* colourMap = new Vector3f[resolution * resolution];
    GEDUtils::SimpleImage textureImage(resolution, resolution);

    for (int x = 0; x < resolution; x++)
    {
        for (int y = 0; y < resolution; y++)
        {
            float height = heightMap[IDX(x, y, resolution)];
            float slope = normalMap[IDX(x, y, resolution)].z;

            float a0 = 1;
            float a1;
            float a2;
            float a3;
            //calcAlphas(height, slope, a1, a2, a3);
            calculateWeights(height, slope, a0, a1, a2, a3);

            float red;
            float green;
            float blue;

            grass.getPixel(x % grass.getWidth(), y % grass.getHeight(), red, green, blue);

            float C0r = red * a0;
            float C0g = green * a0;
            float C0b = blue * a0;

            dirt.getPixel(x % dirt.getWidth(), y % dirt.getHeight(), red, green, blue);

            float C1r = a1 * red + (1 - a1) * C0r;
            float C1g = a1 * green + (1 - a1) * C0g;
            float C1b = a1 * blue + (1 - a1) * C0b;

            pebbles.getPixel(x % pebbles.getWidth(), y % pebbles.getHeight(), red, green, blue);

            float C2r = a2 * red + (1 - a2) * C1r;
            float C2g = a2 * green + (1 - a2) * C1g;
            float C2b = a2 * blue + (1 - a2) * C1b;

            rock.getPixel(x % rock.getWidth(), y % rock.getHeight(), red, green, blue);

            float C3r = a3 * red + (1 - a3) * C2r;
            float C3g = a3 * green + (1 - a3) * C2g;
            float C3b = a3 * blue + (1 - a3) * C2b;

            textureImage.setPixel(x, y, C3r, C3g, C3b);
        }
    }

    return textureImage;
    //textureImage.save(L"texturemap.png");
}

/// <summary>Generates texture and normal maps for a given terrain.</summary>
void generateTexture(std::vector<float> terrain, int resolution, std::wstring color, std::wstring normal)
{
    cout << "generating example textures";

    TextureGenerator texGen(L"..\\..\\..\\..\\external\\textures\\gras15.jpg", L"..\\..\\..\\..\\external\\textures\\ground02.jpg", L"..\\..\\..\\..\\external\\textures\\rock2.jpg", L"..\\..\\..\\..\\external\\textures\\rock5.jpg");

    // Generator expects the terrain to be of resolution: resolution + 1.
    texGen.generateAndStoreImages(terrain, resolution, color, normal);

    cout << " finished" << endl;
}


/// <summary>Wrapper function to create a SimpleImage from an array</summary>
GEDUtils::SimpleImage createSimpleImage(float* start, int resolution)
{
    cout << "creating image\n";

    GEDUtils::SimpleImage simpleImage(resolution, resolution);

    // Fill the image with a color gradient
    for (int y = 0; y < simpleImage.getHeight(); ++y)
    {
        for (int x = 0; x < simpleImage.getWidth(); ++x)
        {
            float val = start[IDX(x, y, resolution)];
            simpleImage.setPixel(x, y, val);
        }
    }

    return simpleImage;
}

GEDUtils::SimpleImage createSimpleColourImage(Vector3f* array, int resolution)
{
    cout << "creating image\n";

    GEDUtils::SimpleImage simpleImage(resolution, resolution);

    // Fill the image with a color gradient
    for (int y = 0; y < simpleImage.getHeight(); ++y)
    {
        for (int x = 0; x < simpleImage.getWidth(); ++x)
        {
            // Set RGB Values at each pixel
            simpleImage.setPixel(x, y,
                array[IDX(x, y, resolution)].x, /* Red */
                array[IDX(x, y, resolution)].y, /* Green */
                array[IDX(x, y, resolution)].z /* Blue */);
        }
    }

    return simpleImage;
}

/// <summary>Wrapper function to safe the given SimpleImage</summary>
void safeImage(GEDUtils::SimpleImage simpleImage, std::wstring path)
{
    std::string s(path.begin(), path.end());
    std::cout << "Image saved at " << s << "\n";

    if (!simpleImage.save(path.c_str()))
    {
        throw "Could not save gray image";
    }
}