#include <stdio.h>

struct ctx {
    int a;
};
struct node {
    int b;
    struct node *next;
};

typedef struct ctx ctx, *pctx;
typedef struct node node, *pnode;

template<typename T> void myfn(pctx c, pnode(*fn)(pctx, T, pnode), T arg) {
    printf("got to myfn %d\n", arg);
    node n;
    node *r = fn(c, arg, &n);
    n.next = r;
}

node* thefn(pctx c, int i, pnode n) {
    printf("got to thefn %d\n", i);
    return n;
}
int main(int argc, char **argv, char **envp) {
    ctx c;
    myfn<int>(&c, &thefn, argc);
    return 0;
}
