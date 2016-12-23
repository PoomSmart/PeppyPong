/*
	The Pong Game
	Developed by PoomSmart
	Best design for size 121x25
	Not for commercial uses
	Redistribution is not permitted
*/

// day1 pc1 136
// day1 pc2 163
// day2 pc1 179
// day2 pc2 192

#include <iostream>
#include <string>
#include <cstdio>
#include <conio.h>
#include <windows.h>
#include <ctime>
#include <cstdlib>
#include <cmath>
using namespace std;

#define KEY_UP 'w' // 72
#define KEY_DOWN 's' // 80
#define WIDTH 119
#define HEIGHT 25
#define GAP_X 9
#define GAP_Y 2
const char *BAR = "±";
#define BALL 254
#define HORIZONTAL 1
#define VERTICAL 2
#define SCORE 3
#define DIV 4
#define D 1
#define div '³'
#define MAXPL 500
#define MAXPLNAMEL 20
#define SCORETXT "score.post.txt"

#define GOD_MODE
//#define MODERN

short stage[WIDTH][HEIGHT];
bool ball[WIDTH][HEIGHT];
bool mine[HEIGHT];
bool com[HEIGHT];
bool repeat;
char last;
int level;
int speed;

struct scorest {
	char name[MAXPLNAMEL];
	double absScore;
	int player;
	int computer;
	int level;
	int speed;
};
struct scorest currentScore;

typedef struct obj {
	short x;
	short y;
} obj;
obj *mine_u, *com_u, *ball_;

bool BOT = true;
bool AUTO = false;
short BALL_dx = 2;
short BALL_dy = 1;
short myScore, comScore = 0;
int turn = 0;

int LENGTH_ME;
int LENGTH_COM;

void reset();
void updateScore();
void showResult();
void setDefaultValues();
void setDefaultColor();
void setKeyboardRate(const char *, const char *);

void setKeyboardDefaultRate()
{
	setKeyboardRate("1000", "90");	
}

void setDefaultValues()
{
	BALL_dx = 2;
	BALL_dy = 1;
	myScore = comScore = turn = 0;
	BOT = true;
	AUTO = false;
	currentScore.name[0] = '\0';
	currentScore.absScore = 0.0;
	currentScore.player = currentScore.computer = currentScore.level = currentScore.speed = 0;
	srand(time(NULL));
}

void consolePreparation()
{
	HANDLE wHnd = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE rHnd = GetStdHandle(STD_INPUT_HANDLE);
	CONSOLE_FONT_INFOEX info = { 0 };
    info.cbSize = sizeof(info);
    info.nFont = 9;
	info.dwFontSize.X = 10;
	info.dwFontSize.Y = 20;
	info.FontFamily = 48;
	info.FontWeight = FW_NORMAL;
    wcscpy(info.FaceName, L"Terminal");
    SetCurrentConsoleFontEx(wHnd, 0, &info);
	SetConsoleTitle("Peppy Pong");
	SMALL_RECT size = { 0, 0, WIDTH + 2, HEIGHT };
	SetConsoleWindowInfo(wHnd, 1, &size);
	COORD bufferSize = { 10, 10 };
	SetConsoleScreenBufferSize(wHnd, bufferSize);
	/*CONSOLE_SCREEN_BUFFER_INFO info2;
    GetConsoleScreenBufferInfo(wHnd, &info2);
    COORD new_size = {
        info2.srWindow.Right - info2.srWindow.Left + 1,
        info2.srWindow.Bottom - info2.srWindow.Top + 1
    };
    SetConsoleScreenBufferSize(wHnd, new_size);*/
    CONSOLE_CURSOR_INFO CursorInfo;
	CursorInfo.dwSize = 1;
	CursorInfo.bVisible = false;
	SetConsoleCursorInfo(wHnd, &CursorInfo);
}

void gotoxy(short x, short y)
{
	COORD pos = { x, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

bool parseDword(const char* in, DWORD* out)
{
	char* end;
	long result = strtol(in, &end, 10);
	bool success = (errno == 0 && end != in);
	if (success)
		*out = result;
	return success;
}

void setKeyboardRate(const char *delay, const char *repeat)
{
	FILTERKEYS keys = { sizeof(FILTERKEYS) };
	if (parseDword(delay, &keys.iDelayMSec) && parseDword(repeat, &keys.iRepeatMSec))
		keys.dwFlags = FKF_FILTERKEYSON | FKF_AVAILABLE;
	SystemParametersInfo(SPI_SETFILTERKEYS, 0, (LPVOID) &keys, 0);
}

void translate(int type, short j)
{
	if (type < 0 || type > 1)
		return;
	obj *bar = type == 0 ? mine_u : com_u;
	short LENGTH = type == 0 ? LENGTH_ME : LENGTH_COM;
	short i;
	short ux = bar->x;
	short uy = bar->y + j;
	short direction = j / abs(j);
	if (j > 0) uy += LENGTH - 1;
	for (i = 0; i < abs(j); i++)
	{
		gotoxy(ux, uy - i * direction);
		if (type == 0)
			mine[uy - i * direction] = true;
		else
			com[uy - i * direction] = true;
		printf(BAR);
	}
	if (j > 0) uy -= LENGTH;
	if (j < 0) uy += LENGTH;
	for (i = 0; i < abs(j); i++)
	{
		gotoxy(ux, uy - i * direction);
		if (type == 0)
			mine[uy - i * direction] = false;
		else
			com[uy - i * direction] = false;
		printf(" ");
	}
	bar->y += j;	
}

bool moveMe(char key)
{
	short j;
	short direction;
	switch (key)
	{
		case 72:
		case KEY_UP:
			direction = -1;
			break;
		case 80:
		case KEY_DOWN:
			direction = 1;
			break;
		default:
			return true;
	}
	j = D * direction;
	if (mine_u->y + j > 0 && mine_u->y + LENGTH_ME + j < HEIGHT)
	{
		// move our bar until it hits border
		translate(0, j);
	}
	return true;
}

bool moveBot()
{
	short bx = ball_->x;
	short by = ball_->y;
	short j = BALL_dy/abs(BALL_dy);
	if (com_u->y + j > 0 && com_u->y + LENGTH_COM + j < HEIGHT && bx <= WIDTH - GAP_X - 1 && bx > GAP_X)
	{
		if (level < 3)
		{
			// my calculation for which x-position of the ball bot should aware of
			// based on trigonometric ball path prediction
			// appropriate movement direction depended on ball direction vector
			// it may not be necessary, as we just follow y-position of the ball and then we should be fine
			// anyway, this locks the pattern of gameplay if both auto and bot mode activated
			// we should add some randomization to it
			short fearX = WIDTH - GAP_X - 1 - 0.25 * LENGTH_COM;
			bool fearReach = bx > WIDTH / 2 && (rand() % 100) > (33 + (level == 1 ? rand() % 7 : 15) * (2 - level));
			if (fearReach && BALL_dx > 0)
			{
				if (com_u->y < HEIGHT - LENGTH_COM - 2*(WIDTH - bx - GAP_X - 1))
				{
					if ((BALL_dy > 0 && j < 0) || (BALL_dy < 0 && j > 0))
						j *= -1;
				}
				translate(1, j);
			}
		} else
			translate(1, j);
	}
	return true;
}

bool autoPlay(bool *repeat)
{
	short bx = ball_->x;
	short by = ball_->y;
	short j = D * BALL_dy/abs(BALL_dy);
	if (BALL_dx < 0)
		moveMe(j > 0 ? KEY_DOWN : KEY_UP);
	return true;
}

void input(bool *repeat, char *last)
{
	char key = getch();
	switch (key)
	{
		case 27:
			setKeyboardDefaultRate();
			*repeat = false;
			break;
		case 72:
        case KEY_UP:
			*repeat = moveMe(key);
			break;
		case 80:
        case KEY_DOWN:
			*repeat = moveMe(key);
			break;
		case 'b':
			BOT = !BOT;
			break;
		case 'l':
			AUTO = !AUTO;
			break;
		case 'p':
			gotoxy(0, 22);
			system("pause");
			gotoxy(0, 22);
			printf("                               ");
			break;
		#ifdef GOD_MODE
		case 'h': // HACK
			myScore++;
			updateScore();
			if (myScore == 9)
			{
				showResult();
				*repeat = false;
			}
			break;
		case 'j': // BE FAIR
			comScore++;
			updateScore();
			if (comScore == 9)
			{
				showResult();
				*repeat = false;
			}
			break;
		#endif
		case 'k':
			level = 3;
			break;
		default:
			break;
	}
	*last = key;
}

void createStage()
{
	short i, j;
	for (i = 0; i < HEIGHT; i++)
	{
		for (j = 0; j < WIDTH; j++)
			stage[j][i] = 0;
	}
}

void createDiv()
{
	short j, i = WIDTH / 2;
	for (j = 1; j < HEIGHT - 1; j++)
	{
		gotoxy(i, j);
		putchar(div);
		stage[i][j] = DIV;
	}		
}

void createMine()
{
	short x = GAP_X + 1;
	short y;
	mine_u = (obj *)malloc(sizeof(obj));
	for (short i = 0; i < LENGTH_ME; i++)
	{
		y = (HEIGHT - LENGTH_ME)/2 + i;
		gotoxy(x, y);
		printf(BAR);
		if (i != 0 && i != HEIGHT - 1)
			mine[y] = true;
		if (i == 0)
		{
			mine_u->x = x;
			mine_u->y = y;
		}
	}
}

void createCom()
{
	short x = WIDTH - GAP_X;
	short y;
	com_u = (obj *)malloc(sizeof(obj));
	for (short i = 0; i < LENGTH_COM; i++)
	{
		y = (HEIGHT - LENGTH_COM)/2 + i;
		gotoxy(x, y);
		printf(BAR);
		if (i != 0 && i != HEIGHT - 1)
			com[y] = true;
		if (i == 0)
		{
			com_u->x = x;
			com_u->y = y;
		}
	}
}

void createBall()
{
	short x = WIDTH / 2;
	short y = HEIGHT / 2 + pow(-1, rand() % 2);
	gotoxy(x, y);
	ball_ = (obj *)malloc(sizeof(obj));
	ball_->x = x;
	ball_->y = y;
	putchar(BALL);
	ball[x][y] = true;
	gotoxy(x, y);
}

void pxy(string str, short x, short y)
{
	gotoxy(x, y);
	cout << str;
}

void printnum(short x, short y, int num)
{
	switch (num)
	{
		case 0:
			pxy("±±±±", x, y);
			pxy("±  ±", x, y + 1); stage[x + 1][y + 1] = stage[x + 2][y + 1] = 0;
			pxy("±  ±", x, y + 2); stage[x + 1][y + 2] = stage[x + 2][y + 2] = 0;
			pxy("±  ±", x, y + 3); stage[x + 1][y + 3] = stage[x + 2][y + 3] = 0;
			pxy("±±±±", x, y + 4);
			break;
		case 1:
			pxy("±±± ", x, y); stage[x + 3][y] = 0;
			pxy("  ± ", x, y + 1); stage[x][y + 1] = stage[x + 1][y + 1] = stage[x + 3][y + 1] = 0;
			pxy("  ± ", x, y + 2); stage[x][y + 2] = stage[x + 1][y + 2] = stage[x + 3][y + 2] = 0;
			pxy("  ± ", x, y + 3); stage[x][y + 3] = stage[x + 1][y + 3] = stage[x + 3][y + 3] = 0;
			pxy("±±±±", x, y + 4);
			break;
		case 2:
			pxy("±±±±", x, y);
			pxy("   ±", x, y + 1); stage[x][y + 1] = stage[x + 1][y + 1] = stage[x + 2][y + 1] = 0;
			pxy("±±±±", x, y + 2);
			pxy("±   ", x, y + 3); stage[x + 1][y + 3] = stage[x + 2][y + 3] = stage[x + 3][y + 3] = 0;
			pxy("±±±±", x, y + 4);
			break;
		case 3:
			pxy("±±±±", x, y);
			pxy("   ±", x, y + 1); stage[x][y + 1] = stage[x + 1][y + 1] = stage[x + 2][y + 1] = 0;
			pxy("±±±±", x, y + 2);
			pxy("   ±", x, y + 3); stage[x][y + 3] = stage[x + 1][y + 3] = stage[x + 2][y + 3] = 0;
			pxy("±±±±", x, y + 4);
			break;
		case 4:
			pxy("±  ±", x, y); stage[x + 1][y] = stage[x + 2][y] = 0;
			pxy("±  ±", x, y + 1); stage[x + 1][y + 1] = stage[x + 2][y + 1] = 0;
			pxy("±±±±", x, y + 2);
			pxy("   ±", x, y + 3); stage[x][y + 3] = stage[x + 1][y + 3] = stage[x + 2][y + 3] = 0;
			pxy("   ±", x, y + 4); stage[x][y + 4] = stage[x + 1][y + 4] = stage[x + 2][y + 4] = 0;
			break;
		case 5:
			pxy("±±±±", x, y);
			pxy("±   ", x, y + 1); stage[x + 1][y + 1] = stage[x + 2][y + 1] = stage[x + 3][y + 1] = 0;
			pxy("±±±±", x, y + 2);
			pxy("   ±", x, y + 3); stage[x][y + 3] = stage[x + 1][y + 3] = stage[x + 2][y + 3] = 0;
			pxy("±±±±", x, y + 4);
			break;
		case 6:
			pxy("±±±±", x, y);
			pxy("±   ", x, y + 1); stage[x + 1][y + 1] = stage[x + 2][y + 1] = stage[x + 3][y + 1] = 0;
			pxy("±±±±", x, y + 2);
			pxy("±  ±", x, y + 3); stage[x + 1][y + 3] = stage[x + 2][y + 3] = 0;
			pxy("±±±±", x, y + 4);
			break;
		case 7:
			pxy("±±±±", x, y);
			pxy("   ±", x, y + 1); stage[x][y + 1] = stage[x + 1][y + 1] = stage[x + 2][y + 1] = 0;
			pxy("   ±", x, y + 2); stage[x][y + 2] = stage[x + 1][y + 2] = stage[x + 2][y + 2] = 0;
			pxy("   ±", x, y + 3); stage[x][y + 3] = stage[x + 1][y + 3] = stage[x + 2][y + 3] = 0;
			pxy("   ±", x, y + 4); stage[x][y + 4] = stage[x + 1][y + 4] = stage[x + 2][y + 4] = 0;
			break;
		case 8:
			pxy("±±±±", x, y);
			pxy("±  ±", x, y + 1); stage[x + 1][y + 1] = stage[x + 2][y + 1] = 0;
			pxy("±±±±", x, y + 2);
			pxy("±  ±", x, y + 3); stage[x + 1][y + 3] = stage[x + 2][y + 3] = 0;
			pxy("±±±±", x, y + 4);
			break;
		case 9:
			pxy("±±±±", x, y);
			pxy("±  ±", x, y + 1); stage[x + 1][y + 1] = stage[x + 2][y + 1] = 0;
			pxy("±±±±", x, y + 2);
			pxy("   ±", x, y + 3); stage[x][y + 3] = stage[x + 1][y + 3] = stage[x + 2][y + 3] = 0;
			pxy("±±±±", x, y + 4);
			break;
		case -1:
			pxy("    ", x, y);
			pxy("    ", x, y + 1);
			pxy("    ", x, y + 2);
			pxy("    ", x, y + 3);
			pxy("    ", x, y + 4);
			for (short i = x; i <= x + 3; i++)
			{
				for (short j = y; j <= y + 4; j++)
					stage[i][j] = 0;	
			}
			break;
		default:
			break;
	}
}

void registerScoreArea(int side) // -1 or 1
{
	short mid = WIDTH / 2;
	short i, j;
	for (i = 1; i <= 4; i++)
	{
		for (j = 1; j <= 5; j++)
		{
			if (side == 1)
				stage[mid + 15 + i - 1][GAP_Y + j - 1] = SCORE;
			else
				stage[mid - (15 + 4) + i - 1][GAP_Y + j - 1] = SCORE;
		}
	}
}

void updateScore()
{
	short mid = WIDTH / 2;
	short x, y;
	y = GAP_Y;
	x = mid - (15 + 4);
	printnum(x, y, -1);
	registerScoreArea(-1);
	printnum(x, y, myScore);
	x = mid + 15;
	printnum(x, y, -1);
	registerScoreArea(1);
	printnum(x, y, comScore);
	if (comScore - myScore >= 3)
	{
		if (level == 4)
			system("color FC");
		else
			system("color CF");
	}
	else if (myScore - comScore >= 3)
	{
		if (level == 4)
			system("color F9");
		else
			system("color 9F");
	}
	else
	{
		if (level == 4)
			system("color F0");
		else
			system("color 0F");
	}
}

void moveBall(bool *repeat)
{
	short ix = ball_->x;
	short iy = ball_->y;
	// runs our bot via ball movement
	ball[ix][iy] = false;
	gotoxy(ix, iy);
	if (stage[ix][iy] == 0)
	{
		if ((ix == GAP_X + 1 && mine[iy]) || (ix == WIDTH - GAP_X && com[iy])); // ?
		else
		{
			if (stage[ix][iy] == SCORE)
				putchar('±');
			else if (stage[ix][iy] == DIV)
				putchar(div);
			else
				putchar(' ');
		}
	}
	else if (stage[ix][iy] == SCORE)
		putchar('±');
	else if (stage[ix][iy] == DIV)
		putchar(div);
	if (ix <= GAP_X)
	{
		comScore++;
		turn = 0;
		#ifdef MODERN
		putchar('\a');
		#endif
		updateScore();
		if (comScore == 9)
		{
			showResult();
			*repeat = false;
			return;	
		}
		reset();
		return;
	}
	else if (ix > WIDTH - GAP_X)
	{
		myScore++;
		turn = 1;
		#ifdef MODERN
		putchar('\a');
		#endif
		updateScore();
		if (myScore == 9)
		{
			showResult();
			*repeat = false;
			return;
		}
		reset();
		return;
	}
	short x = ix + BALL_dx;
	short y = iy + BALL_dy;
	#ifdef DEBUG
	gotoxy(WIDTH - 21, 21);
	printf("Ball {%02d, %02d}", ball_->x, ball_->y);
	#endif
	// dirty calculation for ball collision in every cases, any better solution ?
	if (y < 1 || y > HEIGHT - 2)
	{
		// ball hits top or bottom border
		BALL_dy *= -1;
	}
	if (((ix <= com_u->x && com_u->x <= x) || (x <= com_u->x && com_u->x <= ix)) && ((iy >= com_u->y && BALL_dy < 0 && iy <= com_u->y + LENGTH_COM) || (com_u->y <= iy && iy <= com_u->y + LENGTH_COM && BALL_dy > 0)))
	{
		BALL_dx *= -1;
		if (level > 0 && ((abs(iy - com_u->y) < 2) || (abs(iy - com_u->y + LENGTH_COM) < 2)))
		{
			if (abs(BALL_dx) < 3)
				BALL_dx *= 1.5;
			else if (abs(BALL_dx) > 2)
				BALL_dx *= 2 / 3.0;
		}
	}
	if (((ix <= mine_u->x && mine_u->x <= x) || (x <= mine_u->x && mine_u->x <= ix)) && ((iy >= mine_u->y && BALL_dy < 0 && iy <= mine_u->y + LENGTH_ME) || (mine_u->y <= iy && iy <= mine_u->y + LENGTH_ME && BALL_dy > 0)))
	{
		if (level != 4)
			BALL_dx *= -1;
		if (level > 0 && ((abs(iy - mine_u->y) < 2) || (abs(iy - mine_u->y + LENGTH_ME) < 2)))
			if (abs(BALL_dx) < 3)
				BALL_dx *= 1.5;
			else if (abs(BALL_dx) > 2)
				BALL_dx *= 2 / 3.0;
	}
	if ((x <= 0 || x >= WIDTH) && (y <= 0 || y >= HEIGHT))
	{
		// in case the ball moves towards corner, invert all vectors to prevent crashing
		BALL_dx *= -1;
		BALL_dy *= -1;
	}
	ball_->x += BALL_dx;
	ball_->y += BALL_dy;
	gotoxy(ball_->x, ball_->y);
	if (stage[ball_->x][ball_->y] == 0)
		putchar(BALL);
	ball[ball_->x][ball_->y] = true;
}

void reset()
{
	updateScore();
	BALL_dx = 2;
	if (turn > 0)
		BALL_dx *= -1;
	BALL_dy = pow(-1, rand() % 2);
	gotoxy(ball_->x, ball_->y);
	putchar(' ');
	free(ball_);
	createBall();
	putchar(' ');
}

void setDefaultColor()
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
}

void welcomeMessage()
{
	short mX = WIDTH / 2;
	short mY = HEIGHT / 2;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	gotoxy(mX - 25, mY - 6); printf(" ÄÄÄ");               
	gotoxy(mX - 25, mY - 5); printf("| Ä \\ÄÄÄ Ä ÄÄ Ä ÄÄ Ä  Ä");
	gotoxy(mX - 25, mY - 4); printf("|  Ä/ -Ä) 'Ä \\ 'Ä \\ || |");
	gotoxy(mX - 25, mY - 3); printf("|Ä| \\ÄÄÄ| .ÄÄ/ .ÄÄ/\\Ä, |");
	gotoxy(mX - 25, mY - 2); printf("        |Ä|  |Ä|   |ÄÄ/");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	gotoxy(mX + 19, mY - 1); printf("By PoomSmart");
	setDefaultColor();
	gotoxy(mX - 28, mY); printf("ÄÄÄÄÄÄÄÄÄoÄÄÄÄÄÄÄÄÄoÄÄÄÄÄÄÄÄÄoÄÄÄÄÄÄÄÄÄoÄÄÄÄÄÄÄÄÄoÄÄÄÄÄÄÄÄÄ");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	gotoxy(mX - 1, mY + 1); printf(" ÄÄÄÄÄ  ÄÄÄÄÄ  ÄÄÄÄÄ  ÄÄÄÄÄ");
	gotoxy(mX - 1, mY + 2); printf("|  Ä  ||     ||   | ||   ÄÄ|");
	gotoxy(mX - 1, mY + 3); printf("|   ÄÄ||  |  || | | ||  |  |");
	gotoxy(mX - 1, mY + 4); printf("|ÄÄ|   |ÄÄÄÄÄ||Ä|ÄÄÄ||ÄÄÄÄÄ|");
	setDefaultColor();
	string str = "Press any key to continue";
	int len = str.length();
	gotoxy(mX - (len / 2), mY + 7);
	cout << str;
	getch();
	gotoxy(mX - (len / 2), mY + 7);
	system("cls");
}

void chooseGameLevel()
{
	short mX = WIDTH / 2;
	short mY = HEIGHT / 2;
	string str = "Choose Game Level:";
	gotoxy(mX - str.length() / 2, mY - 4);
	cout << str;
	str = "[1] "; gotoxy(mX - 6, mY - 2);
	cout << str;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	str = "Beginner";
	cout << str;
	setDefaultColor();
	str = " 2  ";	gotoxy(mX - 6, mY - 1);
	cout << str;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	str = "Normal";
	cout << str;
	setDefaultColor();
	str = " 3  "; gotoxy(mX - 6, mY);
	cout << str;
	str = "Hard";
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
	cout << str;
	setDefaultColor();
	/*str = " 3  Crazy"; gotoxy(mX - 6, mY + 1);
	cout << str;
	str = " 4  DON'T CHOOSE"; gotoxy(mX - 6, mY + 2);
	cout << str;*/
	gotoxy(mX - 8, mY - 2);
	char key;
	level = 0;
	bool exit = false;
	while (true)
	{
		key = getch();
		switch (key)
		{
			case 72:
			case KEY_UP:
				if (level > 0)
				{
					gotoxy(mX - 6, mY + (level - 2));
					printf(" %d ", level + 1);
					level--;
					gotoxy(mX - 6, mY + (level - 2));
					printf("[%d]", level + 1);
					gotoxy(mX - 8, mY + (level - 2));
				}
				break;
			case 80:
			case KEY_DOWN:
				if (level < 2)
				{
					gotoxy(mX - 6, mY + (level - 2));
					printf(" %d ", level + 1);
					level++;
					gotoxy(mX - 6, mY + (level - 2));
					printf("[%d]", level + 1);
					gotoxy(mX - 8, mY + (level - 2));
				}
				break;
			case 13:
			case 'x':
				exit = true;
				break;
			default:
				break;
		}
		if (exit)
			break;
	}
}

void chooseGameSpeed(bool *back)
{
	short mX = WIDTH / 2;
	short mY = HEIGHT / 2;
	string str = "Choose Game Speed:";
	gotoxy(mX - str.length() / 2, mY - 4);
	cout << str;
	str = "[1] "; gotoxy(mX - 6, mY - 2);
	cout << str;
	str = "Normal";
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	cout << str;
	setDefaultColor();
	str = " 2  "; gotoxy(mX - 6, mY - 1);
	cout << str;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	str = "Fast & Furious";
	cout << str;
	setDefaultColor();
	str = " 3  "; gotoxy(mX - 6, mY);
	cout << str;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	str = "Speed of light";
	cout << str;
	setDefaultColor();
	gotoxy(mX - 8, mY - 2);
	char key;
	speed = 0;
	bool exit = false;
	while (true)
	{
		key = getch();
		switch (key)
		{
			case 72:
			case KEY_UP:
				if (speed > 0)
				{
					gotoxy(mX - 6, mY + (speed - 2));
					printf(" %d ", speed + 1);
					speed--;
					gotoxy(mX - 6, mY + (speed - 2));
					printf("[%d]", speed + 1);
					gotoxy(mX - 8, mY + (speed - 2));
				}
				break;
			case 80:
			case KEY_DOWN:
				if (speed < 2)
				{
					gotoxy(mX - 6, mY + (speed - 2));
					printf(" %d ", speed + 1);
					speed++;
					gotoxy(mX - 6, mY + (speed - 2));
					printf("[%d]", speed + 1);
					gotoxy(mX - 8, mY + (speed - 2));
				}
				break;
			case 13:
			case 'x':
				exit = true;
				break;
			case '\b':
				exit = true;
				*back = true;
				break;
			default:
				break;	
		}
		if (exit)
			break;
	}
}

void recordResult(string name)
{
	FILE *f;
	f = fopen(SCORETXT, "a+");
	const char *_name = name.c_str();
	// absolute score calculation, thanks to Ice (Ilich) for his idea
	double absScore = (myScore - comScore) + (myScore * (level + 2) * 0.5) + (myScore * (speed + 2));
	strcpy(currentScore.name, _name);
	currentScore.absScore = absScore;
	currentScore.player = myScore;
	currentScore.computer = comScore;
	currentScore.level = level + 1;
	currentScore.speed = speed + 1;
	fprintf(f, "%s/%+06.2lf*%d*%d*%d*%d\n", _name, absScore, myScore, comScore, level + 1, speed + 1);
	fclose(f);
}

const char *skill(double score)
{
	if (score >= 60)
		return "A+";
	if (score >= 54)
		return "A ";
	if (score >= 45)
		return "B+";
	if (score >= 36)
		return "B ";
	if (score >= 27)
		return "C ";
	if (score >= 18)
		return "D ";
	if (score >= 9)
		return "E ";
	return "F ";
}

int tt;
void swap(int *a, int *b)
{
	tt = *a;
	*a = *b;
	*b = tt;
}

void showScoreboard()
{
	short mX = WIDTH / 2;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	gotoxy(mX - 23, 0); printf(" ___                 _                      _ ");
	gotoxy(mX - 23, 1); printf("/ __| __ ___ _ _ ___| |__  ___  __ _ _ _ __| |");
	gotoxy(mX - 23, 2); printf("\\__ \\/ _/ _ \\ '_/ -_) '_ \\/ _ \\/ _` | '_/ _` |");
	gotoxy(mX - 23, 3); printf("|___/\\__\\___/_| \\___|_.__/\\___/\\__,_|_| \\__,_|");
	setDefaultColor();
	FILE *f;
	char str[MAXPL];
	f = fopen(SCORETXT, "r");
	if (f == NULL)
		return;
	printf("\n\n [R]  Name\t\tAbsolute score\t%10s\t%10s\t%10s\t%10s\t%10s\n", "Skill", "Player", "Computer", "Level", "Speed");
	int i, j;
	printf(" ");
	for (i = 0; i < WIDTH - 1; i++) putchar('Ä');
	printf("\n");
	char namet[MAXPLNAMEL];
	char name[MAXPL][MAXPLNAMEL] = { '\0' };
	double temp, absScore[MAXPL];
	int playern[MAXPL], comn[MAXPL], leveln[MAXPL], speedn[MAXPL];
	int p = 0;
	char *str2;
	while (fgets(str, MAXPL, f) != NULL)
	{
		str2 = str;
		i = 0;
		while (*str2 != '/')
		{
			name[p][i] = *(str2++);
			i++;
		}
		name[p][i] = '\0';
		str2++;
		sscanf(str2, "%lf*%d*%d*%d*%d", &absScore[p], &playern[p], &comn[p], &leveln[p], &speedn[p]);
		p++;
		if (p > MAXPL)
			break;
	}
	for (i = 0; i < p - 1; i++)
	{
		for (j = 0; j < p - 1; j++)
		{
			if (absScore[j] < absScore[j + 1])
			{
				strcpy(namet, name[j]);
				strcpy(name[j], name[j + 1]);
				strcpy(name[j + 1], namet);
				temp = absScore[j];
				absScore[j] = absScore[j + 1];
				absScore[j + 1] = temp;
				swap(&playern[j], &playern[j + 1]);
				swap(&comn[j], &comn[j + 1]);
				swap(&leveln[j], &leveln[j + 1]);
				swap(&speedn[j], &speedn[j + 1]);
			}
			else if (absScore[j] == absScore[j + 1])
			{
				if (playern[j + 1] - comn[j + 1] > playern[j] - comn[j])
				{
					strcpy(namet, name[j]);
					strcpy(name[j], name[j + 1]);
					strcpy(name[j + 1], namet);
					swap(&playern[j], &playern[j + 1]);
					swap(&comn[j], &comn[j + 1]);
					swap(&leveln[j], &leveln[j + 1]);
					swap(&speedn[j], &speedn[j + 1]);
				}
			}
		}
	}
	if (p > MAXPL)
		p = MAXPL;
	bool currentPlayer = false;
	int currentPlayerIndex = -1;
	for (i = 0; i < p; i++)
	{
		putchar(' ');
		if (strcmp(currentScore.name, name[i]) == 0 && currentScore.absScore == absScore[i] && currentScore.player == playern[i] && currentScore.computer == comn[i] && currentScore.level == leveln[i] && currentScore.speed == speedn[i])
		{
			currentPlayer = true;
			currentPlayerIndex = i;
		}
		else
			currentPlayer = false;
		if (currentPlayer)
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BACKGROUND_INTENSITY);
		printf("[%d] ", i + 1);
		if (i < 9) printf(" ");
		printf("%-21s", strlen(name[i]) > 0 ? name[i] : "_Unknown");
		printf("%10.2lf\t%10s\t%10d\t%10d\t%10d\t%10d", absScore[i], skill(absScore[i]), playern[i], comn[i], leveln[i], speedn[i]);
		if (currentPlayer)
			printf("      ");
		putchar('\n');
		setDefaultColor();
	}
	fclose(f);
	if (currentPlayerIndex == -1 || p <= 18)
		gotoxy(WIDTH, 0);
	else
	{
		if (currentPlayerIndex > 18)
			gotoxy(WIDTH, currentPlayerIndex + 7);
		else
			gotoxy(WIDTH, 0);
	}
	getch();
}

void showResult()
{
	Sleep(490);
	setKeyboardDefaultRate();
	system("CLS");
	repeat = false;
	if (comScore == myScore)
		return;
	short mX = WIDTH / 2;
	short mY = HEIGHT / 2;
	if (comScore < myScore)
	{
		system("COLOR 09");
		gotoxy(mX - 33, mY - 4); printf("ooooo  oooo ooooooo  ooooo  oooo   oooo     oooo ooooo oooo   oooo");
		gotoxy(mX - 33, mY - 3); printf("  888  88 o888   888o 888    88     88   88  88   888   8888o  88 ");
		gotoxy(mX - 33, mY - 2); printf("    888   888     888 888    88      88 888 88    888   88 888o88 ");
		gotoxy(mX - 33, mY - 1); printf("    888   888o   o888 888    88       888 888     888   88   8888 ");
		gotoxy(mX - 33, mY);	 printf("   o888o    88ooo88    888oo88         8   8     o888o o88o    88 ");
		if (level == 2)
		{
			gotoxy(mX + 25, mY + 2); printf("Hack??");
		}
	}
	else
	{
		system("COLOR 0C");
		gotoxy(mX - 41, mY - 4); printf("ooooo  oooo ooooooo  ooooo  oooo   ooooo         ooooooo    oooooooo8 ooooooooooo");
		gotoxy(mX - 41, mY - 3); printf("  888  88 o888   888o 888    88     888        o888   888o 888         888    88 ");
		gotoxy(mX - 41, mY - 2); printf("    888   888     888 888    88     888        888     888  888oooooo  888ooo8   ");
		gotoxy(mX - 41, mY - 1); printf("    888   888o   o888 888    88     888      o 888o   o888         888 888    oo ");
		gotoxy(mX - 41, mY);	 printf("   o888o    88ooo88    888oo88     o888ooooo88   88ooo88   o88oooo888 o888ooo8888");
		if (level == 0)
		{
			gotoxy(mX + 28, mY + 2); printf("Why so noob?");
		}
	}
	string name;
	gotoxy(mX - 16, mY + 3);
	printf("Enter your name: ");
	getline(cin, name);
	if (name.length() > MAXPLNAMEL)
		name.resize(MAXPLNAMEL);
	recordResult(name);
}

int main()
{
	bool back = false;
	while (true)
	{
		setDefaultValues();
		setKeyboardDefaultRate();
		system("cls");
		system("color 0F");
		consolePreparation();
		welcomeMessage();
		system("cls");
		createStage();
		do {
			back = false;
			chooseGameLevel();
			system("cls");
			if (level != 4)
			{
				chooseGameSpeed(&back);
				system("cls");
			} else
			{
				speed = 1;
				system("color F0");
			}
		} while (back);
		LENGTH_ME = (HEIGHT / 3) - 1;
		LENGTH_COM = LENGTH_ME;
		if (level == 2)
			LENGTH_COM += 2;
		/*switch (level)
		{
			case 2:
				LENGTH_COM += 2; break;
			case 3:
				LENGTH_COM += 4; break;
		}*/
		createStage();
		createDiv();
		createMine();
		createCom();
		updateScore();
		createBall();
		repeat = true;
		last = KEY_UP;
		getch();
		setKeyboardRate("100", "10");
		int delay;
		switch (speed)
		{
			case 0:
				delay = 38; break;
			case 1:
				delay = 16; break;
			case 2:
				delay = 7; break;
		}
		while (repeat)
		{
			Sleep(delay);
			moveBall(&repeat);
			if (BOT)
				moveBot();
			if (AUTO)
				autoPlay(&repeat);
			if (kbhit())
				input(&repeat, &last);
		}
		system("cls");
		if (level == 4)
			system("color F0");
		else
			system("color 0F");
		setKeyboardDefaultRate();
		showScoreboard();
		gotoxy(0, HEIGHT);
	}
	// "Talk is cheap. Show me the code."
	// - Linus Torvalds
	return 0;
}
