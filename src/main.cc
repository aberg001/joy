/* file: main.c */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include <time.h>
#define ALLOC
#include "globals.h"

PRIVATE void enterglobal(p_EC ec)
{
  ec->location = ec->symtabindex++;
D(  printf("getsym, new: '%s'\n",ec->id); )
  char *new_location = static_cast<char *>(malloc(strlen(ec->id) + 1));
  strcpy(new_location, ec->id);
  ec->location->name = new_location;
  ec->location->u.body = NULL; /* may be assigned in definition */
  ec->location->next = ec->hashentry[ec->hashvalue];
D(  printf("entered %s at %ld\n",ec->id,LOC2INT(ec->location)); )
  ec->hashentry[ec->hashvalue] = ec->location;
}
PUBLIC void lookup(p_EC ec)
{
  int i;
D(  printf("%s  hashes to %d\n",ec->id,ec->hashvalue); )

  for (i = ec->display_lookup; i > 0; --i) {
    ec->location = ec->display[i];
    while (ec->location != NULL && strcmp(ec->id,ec->location->name) != 0)
      ec->location = ec->location->next;
    if (ec->location != NULL) /* found in local table */
      return; 
  }

  ec->location = ec->hashentry[ec->hashvalue];
  while (ec->location != ec->symtab && strcmp(ec->id,ec->location->name) != 0)
    ec->location = ec->location->next;
  if (ec->location == ec->symtab) /* not found, enter in global */
    enterglobal(ec);
}

PRIVATE void enteratom(p_EC ec)
{
  if (ec->display_enter > 0) { 
    ec->location = ec->symtabindex++;
D(	printf("hidden definition '%s' at %ld \n",ec->id,LOC2INT(ec->location)); )
    char *new_location = static_cast<char *>(malloc(strlen(ec->id) + 1));
    strcpy(new_location, ec->id);
    ec->location->name = new_location;
    ec->location->u.body = NULL; /* may be assigned later */
    ec->location->next = ec->display[ec->display_enter];
    ec->display[ec->display_enter] = ec->location; 
  }
  else 
    lookup(ec);
}

PRIVATE void defsequence(p_EC);		/* forward */
PRIVATE void compound_def(p_EC);		/* forward */

PRIVATE void definition(p_EC ec)
{
  Entry *here = NULL;
  if (ec->sym == LIBRA || ec->sym == JPRIVATE || ec->sym == HIDE || ec->sym == MODULE) { 
    compound_def(ec);
    if (ec->sym == END || ec->sym == PERIOD)
      getsym(ec);
    else 
      error(ec, " END or period '.' expected in compound definition");
    return; 
  }

  if (ec->sym != ATOM)
/*   NOW ALLOW EMPTY DEFINITION:
      { error("atom expected at start of definition");
	abortexecution_(ec); }
*/
    return;

  /* ec->sym == ATOM : */
  enteratom(ec);
  if (ec->location < ec->firstlibra) {
    printf("warning: overwriting inbuilt '%s'\n",ec->location->name);
    enterglobal(ec); 
  }
  here = ec->location;
  getsym(ec);
  if (ec->sym == EQDEF)
    getsym(ec);
  else 
    error(ec, " == expected in definition");
  readterm(ec);
D(  printf("assigned this body: "); )
D(  writeterm(ec->stk->u.lis, stdout); )
D(  printf("\n"); )
  if (here != NULL) {
    here->u.body = ec->stk->u.lis;
    here->is_module = 0;
  }
  ec->stk = ec->stk->next;
}

PRIVATE void defsequence(p_EC ec)
{
  definition(ec);
  while (ec->sym == SEMICOL) {
    getsym(ec);
    definition(ec);
  }
}

PRIVATE void compound_def(p_EC ec)
{
  switch (ec->sym) { 
    case MODULE : 
      { 
        Entry *here = NULL;
        getsym(ec);
        if (ec->sym != ATOM) {
          error(ec, "atom expected as name of module");
          abortexecution_(ec); }
        enteratom(ec);
        here = ec->location;
        getsym(ec);
        ++(ec->display_enter);
        ++(ec->display_lookup);
        ec->display[ec->display_enter] = NULL;
        compound_def(ec);
        here->is_module = 1;
        here->u.module_fields = ec->display[ec->display_enter];
        --(ec->display_enter);
        --(ec->display_lookup);
        break;
      }
    case JPRIVATE :
    case HIDE :
      { 
        getsym(ec);
        if (ec->display_lookup > ec->display_enter) {
          /* already inside module or hide */
          Entry *oldplace = ec->display[ec->display_lookup];
          /*
             printf("lookup = %d\n",LOC2INT(display[display_lookup]));
             printf("enter = %d\n",LOC2INT(display[display_enter]));
             */
          ++(ec->display_enter);
          defsequence(ec);
          --(ec->display_enter);
          /*
             printf("lookup = %d\n",LOC2INT(display[display_lookup]));
             printf("enter = %d\n",LOC2INT(display[display_enter]));
             */
          compound_def(ec);
          ec->display[ec->display_lookup] = oldplace;
        }
        else {
          ++(ec->display_enter);
          ++(ec->display_lookup);
          ec->display[ec->display_enter] = NULL;
          defsequence(ec);
          --(ec->display_enter);
          compound_def(ec);
          --(ec->display_lookup); 
        }
        break; 
      }
    case JPUBLIC :
    case LIBRA :
    case IN :
      {
        getsym(ec);
        defsequence(ec);
        break;
      }
    default :
      printf("warning: empty compound definition\n");
  }
}

PUBLIC void abortexecution_(p_EC ec)
{
    ec->conts = ec->dump = ec->dump1 = ec->dump2 = ec->dump3 = ec->dump4 = ec->dump5 = NULL;
    longjmp(ec->begin,0);
}

PUBLIC void fail_(p_EC ec)
{
    longjmp(ec->fail,1);
}

PUBLIC void execerror(p_EC ec, const char *message, const char *op)
{
    printf("run time error: %s needed for %s\n",message,op);
    abortexecution_(ec);
}

static int quit_quiet = 1;
   /* was = 0;  but anything with "clock" needs revision */
PUBLIC void quit_(p_EC ec)
{
    long totaltime;
    if (quit_quiet) exit(0);
    totaltime = clock() - ec->startclock;
#ifdef GC_BDW
    printf("Time:  %ld CPU\n", totaltime);
#else
    printf("time:  %ld CPU,  %d gc (= %ld%%)\n",
	totaltime, ec->gc_clock,
	totaltime ? (1004*ec->gc_clock)/(10*totaltime) : 0);
#endif
    exit(0);
}
static int mustinclude = 1;

#define CHECK(D,NAME)						\
    if (D)							\
      { printf("->  %s is not empty:\n",NAME);			\
	writeterm(ec, D, stdout); printf("\n"); }

int main(int argc, char **argv)
{
  int ch;
  ExecutionContext _ec;
  p_EC ec = &_ec;

  inimem0(ec);
  ec->g_argc = argc;
  ec->g_argv = argv;
  if (argc > 1) {
    /*
       FILE *f;
       */
    ec->g_argc--;
    ec->g_argv++;
    ec->srcfile = fopen(argv[1], "r");
    if (!ec->srcfile) { 
      printf("failed to open the file '%s'.\n", argv[1]);
      exit(1);
    }
  } 
  else {
    ec->srcfile = stdin;
#ifdef GC_BDW
    printf("JOY  -  compiled at %s on %s (BDW)\n",__TIME__,__DATE__);
#else
    printf("JOY  -  compiled at %s on %s (NOBDW)\n",__TIME__,__DATE__);
#endif
    printf("Copyright 2001 by Manfred von Thun\n"); 
  }
  ec->startclock = clock();
  ec->gc_clock = 0;
  ec->echoflag = INIECHOFLAG;
  ec->tracegc = INITRACEGC;
  ec->autoput = INIAUTOPUT;
  ch = ' ';
  inilinebuffer(ec);
  inisymboltable(ec);
  ec->display[0] = NULL;
  inimem1(ec); 
  inimem2(ec);
  setjmp(ec->begin);
  setjmp(ec->fail);
  D(printf("starting main loop\n"));
  while (1) {
    if (mustinclude) {
      mustinclude = 0;
      if (fopen("usrlib.joy","r"))
        doinclude(ec, "usrlib.joy");
    }
    getsym(ec);

    if (ec->sym == LIBRA || ec->sym == HIDE || ec->sym == MODULE ) {
      inimem1(ec);
      compound_def(ec);
      inimem2(ec);
    }
    else { 
      readterm(ec);
      D(printf("program is: "); writeterm(ec->stk->u.lis, stdout); printf("\n"));
      ec->prog = ec->stk->u.lis;
      ec->stk = ec->stk->next;
      ec->conts = NULL;
      exeterm(ec, ec->prog);
      if (ec->conts || ec->dump || ec->dump1 || ec->dump2 || ec->dump3 || ec->dump4 || ec->dump5) {
        printf("the dumps are not empty\n");
        CHECK(ec->conts,"conts");
        CHECK(ec->dump,"dump");
        CHECK(ec->dump1,"dump1");
        CHECK(ec->dump2,"dump2");
        CHECK(ec->dump3,"dump3");
        CHECK(ec->dump4,"dump4");
        CHECK(ec->dump5,"dump5"); 
      }
      if (ec->autoput == 2 && ec->stk != NULL) {
        writeterm(ec, ec->stk, stdout);
        printf("\n"); 
      }
      else if (ec->autoput == 1 && ec->stk != NULL) {
        writefactor(ec, ec->stk, stdout); 
        printf("\n"); 
        ec->stk = ec->stk->next;
      }
    }

    if (ec->sym != END && ec->sym != PERIOD)
      error(ec, " END or period '.' expected");
  }
}
