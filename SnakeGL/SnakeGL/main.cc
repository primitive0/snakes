#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <random>
#include <vector>
#include "data_types.h"

#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "msvcmrt.lib")
#pragma comment(lib, "msvcrt.lib")

#define OBJECTS_SIZE 60
#define BETWEEN_PARTS 5

GLuint shaderProgram;
int globalWidth, globalHeight;
int keyEvent;

void keyCallback(GLFWwindow*, int, int, int, int);
void resizeCallback(GLFWwindow*, int, int);
void drawSquare(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLenum);
void moveFruit(GameField&);
void updateScreen(GameField);
bool checkFruitCollision(GameField);
inline void changeDirection(Direction, GameField&);
inline void pushSnake(GameField&);
void onEatFruit(GameField&);
bool checkSnakeCollision(GameField);
bool checkWin(GameField);

constexpr int getCoord(int any)
{
	return any - 1;
}

constexpr int getSquareAera(int x, int y)
{
	return x * y;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	globalWidth = 800;
	globalHeight = 600;

	GLFWwindow* gameWindow = glfwCreateWindow(globalWidth, globalHeight, "SnakeGL", nullptr, nullptr);
	if (gameWindow == nullptr)
	{
		MessageBoxW(GetForegroundWindow(), L"Ошибка инициализации окна", L"Ошибка", MB_OK | MB_ICONERROR | MB_TOPMOST);
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(gameWindow);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		MessageBoxW(GetForegroundWindow(), L"Ошибка инициализации GLEW", L"Ошибка", MB_OK | MB_ICONERROR | MB_TOPMOST);
		glfwTerminate();
		return -1;
	}

	{
		const GLchar* vertexShaderSource = "#version 330 core\n" //Перенести в отдельный файл
			"layout (location = 0) in vec3 position;\n"
			"void main()\n"
			"{\n"
			"gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
			"}\0";
		const GLchar* fragmentShaderSource = "#version 330 core\n"
			"uniform vec4 inColor;\n"
			"out vec4 color;\n"
			"void main()\n"
			"{\n"
			"color = inColor;\n"
			"}\n\0";

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);

		shaderProgram = glCreateProgram();

		GLint success;
		//GLchar infoLog[512];
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			//glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			MessageBoxW(GetForegroundWindow(), L"Ошибка компиляции шейдра, игра может работать не корректно.", L"Ошибка", MB_OK | MB_ICONWARNING | MB_TOPMOST);
		}

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);

		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			//glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
			MessageBoxW(GetForegroundWindow(), L"Ошибка компиляции шейдра, игра может работать не корректно.", L"Ошибка", MB_OK | MB_ICONWARNING | MB_TOPMOST);
		}

		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);

		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
		if (!success)
		{
			//glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
			MessageBoxW(GetForegroundWindow(), L"Ошибка линковки программы, игра может работать не корректно.", L"Ошибка", MB_OK | MB_ICONWARNING | MB_TOPMOST);
		}
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	glfwSetKeyCallback(gameWindow, keyCallback);
	glfwSetWindowSizeCallback(gameWindow, resizeCallback);

	GameField gameField;

	gameField.fieldInfo.x = getCoord(10);
	gameField.fieldInfo.y = getCoord(10);

	gameField.snake.headPosition.x = 0;
	gameField.snake.headPosition.y = 0;

	gameField.snake.direction = DOWN;

	moveFruit(gameField);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	while (1)
	{
		if (glfwWindowShouldClose(gameWindow))
		{
			break;
		}

		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT);

		switch (keyEvent)
		{
		case KEY_UP:
			changeDirection(UP, gameField);
			break;
		case KEY_LEFT:
			changeDirection(LEFT, gameField);
			break;
		case KEY_DOWN:
			changeDirection(DOWN, gameField);
			break;
		case KEY_RIGHT:
			changeDirection(RIGHT, gameField);
			break;
		default:
			break;
		}


		if (checkFruitCollision(gameField))
		{
			onEatFruit(gameField);
			moveFruit(gameField);
		}

		pushSnake(gameField);

		if (checkSnakeCollision(gameField))
		{
			break;
		}
		else if (checkWin(gameField))
		{
			break;
		}
		else
		{
			updateScreen(gameField);
		}

		glfwSwapBuffers(gameWindow);

		Sleep(200);

	}

	glfwTerminate();

	return 0;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_W:
		case GLFW_KEY_UP:
			keyEvent = KEY_UP;
			break;
		case GLFW_KEY_A:
		case GLFW_KEY_LEFT:
			keyEvent = KEY_LEFT;
			break;
		case GLFW_KEY_S:
		case GLFW_KEY_DOWN:
			keyEvent = KEY_DOWN;
			break;
		case GLFW_KEY_D:
		case GLFW_KEY_RIGHT:
			keyEvent = KEY_RIGHT;
			break;
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;
		default:
			keyEvent = NOTHING_KEY;
			break;
		}
	}
}

void resizeCallback(GLFWwindow* window, int width, int height)
{
	globalWidth = width;
	globalHeight = height;
	glViewport(0, 0, width, height);
}

void drawSquare(GLfloat x, GLfloat y, GLfloat xEnd, GLfloat yEnd, GLfloat colorRed, GLfloat colorGreen, GLfloat colorBlue, GLfloat colorAlpha, GLenum drawType)
{
	GLfloat vertices[] = {
		(xEnd - globalWidth / 2) / (globalWidth / 2), -((y - globalHeight / 2) / (globalHeight / 2)), 0.0f,
		(xEnd - globalWidth / 2) / (globalWidth / 2), -((yEnd - globalHeight / 2) / (globalHeight / 2)), 0.0f,
		(x - globalWidth / 2) / (globalWidth / 2), -((yEnd - globalHeight / 2) / (globalHeight / 2)), 0.0f,
		(x - globalWidth / 2) / (globalWidth / 2),  -((y - globalHeight / 2) / (globalHeight / 2)), 0.0f
	};

	GLuint indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	GLuint VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, drawType);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, drawType);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glUseProgram(shaderProgram);

	glProgramUniform4f(shaderProgram, glGetUniformLocation(shaderProgram, "inColor"), colorRed, colorGreen, colorBlue, colorAlpha);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glUseProgram(0);
}


void updateScreen(GameField field)
{
	drawSquare(0, 0, field.fieldInfo.x * OBJECTS_SIZE + OBJECTS_SIZE - BETWEEN_PARTS, field.fieldInfo.y * OBJECTS_SIZE + OBJECTS_SIZE - BETWEEN_PARTS, 0.26f, 0.28f, 0.32f, 1.0f, GL_STATIC_DRAW);
	drawSquare(field.fruitPosition.x * OBJECTS_SIZE, field.fruitPosition.y * OBJECTS_SIZE, field.fruitPosition.x * OBJECTS_SIZE + OBJECTS_SIZE - BETWEEN_PARTS, field.fruitPosition.y * OBJECTS_SIZE + OBJECTS_SIZE - BETWEEN_PARTS, 0.984f, 0.11f, 0.369f, 1.0f, GL_STATIC_DRAW);

	drawSquare(field.snake.headPosition.x * OBJECTS_SIZE, field.snake.headPosition.y * OBJECTS_SIZE, field.snake.headPosition.x * OBJECTS_SIZE + OBJECTS_SIZE - BETWEEN_PARTS, field.snake.headPosition.y * OBJECTS_SIZE + OBJECTS_SIZE - BETWEEN_PARTS, 1.0f, 1.0f, 1.0f, 1.0f, GL_DYNAMIC_DRAW);

	if (field.snake.tail.size() != 0)
		for (int i = 0; i < field.snake.tail.size(); i++)
			drawSquare(field.snake.tail[i].x * OBJECTS_SIZE, field.snake.tail[i].y * OBJECTS_SIZE, field.snake.tail[i].x * OBJECTS_SIZE + OBJECTS_SIZE - BETWEEN_PARTS, field.snake.tail[i].y * OBJECTS_SIZE + OBJECTS_SIZE - BETWEEN_PARTS, 1.0f, 1.0f, 1.0f, 1.0f, GL_DYNAMIC_DRAW);
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

inline void pushSnake(GameField& field)
{
	Point oldHeadPos = field.snake.headPosition;
	switch (field.snake.direction)
	{
	case UP:
		if (field.snake.headPosition.y != 0 && field.snake.direction == UP)
			field.snake.headPosition.y--;
		else
			field.snake.headPosition.y = field.fieldInfo.y;
		break;
	case LEFT:
		if (field.snake.headPosition.x != 0 && field.snake.direction == LEFT)
			field.snake.headPosition.x--;
		else
			field.snake.headPosition.x = field.fieldInfo.x;
		break;
	case DOWN:
		if (field.snake.headPosition.y != field.fieldInfo.y && field.snake.direction == DOWN)
			field.snake.headPosition.y++;
		else
			field.snake.headPosition.y = 0;
		break;
	case RIGHT:
		if (field.snake.headPosition.x != field.fieldInfo.x && field.snake.direction == RIGHT)
			field.snake.headPosition.x++;
		else
			field.snake.headPosition.x = 0;
		break;
	}

	if (field.snake.tail.size() != 0)
	{
		field.snake.tail.insert(field.snake.tail.begin(), oldHeadPos);
		field.snake.tail.pop_back();
	}
}

bool checkFruitCollision(GameField field)
{
	return field.snake.headPosition.x == field.fruitPosition.x && field.snake.headPosition.y == field.fruitPosition.y;
}

void onEatFruit(GameField& field)
{
	field.snake.tail.push_back({ 1, 1 });
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
