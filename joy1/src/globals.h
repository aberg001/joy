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
#define ILLEGAL_	 0
#define COPIED_		 1
#define USR_		 2
#define ANON_FUNCT_	3
#define BOOLEAN_	4
#define CHAR_	5
#define INTEGER_	6
#define SET_	7
#define STRING_	8
#define LIST_	9
#define FLOAT_	10
#define FILE_	11
#define LBRACK		900
#define LBRACE		901
#define LPAREN		902
#define ATOM		999	/* last legal factor begin */
#define RBRACK		1001
#define RPAREN		1003
#define RBRACE		1005
#define PERIOD		1006
#define SEMICOL		1007
#define LIBRA		1100
#define EQDEF		1101
#define HIDE		1102
#define IN		1103
#define END		1104
#define MODULE		1105
#define JPRIVATE	1106
#define JPUBLIC		1107

#ifdef DEBUG
#    define D(x) x
#else
#    define D(x)
#endif

#define PRIVATE static
#define PUBLIC

				/* types			*/
typedef int Symbol;
typedef short Operator;

typedef union { 
  long num;
  long set;
  char *str;
  double dbl;
  FILE *fil;
  struct Node *lis;
  struct Entry *ent;
  void (*proc)(); 
} Types, *p_Types;

typedef struct Node {
  Operator op;
  Types u;
  struct Node *next;
} Node, *p_Node;

typedef struct Entry {
  char *name;
  int is_module;
  union {
    p_Node body;
    struct Entry *module_fields;
    void  (*proc) ();
  } u;
  struct Entry *next;
} Entry, *p_Entry;

typedef struct ExecutionContext { 
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
    p_Node memoryindex,
      mem_low,
      mem_mid;
    int direction;
    int nodesinspected, nodescopied;
    int start_gc_clock;
# endif

  } util;

  Entry					/* symbol table	*/
    symtab[SYMTABMAX];
  p_Entry
    hashentry[HASHSIZE],
    localentry,
    symtabindex,
    display[DISPLAYMAX],
    firstlibra,				/* inioptable	*/
    location;				/* getsym	*/

  p_Node				/* dynamic memory	*/
  /*
      memory[MEMORYMAX],
      *memoryindex,
  */
    prog, stk, conts,
    dump, dump1, dump2, dump3, dump4, dump5;

  jmp_buf begin, fail;
} ExecutionContext, *p_EC;

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
PUBLIC void stack_(p_EC ec);
PUBLIC void dummy_(p_EC ec);
PUBLIC void exeterm(p_EC ec, Node *n);
PUBLIC void inisymboltable(p_EC ec);	/* initialise */
PUBLIC char *opername(p_EC ec, int o);
PUBLIC void lookup(p_EC ec);
PUBLIC void abortexecution_(p_EC ec);
PUBLIC void execerror(p_EC ec, char *message, char *op);
PUBLIC void quit_(p_EC ec);
PUBLIC void inilinebuffer(p_EC ec);
PUBLIC void putline(p_EC ec);
PUBLIC int endofbuffer(p_EC ec);
PUBLIC void error(p_EC ec, char *message);
PUBLIC int doinclude(p_EC ec, char *filnam);
PUBLIC void getsym(p_EC ec);
PUBLIC void inimem0(p_EC ec);
PUBLIC void inimem1(p_EC ec);
PUBLIC void inimem2(p_EC ec);
PUBLIC void printnode(p_EC ec, Node *p);
PUBLIC void gc_(p_EC ec);
PUBLIC Node *newnode(p_EC ec, Operator o, Types u, Node *r);
PUBLIC void memoryindex_(p_EC ec);
PUBLIC void readfactor(p_EC ec);	/* read a JOY factor */
PUBLIC void readterm(p_EC ec);
PUBLIC void writefactor(p_EC ec, Node *n, FILE *stm);
PUBLIC void writeterm(p_EC ec, Node *n, FILE *stm);

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
