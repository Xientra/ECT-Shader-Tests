#include <vector>
#include <string>
#include <SimpleImage.h>
#include <cmath>

struct Vector3f
{
	float x;
	float y;
	float z;

	Vector3f()
	{
		x = 0;
		y = 0;
		z = 0;
	}

	Vector3f(float xX, float yY, float zZ)
	{
		x = xX;
		y = yY;
		z = zZ;
	}

	void normalize()
	{
		float length = sqrt(x * x + y * y + z * z);
		x /= length;
		y /= length;
		z /= length;
	}
};

void safeImage(GEDUtils::SimpleImage simpleImage, std::wstring path);
GEDUtils::SimpleImage createSimpleColourImage(Vector3f* array, int resolution);
GEDUtils::SimpleImage createSimpleImage(float* start, int resolution);
void generateTexture(std::vector<float> terrain, int resolution, std::wstring color, std::wstring normal);

Vector3f* generateNM(float* heightmap, int resolution);
GEDUtils::SimpleImage generateColourMap(float* heightMap, Vector3f* normalMap, int resolution);
