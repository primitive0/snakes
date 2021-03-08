#include <random>
#include <windows.h>
#include "data_types.h"

constexpr int getCoord(int u)
{
	return u - 1;
}

constexpr int getSquareAera(int x, int y)
{
	return x * y;
}

void drawScreenBorders(GameField, HANDLE);
bool getPressedKey(KEY_EVENT_RECORD&, HANDLE);
inline void changeDirection(Direction, GameField&);
bool checkFruitCollision(GameField);
bool checkSnakeCollision(GameField);
bool checkWin(GameField);
void moveFruit(GameField&);
void onEatFruit(GameField&);
void drawFruit(GameField, HANDLE);
inline void pushSnake(GameField&);
void updateSnake(GameField, Point, Point, UINT, HANDLE);

int main()
{
	GameField gameField;
	KEY_EVENT_RECORD key;

	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);

	{
		CONSOLE_CURSOR_INFO cursorInfo;
		cursorInfo.dwSize = 100;
		cursorInfo.bVisible = FALSE;
		SetConsoleCursorInfo(hStdout, &cursorInfo);
	}

	gameField.fieldInfo.x = getCoord(20);
	gameField.fieldInfo.y = getCoord(10);

	gameField.snake.headPosition.x = 1;
	gameField.snake.headPosition.y = 1;


	gameField.snake.direction = DOWN;

	drawScreenBorders(gameField, hStdout);

	moveFruit(gameField);
	drawFruit(gameField, hStdout);

	while (1)
	{
		if (getPressedKey(key, hStdin))
		{
			switch (key.wVirtualKeyCode)
			{
			case KEY_UP:
			case KEY_W:
				changeDirection(UP, gameField);
				break;

			case KEY_LEFT:
			case KEY_A:
				changeDirection(LEFT, gameField);
				break;

			case KEY_DOWN:
			case KEY_S:
				changeDirection(DOWN, gameField);
				break;

			case KEY_RIGHT:
			case KEY_D:
				changeDirection(RIGHT, gameField);
				break;

			case KEY_X:
				return 0;
			}
		}

		if (checkFruitCollision(gameField))
		{
			onEatFruit(gameField);
			moveFruit(gameField);
			drawFruit(gameField, hStdout);
		}
		Point oldHeadPosition = gameField.snake.headPosition;
		Point oldTailEnd = gameField.snake.tail.size() != 0 ? gameField.snake.tail.back() : Point();
		UINT oldTailSize = gameField.snake.tail.size();
		pushSnake(gameField);
		if (checkSnakeCollision(gameField))
		{
			DWORD dwBytesWritten;
			WriteConsoleOutputCharacterW(hStdout, L"Game Over!", 10, { 0, (SHORT)gameField.fieldInfo.y + 1 }, &dwBytesWritten);
			getchar();
			return 0;
		}
		else if (checkWin(gameField))
		{
			DWORD dwBytesWritten;
			WriteConsoleOutputCharacterW(hStdout, L"You win!", 8, { 0, (SHORT)gameField.fieldInfo.y + 1 }, &dwBytesWritten);
			getchar();
			return 0;
		}
		else
		{
			updateSnake(gameField, oldHeadPosition, oldTailEnd, oldTailSize, hStdout);
		}
		
		Sleep(200);
	}
}

void drawScreenBorders(GameField field, HANDLE hOut)
{
	DWORD dwBytesWritten;
	for (int iterX = 0; iterX <= field.fieldInfo.x; iterX++)
	{
		for (int iterY = 0; iterY <= field.fieldInfo.y; iterY++)
		{	
			if (iterX == 0 || iterY == 0 || iterX == field.fieldInfo.x || iterY == field.fieldInfo.y)
				WriteConsoleOutputCharacterW(hOut, L"#", 1, { (SHORT)iterX, (SHORT)iterY }, &dwBytesWritten);
		}
	}
}

bool getPressedKey(KEY_EVENT_RECORD& krec, HANDLE hIn)
{
	if (hIn == NULL)
	{
		throw - 1;
	}

	DWORD eventCount;
	GetNumberOfConsoleInputEvents(hIn, &eventCount);

	if (eventCount == 0)
	{
		return false;
	}
	else
	{
		DWORD cc;
		INPUT_RECORD irec;

		ReadConsoleInputW(hIn, &irec, 1, &cc);
		FlushConsoleInputBuffer(hIn);
		if (irec.EventType == KEY_EVENT && ((KEY_EVENT_RECORD&)irec.Event).bKeyDown)
		{
			krec = (KEY_EVENT_RECORD&)irec.Event;
			return true;
		}
		return false;
	}
}

inline void changeDirection(Direction inputDirection, GameField& field)
{
	if (inputDirection != field.snake.direction && !(inputDirection == UP && field.snake.direction == DOWN
		|| inputDirection == LEFT && field.snake.direction == RIGHT
		|| inputDirection == DOWN && field.snake.direction == UP
		|| inputDirection == RIGHT && field.snake.direction == LEFT))
	{
		field.snake.direction = inputDirection;
	}
}

bool checkFruitCollision(GameField field)
{
	return field.snake.headPosition.x == field.fruitPosition.x && field.snake.headPosition.y == field.fruitPosition.y;
}

bool checkSnakeCollision(GameField field)
{
	for (int i = 0; i < field.snake.tail.size(); i++)
		if (field.snake.tail[i].x == field.snake.headPosition.x && field.snake.tail[i].y == field.snake.headPosition.y)
			return true;
	return false;
}

bool checkWin(GameField field)
{
	return getSquareAera(field.fieldInfo.x, field.fieldInfo.y) == field.snake.tail.size() + 1;
}

void moveFruit(GameField& field)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distX(1, field.fieldInfo.x - 1);
	std::uniform_int_distribution<> distY(1, field.fieldInfo.y - 1);

	while (1)
	{
		Point fruitPosition = { distX(gen), distY(gen) };

		if (field.snake.headPosition.x == fruitPosition.x && field.snake.headPosition.y == fruitPosition.y)
			goto regenerate;
		else if (field.snake.tail.size() != 0)
			for (int i = 0; i < field.snake.tail.size(); i++)
			{
				if (field.snake.tail[i].x == fruitPosition.x && field.snake.tail[i].y == fruitPosition.y)
					goto regenerate;
			}

		field.fruitPosition = fruitPosition;
		break;
	regenerate:
		continue;
	}
}

void onEatFruit(GameField& field)
{
	field.snake.tail.push_back({ 1, 1 });
}

void drawFruit(GameField field, HANDLE hStdout)
{
	DWORD dwBytesWritten;
	WriteConsoleOutputCharacterW(hStdout, L"F", 1, { (SHORT)field.fruitPosition.x, (SHORT)field.fruitPosition.y }, &dwBytesWritten);
}

inline void pushSnake(GameField& field)
{
	Point oldHeadPos = field.snake.headPosition;
	switch (field.snake.direction)
	{
	case UP:
		if (field.snake.headPosition.y != 1 && field.snake.direction == UP)
			field.snake.headPosition.y--;
		else
			field.snake.headPosition.y = field.fieldInfo.y - 1;
		break;
	case LEFT:
		if (field.snake.headPosition.x != 1 && field.snake.direction == LEFT)
			field.snake.headPosition.x--;
		else
			field.snake.headPosition.x = field.fieldInfo.x - 1;
		break;
	case DOWN:
		if (field.snake.headPosition.y != field.fieldInfo.y - 1 && field.snake.direction == DOWN)
			field.snake.headPosition.y++;
		else
			field.snake.headPosition.y = 1;
		break;
	case RIGHT:
		if (field.snake.headPosition.x != field.fieldInfo.x - 1 && field.snake.direction == RIGHT)
			field.snake.headPosition.x++;
		else
			field.snake.headPosition.x = 1;
		break;
	}

	if (field.snake.tail.size() != 0)
	{

		field.snake.tail.insert(field.snake.tail.begin(), oldHeadPos);
		field.snake.tail.pop_back();
	}
}

void updateSnake(GameField field, Point oldHeadPosition, Point oldTailEnd, UINT oldTailSize, HANDLE hOut)
{
	DWORD dwBytesWritten;

	if (oldTailSize != 0)
		WriteConsoleOutputCharacterW(hOut, L" ", 1, { (SHORT)oldTailEnd.x, (SHORT)oldTailEnd.y }, &dwBytesWritten);
	else
		WriteConsoleOutputCharacterW(hOut, L" ", 1, { (SHORT)oldHeadPosition.x, (SHORT)oldHeadPosition.y }, &dwBytesWritten);

	if (field.snake.tail.size() != 0)
		WriteConsoleOutputCharacterW(hOut, L"o", 1, { (SHORT)field.snake.tail[0].x, (SHORT)field.snake.tail[0].y }, &dwBytesWritten);

	/*for (int i = 0; i < field.snake.tail.size(); i++)
	{
		WriteConsoleOutputCharacterW(hOut, L"o", 1, { (SHORT)field.snake.tail[i].x, (SHORT)field.snake.tail[i].y }, &dwBytesWritten);
	}*/

	WriteConsoleOutputCharacterW(hOut, L"0", 1, { (SHORT)field.snake.headPosition.x, (SHORT)field.snake.headPosition.y }, &dwBytesWritten);
}