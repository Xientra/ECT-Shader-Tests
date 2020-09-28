#include "Enemy.h"


Enemy::Enemy(string type, float hitpoints, XMVECTOR pos, XMVECTOR vel)
{
	this->enemyType = type;
	this->hitpoints = hitpoints;
	this->pos = pos;
	this->vel = vel;
}


void Enemy::move(float fElapsedTime)
{
	pos = pos + fElapsedTime * vel;
}


string Enemy::getEnemyType()
{
	return enemyType;
}
float Enemy::getHitpoints()
{
	return hitpoints;
}

XMVECTOR Enemy::getPos()
{
	return pos;
}
XMVECTOR Enemy::getVel()
{
	return vel;
}

bool Enemy::takeDamage(float damage)
{
	hitpoints -= damage;
	return hitpoints <= 0;
}
