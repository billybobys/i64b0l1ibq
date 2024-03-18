#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <curses.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>

#define MIN_Y    2
#define PLAYERS  5

double DELAY = 0.1;
enum {LEFT=1, UP, RIGHT, DOWN, STOP_GAME=KEY_F(10),CONTROLS=4,PAUSE_GAME='p'};
enum {MAX_TAIL_SIZE=15, START_TAIL_SIZE=0, MAX_FOOD_SIZE=20, FOOD_EXPIRE_SECONDS=10,SEED_NUMBER=10};

struct control_buttons
{
    int down;
    int up;
    int left;
    int right;
}control_buttons;

struct control_buttons default_controls[CONTROLS] = {{KEY_DOWN, KEY_UP, KEY_LEFT, KEY_RIGHT}};
struct control_buttons player_ai_controls[CONTROLS] = {{'w', 'a', 's', 'd'}};


typedef struct snake_t
{
    int x;
    int y;
    int direction;
    size_t tsize;
    struct tail_t *tail;
    struct control_buttons* controls;
    int score;
} snake_t;

typedef struct tail_t
{
    int x;
    int y;
} tail_t;

struct food
{
    int x;
    int y;
    time_t put_time;
    char point;
    uint8_t enable;
} food[MAX_FOOD_SIZE];

typedef struct warehouse_
{
    int x;
    int y;
    int UnitsCounter;
}warehouse_;


void Warehouse (warehouse_ *wh)//визуализация склада
{
    char ch = 'W';
    wh->x = 3;
    wh->y = 3;
    mvprintw(wh->x,wh->y,"%c",ch);
    mvprintw(wh->x-1,wh->y,"%c",ch);
    mvprintw(wh->x-1,wh->y-2,"%c",ch);
    mvprintw(wh->x,wh->y-2,"%c",ch);

}

void stackWarehouse(warehouse_ *wh,snake_t *head)//функция склада
{
    if (head->x == wh->y   && head->y == wh->x   || 
        head->x == wh->y && head->y == wh->x-1   || 
        head->x == wh->y-2 && head->y == wh->x-1 || 
        head->x == wh->y-2   && head->y == wh->x)

    {
        wh->UnitsCounter += head->score;
        mvprintw(1, 75," Units in warehouse = %d",wh->UnitsCounter);
        head->tsize -= head->score;
        head->score = 0;
        mvprintw(0, 75," Units collected drone = %d",head->score);
    }
}

void initTail(tail_t t[], size_t size)
{
    struct tail_t init_t={0,0};
    for(size_t i=0; i<size; i++)
    {
        t[i]=init_t;
    }
}

void initHead(snake_t *head, int x, int y)
{
    head->x = x;
    head->y = y;
    head->direction = RIGHT;
    
}

void initSnakes(snake_t *head[], size_t size, int x, int y,int i)
{
    head[i]    = (snake_t*)malloc(sizeof(snake_t));
tail_t*  tail  = (tail_t*) malloc(MAX_TAIL_SIZE*sizeof(tail_t));
    initTail(tail, MAX_TAIL_SIZE);
    initHead(head[i], x, y);
    head[i]->tail     = tail;
    head[i]->tsize    = size+1;
    head[i]->controls = default_controls;
}

void initFood(struct food f[], size_t size)
{
    struct food init = {0,0,0,0,0};
    for(size_t i=0; i<size; i++)
    {
        f[i] = init;
    }
}

void go(struct snake_t *head) //Движение головы с учетом текущего направления движения
{
    char ch = 'X';
    int max_x=0, max_y=0;
    getmaxyx(stdscr, max_y, max_x); // macro - размер терминала
    mvprintw(head->y, head->x, " "); // очищаем один символ
    switch (head->direction)
    {
        case LEFT:
            if(head->x <= 0) // Циклическое движение, чтобы не
// уходить за пределы экрана
                head->x = max_x;
            mvprintw(head->y, --(head->x), "%c", ch);
        break;
        case RIGHT:
            if(head->x >= max_x)
                head->x = 0;
            mvprintw(head->y, ++(head->x), "%c", ch);
        break;
        case UP:
            if(head->y <= MIN_Y)
                head->y = max_y;
            mvprintw(--(head->y), head->x, "%c", ch);
        break;
        case DOWN:
            if(head->y >= max_y)
                head->y = MIN_Y;
            mvprintw(++(head->y), head->x, "%c", ch);
        break;
        default:
        break;
    }
    refresh();
}

void changeDirection(struct snake_t* snake, const int32_t key)
{
    for (int i = 0; i < CONTROLS; i++)
    {
        if (key == snake->controls[i].down)
            snake->direction = DOWN;
        else if (key == snake->controls[i].up)
            snake->direction = UP;
        else if (key == snake->controls[i].right)
            snake->direction = RIGHT;
        else if (key == snake->controls[i].left)
            snake->direction = LEFT;
    }
}

int checkDirection(snake_t* snake, int32_t key)
{
    for (int i = 0; i < CONTROLS; i++)
        if((snake->controls[i].down  == key && snake->direction==UP)    ||
           (snake->controls[i].up    == key && snake->direction==DOWN)  ||
           (snake->controls[i].left  == key && snake->direction==RIGHT) ||
           (snake->controls[i].right == key && snake->direction==LEFT))
        {
            return 0;
        }
    return 1;

}

void goTail(struct snake_t *head) //Движение хвоста с учетом движения головы
{
    char ch = '*';
    mvprintw(head->tail[head->tsize-1].y, head->tail[head->tsize-1].x, " ");
    for(size_t i = head->tsize-1; i>0; i--)
    {
        head->tail[i] = head->tail[i-1];
        if( head->tail[i].y || head->tail[i].x)
            mvprintw(head->tail[i].y, head->tail[i].x, "%c", ch);
    }
    head->tail[0].x = head->x;
    head->tail[0].y = head->y;
}

void putFoodSeed(struct food *fp) //Обновить/разместить текущее зерно на поле
{
    int max_x=0, max_y=0;
    char spoint[2] = {0};
    getmaxyx(stdscr, max_y, max_x);
    mvprintw(fp->y, fp->x, " ");
    fp->x = rand() % (max_x - 1);
    fp->y = rand() % (max_y - 2) + 1; //Не занимаем верхнюю строку
    fp->put_time = time(NULL);
    fp->point = 'Q';
    fp->enable = 1;
    spoint[0] = fp->point;
    mvprintw(fp->y, fp->x, "%s", spoint);
}

void putFood(struct food f[], size_t number_seeds) //Разместить еду на поле
{
    for(size_t i=0; i<number_seeds; i++)
    {
        putFoodSeed(&f[i]);
    }
}

void refreshFood(struct food f[], int nfood)
{
    for(size_t i=0; i<nfood; i++)
    {
        if( f[i].put_time )
        {
            if( !f[i].enable || (time(NULL) - f[i].put_time) > FOOD_EXPIRE_SECONDS )
            {
                putFoodSeed(&f[i]);
            }
        }
    }
}

_Bool haveEat(struct snake_t *head, struct food f[])
{
    for(size_t i=0; i<MAX_FOOD_SIZE; i++)
        if( f[i].enable && head->x == f[i].x && head->y == f[i].y  )
        {
            f[i].enable = 0;
            return 1;
        }
    return 0;
}

void addTail(struct snake_t *head) // Увеличение хвоста на 1 элемент
{
    if(head == NULL || head->tsize>MAX_TAIL_SIZE)
    {
        mvprintw(0, 0, "Can't add tail");
        return;
    }
    head->tsize++;
}

void printLevel(struct snake_t *head)
{
    head->score = head->score + 1;
    mvprintw(0, 75," Units collected drone = %d",head->score);
}

void printExit(struct warehouse_ *wh)
{
    while (getch() != STOP_GAME)
    {
    mvprintw(7, 15,"Units collected warehouse - %d",wh->UnitsCounter);
    }
}

int distance(const snake_t snake, const struct food food) {   // вычисляет количество ходов до еды                              
    return (abs(snake.x - food.x) + abs(snake.y - food.y));
}

void autoChangeDirection(snake_t *snake, struct food food[], int foodSize)                                                      
{
    int pointer = 0;
    for (int i = 1; i < foodSize; i++) {   // ищем ближайшую еду
        pointer = (distance(*snake, food[i]) < distance(*snake, food[pointer])) ? i : pointer;
    }
    if ((snake->direction == RIGHT || snake->direction == LEFT) &&
        (snake->y != food[pointer].y)) {  // горизонтальное движение
        snake->direction = (food[pointer].y > snake->y) ? DOWN : UP;
    } else if ((snake->direction == DOWN || snake->direction == UP) &&
               (snake->x != food[pointer].x)) {  // вертикальное движение
        snake->direction = (food[pointer].x > snake->x) ? RIGHT : LEFT;
    }
}

void update(snake_t *head, struct food f[], int key)                                                                            
{
    autoChangeDirection(head,f,SEED_NUMBER);
    go(head);
    goTail(head);
    if (checkDirection(head,key))
    {
        changeDirection(head, key);
    }
    refreshFood(food, SEED_NUMBER);// Обновляем еду
    if (haveEat(head,food))
    {
        addTail(head);
        printLevel(head);
        //~ DELAY -= 0.009;
    }
}

int pause(void)
{
    int max_x = 0, max_y = 0;
    getmaxyx(stdscr, max_y, max_x);
    mvprintw(max_y / 2, max_x / 2 - 5, "Press P to continue game");
    while (getch() != PAUSE_GAME)
    {}
    mvprintw(max_y / 2, max_x / 2 - 5, "                        ");
}

_Bool isCrush(snake_t * snake)
{
        for(size_t i=1; i<snake->tsize; i++)
            if(snake->x == snake->tail[i].x && snake->y == snake->tail[i].y)
                return 1;
    return 0;
}



int main()
{
    int key_pressed      = 0;
    int isFinish         = 0;
    int mode_ai_or_human = 0;

    printf("0. AI mode\n1. Human mode\n");
    scanf ("%d",&mode_ai_or_human);

//----------------------------------------INIT----------------------------------------//
    warehouse_* wh = (warehouse_*)malloc(sizeof(warehouse_));
    snake_t* snakes[PLAYERS];
    if (mode_ai_or_human == 0)//ai
    {
        for (int i = 0; i < PLAYERS; i++)
            initSnakes(snakes,START_TAIL_SIZE,10+i*10,10+i*10,i);
    }
    else//human
    {
            initSnakes(snakes,START_TAIL_SIZE,10,10,1);

    }

    initscr();
    keypad(stdscr, TRUE); // Включаем F1, F2, стрелки и т.д.
    raw();                // Откдючаем line buffering
    noecho();            // Отключаем echo() режим при вызове getch
    curs_set(FALSE);    //Отключаем курсор
    mvprintw(0, 0,"Use arrows for control.Double press 'F10' for EXIT. 'P' - pause");
    timeout(0);    //Отключаем таймаут после нажатия клавиши в цикле
    initFood(food, MAX_FOOD_SIZE);
    putFood(food, SEED_NUMBER);// Кладем зерна
//------------------------------------------------------------------------------------//
   

//------------------------------------MAIN-WORK---------------------------------------//
    while( key_pressed != STOP_GAME && !isFinish)//
    {
        clock_t begin = clock();
        key_pressed = getch();
        Warehouse (wh);

        if (mode_ai_or_human == 0)      //режим AI
        {
            for (int i = 0; i < PLAYERS; i++)                                       
            {
                update(snakes[i], food, key_pressed);

            if(isCrush(snakes[i]))
            {
                key_pressed = STOP_GAME;
                mvprintw(6, 15,"Drone is crashed!");
            }
            }
        }
        else                           //режим human
        {
            go(snakes[mode_ai_or_human]);
            goTail(snakes[mode_ai_or_human]);
            stackWarehouse(wh,snakes[mode_ai_or_human]);
            if (checkDirection(snakes[mode_ai_or_human],key_pressed))
            {
                changeDirection(snakes[mode_ai_or_human], key_pressed);
            }

            if(isCrush(snakes[mode_ai_or_human]))
            {
                key_pressed = STOP_GAME;
                mvprintw(6, 15,"Drone is crashed!");
            }

            refreshFood(food, SEED_NUMBER);
            if (haveEat(snakes[mode_ai_or_human],food))
            {
                addTail(snakes[mode_ai_or_human]);
                printLevel(snakes[mode_ai_or_human]);
            }
        }
        if (key_pressed == PAUSE_GAME)
        {
            pause();
        }
        refresh();//Обновление экрана, вывели кадр анимации
        while ((double)(clock() - begin)/CLOCKS_PER_SEC<DELAY)
        {}
    }
//------------------------------------------------------------------------------------//


//------------------------------------STOP-PROGRAMM-----------------------------------//

    if (mode_ai_or_human == 0)                      //освобождение памяти в режиме AI
    {
        for (int i = 0; i < PLAYERS; i++)
        {
            free(snakes[i]->tail);
            free(snakes[i]);
        }
        printExit(wh);
        free(wh);
    }
    else                                           //освобождение памяти в режиме human
    {
        printExit(wh);
        free(snakes[mode_ai_or_human]->tail);
        free(snakes[mode_ai_or_human]);
        free(wh);
    }
    endwin();
    return 0;
}

