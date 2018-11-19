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
