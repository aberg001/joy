# [[ExecutionContext]]
# Proc
`typedef void (*Proc)(pEC);`
# Types
```
union Types {
  long num;
  long set;
  const char *str;
  double dbl;
  FILE *fil;
  pNode lis;
  pEntry ent;
  Proc proc;
};
```
This is the structure to hold a single value.  
# Node
```
struct Node {
  Operator op;
  Types u;
  struct Node *next;
};
```
# Entry
```
struct Entry {
  const char *name;
  int is_module;
  union {
    pNode body;
    struct Entry *module_fields;
    Proc proc;
  } u;
  struct Entry *next;
};
```
