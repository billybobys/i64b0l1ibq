#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <curses.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include "func.h"

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