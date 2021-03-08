#pragma once

#include <vector>

#define NOTHING_KEY -1
#define KEY_UP 0
#define KEY_LEFT 1
#define KEY_DOWN 2
#define KEY_RIGHT 3

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