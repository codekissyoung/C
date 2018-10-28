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

