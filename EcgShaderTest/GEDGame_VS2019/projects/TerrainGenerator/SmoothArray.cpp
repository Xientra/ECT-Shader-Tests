#include "SmoothArray.h"


// Helper macro for array index.
#define IDX(x, y, w) ((x) + (y) * (w))


/// <summary>Helper function for smoothArray</summary>
float getAvarage(float* arrPointer, int totalwidth, int startWidth, int startHeight, int width, int height)
{
    float value = 0;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float current = arrPointer[IDX(x + startWidth, y + startHeight, totalwidth)];
            value += current;
        }
    }

    return value / (float)(width * height);
}

/// <summary>Smoothes a given array by building the average over a certain size</summary>
float* smoothArray(float* arrPointer, int width, int height, int smoothAreaSize)
{
    float* smoothedArr = new float[(width * height)];

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            smoothAreaSize = (smoothAreaSize % 2 == 0) ? smoothAreaSize - 1 : smoothAreaSize;
            int halfSize = (smoothAreaSize - 1) / 2;

            // Calculations.
            int xStart = (x > halfSize) ? x - halfSize : 0;
            int yStart = (y > halfSize) ? y - halfSize : 0;

            int widthSmoothing = smoothAreaSize - ((x < halfSize) ? halfSize - x : 0) - ((x + halfSize >= width) ? x + halfSize - width + 1 : 0);
            int heightSmoothing = smoothAreaSize - ((y < halfSize) ? halfSize - y : 0) - ((y + halfSize >= height) ? y + halfSize - height + 1 : 0);

            smoothedArr[IDX(x, y, width)] = getAvarage(arrPointer, width, xStart, yStart, widthSmoothing, heightSmoothing);
        }
    }

    return smoothedArr;
    delete[] arrPointer;
}