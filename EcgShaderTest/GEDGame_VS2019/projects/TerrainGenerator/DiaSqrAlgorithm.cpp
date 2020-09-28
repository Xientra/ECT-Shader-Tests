#include "DiaSqrAlgorithm.h"

#define IDX(x, y, w) ((x) + (y) * w)

const float diaSqrSeed = time(0);
default_random_engine diaSqrRandomGenerator(diaSqrSeed);

const float normalDistribution = (1 / 6);
normal_distribution<float> diaSqrDistribution(0.5f, ((float)1 / 6));

const float randomnessStrength = 0.5f;


float diamonSquareRandomness(float step, float res);
void setCorners(float* arr, int arrSize);
void diamondStep(float* arr, int arrSize, int x, int y, int step);
bool squareStep(float* arr, int arrSize, int x, int y, int step);
void printArr(float* arr, int size);

//bool* fuckthis = new bool[arrSize * arrSize]{};


/* =:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:= */


float random01()
{
	float rnd = diaSqrDistribution(diaSqrRandomGenerator);
	return (rnd < 0) ? 0 : ((rnd > 1) ? 1 : rnd);
}


void diamondSquareAlgorithm(float* arr, int arrSize)
{

	setCorners(arr, arrSize);

	//diamondStep(arr, arrSize, arrSize / 2, arrSize / 2, arrSize / 2);
	//return;

	for (int size = arrSize; size > 1; size /= 2) // size: 16, 8, 4, 2, 1
	{ 

		int step = size / 2; // step: 8, 4, 2, 1

		// Debug counting
		int sqrSteps = 0;
		int diaSteps = 0;

		// diamond steps
		for (int y = step; y < arrSize; y += size)
		{
			for (int x = step; x < arrSize; x += size)
			{
				diamondStep(arr, arrSize, x, y, step);
				diaSteps++;
			}
		}

		// square steps
		for (int y = 0; y < arrSize; y += size)
		{
			for (int x = 0; x < arrSize; x += size)
			{
				sqrSteps += squareStep(arr, arrSize, x + step, y, step);
				sqrSteps += squareStep(arr, arrSize, x, y + step, step);
			}
		}

		printArr(arr, arrSize);
		cout << "-------------------" << "sqe Steps: " << sqrSteps << " dia Steps: " << diaSteps << endl;
	}

	cout << arr[IDX(arrSize / 2, arrSize / 2, arrSize)] << endl;

}

float diamonSquareRandomness(float step, float res)
{
	//return (random01() - 0.5f) * step * randomnessStrength;
	return (random01() - 0.5f) * ((step * 2) / (res - 1)) * randomnessStrength;
}

void setCorners(float* arr, int arrSize)
{
	for (int y = 0; y < arrSize; y += (arrSize - 1))
	{
		for (int x = 0; x < arrSize; x += (arrSize - 1))
		{
			arr[IDX(x, y, arrSize)] = random01();
			cout << arr[IDX(x, y, arrSize)] << endl;
		}
	}
}

void diamondStep2(float* arr, int arrSize, int x, int y, int step)
{
	int divBy = 0;
	float avg = 0;

	// top left
	if (x - step >= 0 && y - step >= 0)
	{
		avg += arr[(int)IDX(x - step, y - step, arrSize)];
		divBy++;
	}
	// bottom left
	if (x - step >= 0 && y + step < arrSize)
	{
		avg += arr[(int)IDX(x - step, y + step, arrSize)];
		divBy++;
	}
	// top right
	if (x + step < arrSize && y - step >= 0)
	{
		avg += arr[(int)IDX(x + step, y - step, arrSize)];
		divBy++;
	}
	// botton right
	if (x + step < arrSize && y + step < arrSize)
	{
		avg += arr[(int)IDX(x + step, y + step, arrSize)];
		divBy++;
	}

	avg /= divBy;
	// add randomness
	avg += diamonSquareRandomness(step, arrSize);
	// clamp 0 1
	avg = (avg < 0) ? 0 : ((avg > 1) ? 1 : avg);

	arr[(int)IDX(x, y, arrSize)] = avg;
}

void diamondStep(float* arr, int arrSize, int x, int y, int step)
{
	float avg = 0;

	avg += arr[(int)IDX(x - step, y - step, arrSize)];
	avg += arr[(int)IDX(x - step, y + step, arrSize)];
	avg += arr[(int)IDX(x + step, y - step, arrSize)];
	avg += arr[(int)IDX(x + step, y + step, arrSize)];

	avg /= 4;
	// add randomness
	avg += diamonSquareRandomness(step, arrSize);
	// clamp 0 1
	avg = (avg < 0) ? 0 : ((avg > 1) ? 1 : avg);

	arr[(int)IDX(x, y, arrSize)] = avg;
}

bool squareStep2(float* arr, int arrSize, int x, int y, int step)
{
	// checks if the points is outside of the array
	if ((x < 0 || x > arrSize - 1) || (y < 0 || y > arrSize - 1))
		return false;

	int divBy = 0;
	float avg = 0;

	// left
	if (x - step >= 0)
	{
		avg += arr[(int)IDX(x - step, y, arrSize)];
		divBy++;
	}
	// right
	if (x + step < arrSize)
	{
		avg += arr[(int)IDX(x + step, y, arrSize)];
		divBy++;
	}
	// top
	if (y - step >= 0)
	{
		avg += arr[(int)IDX(x, y - step, arrSize)];
		divBy++;
	}
	// bottom
	if (y + step < arrSize)
	{
		avg += arr[(int)IDX(x, y + step, arrSize)];
		divBy++;
	}

	avg /= divBy;
	// add randomness
	avg += diamonSquareRandomness(step, arrSize);
	// clamp 0 1
	avg = (avg < 0) ? 0 : ((avg > 1) ? 1 : avg);

	arr[(int)IDX(x, y, arrSize)] = avg;

	return true;
}

bool squareStep(float* arr, int arrSize, int x, int y, int step)
{
	// checks if the points is outside of the array
	if ((x < 0 || x > arrSize - 1) || (y < 0 || y > arrSize - 1))
		return false;

	int divBy = 0;
	float avg = 0;

	// left
	if (x - step >= 0)
	{
		avg += arr[(int)IDX(x - step, y, arrSize)];
		divBy++;
	}
	// right
	if (x + step < arrSize)
	{
		avg += arr[(int)IDX(x + step, y, arrSize)];
		divBy++;
	}
	// top
	if (y - step >= 0)
	{
		avg += arr[(int)IDX(x, y - step, arrSize)];
		divBy++;
	}
	// bottom
	if (y + step < arrSize)
	{
		avg += arr[(int)IDX(x, y + step, arrSize)];
		divBy++;
	}

	avg /= divBy;
	// add randomness
	avg += diamonSquareRandomness(step, arrSize);
	// clamp 0 1
	avg = (avg < 0) ? 0 : ((avg > 1) ? 1 : avg);

	arr[(int)IDX(x, y, arrSize)] = avg;

	return true;
}

void printArr(float* arr, int size)
{
	for (int y = 0; y < size; y++)
	{
		for (int x = 0; x < size; x++)
		{
			//string s = arr[IDX(x, y, size)] == 0 ? "-" : to_string(arr[IDX(x, y, size)]).substr(0, 4);
			string s = to_string(arr[IDX(x, y, size)]).substr(0, 4);
			cout << s << "\t";
		}
		cout << endl;
	}
	cout << endl;
}