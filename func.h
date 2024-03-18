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

void Warehouse (warehouse_ *wh);//визуализация склада
void stackWarehouse(warehouse_ *wh,snake_t *head);//функция склада
void initTail(tail_t t[], size_t size);
void initHead(snake_t *head, int x, int y);
void initSnakes(snake_t *head[], size_t size, int x, int y,int i);
void initFood(struct food f[], size_t size);
void go(struct snake_t *head); //Движение головы с учетом текущего направления движения
void changeDirection(struct snake_t* snake, const int32_t key);
int checkDirection(snake_t* snake, int32_t key);
void goTail(struct snake_t *head); //Движение хвоста с учетом движения головы
void putFoodSeed(struct food *fp); //Обновить/разместить текущее зерно на поле
void putFood(struct food f[], size_t number_seeds); //Разместить еду на поле
void refreshFood(struct food f[], int nfood);
_Bool haveEat(struct snake_t *head, struct food f[]);
void addTail(struct snake_t *head); // Увеличение хвоста на 1 элемент
void printLevel(struct snake_t *head);
void printExit(struct warehouse_ *wh);
int distance(const snake_t snake, const struct food food);   // вычисляет количество ходов до еды                              
void autoChangeDirection(snake_t *snake, struct food food[], int foodSize);                                                      
void update(snake_t *head, struct food f[], int key);                                                                            
int pause(void);
_Bool isCrush(snake_t * snake);

