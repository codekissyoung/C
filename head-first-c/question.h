typedef struct node{
    char *question;
    struct node *no;
    struct node *yes;
} node;

int yes_no( char *question );
node *create( char *question );
void release( node *n );
