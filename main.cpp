#include <iostream>
#include <conio.h>  // For _kbhit() and _getch()
#include <windows.h> // For Sleep() and system("cls")
#include <vector> // Vectors
#include <cstdlib>  // For rand() and srand()
#include <ctime>    // For time()
#include <fstream>  // For file handling
#include <string> // string for files
using namespace std;

//CONSTANTS
const int WIDTH = 40;
const int HEIGHT = 20;
const int MAX_TAIL_LENGTH = 100;
const int SPEED = 100; // milliseconds per frame

enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };

//declare functions for high score
void saveHighScore(int score);
int loadHighScore();

//CLASSES
//point class for coordinates
class Point {
    public:
    int x;
    int y;
	Point(int x = 0, int y = 0) : x(x), y(y) {}
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

//food
class Food {
private:
    Point position;
    char symbol;

public:
    Food(char sym = '@') : symbol(sym) {
        position = Point(0, 0);
    }

    virtual ~Food() {}

	// virtual method for score value
    virtual int getScoreValue() const {
        return 10;  // regular food gives 10 points
    }

    void generate(const vector<Point>& snakeBody) {
        //rand position for food
        bool onSnake;
        do {
            onSnake = false;
            position.x = rand() % (WIDTH - 2) + 1;
            position.y = rand() % (HEIGHT - 2) + 1;

            //check if food is on snake body
            for (const auto& segment : snakeBody) {
                if (position == segment) {
                    onSnake = true;
                    break;
                }
            }
        } while (onSnake);
    }

	// test virtual method for special effect
    virtual void specialEffect() {
        // does nothing
    }
    //getters
    Point getPosition() const { return position; }
	char getSymbol() const { return symbol; }
};
//special food derived from food
class SpecialFood : public Food {
private:
    int bonusMultiplier;

public:
	SpecialFood() : Food('$') {  // base constructor with special symbol
		bonusMultiplier = 2;  // double points
    }

    // POLYMORPHISM
    int getScoreValue() const override {
        return 10 * bonusMultiplier; 
    }

	//override special effect method
    void specialEffect() override {
		//still does nothing for now
    }
};

//snake classsss
class Snake {
private:
    Point head;
    vector<Point> tail;
    Direction dir;
	char headSymbol;
    char tailSymbol;
	int tailLength;
	bool gameOver;

public:
    Snake() : headSymbol('O'), tailSymbol('o'), tailLength(0), gameOver(false) {
        // spawn in middle
        head = Point(WIDTH / 2, HEIGHT / 2);
        dir = (STOP);
        tail.clear();
    }
    //direction change
    void changeDirection(char key) {
        switch (key)
        {
        case 'a':
        case 'A':
			if (dir != RIGHT)dir = LEFT;// prevent reverse
            break;
		case 'd':
        case 'D':
            if (dir != LEFT)dir = RIGHT;
			break;
		case 'w':
        case 'W':
			if (dir != DOWN)dir = UP;
            break;
		case 's':
		case 'S':
			if (dir != UP)dir = DOWN;
            break;
        case 'x':
        case 'X':
            gameOver = true;
			break;
        }
    }

    void move() {
        if (dir == STOP) return;

		Point prevHead = head; // save previous head position

		//move head with direction
        switch (dir) {
        case LEFT:
            head.x--;
            break;
        case RIGHT:
            head.x++;
			break;
        case UP:
            head.y--;
			break;
        case DOWN:
			head.y++;
            break;
        }

        //check wall
        if (head.x <= 0 || head.x >= WIDTH - 1 ||
            head.y <= 0 || head.y >= HEIGHT - 1) {
			gameOver = true;
            return;
        }

        //move tail
        if (tailLength > 0) {
            //check self
            for (const auto& segment : tail) {
                if (head == segment) {
					gameOver = true;
					return;
                }
            }

            //move tail segments
            for (int i = tailLength - 1; i > 0; i--) {
				tail[i] = tail[i - 1];
            }
            tail[0] = prevHead;
        }
    }
	//check food collision
    bool checkFoodCollision(const Point& foodPos) {
        return head == foodPos;
    }
	//grow snake
    void grow() {
        if (tailLength < MAX_TAIL_LENGTH) {
            tailLength++;
			tail.resize(tailLength);
            if (tailLength == 1) {
                tail[0] = head; // puts first segment
            }
        }
    }

    //getters

	Point getHead() const { return head; }
    const vector<Point>& getTail() const { return tail; }
    int getTailLength() const { return tailLength; }
    char getTailSymbol() const { return tailSymbol; }
	char getHeadSymbol() const { return headSymbol; }
	bool isGameOver() const { return gameOver; }

	//reset snake
    void reset() {
        head = Point(WIDTH / 2, HEIGHT / 2);
        dir = STOP;
        tail.clear();
        tailLength = 0;
        gameOver = false;
	}
};

//game class
class Game {
    private:
    Snake snake;
    Food* currentFood;  //pointer to base class
	int score;
	int highScore; //high score tracking for file
public:
    Game() : score(0) {
        srand(time(NULL)); //seed random using time
		currentFood = new Food(); //start with regular food
        highScore = loadHighScore();
    }
    ~Game() {
        delete currentFood;  //clean up dynamically allocated memory
    }

	//draw game state
    void draw() {
		system("cls"); //clear console
        //draw top wall
        for (int i = 0; i < WIDTH; i++) {
            cout << "#";
        }
        cout << endl;

        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                Point current(x, y);
                //draw walls
                if (x == 0 || x == WIDTH - 1 || y == 0 || y == HEIGHT - 1) {
                    cout << "#";
                }
                //draw head
                else if (current == snake.getHead()) {
                    cout << snake.getHeadSymbol();
                }
                //draw tail
                else if (containsSnakeTail(current)) {
                    cout << snake.getTailSymbol();
                }
                //draw food
                else if (current == currentFood->getPosition()) {
                    cout << currentFood->getSymbol();
                }
                //empty space
                else {
                    cout << " ";
                }
            }
            cout << endl;
        }
        // Draw bottom wall
        for (int i = 0; i < WIDTH; i++)
            cout << "#";
        cout << endl;

		// Display score and controls
        cout << "Score: " << score << endl;
        cout << "Food: "<<endl;
        cout << "@ REGULAR (10 points)" << endl;
        cout << "$ SPECIAL (20 points)" << endl;
		cout << "High Score: " << highScore << endl;
        cout << endl;
        cout << "Controls: W (Up), A (Left), S (Down), D (Right), X (Exit)" << endl;
    }

	//update game state
    void update() {
		if (_kbhit()) {//check for key press
			char key = _getch();//get key
			snake.changeDirection(key);//change direction
        }

        snake.move();

        if (snake.isGameOver()) {
            if (score > highScore) {
                highScore = score;
                saveHighScore(highScore);  // save to file
            }
            return;
        }

        //check if food eaten
        if (snake.checkFoodCollision(currentFood->getPosition())) {
            snake.grow();
            score += currentFood->getScoreValue();

            // Delete old food
            delete currentFood;

            // Randomly choose food type
            // Every 3rd food is special (30% chance)
            if (rand() % 3 == 0) {  // 1 in 3 chance
                // Create SpecialFood object
                currentFood = new SpecialFood();
            }
            else {
                // Create regular Food object
                currentFood = new Food();
            }

            // Generate new food position
            currentFood->generate(snake.getTail());
        }
    }

	//main game loop
    void run() {
        //generate food at start
        currentFood->generate(vector<Point>());

        while (!snake.isGameOver()) {
            draw();
            update();
            Sleep(SPEED); // speed in milliseconds
        }
        //game over
        system("cls");
        cout << "==========================" << endl;
        cout << "       GAME OVER!" << endl;
        cout << "   Final Score: " << score << endl;
        cout << "   High Score: " << highScore << endl;  

        if (score == highScore && score > 0) {
            cout << "   NEW HIGH SCORE! " << endl; 
        }
        cout << "==========================" << endl;
        cout << "Press any key to exit..." << endl;
    }

private:
	//check if point is in snake tail
    bool containsSnakeTail(const Point& point) {
        for (const auto& segment : snake.getTail()) {
            if (point == segment) {
                return true;
            }
        }
        return false;
    }
};

//save high score to filew
void saveHighScore(int score) {
    ofstream file("highscore.txt");
    if (file.is_open()) {
        file << score;
        file.close();
    }
	else cout << "Unable to open high score file for writing." << endl;
}
//load high score from file
int loadHighScore() {
    ifstream file("highscore.txt");
    if (file.is_open()) {
        int highscore;
        file >> highscore;
        file.close();
        return highscore;
    }
	else cout << "No high score file found." << endl;
    return 0;
}

//main function
int main()
{
    Game game;
	game.run();
	return 0;
}
