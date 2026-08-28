#include <stdio.h>    // for printf
#include <stdlib.h>   // for rand()
#include <time.h>     // for time()
#include <unistd.h>   // for usleep()
#include <termios.h>  // for terminal settings
#include <fcntl.h>    // for file operations

#define ROW 32        // number of field rows
#define COLUMN 62    // number of field columns  

int user_point = 0; // player's score
int length_snake = 1; // snake length
int snake_points[ROW][2] = {{16, 5}}; // snake's position
int apple_point[2] = {16, 10}; // apple position
char snake[ROW] = {'0'}; // snake body
char grid[ROW][COLUMN]; // game grid

int dir_x = 1; //start moving to right
int dir_y = 0;

//terminal settings
struct termios orig_termios;

//reset terminal to normal
void reset_terminal_mode() {
    tcsetattr(0, TCSANOW, &orig_termios);
}

//set terminal to raw mode
void set_conio_terminal_mode() {
    struct termios new_termios;
    tcgetattr(0, &orig_termios);
    new_termios = orig_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO); //turn off line mode and echo
    tcsetattr(0, TCSANOW, &new_termios);
    atexit(reset_terminal_mode);  //back to normal when game ends
}

//check if key is pressed
int kbhit() {
    struct timeval tv = {0L, 0L};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

 //read the key user pressed
int getch() {
    int r;
    unsigned char c;
    if ((r = read(0, &c,sizeof(c))) < 0) {
        return 0;
    }
    else {
        return c;
    }
}


////game logic(generate the grid with borders and snake)
void GridGenarating() {
    for(int i = 0;i < ROW;i++)
        for(int j = 0;j < COLUMN;j++)
            grid[i][j] = ' ';//make all cells empty

    for(int i = 0;i < COLUMN;i++)
        grid[0][i] = grid[ROW- 1][i] = '-'; //top and bottom borders

    for(int i = 0;i < ROW;i++)
        grid[i][0] = grid[i][COLUMN- 1] = '|';//left and right borders

    for(int i = 0;i < length_snake;i++)
        grid[ snake_points[i][0] ][ snake_points[i][1] ] = snake[i];//draw snake

    grid[ apple_point[0] ][apple_point[1] ]='O';//draw apple
}

//show grid on screen
void displayGrid() {
    printf("\033[H"); //move to top of terminal
    for(int i = 0; i < ROW; i++) {
        for(int j = 0; j < COLUMN; j++)
            printf("%c", grid[i][j]);
        printf("\n");
    }
    printf("Points: %d\n", user_point); 
}

//check if game should stop
int gameover() {
    int head_x = snake_points[0][0];
    int head_y = snake_points[0][1];

    //check wall hit
    if (head_x < 1 || head_x > ROW - 2 || head_y < 1 || head_y > COLUMN-2) {
        printf("\n--- GAME OVER :( ---\n");
        printf("--- YOUR POINTS: %d ---\n",user_point);
        return 0;
    }

    //check if snake hits itself
    for (int i = 1; i < length_snake; i++) {
        if (snake_points[i][0] == head_x && snake_points[i][1] == head_y)
            return 0;
    }

    return 1;
}

//check if snake eat apple
void grown() {
    int next_x = snake_points[0][0] + dir_y;
    int next_y = snake_points[0][1] + dir_x;

    if (next_x == apple_point[0] && next_y == apple_point[1]) {
        user_point++;
        length_snake++;
        snake[length_snake-1] = '*'; //add body part

        //find new apple place
        do {
            apple_point[0]=rand() % (ROW - 2) + 1;
            apple_point[1]=rand() % (COLUMN - 2) + 1;

            int conflict = 0;
            for (int i = 0; i < length_snake; i++) {
                if (snake_points[i][0] == apple_point[0] && snake_points[i][1] == apple_point[1]) {
                    conflict = 1; //apple on snake body
                    break;
                }
            }

            if (!conflict) {
                break;
            }
        } while (1);
    }
}

//move snake one step
void move_snake() {
    grown(); //check apple first

    //move body from tail to head
    for (int i =length_snake-1;i > 0;i--) {
        snake_points[i][0] =snake_points[i - 1][0];
        snake_points[i][1] =snake_points[i - 1][1];
    }

    //move head
    snake_points[0][0] += dir_y;
    snake_points[0][1] += dir_x;

    GridGenarating(); //update grid
}

//read user input and change direction
void process_input() {
    if (!kbhit()) {
        return;
    }
    char ch = getch();

    //don't allow reverse move
    if (ch == 'w' && dir_y != 1)  { 
        dir_y = -1; 
        dir_x = 0; 
    }
    if (ch == 's' && dir_y != -1) { 
        dir_y = 1;  
        dir_x = 0;
    }
    if (ch == 'a' && dir_x != 1)  { 
        dir_y = 0;
        dir_x = -1; 
    }
    if (ch == 'd' && dir_x != -1) {
        dir_y = 0; 
        dir_x = 1; 
    }
}

int main() {
    srand(time(NULL)); //random seed
    set_conio_terminal_mode();//terminal setup

    GridGenarating();  //first grid
    printf("\033[2J"); //clear screen

    while (1) {
        process_input(); //get directiona
        move_snake();    //move snake
        displayGrid();   //show game

        if (!gameover()) {
            break; //check end
        }
        usleep(120000); //speed of game
    }

    return 0;
}