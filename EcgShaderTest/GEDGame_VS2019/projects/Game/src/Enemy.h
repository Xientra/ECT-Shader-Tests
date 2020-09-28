#pragma once
#include <string>
#include <DirectXMath.h>

using namespace std;
using namespace DirectX;

class Enemy
{

public:
	Enemy(string type, float hitpoints, XMVECTOR pos, XMVECTOR vel);

	void move(float fElapsedTime);

	string getEnemyType();
	float getHitpoints();

	bool takeDamage(float damage);

	XMVECTOR getPos();
	XMVECTOR getVel();



private:
	string enemyType;
	float hitpoints;
	XMVECTOR pos;
	XMVECTOR vel;
};
