# Structure
```
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
```
# Variables
## srcfile
## g_argc
## g_argv
## echoflag
## autoput
## undeferror
## tracegc
## startclock
## gc_clock
## ch
## sym
## num
## dbl
## id
## hashvalue
## bucket
## display_enter
## display_lookup
## scanner
Holds info for nested file inclusions. 
## util

## symtab\[\]
## hashentry\[\]
## localentry
## symtabindex
## display\[\]
## firstlibra
## location
## prog
## stk
## conts
## dump
## dumpN
## begin
## fail
# Structures
## Scanner
## Util