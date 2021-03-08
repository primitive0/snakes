#pragma once

#include <vector>

#define KEY_W 87
#define KEY_A 65
#define KEY_S 83
#define KEY_D 68
#define KEY_X 88

#define KEY_UP 38
#define KEY_LEFT 37
#define KEY_DOWN 40
#define KEY_RIGHT 39

struct FieldInfo
{
	unsigned int x;
	unsigned int y;
};

enum Direction
{
	UP,
	LEFT,
	DOWN,
	RIGHT
};

struct Point
{
	unsigned int x;
	unsigned int y;
};

struct Snake
{
	Point headPosition;
	Direction direction;
	std::vector<Point> tail;
};

struct GameField
{
	FieldInfo fieldInfo;
	Snake snake;
	Point fruitPosition;
};