#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <ncurses.h>
#include <thread>
#include <chrono>
#include <cctype> 
#include <cstring>

using namespace std;

// Constants for key codes
#define KEY_UP 0403
#define KEY_DOWN 0402 
#define KEY_LEFT 0404 
#define KEY_RIGHT 0405 

// Define maximum width and height of the game area
const int WIDTH = 40;
const int HEIGHT = 20;

// Define class for handling positions
class Position {
public:
    int x, y;
    Position(int x = 0, int y = 0) : x(x), y(y) {}
};

// Define SnakeGame class
class SnakeGame {
public:
    SnakeGame();
    void Run();

private:
    bool gameOver;
    const int width, height;
    Position fruit;
    int score;
    enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };
    Direction dir;
    vector<Position> snake;
    int nTail;
    string playerName;

    void Setup();
    void Draw();
    void Input();
    void Logic();
    void GameOver();
    void SaveScore();
    void DisplayLeaderboard();
};

// Constructor initializes game settings
SnakeGame::SnakeGame() : width(WIDTH), height(HEIGHT), nTail(1), score(0), gameOver(false), dir(STOP) {
    snake.push_back(Position(width / 2, height / 2));
    fruit = Position((rand() % (width - 2) + 2) * 2, rand() % (height - 2) + 2);
}

// Setup initializes ncurses and game settings
void SnakeGame::Setup() {
    initscr();
    clear();
    noecho();
    cbreak();
    curs_set(0);
    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);  // Define color pair for yellow fruit
}

// Draw function to render the game area and snake
void SnakeGame::Draw() {
    clear();
    for (int i = 0; i < width + 2; i++)
        mvprintw(0, i, "-");

    for (int i = 0; i < height + 2; i++) {
        for (int j = 0; j < (width + 2) * 2; j++) {
            if (i == 0 || i == height + 1)
                mvprintw(i, j, "-");
            else if (j == 0 || j == (width + 1) * 2)
                mvprintw(i, j, "|");
            else if (i == snake[0].y && j == snake[0].x)
                mvprintw(i, j, "O");
            else if (i == fruit.y && j == fruit.x) {
                attron(COLOR_PAIR(1));  // Turn on yellow color
                mvprintw(i, j, "*");  // Use asterisk for fruit
                attroff(COLOR_PAIR(1));  // Turn off yellow color
            } else {
                bool printTail = false;
                for (int k = 1; k < nTail; k++) {
                    if (snake[k].x == j && snake[k].y == i) {
                        mvprintw(i, j, "o");
                        printTail = true;
                    }
                }
                if (!printTail)
                    mvprintw(i, j, " ");
            }
        }
    }
    mvprintw(height + 2, 0, "Score: %d", score);
    refresh();
}

// Input function to handle user keyboard input
void SnakeGame::Input() {
    keypad(stdscr, true);
    halfdelay(1);
    int c = getch();

    switch (c) {
    case KEY_LEFT:
    case 'a':
    case 'A':
        if (dir != RIGHT)
            dir = LEFT;
        break;
    case KEY_RIGHT:
    case 'd':
    case 'D':
        if (dir != LEFT)
            dir = RIGHT;
        break;
    case KEY_UP:
    case 'w':
    case 'W':
        if (dir != DOWN)
            dir = UP;
        break;
    case KEY_DOWN:
    case 's':
    case 'S':
        if (dir != UP)
            dir = DOWN;
        break;
    case 'q':
    case 'Q':
        gameOver = true;
        break;
    }
}

// Logic function to update game state and check for collisions
void SnakeGame::Logic() {
    Position prev = snake[0];
    Position prev2;

    // Update snake position
    for (int i = 1; i < nTail; i++) {
        prev2 = snake[i];
        snake[i] = prev;
        prev = prev2;
    }

    // Move snake's head according to direction
    switch (dir) {
    case LEFT:
        snake[0].x -= 2;  // Move 2 steps to compensate for character width
        break;
    case RIGHT:
        snake[0].x += 2;  // Move 2 steps to compensate for character width
        break;
    case UP:
        snake[0].y--;  // Move 1 step vertically
        break;
    case DOWN:
        snake[0].y++;  // Move 1 step vertically
        break;
    default:
        break;
    }

    // Check if the snake eats the fruit
    if (snake[0].x == fruit.x && snake[0].y == fruit.y) {
        score++;
        // Generate a new random position for the fruit
        fruit = Position((rand() % (width - 2) + 2) * 2, rand() % (height - 2) + 2);
        // Increase snake length
        nTail++;
        // Add new segment to snake's body at the end
        snake.push_back(Position(snake[nTail - 1].x, snake[nTail - 1].y));
    }

    // Check for collision with snake's own body
    for (int i = 1; i < nTail; i++) {
        if (snake[i].x == snake[0].x && snake[i].y == snake[0].y) {
            GameOver();
        }
    }

    // Check for collision with walls
    if (snake[0].x < 2 || snake[0].x > width * 2 || snake[0].y < 1 || snake[0].y > height) {
        GameOver();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

// Game over function to handle end of game
void SnakeGame::GameOver() {
    clear();
    mvprintw(height / 2, width / 2 - 5, "Game Over!");
    mvprintw(height / 2 + 1, width / 2 - 10, "Your score: %d", score);
    mvprintw(height / 2 + 2, width / 2 - 15, "Enter 3-letter name: ");
    refresh();

    char name[4] = {'\0'}; // Buffer to hold the name
    int nameLength = 0; // Current length of the name
    echo(); // Enable echoing of characters

    while (nameLength < 3) {
        int ch = getch();
        if ((ch == 127 || ch == KEY_BACKSPACE) && nameLength > 0) {
            // Handle backspace (ASCII 127 or KEY_BACKSPACE) and decrement name length
            name[--nameLength] = '\0';
        } else if (isalpha(ch) && nameLength < 3) {
            // Allow only alphabetic characters and limit to exactly 3 characters
            name[nameLength++] = ch;
            name[nameLength] = '\0'; // Ensure the string is null-terminated
        }

        // Clear the previous input area by printing spaces over it
        mvprintw(height / 2 + 2, width / 2, "%-15s", " "); // Clear previous input area
        // Print the current input
        mvprintw(height / 2 + 2, width / 2 - 15, "Enter 3-letter name: %-3s", name);
        refresh();
    }

    noecho(); // Disable echoing of characters
    playerName = std::string(name); // Correctly assign the name to playerName

    SaveScore(); // Save the score with the player's name

    mvprintw(height / 2 + 4, width / 2 - 20, "Press '1' to play again or '2' for main menu");
    refresh();

    int choice;
    while (true) {
        choice = getch();
        if (choice == '1') {
            score = 0;
            nTail = 1;
            snake.clear();
            snake.push_back(Position(width / 2, height / 2));
            fruit = Position(rand() % (width - 2) + 2, rand() % (height - 2) + 2);
            dir = STOP;
            gameOver = false;
            break;
        } else if (choice == '2') {
            gameOver = true;
            break;
        }
    }
}

// Function to save player's score to a text file
void SnakeGame::SaveScore() {
    ofstream file("scores.txt", ios::app);
    if (file.is_open()) {
        file << playerName << " " << score << endl;
        file.close();
    } else {
        cerr << "Unable to open file for saving scores." << endl;
    }
}

// Function to display leaderboard from the scores text file
void SnakeGame::DisplayLeaderboard() {
    clear();
    mvprintw(0, width / 2 - 5, "Leaderboard");
    mvprintw(2, width / 2 - 15, "Name");
    mvprintw(2, width / 2 + 5, "Score");

    ifstream file("scores.txt");
    if (file.is_open()) {
        string line;
        vector<pair<string, int>> scores;
        while (getline(file, line)) {
            string name = line.substr(0, 3);
            int score = stoi(line.substr(4));
            scores.push_back(make_pair(name, score));
        }
        file.close();

        sort(scores.begin(), scores.end(), [](const pair<string, int>& a, const pair<string, int>& b) {
            return a.second > b.second;
        });

        int row = 4;
        for (size_t i = 0; i < scores.size() && i < 10; ++i) {
            mvprintw(row + i, width / 2 - 15, scores[i].first.c_str());
            mvprintw(row + i, width / 2 + 5, to_string(scores[i].second).c_str());
        }
    } else {
        mvprintw(4, width / 2 - 10, "No scores available.");
    }

    mvprintw(height - 1, 0, "Press any key to return to the main menu.");
    refresh();
    getch();
}

// Run function to start the game
void SnakeGame::Run() {
    Setup();
    int choice;
    while (!gameOver) {
        clear();
        mvprintw(0, width / 2 - 3, "Snake");
        mvprintw(2, width / 2 - 10, "1. Play Game");
        mvprintw(3, width / 2 - 10, "2. Leaderboard");
        mvprintw(4, width / 2 - 10, "3. Quit");
        mvprintw(height - 1, 0, "Enter your choice: ");
        refresh();

        choice = getch();
        switch (choice) {
            case '1':
                score = 0;
                nTail = 1;
                snake.clear();
                snake.push_back(Position(width / 2, height / 2));
                fruit = Position((rand() % (width - 2) + 2) * 2, rand() % (height - 2) + 2);
                dir = STOP;
                while (!gameOver) {
                    Draw();
                    Input();
                    Logic();
                }
                break;
            case '2':
                DisplayLeaderboard();
                break;
            case '3':
                gameOver = true;
                break;
            default:
                break;
        }
    }

    endwin();
}

// Main function
int main() {
    srand(time(0));
    SnakeGame game;
    game.Run();
    return 0;
}
