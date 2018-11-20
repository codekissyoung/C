struct preferences{
    const char *food;
    float exercise_hours;
};

struct fish
{
    const char *name;
    const char *species;
    int teeth;
    int age;
    struct preferences care;
};

typedef struct {
    char *name;
    char *opens;
    char *closes;
    struct island *next;
} island;

typedef struct Test_arr_struct {
    int age;
    double weight;
    char name[10];
    uint64_t user_ids[1000];
} Test_arr_struct;

typedef struct Node
{
    int nodeSize;
    int bufSize;
    // char buf[0]; // gnu 拓展 -pedantic 会报 warning: ISO C forbids zero-size array ‘buf’ [-Wpedantic]
    char buf[]; // c99 之后的拓展
} Node;