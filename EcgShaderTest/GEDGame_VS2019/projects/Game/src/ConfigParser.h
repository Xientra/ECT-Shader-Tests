#include <string>
#include <vector>

using namespace std;

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


struct MeshData
{
	MeshData() {};

	MeshData(string indentifier, string mf, string diffTex, string specTex, string norTex, string glowTex, string transTex, string diffTex2) {
		setAll(indentifier, mf, diffTex, specTex, norTex, glowTex, transTex, diffTex2);
	}

	string meshIdentifier;
	string meshFile;
	string diffuseTexture;
	string specularTexture;
	string normalTexture;
	string glowTexture;
	string transparencyTexture;
	string diffuseTexture2;

	void setAll(string indentifier, string mf, string diffTex, string specTex, string norTex, string glowTex, string transTex, string diffTex2)
	{
		meshIdentifier = indentifier;
		meshFile = mf;
		diffuseTexture = diffTex;
		specularTexture = specTex;
		normalTexture = norTex;
		glowTexture = glowTex;
		transparencyTexture = transTex;
		diffuseTexture2 = diffTex2;
	}
};

struct ObjectData
{
	string meshIdentifier;
	float scale;

	float xRotation;
	float yRotation;
	float zRotation;

	float xTranslation;
	float yTranslation;
	float zTranslation;

	ObjectData()
	{
		meshIdentifier = "";
		scale = 0;

		xTranslation = 0;
		yTranslation = 0;
		zTranslation = 0;

		xRotation = 0;
		yRotation = 0;
		zRotation = 0;
	}

	ObjectData(string meshIdentifier, float scale, float xRotation, float yRotation, float zRotation, float xTranslation, float yTranslation, float zTranslation)
	{
		setAll(meshIdentifier, scale, xRotation, yRotation, zRotation, xTranslation, yTranslation, zTranslation);
	}

	void setAll(string meshIdentifier, float scale, float xRotation, float yRotation, float zRotation, float xTranslation, float yTranslation, float zTranslation)
	{
		this->meshIdentifier = meshIdentifier;
		this->scale = scale;

		this->xTranslation = xTranslation;
		this->yTranslation = yTranslation;
		this->zTranslation = zTranslation;

		this->xRotation = xRotation;
		this->yRotation = yRotation;
		this->zRotation = zRotation;
	}
};

struct EnemyType 
{
	string typeIdentifier;
	
	float hitpoints;
	float size;
	float speed;
	
	string meshIdentifier;
	
	float scale;

	float xRotation;
	float yRotation;
	float zRotation;

	float xTranslation;
	float yTranslation;
	float zTranslation;

	EnemyType()
	{
		typeIdentifier = "";

		hitpoints = 0;
		size = 0;
		speed = 0;
		
		meshIdentifier = "";
		
		scale = 0;

		xTranslation = 0;
		yTranslation = 0;
		zTranslation = 0;

		xRotation = 0;
		yRotation = 0;
		zRotation = 0;
	}

	EnemyType(string typeIdentifier, float hitpoints, float size, float speed, string meshIdentifier, float scale, float xRotation, float yRotation, float zRotation, float xTranslation, float yTranslation, float zTranslation)
	{
		setAll(typeIdentifier, hitpoints, size, speed, meshIdentifier, scale, xRotation, yRotation, zRotation, xTranslation, yTranslation, zTranslation);
	}

	void setAll(string typeIdentifier, float hitpoints, float size, float speed, string meshIdentifier, float scale, float xRotation, float yRotation, float zRotation, float xTranslation, float yTranslation, float zTranslation)
	{
		this->typeIdentifier = typeIdentifier;

		this->hitpoints = hitpoints;
		this->size = size;
		this->speed = speed;
		
		this->meshIdentifier = meshIdentifier;
		
		this->scale = scale;

		this->xTranslation = xTranslation;
		this->yTranslation = yTranslation;
		this->zTranslation = zTranslation;

		this->xRotation = xRotation;
		this->yRotation = yRotation;
		this->zRotation = zRotation;
	}
};

struct GunConfiguration
{
	string configuration;

	float speed;
	float fireCooldown;
	int damage;
	float gravity;

	int particleSpriteIndex;

	float spriteRadius;

	Vector3f projectileSpawn;
};

class ConfigParser
{

public:
	struct Color
	{
		float r = 0;
		float g = 0;
		float b = 0;
	};

	bool load(string fileName);

	float getSpinning();
	float getspinSpeed();
	Color getBackgroundColor();
	float getTerrainWidth();
	float getTerrainDepth();
	float getTerrainHeight();
	string getTerrainHeightPath();
	string getTerrainColorPath();
	string getTerrainNormalPath();

	void getPlayerPosition(float& x, float& y, float& z);

	vector<ObjectData> cockpitObjects;
	vector<ObjectData> groundObjects;
	vector<EnemyType> enemyObjects;

	vector<MeshData> meshes;

	vector<GunConfiguration> gunConfigurations;
	vector<string> particlePaths;

	EnemyType* getEnemyTypeByName(string name);

	float getSpawnInterval();
	float getSpawnHeightMultiplierMin();
	float getSpawnHeightMultiplierMax();

	float getInnerCircle();
	float getSpawnCircle();
	float getOuterCircle();


private:
	float spinning = 0;
	float spinSpeed = 0;

	Color backgroundColor;

	string terrainPath = "";

	float terrainWidth = 0;
	float terrainDepth = 0;
	float terrainHeight = 0;

	string terrainHeightPath = "";
	string terrainColorPath = "";
	string terrainNormalPath = "";

	float playerPosX = 0;
	float playerPosY = 0;
	float playerPosZ = 0;

	float spawnInterval = 0;
	float spawnHeightMultiplierMin = 0;
	float spawnHeightMultiplierMax = 0;

	float innerCircle;
	float spawnCircle;
	float outerCircle;
};

extern ConfigParser configParserInstance;