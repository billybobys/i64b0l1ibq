#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <curses.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include "func.h"

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

