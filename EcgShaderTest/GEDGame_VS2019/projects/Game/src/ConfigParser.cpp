#include <iostream>
#include <string>
#include <fstream>
#include "ConfigParser.h"

using namespace std;


bool ConfigParser::load(string fileName)
{

	// Create a new filestream
	std::ifstream configfile;

	// Open the config file in read mode (default)
	configfile.open(fileName);

	// If we could not open the file, return early with an "error"
	if (!configfile.is_open())
	{
		return false;
	}

	// Read the file while we have not reached the end and the filestream is working
	while (configfile.is_open() && configfile.good() && !configfile.eof())
	{
		std::string key;
		// Stream the next word into key
		configfile >> key;

		// Streams cast their content automatically
		if (key == "spinning")
			configfile >> spinning;
		else if (key == "spinSpeed")
			configfile >> spinSpeed;
		else if (key == "backgroundColor")
		{
			configfile >> backgroundColor.r;
			configfile >> backgroundColor.g;
			configfile >> backgroundColor.b;
		}
		else if (key == "TerrainPath")
		{
			configfile >> terrainHeightPath;
			configfile >> terrainColorPath;
			configfile >> terrainNormalPath;
		}
		else if (key == "TerrainWidth")
			configfile >> terrainWidth;
		else if (key == "TerrainHeight")
			configfile >> terrainHeight;
		else if (key == "TerrainDepth")
			configfile >> terrainDepth;
		else if (key == "Spawn")
		{
			#pragma region pars spawn parameters

			string intervalStr;
			string minHeightStr;
			string maxHeightStr;

			string innerCStr;
			string spawCStr;
			string outerCStr;


			configfile >> intervalStr;
			configfile >> minHeightStr;
			configfile >> maxHeightStr;

			configfile >> innerCStr;
			configfile >> spawCStr;
			configfile >> outerCStr;


			spawnInterval = stof(intervalStr);
			spawnHeightMultiplierMin = stof(minHeightStr);
			spawnHeightMultiplierMax = stof(maxHeightStr);

			innerCircle = stof(innerCStr);
			spawnCircle = stof(spawCStr);
			outerCircle = stof(outerCStr);

			#pragma endregion
		}
		else if (key == "PlayerPosition")
		{
			string input;

			configfile >> input;
			playerPosX = stof(input);
			configfile >> input;
			playerPosY = stof(input);
			configfile >> input;
			playerPosZ = stof(input);
		}
		else if (key == "Mesh")
		{
			#pragma region pars meshes

			string meshIdentifier;
			configfile >> meshIdentifier;
			string meshFile;
			configfile >> meshFile;
			string diffuseTexture;
			configfile >> diffuseTexture;
			string specularTexture;
			configfile >> specularTexture;
			string normalTexture;
			configfile >> normalTexture;
			string glowTexture;
			configfile >> glowTexture;
			string transparencyTexture;
			configfile >> transparencyTexture;

			string diffuseTexture2;
			configfile >> diffuseTexture2;

			meshes.push_back(MeshData(meshIdentifier, meshFile, diffuseTexture, specularTexture, normalTexture, glowTexture, transparencyTexture, diffuseTexture2));

			#pragma endregion
		}
		else if (key == "CockpitObject")
		{
			#pragma region parse cockpit objects

			string meshIdentifier;
			string scaleStr;
			string xRotStr;
			string yRotStr;
			string zRotStr;
			string xTransStr;
			string yTransStr;
			string zTransStr;


			// read file into variables

			configfile >> meshIdentifier;
			configfile >> scaleStr;

			configfile >> xRotStr;
			configfile >> yRotStr;
			configfile >> zRotStr;

			configfile >> xTransStr;
			configfile >> yTransStr;
			configfile >> zTransStr;


			// Convert string to floats;

			float scale = stof(scaleStr);

			float xRot = stof(xRotStr);
			float yRot = stof(yRotStr);
			float zRot = stof(zRotStr);

			float xTrans = stof(xTransStr);
			float yTrans = stof(yTransStr);
			float zTrans = stof(zTransStr);

			cockpitObjects.push_back(ObjectData(meshIdentifier, scale, xRot, yRot, zRot, xTrans, yTrans, zTrans));

			#pragma endregion
		}
		else if (key == "GroundObject")
		{
			#pragma region parse ground objects

			string meshIdentifier;
			string scaleStr;
			string xRotStr;
			string yRotStr;
			string zRotStr;
			string xTransStr;
			string yTransStr;
			string zTransStr;


			// read file into variables

			configfile >> meshIdentifier;
			configfile >> scaleStr;

			configfile >> xRotStr;
			configfile >> yRotStr;
			configfile >> zRotStr;

			configfile >> xTransStr;
			configfile >> yTransStr;
			configfile >> zTransStr;


			// Convert string to floats;

			float scale = stof(scaleStr);

			float xRot = stof(xRotStr);
			float yRot = stof(yRotStr);
			float zRot = stof(zRotStr);

			float xTrans = stof(xTransStr);
			float yTrans = stof(yTransStr);
			float zTrans = stof(zTransStr);

			groundObjects.push_back(ObjectData(meshIdentifier, scale, xRot, yRot, zRot, xTrans, yTrans, zTrans));

			#pragma endregion
		}
		else if (key == "EnemyType")
		{
			#pragma region parse enemy objects

			string typeIdentifier;
			string hitpointsStr;
			string sizeStr;
			string speedStr;
			string meshIdentifier;
			string scaleStr;
			string xRotStr;
			string yRotStr;
			string zRotStr;
			string xTransStr;
			string yTransStr;
			string zTransStr;


			// read file into variables

			configfile >> typeIdentifier;

			configfile >> hitpointsStr;
			configfile >> sizeStr;
			configfile >> speedStr;

			configfile >> meshIdentifier;
			configfile >> scaleStr;

			configfile >> xRotStr;
			configfile >> yRotStr;
			configfile >> zRotStr;

			configfile >> xTransStr;
			configfile >> yTransStr;
			configfile >> zTransStr;


			// Convert string to floats;

			float hitpoints = stof(hitpointsStr);
			float size = stof(sizeStr);
			float speed = stof(speedStr);

			float scale = stof(scaleStr);

			float xRot = stof(xRotStr);
			float yRot = stof(yRotStr);
			float zRot = stof(zRotStr);

			float xTrans = stof(xTransStr);
			float yTrans = stof(yTransStr);
			float zTrans = stof(zTransStr);

			enemyObjects.push_back(EnemyType(typeIdentifier, hitpoints, size, speed, meshIdentifier, scale, xRot, yRot, zRot, xTrans, yTrans, zTrans));

			#pragma endregion
		}
		else if (key == "GunConfiguration")
		{
			#pragma region parse gun configuration

			string configuration;

			float speed;
			float fireCooldown;
			int damage;
			float gravity;

			int particleSpriteIndex;
			float spriteRadius;
			Vector3f projectileSpawn;

			// pars variable
			string input;

			configfile >> input;
			configuration = input;

			configfile >> input;
			speed = stof(input);

			configfile >> input;
			fireCooldown = stof(input);

			configfile >> input;
			damage = stoi(input);

			configfile >> input;
			gravity = stof(input);

			configfile >> input;
			particleSpriteIndex = stoi(input);

			configfile >> input;
			spriteRadius = stof(input);

			configfile >> input;
			projectileSpawn.x = stoi(input);
			configfile >> input;
			projectileSpawn.y = stoi(input);
			configfile >> input;
			projectileSpawn.z = stoi(input);

			gunConfigurations.push_back(GunConfiguration{ configuration, speed, fireCooldown, damage, gravity, particleSpriteIndex, spriteRadius, projectileSpawn });

			#pragma endregion
		}
		else if (key == "Particles")
		{
			string sizeStr;
			configfile >> sizeStr;

			int size = stoi(sizeStr);

			for (int i = 0; i < size; i++)
			{
				string particlePath;
				configfile >> particlePath;

				particlePaths.push_back(particlePath);
			}
		}
	}

	// Close the file
	configfile.close();

	return true;
}

EnemyType* ConfigParser::getEnemyTypeByName(string name)
{
	for (int i = 0; i < enemyObjects.size(); i++)
		if (enemyObjects.at(i).typeIdentifier == name)
			return &enemyObjects.at(i);

	return nullptr;
}

float ConfigParser::getSpinning()
{
	return spinning;
}
float ConfigParser::getspinSpeed()
{
	return spinSpeed;
}
ConfigParser::Color ConfigParser::getBackgroundColor()
{
	return backgroundColor;
}
float ConfigParser::getTerrainWidth()
{
	return terrainWidth;
}
float ConfigParser::getTerrainDepth()
{
	return terrainDepth;
}
float ConfigParser::getTerrainHeight()
{
	return terrainHeight;
}

string ConfigParser::getTerrainHeightPath()
{
	return terrainHeightPath;
}
string ConfigParser::getTerrainColorPath()
{
	return terrainColorPath;
}
string ConfigParser::getTerrainNormalPath()
{
	return terrainNormalPath;
}

void ConfigParser::getPlayerPosition(float& x, float& y, float& z)
{
	x = playerPosX;
	y = playerPosY;
	z = playerPosZ;
}

float ConfigParser::getSpawnInterval()
{
	return spawnInterval;
}
float ConfigParser::getSpawnHeightMultiplierMin()
{
	return spawnHeightMultiplierMin;
}
float ConfigParser::getSpawnHeightMultiplierMax()
{
	return spawnHeightMultiplierMax;
}

float ConfigParser::getInnerCircle()
{
	return innerCircle;
}
float ConfigParser::getSpawnCircle()
{
	return spawnCircle;
}
float ConfigParser::getOuterCircle()
{
	return outerCircle;
}

ConfigParser configParserInstance;