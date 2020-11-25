/* FILE : globals.h */

				/* configure			*/
#define SHELLESCAPE	'$'
#define INPSTACKMAX	10
#define INPLINEMAX	255
#define ALEN		20
#define HASHSIZE	9
#define SYMTABMAX	1000
#define DISPLAYMAX	10	/* nesting in HIDE & MODULE	*/
# ifndef GC_BDW
#    define MEMORYMAX	20000
# else
#    define MEMORYMAX	0
# endif
#define INIECHOFLAG	0
#define INIAUTOPUT	1
#define INITRACEGC	1
				/* installation dependent	*/
#define SETSIZE		32
#define MAXINT		2147483647
				/* symbols from getsym		*/
typedef enum {
  ILLEGAL_ = 0,
  COPIED_,
  USR_,
  ANON_FUNCT_,
  BOOLEAN_,
  CHAR_,
  INTEGER_,
  SET_,
  STRING_,
  LIST_,
  FLOAT_,
  FILE_,
  LBRACK = 900,
  LBRACE,
  LPAREN,
  ATOM = 999,  /* last legal factor begin */
  RBRACK = 1001,
  RPAREN = 1003,
  RBRACE = 1005,
  PERIOD,
  SEMICOL,
  LIBRA = 1100,
  EQDEF,
  HIDE,
  IN,
  END,
  MODULE,
  JPRIVATE,
  JPUBLIC,
} Operator;

#ifdef DEBUG
#    define D(x) x
#else
#    define D(x)
#endif

#define PRIVATE static
#define PUBLIC

/* Forwards */
typedef struct ExecutionContext ExecutionContext, EC, *pEC;
typedef struct Node Node, *pNode;
typedef struct Entry Entry, *pEntry;
typedef union Types Types, *pTypes;
typedef void (*Proc)(pEC);
				/* types			*/
typedef int Symbol;
// typedef short Operator;

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

struct Node {
  Operator op;
  Types u;
  struct Node *next;
};

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

struct ExecutionContext {
  FILE *srcfile;
  int g_argc;
  char **g_argv;
  int echoflag;
  int autoput;
  int undeferror;
  int tracegc;
  int startclock,gc_clock;		/* main		*/
  int ch;				/* scanner	*/
  Symbol sym;
  long num;
  double dbl;
  char id[ALEN];
  int hashvalue;
  Types bucket;				/* used by NEWNODE defines */
  int display_enter;
  int display_lookup;

  struct Scanner {
    FILE *infile[INPSTACKMAX];
    int ilevel;
    int linenumber;
    char linbuf[INPLINEMAX];
    int linelength, currentcolumn;
    int errorcount;
  } scanner;

  struct Util {
# ifndef GC_BDW
    Node memory[MEMORYMAX];
    pNode memoryindex,
      mem_low,
      mem_mid;
    int direction;
    int nodesinspected, nodescopied;
    int start_gc_clock;
# endif

  } util;

  Entry					/* symbol table	*/
    symtab[SYMTABMAX];
  pEntry
    hashentry[HASHSIZE],
    localentry,
    symtabindex,
    display[DISPLAYMAX],
    firstlibra,				/* inioptable	*/
    location;				/* getsym	*/

  pNode				/* dynamic memory	*/
  /*
      memory[MEMORYMAX],
      *memoryindex,
  */
    prog, stk, conts,
    dump, dump1, dump2, dump3, dump4, dump5;

  jmp_buf begin, fail;
};

#define LOC2INT(e) (((long)e - (long)ec->symtab) / sizeof(Entry))
#define INT2LOC(x) ((Entry*) ((x + (long)ec->symtab)) * sizeof(Entry))


#define MEM2INT(n) (((long)n - (long)ec->util.memory) / sizeof(Node))
#define INT2MEM(x) ((Node*) ((x + (long)&ec->util.memory) * sizeof(Node)))

/* GOOD REFS:
	005.133l H4732		A LISP interpreter in C
	Manna p139  recursive Ackermann SCHEMA

   OTHER DATA TYPES
	WORD = "ABCD" - up to four chars
	LIST of SETs of char [S0 S1 S2 S3]
	        LISTS - binary tree [left right]
			" with info [info left right]
	STRING of 32 chars = 32 * 8 bits = 256 bits = bigset
	CHAR = 2 HEX
	32 SET = 2 * 16SET
*/

/* Public procedures: */
PUBLIC void stack_(pEC ec);
PUBLIC void dummy_(pEC ec);
PUBLIC void exeterm(pEC ec, pNode n);
PUBLIC void inisymboltable(pEC ec);	/* initialise */
PUBLIC const char *opername(pEC ec, int o);
PUBLIC void lookup(pEC ec);
PUBLIC void abortexecution_(pEC ec);
PUBLIC void execerror(pEC ec, const char *message, const char *op);
PUBLIC void quit_(pEC ec);
PUBLIC void inilinebuffer(pEC ec);
PUBLIC void putline(pEC ec);
PUBLIC int endofbuffer(pEC ec);
PUBLIC void error(pEC ec, const char *message);
PUBLIC int doinclude(pEC ec, const char *filnam);
PUBLIC void getsym(pEC ec);
PUBLIC void inimem0(pEC ec);
PUBLIC void inimem1(pEC ec);
PUBLIC void inimem2(pEC ec);
PUBLIC void printnode(pEC ec, pNode p);
PUBLIC void gc_(pEC ec);
PUBLIC pNode newnode(pEC ec, Operator o, Types u, pNode r);
PUBLIC void memoryindex_(pEC ec);
PUBLIC void readfactor(pEC ec);	/* read a JOY factor */
PUBLIC void readterm(pEC ec);
PUBLIC void writefactor(pEC ec, pNode n, FILE *stm);
PUBLIC void writeterm(pEC ec, pNode n, FILE *stm);

PUBLIC pNode usrNewnode(pEC ec, Types u, pNode r);
PUBLIC pNode anonFunctNewnode(pEC ec, Types u, pNode r);
PUBLIC pNode booleanNewnode(pEC ec, long u, pNode r);
PUBLIC pNode charNewnode(pEC ec, long u, pNode r);
PUBLIC pNode integerNewnode(pEC ec, long u, pNode r);
PUBLIC pNode setNewnode(pEC ec, long u, pNode r);
PUBLIC pNode stringNewnode(pEC ec, const char *u, pNode r);
PUBLIC pNode listNewnode(pEC ec, pNode u, pNode r);
PUBLIC pNode floatNewnode(pEC ec, double u, pNode r);
PUBLIC pNode fileNewnode(pEC ec, FILE *u, pNode r);

#define USR_NEWNODE(u,r)	(ec->bucket.ent = u, newnode(ec, USR_, ec->bucket, r))
#define ANON_FUNCT_NEWNODE(u,r)	(ec->bucket.proc = u, newnode(ec, ANON_FUNCT_, ec->bucket, r))
#define BOOLEAN_NEWNODE(u,r)	(ec->bucket.num = u, newnode(ec, BOOLEAN_, ec->bucket, r))
#define CHAR_NEWNODE(u,r)	(ec->bucket.num = u, newnode(ec, CHAR_, ec->bucket, r))
#define INTEGER_NEWNODE(u,r)	(ec->bucket.num = u, newnode(ec, INTEGER_, ec->bucket, r))
#define SET_NEWNODE(u,r)	(ec->bucket.num = u, newnode(ec, SET_, ec->bucket, r))
#define STRING_NEWNODE(u,r)	(ec->bucket.str = u, newnode(ec, STRING_, ec->bucket, r))
#define LIST_NEWNODE(u,r)	(ec->bucket.lis = u, newnode(ec, LIST_, ec->bucket, r))
#define FLOAT_NEWNODE(u,r)	(ec->bucket.dbl = u, newnode(ec, FLOAT_, ec->bucket, r))
#define FILE_NEWNODE(u,r)	(ec->bucket.fil = u, newnode(ec, FILE_, ec->bucket, r))
