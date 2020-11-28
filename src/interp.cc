/* FILE: interp.c */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <setjmp.h>
#include "globals.h"
# ifdef GC_BDW
#    include "gc/gc.h"
#    define malloc GC_malloc_atomic
#    define realloc GC_realloc
#    define free(X)
# endif


PRIVATE void helpdetail_(pEC ec);		/* this file		*/
PRIVATE void undefs_(pEC ec);
PRIVATE void make_manual(pEC ec, int style /* 0=plain, 1=html, 2=latex */);
PRIVATE void manual_list_(pEC ec);
PRIVATE void manual_list_aux_(pEC ec);

template<typename T> PUBLIC void nullary(pEC ec, pNode (*constructor)(pEC, T, pNode), T value);
template<typename T> PUBLIC void unary(pEC ec, pNode (*constructor)(pEC, T, pNode), T value);
template<typename T> PUBLIC void binary(pEC ec, pNode (*constructor)(pEC, T, pNode), T value);
PRIVATE void gnullary(pEC ec, Operator type, Types value);

PRIVATE void hasOneParam(pEC ec, const char *name) {
  if (ec->stk == NULL)
    execerror(ec, "one parameter", name);
}
PRIVATE void hasTwoParams(pEC ec, const char *name) {
  if (ec->stk == NULL || ec->stk->next == NULL)
    execerror(ec, "two parameters", name);
}
PRIVATE void hasThreeParams(pEC ec, const char *name) {
  if (ec->stk == NULL || ec->stk->next == NULL
      || ec->stk->next->next == NULL)
    execerror(ec, "three parameters", name);
}
PRIVATE void hasFourParams(pEC ec, const char *name) {
  if (ec->stk == NULL || ec->stk->next == NULL
      || ec->stk->next->next == NULL
      || ec->stk->next->next->next == NULL)
    execerror(ec, "four parameters", name);
}
PRIVATE void hasFiveParams(pEC ec, const char *name) {
  if (ec->stk == NULL || ec->stk->next == NULL
      || ec->stk->next->next == NULL
      || ec->stk->next->next->next == NULL
      || ec->stk->next->next->next->next == NULL)
    execerror(ec, "five parameters", name);
}

PRIVATE void hasOneQuote(pEC ec, const char *name) {
  if (ec->stk->op != LIST_)
    execerror(ec, "quotation as top parameter", name);
}
PRIVATE void hasTwoQuotes(pEC ec, const char *name) {
  hasOneQuote(ec, name);
  if (ec->stk->next->op != LIST_)
    execerror(ec, "quotation as second parameter", name);
}
PRIVATE void hasThreeQuotes(pEC ec, const char *name) {
  hasTwoQuotes(ec, name);
  if (ec->stk->next->next->op != LIST_)
    execerror(ec, "quotation as third parameter", name);
}
PRIVATE void hasFourQuotes(pEC ec, const char *name) {
  hasThreeQuotes(ec, name);
  if (ec->stk->next->next->next->op != LIST_)
    execerror(ec, "quotation as fourth parameter", name);
}
PRIVATE void hasSame2types(pEC ec, const char *name) {
  if (ec->stk->op != ec->stk->next->op)
    execerror(ec, "two parameters of the same type", name);
}
PRIVATE void hasString(pEC ec, const char *name) {
  if (ec->stk->op != STRING_)
    execerror(ec, "string", name);
}
PRIVATE void hasString2(pEC ec, const char *name) {
  if (ec->stk->next->op != STRING_)
    execerror(ec, "string as second parameter", name);
}
PRIVATE void hasInteger(pEC ec, const char *name) {
  if (ec->stk->op != INTEGER_)
    execerror(ec, "integer", name);
}
PRIVATE void hasInteger2(pEC ec, const char *name) {
  if (ec->stk->next->op != INTEGER_)
    execerror(ec, "integer as second parameter", name);
}
PRIVATE void hasCharacter(pEC ec, const char *name) {
  if (ec->stk->op != CHAR_)
    execerror(ec, "character", name);
}
PRIVATE void hasIntegers2(pEC ec, const char *name) {
  if (ec->stk->op != INTEGER_ || ec->stk->next->op != INTEGER_)
    execerror(ec, "two integers", name);
}
PRIVATE void isNumerictype(pEC ec, const char *name) {
  if (ec->stk->op != INTEGER_ && ec->stk->op !=  CHAR_
      && ec->stk->op != BOOLEAN_)
    execerror(ec, "numeric", name);
}
PRIVATE void hasNumeric2(pEC ec, const char *name) {
  if (ec->stk->next->op != INTEGER_ && ec->stk->next->op != CHAR_)	\
    execerror(ec, "numeric second parameter", name);
}
PRIVATE bool isFloatable(pEC ec) {
  return (ec->stk->op == INTEGER_ || ec->stk->op == FLOAT_);
}
PRIVATE bool isFloatable2(pEC ec) {
  return ((ec->stk->op == FLOAT_ && ec->stk->next->op == FLOAT_) ||
          (ec->stk->op == FLOAT_ && ec->stk->next->op == INTEGER_) ||
          (ec->stk->op == INTEGER_ && ec->stk->next->op == FLOAT_));
}
PRIVATE void hasFloat(pEC ec, const char *name) {
  if (!isFloatable(ec))
    execerror(ec, "float or integer", name);
}
PRIVATE void hasFloat2(pEC ec, const char *name) {
  if (!(isFloatable(ec)
        || (ec->stk->op == INTEGER_ && ec->stk->next->op == INTEGER_)))
    execerror(ec, "two floats or integers", name);
}
PRIVATE double floatVal(pEC ec) {
  return (ec->stk->op == FLOAT_
          ? ec->stk->u.dbl
          : static_cast<double>(ec->stk->u.num));
}
PRIVATE double floatVal2(pEC ec) {
  return (ec->stk->next->op == FLOAT_
          ? ec->stk->next->u.dbl
          : static_cast<double>(ec->stk->next->u.num));
}
PRIVATE bool float_u(pEC ec, double(*oper)(double)) {
  if (isFloatable(ec)) {
    unary<double>(ec, &floatNewnode, oper(floatVal(ec)));
    return true;
  }
  return false;
}
PRIVATE bool float_p(pEC ec, double(*oper)(double, double)) {
  if (isFloatable2(ec)) {
    binary<double>(ec, &floatNewnode, oper(floatVal2(ec), floatVal(ec)));
    return true;
  }
  return false;
}
// PRIVATE bool float_i(pEC ec, const char *name) {
//   if (FLOATABLE2) {
//     binary<double>(ec, &floatNewnode, (FLOATVAL2) OPER (FLOATVAL)); return; }
// }
PRIVATE void hasFile(pEC ec, const char *name) {
  if (ec->stk->op != FILE_ || ec->stk->u.fil == NULL)
    execerror(ec, "file", name);
}
PRIVATE void checkZero(pEC ec, const char *name) {
  if (ec->stk->u.num == 0)
    execerror(ec, "non-zero operand", name);
}
PRIVATE void hasList(pEC ec, const char *name) {
  if (ec->stk->op != LIST_)
    execerror(ec, "list", name);
}
PRIVATE void hasList2(pEC ec, const char *name) {
  if (ec->stk->next->op != LIST_)
    execerror(ec, "list as second parameter", name);
}
PRIVATE void hasUserdef(pEC ec, const char *name) {
    if (ec->stk->op != USR_)					\
	execerror(ec, "user defined symbol", name);
}
PRIVATE void checkList(pEC ec, Operator opr, const char *name) {
  if (opr != LIST_)
    execerror(ec, "internal list", name);
}
PRIVATE void checkSetMember(pEC ec, pNode node, const char *name) {
  if ((node->op != INTEGER_ && node->op != CHAR_) ||
      node->u.num >= SETSIZE)
    execerror(ec, "small numeric", name);
}
PRIVATE void checkEmptySet(pEC ec, long set, const char *name) {
  if (set == 0)
    execerror(ec, "non-empty set", name);
}
PRIVATE void checkEmptyString(pEC ec, const char *string, const char *name) {
  if (*string == '\0')
    execerror(ec, "non-empty string", name);
}
PRIVATE void checkEmptyList(pEC ec, pNode list, const char *name) {
  if (list == NULL)
    execerror(ec, "non-empty list", name);
}
PRIVATE void indexTooLarge(pEC ec, const char *name) {
  execerror(ec, "smaller index", name);
}
PRIVATE void badAggregate(pEC ec, const char *name) {
  execerror(ec, "aggregate parameter", name);
}
PRIVATE void badData(pEC ec, const char *name) {
  execerror(ec, "different type", name);
}

PRIVATE pNode& dmp(pEC ec) {
  return ec->dump->u.lis;
}
PRIVATE pNode& dmp1(pEC ec) {
  return ec->dump1->u.lis;
}
PRIVATE pNode& dmp2(pEC ec) {
  return ec->dump2->u.lis;
}
PRIVATE pNode& dmp3(pEC ec) {
  return ec->dump3->u.lis;
}
PRIVATE pNode& dmp4(pEC ec) {
  return ec->dump4->u.lis;
}
PRIVATE pNode& dmp5(pEC ec) {
  return ec->dump5->u.lis;
}

PRIVATE pNode saveStack(pEC ec) {
  pNode dump = listNewnode(ec, ec->stk, ec->dump);
  ec->dump = dump;
  return ec->dump;
}
PRIVATE pNode& saved1(pEC ec) {
  return dmp(ec);
}
PRIVATE pNode& saved2(pEC ec) {
  return dmp(ec)->next;
}
PRIVATE pNode& saved3(pEC ec) {
  return dmp(ec)->next->next;
}
PRIVATE pNode& saved4(pEC ec) {
  return dmp(ec)->next->next->next;
}
PRIVATE pNode& saved5(pEC ec) {
  return dmp(ec)->next->next->next->next;
}
PRIVATE pNode& saved6(pEC ec) {
  return dmp(ec)->next->next->next->next->next;
}

// #define DMP1 ec->dump1->u.lis
// #define DMP2 ec->dump2->u.lis
// #define DMP3 ec->dump3->u.lis
// #define DMP4 ec->dump4->u.lis
// #define DMP5 ec->dump5->u.lis
// #define SAVESTACK  ec->dump = LIST_NEWNODE(ec->stk,ec->dump)
// #define SAVED1 DMP
// #define SAVED2 DMP->next
// #define SAVED3 DMP->next->next
// #define SAVED4 DMP->next->next->next
// #define SAVED5 DMP->next->next->next->next
// #define SAVED6 DMP->next->next->next->next->next

PRIVATE void pop(pNode &N) {
  N = N->next;
}
PRIVATE void pop(pEntry &N) {
  N = N->next;
}
template<typename T> PUBLIC void nullary(pEC ec, pNode (*constructor)(pEC, T, pNode), T value) {
  ec->stk = constructor(ec, value, ec->stk);
}
template<typename T> PUBLIC void unary(pEC ec, pNode (*constructor)(pEC, T, pNode), T value) {
  ec->stk = constructor(ec, value, ec->stk->next);
}
template<typename T> PUBLIC void binary(pEC ec, pNode (*constructor)(pEC, T, pNode), T value) {
  ec->stk = constructor(ec, value, ec->stk->next->next);
}
PRIVATE void gnullary(pEC ec, Operator type, Types value) {
  ec->stk = newnode(ec, type, value, ec->stk);
}
PRIVATE void gunary(pEC ec, Operator type, Types value) {
  ec->stk = newnode(ec, type, value, ec->stk->next);
}
PRIVATE void gbinary(pEC ec, Operator type, Types value) {
  ec->stk = newnode(ec, type, value, ec->stk->next->next);
}
PRIVATE void gternary(pEC ec, Operator type, Types value) {
  ec->stk = newnode(ec, type, value, ec->stk->next->next->next);
}

// #define pop(X) X = X->next

#define NULLARY(CONSTRUCTOR,VALUE)                                     \
    ec->stk = CONSTRUCTOR(VALUE, ec->stk)
#define UNARY(CONSTRUCTOR,VALUE)                                       \
    ec->stk = CONSTRUCTOR(VALUE, ec->stk->next)
#define BINARY(CONSTRUCTOR,VALUE)                                      \
    ec->stk = CONSTRUCTOR(VALUE, ec->stk->next->next)
#define GNULLARY(TYPE,VALUE)                                    \
    ec->stk = newnode(ec, TYPE, (VALUE), ec->stk)
#define GUNARY(TYPE,VALUE)                                      \
    ec->stk = newnode(ec, TYPE, (VALUE), ec->stk->next)
#define GBINARY(TYPE,VALUE)                                     \
    ec->stk = newnode(ec, TYPE, (VALUE), ec->stk->next->next)
#define GTERNARY(TYPE,VALUE)					\
    ec->stk = newnode(ec, TYPE, (VALUE), ec->stk->next->next->next)

#define GETSTRING(NODE)						\
  ( NODE->op == STRING_  ?  NODE->u.str :			\
   (NODE->op == USR_  ?  NODE->u.ent->name :			\
    opername(ec, NODE->op) ) )

/* - - - -  O P E R A N D S   - - - - */

// #define PUSH(PROCEDURE,CONSTRUCTOR,VALUE)				\
// PRIVATE void PROCEDURE(pEC ec)					\
// {   NULLARY(CONSTRUCTOR,VALUE); }
// PUSH(true_, BOOLEAN_NEWNODE, 1L)				/* constants	*/
// PUSH(false_, BOOLEAN_NEWNODE, 0L)
// PUSH(setsize_, INTEGER_NEWNODE, (long)SETSIZE)
// PUSH(maxint_, INTEGER_NEWNODE, (long)MAXINT)
// PUSH(symtabmax_, INTEGER_NEWNODE, (long)SYMTABMAX)
// PUSH(memorymax_, INTEGER_NEWNODE, (long)MEMORYMAX)
// PUSH(stdin_, FILE_NEWNODE, stdin)
// PUSH(stdout_, FILE_NEWNODE, stdout)
// PUSH(stderr_, FILE_NEWNODE, stderr)
// PUSH(dump_, LIST_NEWNODE, ec->dump)				/* variables	*/
// PUSH(conts_, LIST_NEWNODE, LIST_NEWNODE(ec->conts->u.lis->next, ec->conts->next))
// PUSH(symtabindex_, INTEGER_NEWNODE, (long)LOC2INT(ec->symtabindex))
// PUSH(rand_, INTEGER_NEWNODE, (long)rand())
// /* this is now in utils.c
// PUSH(memoryindex_, INTEGER_NEWNODE, MEM2INT(memoryindex))
// */
// PUSH(echo_, INTEGER_NEWNODE, (long)ec->echoflag)
// PUSH(autoput_, INTEGER_NEWNODE, (long)ec->autoput)
// PUSH(undeferror_, INTEGER_NEWNODE, (long)ec->undeferror)
// PUSH(clock_, INTEGER_NEWNODE, (long)(clock() - ec->startclock))
// PUSH(time_, INTEGER_NEWNODE, (long)time(NULL))
// PUSH(argc_, INTEGER_NEWNODE, (long)ec->g_argc)

PRIVATE void true_(pEC ec) {
  nullary<long>(ec, booleanNewnode, 1L);
}
PRIVATE void false_(pEC ec) {
  nullary<long>(ec, booleanNewnode, 1L);
}
PRIVATE void setsize_(pEC ec) {
  nullary<long>(ec, integerNewnode, static_cast<long>(SETSIZE));
}
PRIVATE void maxint_(pEC ec) {
  nullary<long>(ec, integerNewnode, static_cast<long>(MAXINT));
}
PRIVATE void symtabmax_(pEC ec) {
  nullary<long>(ec, integerNewnode, static_cast<long>(SYMTABMAX));
}
PRIVATE void memorymax_(pEC ec) {
  nullary<long>(ec, integerNewnode, static_cast<long>(MEMORYMAX));
}
PRIVATE void stdin_(pEC ec) {
  nullary<FILE *>(ec, fileNewnode, stdin);
}
PRIVATE void stdout_(pEC ec) {
  nullary<FILE *>(ec, fileNewnode, stdout);
}
PRIVATE void stderr_(pEC ec) {
  nullary<FILE *>(ec, fileNewnode, stderr);
}
PRIVATE void dump_(pEC ec) {
  nullary<pNode>(ec, listNewnode, ec->dump);
}
PRIVATE void conts_(pEC ec) {
  nullary<pNode>(ec, listNewnode, listNewnode(ec, ec->conts->u.lis->next, ec->conts->next));
}
PRIVATE void symtabindex_(pEC ec) {
  nullary<long>(ec, integerNewnode, static_cast<long>(LOC2INT(ec->symtabindex)));
}
PRIVATE void rand_(pEC ec) {
  nullary<long>(ec, integerNewnode, static_cast<long>(rand()));
}
PRIVATE void echo_(pEC ec) {
  nullary<long>(ec, integerNewnode, static_cast<long>(ec->echoflag));
}
PRIVATE void autoput_(pEC ec) {
  nullary<long>(ec, integerNewnode, static_cast<long>(ec->autoput));
}
PRIVATE void undeferror_(pEC ec) {
  nullary<long>(ec, integerNewnode, static_cast<long>(ec->undeferror));
}
PRIVATE void clock_(pEC ec) {
  nullary<long>(ec, integerNewnode, static_cast<long>(ec->autoput));
}
PRIVATE void time_(pEC ec) {
  nullary<long>(ec, integerNewnode, static_cast<long>(time(NULL)));
}
PRIVATE void argc_(pEC ec) {
  nullary<long>(ec, integerNewnode, static_cast<long>(ec->g_argc));
}

PUBLIC void stack_(pEC ec) {
  nullary<pNode>(ec, listNewnode, ec->stk);
}

/* - - - - -   O P E R A T O R S   - - - - - */

PRIVATE void id_(pEC ec) {
    /* do nothing */
}
PRIVATE void unstack_(pEC ec) {
    hasOneParam(ec, "unstack");
    hasList(ec, "unstack");
    ec->stk = ec->stk->u.lis;
}
/*
PRIVATE void newstack_(pEC ec) {
    ec->stk = NULL;
}
*/

/* - - -   STACK   - - - */

PRIVATE void name_(pEC ec) {
  hasOneParam(ec, "name");
  unary<const char *>(ec, stringNewnode,
      ec->stk->op == USR_ ? ec->stk->u.ent->name : opername(ec, ec->stk->op));
}

PRIVATE void intern_(pEC ec) {
  char *p;
  hasOneParam(ec, "intern");
  hasString(ec, "intern");
  strcpy(ec->id, ec->stk->u.str);
  ec->hashvalue = 0;
  for (p = ec->id; *p; p++) ec->hashvalue += *p;
  ec->hashvalue %= HASHSIZE;
  lookup(ec);
  if (ec->location < ec->firstlibra) {
    ec->bucket.proc = ec->location->u.proc;
    GUNARY(static_cast<Operator>(LOC2INT(ec->location)), ec->bucket);
  }
  else 
    unary<pEntry>(ec, usrNewnode, ec->location);
}

PRIVATE void getenv_(pEC ec) {
  hasOneParam(ec, "getenv");
  hasString(ec, "getenv");
  unary<const char *>(ec, stringNewnode, getenv(ec->stk->u.str));
}

PRIVATE void body_(pEC ec) {
  hasOneParam(ec, "body");
  hasUserdef(ec, "body");
  unary<pNode>(ec, listNewnode, ec->stk->u.ent->u.body);
}

PRIVATE void pop_(pEC ec) {
  hasOneParam(ec, "pop");
  pop(ec->stk);
}

PRIVATE void swap_(pEC ec) {
  hasTwoParams(ec, "swap");
  saveStack(ec);
  GBINARY(saved1(ec)->op, saved1(ec)->u);
  GNULLARY(saved2(ec)->op, saved2(ec)->u);
  pop(ec->dump);
}

PRIVATE void rollup_(pEC ec) {
  hasThreeParams(ec, "rollup");
  saveStack(ec);
  GTERNARY(saved1(ec)->op, saved1(ec)->u);
  GNULLARY(saved3(ec)->op, saved3(ec)->u);
  GNULLARY(saved2(ec)->op, saved2(ec)->u);
  pop(ec->dump);
}

PRIVATE void rolldown_(pEC ec) {
  hasThreeParams(ec, "rolldown");
  saveStack(ec);
  GTERNARY(saved2(ec)->op, saved2(ec)->u);
  GNULLARY(saved1(ec)->op, saved1(ec)->u);
  GNULLARY(saved3(ec)->op, saved3(ec)->u);
  pop(ec->dump);
}

PRIVATE void rotate_(pEC ec) {
  hasThreeParams(ec, "rotate");
  saveStack(ec);
  GTERNARY(saved1(ec)->op, saved1(ec)->u);
  GNULLARY(saved2(ec)->op, saved2(ec)->u);
  GNULLARY(saved3(ec)->op, saved3(ec)->u);
  pop(ec->dump);
}

PRIVATE void dup_(pEC ec) {
  hasOneParam(ec, "dup");
  GNULLARY(ec->stk->op, ec->stk->u);
}

PRIVATE void dipped(pEC ec, const char *name,
                    void (*paramcount)(pEC, const char *),
                    void(*argument)(pEC)) {
  paramcount(ec, name);
  saveStack(ec);
  pop(ec->stk);
  argument(ec);
  gnullary(ec, saved1(ec)->op, saved1(ec)->u);
  pop(ec->dump);
}
PRIVATE void popd_(pEC ec) {
  dipped(ec, "popd", &hasTwoParams, &pop_);
}
PRIVATE void dupd_(pEC ec) {
  dipped(ec, "dupd", &hasTwoParams, &dup_);
}
PRIVATE void swapd_(pEC ec) {
  dipped(ec, "swapd", &hasThreeParams, &swapd_);
}
PRIVATE void rolldownd_(pEC ec) {
  dipped(ec, "rolldownd", &hasFourParams, &rolldown_);
}
PRIVATE void rollupd_(pEC ec) {
  dipped(ec, "rollupd", &hasFourParams, &rollup_);
}
PRIVATE void rotated_(pEC ec) {
  dipped(ec, "rotated", &hasFourParams, &rotate_);
}

// #define DIPPED(PROCEDURE, NAME, PARAMCOUNT, ARGUMENT)              \
// PRIVATE void PROCEDURE(pEC ec) {                               \
//     PARAMCOUNT(NAME);                                           \
//     SAVESTACK;                                                  \
//     pop(ec->stk);                                                   \
//     ARGUMENT(ec);                                                 \
//     GNULLARY(SAVED1->op, SAVED1->u);                             \
//     pop(ec->dump);                                                  \
// }
// DIPPED(popd_, "popd", HASTWOPARAMS, pop_)
// DIPPED(dupd_, "dupd", HASTWOPARAMS, dup_)
// DIPPED(swapd_, "swapd", HASTHREEPARAMS, swap_)
// DIPPED(rolldownd_, "rolldownd", HASFOURPARAMS, rolldown_)
// DIPPED(rollupd_, "rollupd", HASFOURPARAMS, rollup_)
// DIPPED(rotated_, "rotated", HASFOURPARAMS, rotate_)

/* - - -   BOOLEAN   - - - */

#define ANDORXOR(PROCEDURE, NAME, OPER1, OPER2)			\
PRIVATE void PROCEDURE(pEC ec)	{				\
    hasTwoParams(ec, NAME);						\
    hasSame2types(ec, NAME);						\
    switch (ec->stk->next->op) {				\
        case SET_:						\
	    binary<long>(ec, &setNewnode, static_cast<long>(ec->stk->next->u.set OPER1 ec->stk->u.set));	\
	    return;						\
	case BOOLEAN_: case CHAR_: case INTEGER_: case LIST_:	\
	    binary<long>(ec, &booleanNewnode, static_cast<long>(ec->stk->next->u.num OPER2 ec->stk->u.num));	\
	    return;						\
	default:						\
	    badData(ec, NAME); } }
ANDORXOR(and_, "and", &, &&)
ANDORXOR(or_, "or", |, ||)
ANDORXOR(xor_, "xor", ^, !=)


/* - - -   INTEGER   - - - */

#define ORDCHR(PROCEDURE, NAME, RESULTTYP)			\
PRIVATE void PROCEDURE(pEC ec)	{				\
    hasOneParam(ec, NAME);						\
    isNumerictype(ec, NAME);						\
    unary<long>(ec, RESULTTYP, ec->stk->u.num);				\
}
ORDCHR(ord_, "ord", &integerNewnode)
ORDCHR(chr_, "chr", &charNewnode)

PRIVATE void abs_(pEC ec) {
  hasOneParam(ec, "abs");
  /* start new */
  hasFloat(ec, "abs");
  if (ec->stk->op == INTEGER_) { 
    if (ec->stk->u.num >= 0) 
      return;
    else { 
      unary<long>(ec, &integerNewnode, - ec->stk->u.num);
      return; 
    } 
  }
  /* end new */
  if (float_u(ec, fabs))
    return;
  hasInteger(ec, "abs");
  if (ec->stk->u.num < 0) 
    unary<long>(ec, &integerNewnode, - ec->stk->u.num);
}

PRIVATE double fsgn(double f) {
  if (f < 0)
    return -1.0;
  else 
    if (f > 0) 
      return 1.0;
    else 
      return 0.0;
}

PRIVATE void sign_(pEC ec) {
  hasOneParam(ec, "sign");
  /* start new */
  hasFloat(ec, "sign");
  if (ec->stk->op == INTEGER_) { 
    long i = ec->stk->u.num;
    if (i == 0 || i == 1) 
      return;
    else { 
      unary<long>(ec, &integerNewnode, i > 0 ? 1 : -1);
      return; 
    } 
  }
  /* end new */
  if (float_u(ec, fsgn))
    return;
  hasInteger(ec, "sign");
  if (ec->stk->u.num < 0)
    unary<long>(ec, &integerNewnode, -1L);
  else 
    if (ec->stk->u.num > 0)
      unary<long>(ec, &integerNewnode, 1L);
}

PRIVATE void neg_(pEC ec) {
  hasOneParam(ec, "neg");
  if (float_u(ec, [](double a) -> double { return -a; }))
    return;
  hasInteger(ec, "neg");
  unary<long>(ec, &integerNewnode, -ec->stk->u.num);
}

/* probably no longer needed:
#define MULDIV(PROCEDURE, NAME, OPER, CHECK)			\
PRIVATE void PROCEDURE(pEC ec)	{				\
    HASTWOPARAMS(NAME);						\
    FLOAT_I(OPER);						\
    INTEGERS2(NAME);						\
    CHECK;							\
    BINARY(INTEGER_NEWNODE, ec->stk->next->u.num OPER ec->stk->u.num); }
MULDIV(mul_, "*", *,)
MULDIV(divide_, "/", /, checkZero(ec, "/"))
*/

PRIVATE void mul_(pEC ec) {
  hasTwoParams(ec, "*");
  if (float_p(ec, [](double a, double b)->double { return a * b; }))
    return;
  hasIntegers2(ec, "*");
  binary<long>(ec, &integerNewnode, ec->stk->next->u.num * ec->stk->u.num);
}

PRIVATE void divide_(pEC ec) {
  hasTwoParams(ec, "/");
  if ((ec->stk->op == FLOAT_   && ec->stk->u.dbl == 0.0)  ||
      (ec->stk->op == INTEGER_ && ec->stk->u.num == 0))
    execerror(ec, "non-zero divisor", "/");
  if (float_p(ec, [](double a, double b)->double { return a / b; }))
    return;
  hasIntegers2(ec, "/");
  binary<long>(ec, &integerNewnode, ec->stk->next->u.num / ec->stk->u.num);
}

PRIVATE void rem_(pEC ec) {
  hasTwoParams(ec, "rem");
  if (float_p(ec, fmod))
    return;
  hasIntegers2(ec, "rem");
  checkZero(ec, "rem");
  binary<long>(ec, &integerNewnode, ec->stk->next->u.num % ec->stk->u.num);
}

PRIVATE void div_(pEC ec) {
  ldiv_t result;
  hasTwoParams(ec, "div");
  hasIntegers2(ec, "div");
  checkZero(ec, "div");
  result = ldiv(ec->stk->next->u.num, ec->stk->u.num);
  binary<long>(ec, &integerNewnode, result.quot);
  nullary<long>(ec, &integerNewnode, result.rem);
}

PRIVATE void strtol_(pEC ec) {
  hasTwoParams(ec, "strtol");
  saveStack(ec);
  hasInteger(ec, "strtol");
  pop(ec->stk);
  hasString(ec, "strtol");
  unary<long>(ec, &integerNewnode, strtol(saved2(ec)->u.str, NULL, saved1(ec)->u.num));
  pop(ec->dump);
}

PRIVATE void strtod_(pEC ec) {
  hasOneParam(ec, "strtod");
  hasString(ec, "strtod");
  unary<double>(ec, &floatNewnode, strtod(ec->stk->u.str, NULL));
}

PRIVATE void format_(pEC ec) {
  int width, prec;
  char spec;
  char format[7];
  char *result;
  hasFourParams(ec, "format");
  hasInteger(ec, "format");
  hasInteger2(ec, "format");
  prec = ec->stk->u.num;
  pop(ec->stk);
  width = ec->stk->u.num;
  pop(ec->stk);
  hasCharacter(ec, "format");
  spec = ec->stk->u.num;
  pop(ec->stk);
  if (!strchr("dioxX", spec))
    execerror(ec, "one of: d i o x X", "format");
  strcpy(format, "%*.*ld");
  format[5] = spec;
  result = static_cast<char *>(malloc(INPLINEMAX));			/* should be sufficient */
  isNumerictype(ec, "format");
  sprintf(result, format, width, prec, ec->stk->u.num);
  unary<const char *>(ec, &stringNewnode, result);
  return; 
}

PRIVATE void formatf_(pEC ec) {
  int width, prec;
  char spec;
  char format[7];
  char *result;
  hasFourParams(ec, "format");
  hasInteger(ec, "format");
  hasInteger2(ec, "format");
  prec = ec->stk->u.num;
  pop(ec->stk);
  width = ec->stk->u.num;
  pop(ec->stk);
  hasCharacter(ec, "format");
  spec = ec->stk->u.num;
  pop(ec->stk);
  if (!strchr("eEfgG", spec))
    execerror(ec, "one of: e E f g G", "format");
  strcpy(format, "%*.*lg");
  format[5] = spec;
  result = static_cast<char *>(malloc(INPLINEMAX));			/* should be sufficient */
  hasFloat(ec, "formatf");
  sprintf(result, format, width, prec, ec->stk->u.dbl);
  unary<const char *>(ec, &stringNewnode, result);
}


/* - - -   TIME   - - - */

#define UNMKTIME(PROCEDURE, NAME, FUNC)				\
PRIVATE void PROCEDURE(pEC ec) {				\
    struct tm *t;						\
    long wday;							\
    time_t timval;						\
    hasOneParam(ec, NAME);						\
    hasInteger(ec, NAME);						\
    timval = ec->stk->u.num;					\
    t = FUNC(&timval);						\
    wday = t->tm_wday;						\
    if (wday == 0) 						\
        wday = 7;					\
    ec->dump1 = listNewnode(ec, NULL, ec->dump1);			\
    dmp1(ec) = integerNewnode(ec, wday, dmp1(ec));		\
    dmp1(ec) = integerNewnode(ec, (long)t->tm_yday, dmp1(ec));	\
    dmp1(ec) = booleanNewnode(ec, (long)t->tm_isdst, dmp1(ec));	\
    dmp1(ec) = integerNewnode(ec, (long)t->tm_sec, dmp1(ec));	\
    dmp1(ec) = integerNewnode(ec, (long)t->tm_min, dmp1(ec));	\
    dmp1(ec) = integerNewnode(ec, (long)t->tm_hour, dmp1(ec));	\
    dmp1(ec) = integerNewnode(ec, (long)t->tm_mday, dmp1(ec));	\
    dmp1(ec) = integerNewnode(ec, (long)(t->tm_mon + 1), dmp1(ec)); \
    dmp1(ec) = integerNewnode(ec, (long)(t->tm_year + 1900), dmp1(ec)); \
    unary<pNode>(ec, &listNewnode, dmp1(ec));						\
    pop(ec->dump1);							\
    return; 							\
}

UNMKTIME(localtime_, "localtime", localtime)
UNMKTIME(gmtime_, "gmtime", gmtime)

PRIVATE void decode_time(pEC ec, struct tm *t) {
  Node *p;
  t->tm_year = t->tm_mon = t->tm_mday =
    t->tm_hour = t->tm_min = t->tm_sec = t->tm_isdst =
    t->tm_yday = t->tm_wday = 0;
  p = ec->stk->u.lis;
  if (p && p->op == INTEGER_) {
    t->tm_year = p->u.num - 1900;
    pop(p);
  }
  if (p && p->op == INTEGER_) {
    t->tm_mon = p->u.num - 1;
    pop(p);
  }
  if (p && p->op == INTEGER_) {
    t->tm_mday = p->u.num;
    pop(p);
  }
  if (p && p->op == INTEGER_) {
    t->tm_hour = p->u.num;
    pop(p);
  }
  if (p && p->op == INTEGER_) {
    t->tm_min = p->u.num;
    pop(p);
  }
  if (p && p->op == INTEGER_) {
    t->tm_sec = p->u.num;
    pop(p);
  }
  if (p && p->op == BOOLEAN_) {
    t->tm_isdst = p->u.num;
    pop(p);
  }
  if (p && p->op == INTEGER_) {
    t->tm_yday = p->u.num;
    pop(p);
  }
  if (p && p->op == INTEGER_) {
    t->tm_wday = p->u.num;
    pop(p);
  }
  return; 
}

PRIVATE void mktime_(pEC ec) {
  struct tm t;
  hasOneParam(ec, "mktime");
  hasList(ec, "mktime");
  decode_time(ec, &t);
  unary<long>(ec, &integerNewnode, (long)mktime(&t));
  return; 
}

PRIVATE void strftime_(pEC ec) {
  struct tm t;
  const char *fmt;
  char *result;
  size_t length;
  hasTwoParams(ec, "strftime");
  hasString(ec, "strftime");
  fmt = ec->stk->u.str;
  pop(ec->stk);
  hasList(ec, "strftime");
  decode_time(ec, &t);
  length = strlen(fmt) * 3 + 1;		/* should be sufficient */
  result = static_cast<char *>(malloc(length));
  strftime(result, length, fmt, &t);
  unary<const char *>(ec, &stringNewnode, result);
  return; 
}

/* - - -   FLOAT   - - - */

#define UFLOAT(PROCEDURE, NAME, FUNC)				\
PRIVATE void PROCEDURE(pEC ec)	{				\
    hasOneParam(ec, NAME);						\
    hasFloat(ec, NAME);						\
    unary<double>(ec, &floatNewnode, FUNC(floatVal(ec)));				\
    return; }
UFLOAT(acos_, "acos", acos)
UFLOAT(asin_, "asin", asin)
UFLOAT(atan_, "atan", atan)
UFLOAT(ceil_, "ceil", ceil)
UFLOAT(cos_, "cos", cos)
UFLOAT(cosh_, "cosh", cosh)
UFLOAT(exp_, "exp", exp)
UFLOAT(floor_, "floor", floor)
UFLOAT(log_, "log", log)
UFLOAT(log10_, "log10", log10)
UFLOAT(sin_, "sin", sin)
UFLOAT(sinh_, "sinh", sinh)
UFLOAT(sqrt_, "sqrt", sqrt)
UFLOAT(tan_, "tan", tan)
UFLOAT(tanh_, "tanh", tanh)

#define BFLOAT(PROCEDURE, NAME, FUNC)				\
PRIVATE void PROCEDURE(pEC ec)	{				\
    hasTwoParams(ec, NAME);						\
    hasFloat2(ec, NAME);						\
    binary<double>(ec, &floatNewnode, FUNC(floatVal2(ec), floatVal(ec)));			\
    return; }
BFLOAT(atan2_, "atan2", atan2)
BFLOAT(pow_, "pow", pow)

PRIVATE void frexp_(pEC ec) {
  int exp;
  hasOneParam(ec, "frexp");
  hasFloat(ec, "frexp");
  unary<double>(ec, &floatNewnode, frexp(floatVal(ec), &exp));
  nullary<long>(ec, &integerNewnode, (long)exp);
  return; 
}

PRIVATE void modf_(pEC ec) {
  double exp;
  hasOneParam(ec, "frexp");
  hasFloat(ec, "frexp");
  unary<double>(ec, &floatNewnode, modf(floatVal(ec), &exp));
  nullary<double>(ec, &floatNewnode, exp);
  return; 
}

PRIVATE void ldexp_(pEC ec) {
  long exp;
  hasTwoParams(ec, "ldexp");
  hasInteger(ec, "ldexp");
  exp = ec->stk->u.num;
  pop(ec->stk);
  hasFloat(ec, "ldexp");
  unary<double>(ec, floatNewnode, ldexp(floatVal(ec), static_cast<int>(exp)));
  return; 
}

PRIVATE void trunc_(pEC ec) {
  hasOneParam(ec, "trunc");
  hasFloat(ec, "trunc");
  unary<long>(ec, integerNewnode, static_cast<long>(floatVal(ec)));
}

/* - - -   NUMERIC   - - - */

#define PREDSUCC(PROCEDURE, NAME, OPER)				\
PRIVATE void PROCEDURE(pEC ec)	{				\
    hasOneParam(ec, NAME);						\
    isNumerictype(ec, NAME);						\
    if (ec->stk->op == CHAR_)					\
	unary<long>(ec, charNewnode, ec->stk->u.num OPER 1);			\
    else unary<long>(ec, integerNewnode, ec->stk->u.num OPER 1); }
PREDSUCC(pred_, "pred", -)
PREDSUCC(succ_, "succ", +)

#define PLUSMINUS(PROCEDURE, NAME, OPER)				\
PRIVATE void PROCEDURE(pEC ec)	{				\
    hasTwoParams(ec, NAME);						\
    float_p(ec, [](double a, double b) -> double { return a OPER b; }); \
    hasInteger(ec, NAME);						\
    hasNumeric2(ec, NAME);						\
    if (ec->stk->next->op == CHAR_)					\
	binary<long>(ec, &charNewnode, ec->stk->next->u.num OPER ec->stk->u.num);	\
    else binary<long>(ec, &integerNewnode, ec->stk->next->u.num OPER ec->stk->u.num); }
PLUSMINUS(plus_, "+", +)
PLUSMINUS(minus_, "-", -)

#define MAXMIN(PROCEDURE, NAME, OPER)				\
PRIVATE void PROCEDURE(pEC ec)	{				\
    hasTwoParams(ec, NAME);						\
    if (isFloatable2(ec))						\
      { binary<double>(ec, &floatNewnode,						\
	    floatVal(ec) OPER floatVal2(ec) ?				\
	    floatVal2(ec) : floatVal(ec));				\
	return; } 						\
    hasSame2types(ec, NAME);						\
    isNumerictype(ec, NAME);						\
    if (ec->stk->op == CHAR_)					\
	binary<long>(ec, &charNewnode,					\
	    ec->stk->u.num OPER ec->stk->next->u.num ?			\
	    ec->stk->next->u.num : ec->stk->u.num);			\
    else binary<long>(ec, &integerNewnode,					\
	    ec->stk->u.num OPER ec->stk->next->u.num ?			\
	    ec->stk->next->u.num : ec->stk->u.num); }
MAXMIN(max_, "max", <)
MAXMIN(min_, "min", >)

#define COMPREL(PROCEDURE, NAME, CONSTRUCTOR, OPR)				\
PRIVATE void PROCEDURE(pEC ec)	{				\
    long comp = 0;						\
    hasTwoParams(ec, NAME);						\
    switch (ec->stk->op)						\
      { case BOOLEAN_: case CHAR_: case INTEGER_:		\
	    if (isFloatable2(ec))					\
		comp = floatVal2(ec) - floatVal(ec) OPR 0;		\
	    else						\
		comp = ec->stk->next->u.num - ec->stk->u.num OPR 0;	\
	    break;						\
	case FLOAT_:						\
	    if (isFloatable2(ec))					\
		comp = floatVal2(ec) - floatVal(ec) OPR 0;		\
	    else						\
		comp = 0;					\
	    break;						\
	case SET_:						\
	  { int i = 0;						\
	    while ( i < SETSIZE &&				\
		    ( (ec->stk->next->u.set & 1 << i) ==		\
		      (ec->stk->u.set & 1 << i) )  )		\
		++i; 						\
	    if (i == SETSIZE) i = 0; else ++i;			\
	    if (!(ec->stk->u.set & 1 << i)) i = -i;			\
	    comp = i OPR 0;					\
	    break; }						\
	case LIST_:						\
	    badData(ec, NAME);					\
	default:						\
	    if (ec->stk->next->op == LIST_)				\
	      badData(ec, NAME);					\
	    comp = strcmp(GETSTRING(ec->stk->next), GETSTRING(ec->stk))	\
		   OPR 0;					\
	    break; }						\
    ec->stk = CONSTRUCTOR(ec, comp, ec->stk->next->next); }

COMPREL(eql_, "=", booleanNewnode, ==)
COMPREL(neql_, "!=", booleanNewnode, !=)
COMPREL(less_, "<", booleanNewnode, <)
COMPREL(leql_, "<=", booleanNewnode, <=)
COMPREL(greater_, ">", booleanNewnode, >)
COMPREL(geql_, ">=", booleanNewnode, >=)
COMPREL(compare_, "compare", integerNewnode, +)

/* - - -   FILES AND STREAMS   - - - */

PRIVATE void fopen_(pEC ec) {
  hasTwoParams(ec, "fopen");
  hasString(ec, "fopen");
  hasString2(ec, "fopen");
  binary<FILE *>(ec, &fileNewnode, fopen(ec->stk->next->u.str, ec->stk->u.str));
  return; 
}

PRIVATE void fclose_(pEC ec) {
  hasOneParam(ec, "fclose");
  if (ec->stk->op == FILE_ && ec->stk->u.fil == NULL) { 
    pop(ec->stk);
    return; 
  }
  hasFile(ec, "fclose");
  fclose(ec->stk->u.fil);
  pop(ec->stk);
  return; 
}

PRIVATE void fflush_(pEC ec) {
  hasOneParam(ec, "fflush");
  hasFile(ec, "fflush");
  fflush(ec->stk->u.fil);
  return; 
}

PRIVATE void fremove_(pEC ec) {
  hasOneParam(ec, "fremove");
  hasString(ec, "fremove");
  unary<long>(ec, &booleanNewnode, (long)!remove(ec->stk->u.str));
  return; 
}

PRIVATE void frename_(pEC ec) {
  hasTwoParams(ec, "frename");
  hasString(ec, "frename");
  hasString2(ec, "frename");
  binary<long>(ec, &booleanNewnode, (long)!rename(ec->stk->next->u.str, ec->stk->u.str));
  return; 
}

#define FILEGET(PROCEDURE,NAME,CONSTRUCTOR,EXPR)			\
PRIVATE void PROCEDURE(pEC ec)	{				\
    hasOneParam(ec, NAME);						\
    hasFile(ec, NAME);							\
    nullary<long>(ec, CONSTRUCTOR, EXPR);						\
    return; }
FILEGET(feof_,"feof",booleanNewnode,(long)feof(ec->stk->u.fil))
FILEGET(ferror_,"ferror",booleanNewnode,(long)ferror(ec->stk->u.fil))
FILEGET(fgetch_,"fgetch",charNewnode,(long)getc(ec->stk->u.fil))
FILEGET(ftell_,"ftell",integerNewnode,ftell(ec->stk->u.fil))

PRIVATE void fgets_(pEC ec) {
  int length = 0;
  int size = INPLINEMAX;
  char *buff = NULL;
  hasOneParam(ec, "fgets");
  hasFile(ec, "fgets");
  for (;;) { 
    buff = static_cast<char *>(realloc(buff, size));
    if (fgets(buff + length, size - length, ec->stk->u.fil) == NULL) {
      buff[length] = 0;
      break;
    }
    if (strchr(buff, '\n')) break;
    length += strlen(buff);
    size = size * 2; }
  nullary<const char *>(ec, &stringNewnode, buff);
  return;
}

PRIVATE void fput_(pEC ec) {
  FILE *stm;
  hasTwoParams(ec, "fput");
  if (ec->stk->next->op != FILE_ || (stm = ec->stk->next->u.fil) == NULL)
    execerror(ec, "file", "fput");
  writefactor(ec, ec->stk, stm);
  fprintf(stm, " ");
  pop(ec->stk);
  return; 
}

PRIVATE void fputch_(pEC ec) {
  int ch;
  hasTwoParams(ec, "fputch");
  hasInteger(ec, "fputch");
  ch = ec->stk->u.num;
  pop(ec->stk);
  hasFile(ec, "fputch");
  putc(ch, ec->stk->u.fil);
  return; 
}

PRIVATE void fputchars_(pEC ec) { /* suggested by Heiko Kuhrt, as "fputstring_" */
  FILE *stm;
  hasTwoParams(ec, "fputchars");
  if (ec->stk->next->op != FILE_ || (stm = ec->stk->next->u.fil) == NULL)
    execerror(ec, "file", "fputchars");
  /* fprintf(stm, ec->stk->u.str); */
  fputs(ec->stk->u.str, stm);
  pop(ec->stk);
  return; 
}

PRIVATE void fread_(pEC ec) {
  unsigned char *buf;
  long count;
  hasTwoParams(ec, "fread");
  hasInteger(ec, "fread");
  count = ec->stk->u.num;
  pop(ec->stk);
  hasFile(ec, "fread");
  buf = static_cast<unsigned char *>(malloc(count));
  ec->dump1 = listNewnode(ec, NULL, ec->dump1);
  for (count = fread(buf, (size_t)1, (size_t)count, ec->stk->u.fil) - 1; count >= 0; count--)
    dmp1(ec) = integerNewnode(ec, (long)buf[count], dmp1(ec));
  free(buf);
  unary<pNode>(ec, &listNewnode, dmp1(ec));
  pop(ec->dump1);
  return; 
}

PRIVATE void fwrite_(pEC ec) {
  int length;
  int i;
  unsigned char *buff;
  Node *n;
  hasTwoParams(ec, "fwrite");
  hasList(ec, "fwrite");
  for (n = ec->stk->u.lis, length = 0; n; n = n->next, length++)
    if (n->op != INTEGER_) execerror(ec, "numeric list", "fwrite");
  buff = static_cast<unsigned char *>(malloc(length));
  for (n = ec->stk->u.lis, i = 0; n; n = n->next, i++)
    buff[i] = n->u.num;
  pop(ec->stk);
  hasFile(ec, "fwrite");
  fwrite(buff, (size_t)length, (size_t)1, ec->stk->u.fil);
  return; 
}

PRIVATE void fseek_(pEC ec) {
  long pos;
  int whence;
  hasThreeParams(ec, "fseek");
  hasInteger(ec, "fseek");
  hasInteger2(ec, "fseek");
  whence = ec->stk->u.num;
  pop(ec->stk);
  pos = ec->stk->u.num;
  pop(ec->stk);
  hasFile(ec, "fseek");
  nullary<long>(ec, &booleanNewnode, (long)!!fseek(ec->stk->u.fil, pos, whence));
  return; 
}


/* - - -   AGGREGATES   - - - */

PRIVATE void first_(pEC ec) {
  hasOneParam(ec, "first");
  switch (ec->stk->op) {
    case LIST_:
      checkEmptyList(ec, ec->stk->u.lis, "first");
      GUNARY(ec->stk->u.lis->op,ec->stk->u.lis->u);
      return;
    case STRING_:
      checkEmptyString(ec, ec->stk->u.str, "first");
      unary<long>(ec, &charNewnode,(long)*(ec->stk->u.str));
      return;
    case SET_: 
      { 
        long i = 0;
        checkEmptySet(ec, ec->stk->u.set, "first");
        while (!(ec->stk->u.set & (1 << i))) i++;
        unary<long>(ec, &integerNewnode,i);
        return; 
      }
    default:
      badAggregate(ec, "first");
  }
}

PRIVATE void rest_(pEC ec) {
  hasOneParam(ec, "rest");
  switch (ec->stk->op) {
    case SET_:
      { 
        int i = 0;
        checkEmptySet(ec, ec->stk->u.set, "rest");
        while (!(ec->stk->u.set & (1 << i))) i++;
        unary<long>(ec, &setNewnode,ec->stk->u.set & ~(1 << i));
        break; 
      }
    case STRING_:
      {
        const char *s = ec->stk->u.str;
        checkEmptyString(ec, s, "rest");
        unary<const char *>(ec, &stringNewnode, ++s);
        break; 
      }
    case LIST_:
      checkEmptyList(ec, ec->stk->u.lis, "rest");
      unary<pNode>(ec, &listNewnode,ec->stk->u.lis->next);
      return;
    default:
      badAggregate(ec, "rest"); 
  }
}

PRIVATE void uncons_(pEC ec) {
  hasOneParam(ec, "uncons");
  switch (ec->stk->op) {
    case SET_:
      {
        long i = 0; long set = ec->stk->u.set;
        checkEmptySet(ec, set,"uncons");
        while (!(set & (1 << i))) i++;
        unary<long>(ec, &integerNewnode,i);
        nullary<long>(ec, &setNewnode,set & ~(1 << i));
        break; 
      }
    case STRING_:
      {
        const char *s = ec->stk->u.str;
        checkEmptyString(ec, s,"uncons");
        unary<long>(ec, &charNewnode,(long)*s);
        nullary<const char *>(ec, &stringNewnode,++s);
        break; 
      }
    case LIST_:
      saveStack(ec);
      checkEmptyList(ec, saved1(ec)->u.lis, "uncons");
      gunary(ec, saved1(ec)->u.lis->op, saved1(ec)->u.lis->u);
      nullary<pNode>(ec, &listNewnode, saved1(ec)->u.lis->next);
      pop(ec->dump);
      return;
    default:
      badAggregate(ec, "uncons"); 
  }
}

PRIVATE void unswons_(pEC ec) {
  hasOneParam(ec, "unswons");
  switch (ec->stk->op)
  { 
    case SET_:
      {
        long i = 0; long set = ec->stk->u.set;
        checkEmptySet(ec, set,"unswons");
        while (!(set & (1 << i))) i++;
        unary<long>(ec, &setNewnode,set & ~(1 << i));
        nullary<long>(ec, &integerNewnode,i);
        break; 
      }
    case STRING_:
      {
        const char *s = ec->stk->u.str;
        checkEmptyString(ec, s,"unswons");
        unary<const char *>(ec, &stringNewnode,++s);
        nullary<long>(ec, &charNewnode,(long)*(--s));
        break; 
      }
    case LIST_:
      saveStack(ec);
      checkEmptyList(ec, saved1(ec)->u.lis,"unswons");
      unary<pNode>(ec, &listNewnode,saved1(ec)->u.lis->next);
      gnullary(ec, saved1(ec)->u.lis->op,saved1(ec)->u.lis->u);
      pop(ec->dump);
      return;
    default:
      badAggregate(ec, "unswons"); 
  }
}

PRIVATE long equal_aux(pEC ec, pNode n1, pNode n2); /* forward */

PRIVATE int equal_list_aux(pEC ec, pNode n1, pNode n2) {
  if (n1 == NULL && n2 == NULL) return 1;
  if (n1 == NULL || n2 == NULL) return 0;
  if (equal_aux(ec, n1, n2))
    return equal_list_aux(ec, n1->next, n2->next);
  else return 0;
}

PRIVATE long equal_aux(pEC ec, pNode n1, pNode n2) {
  if (n1 == NULL && n2 == NULL) return 1;
  if (n1 == NULL || n2 == NULL) return 0;
  switch (n1->op) {
    case BOOLEAN_: case CHAR_: case INTEGER_:
      if (n2->op != BOOLEAN_ && n2->op != CHAR_ && n2->op != INTEGER_)
        return 0;
      return n1->u.num == n2->u.num;
    case SET_ :
      if (n2->op != SET_) return 0;
      return n1->u.num == n2->u.num;
    case LIST_ :
      if (n2->op != LIST_) return 0;
      return equal_list_aux(ec, n1->u.lis, n2->u.lis);
    default:
      return strcmp(GETSTRING(n1),GETSTRING(n2)) == 0; 
  }
}

PRIVATE void equal_(pEC ec) {
  hasTwoParams(ec, "equal");
  binary<long>(ec, &booleanNewnode, equal_aux(ec, ec->stk, ec->stk->next));
}
#define INHAS(PROCEDURE,NAME,AGGR,ELEM)				\
PRIVATE void PROCEDURE(pEC ec)	{				\
    int found = 0;						\
    hasTwoParams(ec, NAME);						\
    switch (AGGR->op)						\
      { case SET_:						\
	    found = ((AGGR->u.set) & (1 << ELEM->u.num)) > 0;	\
	    break;						\
	case STRING_:						\
	  { const char *s;						\
	    for (s = AGGR->u.str;				\
		 *s != '\0' && *s != ELEM->u.num;		\
		 s++);						\
	    found = *s != '\0';					\
	    break; }						\
	case LIST_:						\
	  { Node *n = AGGR->u.lis;				\
	    while (n != NULL && n->u.num != ELEM->u.num)	\
		n = n->next;					\
	    found = n != NULL;					\
	    break; }						\
	default:						\
	    badAggregate(ec, NAME); }				\
    binary<long>(ec, &booleanNewnode,(long)found);					\
}
INHAS(in_,"in",ec->stk,ec->stk->next)
INHAS(has_,"has",ec->stk->next,ec->stk)

#define OF_AT(PROCEDURE,NAME,AGGR,INDEX)			\
PRIVATE void PROCEDURE(pEC ec)	{				\
    hasTwoParams(ec, NAME);						\
    if (INDEX->op != INTEGER_ || INDEX->u.num < 0)		\
	execerror(ec, "non-negative integer", NAME);		\
    switch (AGGR->op)						\
      { case SET_:						\
	  { long i; int indx = INDEX->u.num;			\
	    checkEmptySet(ec, AGGR->u.set,NAME);			\
	    for (i = 0; i < SETSIZE; i++)			\
	      { if (AGGR->u.set & (1 << i))			\
		  { if (indx == 0)				\
			{binary<long>(ec, &integerNewnode, i); return;}		\
		    indx--; } }					\
	    indexTooLarge(ec, NAME);				\
	    return; }						\
	case STRING_:						\
	    if (strlen(AGGR->u.str) < INDEX->u.num)		\
		indexTooLarge(ec, NAME);				\
	    binary<long>(ec, &charNewnode,(long)AGGR->u.str[INDEX->u.num]);		\
	    return;						\
	case LIST_:						\
	  { Node *n = AGGR->u.lis;  int i  = INDEX->u.num;	\
	    checkEmptyList(ec, n,NAME);				\
	    while (i > 0)					\
	      { if (n->next == NULL)				\
		    indexTooLarge(ec, NAME);			\
		n = n->next; i--; }				\
	    gbinary(ec, n->op,n->u);					\
	    return; }						\
	default:						\
	    badAggregate(ec, NAME); }				\
}
OF_AT(of_,"of",ec->stk,ec->stk->next)
OF_AT(at_,"at",ec->stk->next,ec->stk)

PRIVATE void choice_(pEC ec) {
  hasThreeParams(ec, "choice");
  if (ec->stk->next->next->u.num) 
    ec->stk = newnode(ec, ec->stk->next->op, ec->stk->next->u, ec->stk->next->next->next);
  else
    ec->stk = newnode(ec, ec->stk->op, ec->stk->u, ec->stk->next->next->next);
}

PRIVATE void case_(pEC ec) {
  Node *n;
  hasTwoParams(ec, "case");
  hasList(ec, "case");
  n = ec->stk->u.lis;
  checkEmptyList(ec, n,"case");
  while ( n->next != NULL && n->u.lis->u.num != ec->stk->next->u.num )
    n = n->next;
  /*
     printf("case : now execute : ");
     writefactor(n->u.lis, stdout); printf("\n");
     ec->stk = ec->stk->next->next;
     exeterm(ec, n->next != NULL ? n->u.lis->next : n->u.lis);
     */
  if (n->next != NULL) {
    ec->stk = ec->stk->next->next;
    exeterm(ec, n->u.lis->next);
  }
  else {
    ec->stk = ec->stk->next;
    exeterm(ec, n->u.lis);
  }
}

PRIVATE void opcase_(pEC ec) {
  Node *n;
  hasOneParam(ec, "opcase");
  hasList(ec, "opcase");
  n = ec->stk->u.lis;
  checkEmptyList(ec, n,"opcase");
  while ( n->next != NULL && n->op == LIST_ && n->u.lis->op != ec->stk->next->op )
    n = n->next;
  checkList(ec, n->op, "opcase");
  unary<pNode>(ec, &listNewnode, n->next != NULL ? n->u.lis->next : n->u.lis);
}

#define CONS_SWONS(PROCEDURE,NAME,AGGR,ELEM)			\
PRIVATE void PROCEDURE(pEC ec)	{				\
    hasTwoParams(ec, NAME);						\
    switch (AGGR->op)						\
      { case LIST_:						\
	    binary<pNode>(ec, &listNewnode,newnode(ec, ELEM->op,			\
				 ELEM->u,AGGR->u.lis));	\
	    break;						\
	case SET_:						\
	    checkSetMember(ec, ELEM,NAME);				\
	    binary<long>(ec, &setNewnode,AGGR->u.set | (1 << ELEM->u.num));	\
	    break;						\
	case STRING_:						\
	  { char *s;						\
	    if (ELEM->op != CHAR_)				\
		execerror(ec, "character", NAME);			\
	    s = (char *) malloc(strlen(AGGR->u.str) + 2);	\
	    s[0] = ELEM->u.num;					\
	    strcpy(s + 1,AGGR->u.str);				\
	    binary<const char *>(ec, &stringNewnode, s);				\
	    break; }						\
	default:						\
	    badAggregate(ec, NAME); }				\
}
CONS_SWONS(cons_, "cons", ec->stk, ec->stk->next)
CONS_SWONS(swons_, "swons", ec->stk->next, ec->stk)

PRIVATE void drop_(pEC ec) {
  int n = ec->stk->u.num;
  hasTwoParams(ec, "drop");
  switch (ec->stk->next->op) {
    case SET_:
      {
        int i; long result = 0;
        for (i = 0; i < SETSIZE; i++)
          if (ec->stk->next->u.set & (1 << i)) { 
            if (n < 1)
              result = result | (1 << i);
            else
              n--; 
          }
        binary<long>(ec, &setNewnode,result);
        return; 
      }
    case STRING_:
      {
        const char *result = ec->stk->next->u.str;
        while (n-- > 0  &&  *result != '\0')
          ++result;
        binary<const char *>(ec, &stringNewnode,result);
        return; 
      }
    case LIST_:
      {
        Node *result = ec->stk->next->u.lis;
        while (n-- > 0 && result != NULL)
          result = result->next;
        binary<pNode>(ec, &listNewnode, result);
        return; 
      }
    default:
      badAggregate(ec, "drop");
  }
}

PRIVATE void take_(pEC ec) {
  int n = ec->stk->u.num;
  hasTwoParams(ec, "take");
  switch (ec->stk->next->op) {
    case SET_:
      {
        int i; long result = 0;
        for (i = 0; i < SETSIZE; i++)
          if (ec->stk->next->u.set & (1 << i)) {
            if (n > 0) {
              --n;
              result = result | (1 << i); 
            }
            else 
              break; 
          }
        binary<long>(ec, &setNewnode,result);
        return; 
      }
    case STRING_:
      {
        int i;
        const char *old;
        char *p;
        const char *result;
        i = ec->stk->u.num;
        old = ec->stk->next->u.str;
        pop(ec->stk);
        /* do not swap the order of the next two statements ! ! ! */
        if (i < 0)
          i = 0;
        if (i > strlen(old))
          return; /* the old string unchanged */
        result = p = (char *) malloc(strlen(old) - i + 1);
        while (i-- > 0)
          *p++ = *old++;
        unary<const char *>(ec, &stringNewnode,result);
        return; 
      }
    case LIST_:
      {
        int i = ec->stk->u.num;
        if (i < 1) {	/* null string */
          binary<pNode>(ec, &listNewnode,NULL); return;
        }
        ec->dump1 = newnode(ec, LIST_,ec->stk->next->u, ec->dump1);/* old  */
        ec->dump2 = listNewnode(ec, 0L, ec->dump2);		  /* head */
        ec->dump3 = listNewnode(ec, 0L, ec->dump3);		  /* last */
        while (dmp1(ec) != NULL && i-- > 0) {
          if (dmp2(ec) == NULL) {				/* first */
            dmp2(ec) = newnode(ec, dmp1(ec)->op, dmp1(ec)->u, NULL);
            dmp3(ec) = dmp2(ec);
          }
          else {					/* further */
            dmp3(ec)->next = newnode(ec, dmp1(ec)->op,dmp1(ec)->u,NULL);
            dmp3(ec) = dmp3(ec)->next;
          }
          dmp1(ec) = dmp1(ec)->next;
        }
        dmp3(ec)->next = NULL;
        binary<pNode>(ec, &listNewnode, dmp2(ec));
        pop(ec->dump1);
        pop(ec->dump2);
        pop(ec->dump3);
        return; 
      }
    default:
      badAggregate(ec, "take");
  }
}

PRIVATE void concat_(pEC ec) {
  hasTwoParams(ec, "concat");
  hasSame2types(ec, "concat");
  switch (ec->stk->op) {
    case SET_:
      binary<long>(ec, &setNewnode,ec->stk->next->u.set | ec->stk->u.set);
      return;
    case STRING_:
      {
        char *s, *p;
        s = p = (char *)malloc(strlen(ec->stk->next->u.str) +
            strlen(ec->stk->u.str) + 1);
        while ((*p++ = *(ec->stk->next->u.str)++) != '\0')
          ;
        --p; /* don't want terminating null */
        while ((*p++ = *(ec->stk->u.str)++) != '\0')
          ;
        binary<const char *>(ec, &stringNewnode, s);
        return; 
      }
    case LIST_:
      if (ec->stk->next->u.lis == NULL) {
        binary<pNode>(ec, &listNewnode, ec->stk->u.lis);
        return;
      }
      ec->dump1 = listNewnode(ec, ec->stk->next->u.lis, ec->dump1);/* old  */
      ec->dump2 = listNewnode(ec, 0L, ec->dump2);		 /* head */
      ec->dump3 = listNewnode(ec, 0L, ec->dump3);		 /* last */
      while (dmp1(ec) != NULL) {
        if (dmp2(ec) == NULL) {			/* first */
          dmp2(ec) =
            newnode(ec, dmp1(ec)->op, dmp1(ec)->u,NULL);
          dmp3(ec) = dmp2(ec);
        }
        else {					/* further */
          dmp3(ec)->next = newnode(ec, dmp1(ec)->op, dmp1(ec)->u,NULL);
          dmp3(ec) = dmp3(ec)->next;
        }
        dmp1(ec) = dmp1(ec)->next;
      }
      dmp3(ec)->next = ec->stk->u.lis;
      binary<pNode>(ec, &listNewnode, dmp2(ec));
      pop(ec->dump1);
      pop(ec->dump2);
      pop(ec->dump3);
      return;
    default:
      badAggregate(ec, "concat");
  }
}

PRIVATE void enconcat_(pEC ec) {
  hasThreeParams(ec, "enconcat");
  hasSame2types(ec, "enconcat");
  swapd_(ec); cons_(ec); concat_(ec);
}

PRIVATE void null_(pEC ec) {
  hasOneParam(ec, "null");
  switch (ec->stk->op) {
    case STRING_:
      unary<long>(ec, &booleanNewnode, (long)(*(ec->stk->u.str) == '\0'));
      break;
    case FLOAT_:
      unary<long>(ec, &booleanNewnode, (long)(ec->stk->u.dbl == 0.0));
      break;
    case FILE_:
      unary<long>(ec, &booleanNewnode, (long)(ec->stk->u.fil == NULL));
      break;
    default:
      unary<long>(ec, &booleanNewnode, (long)(! ec->stk->u.num));
  }
}

PRIVATE void not_(pEC ec) {
  hasOneParam(ec, "not");
  switch (ec->stk->op) {
    case SET_:
      unary<long>(ec, &setNewnode, ~ ec->stk->u.set);
      break;
    case STRING_:
      unary<long>(ec, &booleanNewnode, (long)(*(ec->stk->u.str) != '\0'));
      break;
    case BOOLEAN_: case CHAR_: case INTEGER_: case LIST_:
      unary<long>(ec, &booleanNewnode, (long)(! ec->stk->u.num));
      break;
    default:
      badData(ec, "not");
  }
}

PRIVATE void size_(pEC ec) {
  long siz = 0;
  hasOneParam(ec, "size");
  switch (ec->stk->op) {
    case SET_:
      {
        int i;
        for (i = 0; i < SETSIZE; i++)
          if (ec->stk->u.set & (1 << i)) siz++;
        break; 
      }
    case STRING_:
      siz = strlen(ec->stk->u.str);
      break;
    case LIST_:
      {
        Node *e = ec->stk->u.lis;
        while (e != NULL) {
          e = e->next; siz++;
        };
        break; 
      }
    default :
      badData(ec, "size");
  }
  unary<long>(ec, &integerNewnode,siz);
}

PRIVATE void small_(pEC ec) {
  long sml = 0;
  hasOneParam(ec, "small");
  switch (ec->stk->op) {
    case BOOLEAN_: case INTEGER_:
      sml = ec->stk->u.num < 2;
      break;
    case SET_:
      if (ec->stk->u.set == 0)
        sml = 1; 
      else {
        int i = 0;
        while  (!(ec->stk->u.set & (1 << i))) i++;
        D(		printf("small: first member found is %d\n",i); )
          sml = (ec->stk->u.set & ~(1 << i)) == 0; 
      }
      break;
    case STRING_:
      sml = ec->stk->u.str[0] == '\0' || ec->stk->u.str[1] == '\0';
      break;
    case LIST_:
      sml = ec->stk->u.lis == NULL || ec->stk->u.lis->next == NULL;
      break;
    default:
      badData(ec, "small");
  }
  unary<long>(ec, &booleanNewnode,sml);
}

#define TYPE(PROCEDURE,NAME,REL,TYP)				\
    PRIVATE void PROCEDURE(pEC ec) {					\
        hasOneParam(ec, NAME);						\
	unary<long>(ec, &booleanNewnode,(long)(ec->stk->op REL TYP)); }
TYPE(integer_,"integer",==,INTEGER_)
TYPE(char_,"char",==,CHAR_)
TYPE(logical_,"logical",==,BOOLEAN_)
TYPE(string_,"string",==,STRING_)
TYPE(set_,"set",==,SET_)
TYPE(list_,"list",==,LIST_)
TYPE(leaf_,"leaf",!=,LIST_)
TYPE(float_,"float",==,FLOAT_)
TYPE(file_,"file",==,FILE_)
TYPE(user_,"user",==,USR_)

PRIVATE void usetop(pEC ec, const char *name, void(*type)(pEC, const char*), void(*body)(pEC)) {
  hasOneParam(ec, name);
  type(ec, name);
  body(ec);
  pop(ec->stk);
}

PRIVATE void put_(pEC ec) {
  usetop(ec, "put", &hasOneParam, [](pEC ec)->void {
    writefactor(ec, ec->stk, stdout);
    printf(" ");
  });
}
PRIVATE void putch_(pEC ec) {
  usetop(ec, "putch", &isNumerictype, [](pEC ec)->void {
    printf("%c", static_cast<char>(ec->stk->u.num));
  });
}
PRIVATE void putchars_(pEC ec) {
  usetop(ec, "putchars", &hasString, [](pEC ec)->void {
    printf("%s", ec->stk->u.str);
  });
}
PRIVATE void setecho_(pEC ec) {
  usetop(ec, "setecho", &isNumerictype, [](pEC ec)->void {
    ec->echoflag = ec->stk->u.num;
  });
}
PRIVATE void setautoput_(pEC ec) {
  usetop(ec, "setautoput", &isNumerictype, [](pEC ec)->void {
    ec->autoput = ec->stk->u.num;
  });
}
PRIVATE void setundeferror_(pEC ec) {
  usetop(ec, "setundeferror", &isNumerictype, [](pEC ec)->void {
    ec->undeferror = ec->stk->u.num;
  });
}
PRIVATE void settracegc_(pEC ec) {
  usetop(ec, "settracegc", &isNumerictype, [](pEC ec)->void {
    ec->tracegc = ec->stk->u.num;
  });
}
PRIVATE void srand_(pEC ec) {
  usetop(ec, "srand", &hasInteger, [](pEC ec)->void {
    srand(static_cast<unsigned int>(ec->stk->u.num));
  });
}
PRIVATE void include_(pEC ec) {
  usetop(ec, "include", &hasString, [](pEC ec)->void {
    doinclude(ec, ec->stk->u.str);
  });
}
PRIVATE void system_(pEC ec) {
  usetop(ec, "system", &hasString, [](pEC ec)->void {
    system(ec->stk->u.str);
  });
}

#define USETOP(PROCEDURE,NAME,TYPE,BODY)			\
    PRIVATE void PROCEDURE(pEC ec) {					\
      hasOneParam(ec, NAME); TYPE(NAME); BODY; pop(ec->stk); }
// USETOP( put_,"put",HASONEPARAM, writefactor(ec, ec->stk, stdout); printf(" "))
// USETOP( putch_,"putch",NUMERICTYPE, printf("%c", (char) ec->stk->u.num) )
// USETOP( putchars_,"putchars",STRING, printf("%s", ec->stk->u.str) )
// USETOP( setecho_,"setecho",NUMERICTYPE, ec->echoflag = ec->stk->u.num )
// USETOP( setautoput_,"setautoput",NUMERICTYPE, ec->autoput = ec->stk->u.num )
// USETOP( setundeferror_, "setundeferror", NUMERICTYPE, ec->undeferror = ec->stk->u.num )
// USETOP( settracegc_,"settracegc",NUMERICTYPE, ec->tracegc = ec->stk->u.num )
// USETOP( srand_,"srand",INTEGER, srand((unsigned int) ec->stk->u.num) )
// USETOP( include_,"include",STRING, doinclude(ec, ec->stk->u.str) )
// USETOP( system_,"system",STRING, system(ec->stk->u.str) )

PRIVATE void undefs_(pEC ec) {
  Entry *i = ec->symtabindex;
  Node *n = 0;
  while (i != ec->symtab) {
    --i;
    if ( (i->name[0] != 0) && (i->name[0] != '_') && (i->u.body == NULL) )
      n = stringNewnode(ec, i->name, n);
  }
  ec->stk = listNewnode(ec, n, ec->stk);
}

PRIVATE void argv_(pEC ec) {
  int i;
  ec->dump1 = listNewnode(ec, NULL, ec->dump1);
  for (i = ec->g_argc - 1; i >= 0; i--) {
    dmp1(ec) = stringNewnode(ec, ec->g_argv[i], dmp1(ec));
  }
  nullary<pNode>(ec, &listNewnode, dmp1(ec));
  pop(ec->dump1);
  return;
}

PRIVATE void get_(pEC ec) {
  getsym(ec);
  readfactor(ec);
}

PUBLIC void dummy_(pEC ec) {
    /* never called */
}

#define HELP(PROCEDURE,REL)					\
PRIVATE void PROCEDURE(pEC ec) {				\
    Entry *i = ec->symtabindex;					\
    int column = 0;						\
    int name_length;						\
    while (i != ec->symtab)						\
	if ((--i)->name[0] REL '_')				\
	  { name_length = strlen(i->name) + 1;			\
	    if (column + name_length > 72)		\
	      { printf("\n"); column = 0; }			\
	    printf("%s ", i->name);  				\
	    column += name_length; }				\
    printf("\n"); }
HELP(help1_,!=)
HELP(h_help1_,==)

/* - - - - -   C O M B I N A T O R S   - - - - - */

PUBLIC void exeterm(pEC ec, Node *n) {
  Node *stepper;
start:
  if (n == NULL)
    return;
  ec->conts = listNewnode(ec, n, ec->conts);
  while (ec->conts->u.lis != NULL) {
    if (ec->tracegc > 5) {
      printf("exeterm1: %ld ",(long)ec->conts->u.lis);
      printnode(ec, ec->conts->u.lis); 
    }
    stepper = ec->conts->u.lis;
    ec->conts->u.lis = ec->conts->u.lis->next;
    switch (stepper->op) {
      case BOOLEAN_: case CHAR_: case INTEGER_: case FLOAT_:
      case SET_: case STRING_: case LIST_:
        ec->stk = newnode(ec, stepper->op, stepper->u, ec->stk); break;
      case USR_:
        if (stepper->u.ent->u.body == NULL && ec->undeferror)
          execerror(ec, "definition", stepper->u.ent->name);
        if (stepper->next == NULL) {
          pop(ec->conts);
          n = stepper->u.ent->u.body;
          goto start;
        }
        else 
          exeterm(ec, stepper->u.ent->u.body );
        break;
      case COPIED_: case ILLEGAL_:
        printf("exeterm: attempting to execute bad node\n");
        printnode(ec, stepper);
        break;
      default:
        D(printf("trying to do "));
        D(writefactor(ec->dump1, stdout));
        (*(stepper->u.proc))(ec);
        break;
    }
    if (ec->tracegc > 5) {
      printf("exeterm2: %ld ",(long)stepper);
      printnode(ec, stepper);
    }
    /*
	stepper = stepper->next; }
    */
  }

  pop(ec->conts);
  D(printf("after execution, ec->stk is:\n"));
  D(writeterm(ec->stk, stdout));
  D(printf("\n"));
}

PRIVATE void x_(pEC ec) {
    hasOneParam(ec, "x");
    hasOneQuote(ec, "x");
    exeterm(ec, ec->stk->u.lis);
}

PRIVATE void i_(pEC ec) {
    hasOneParam(ec, "i");
    hasOneQuote(ec, "i");
    saveStack(ec);
    pop(ec->stk);
    exeterm(ec, saved1(ec)->u.lis);
    pop(ec->dump);
}

PRIVATE void dip_(pEC ec) {
    hasTwoParams(ec, "dip");
    hasOneQuote(ec, "dip");
    saveStack(ec);
    ec->stk = ec->stk->next->next;
    exeterm(ec, saved1(ec)->u.lis);
    GNULLARY(saved2(ec)->op,saved2(ec)->u);
    pop(ec->dump);
}

PRIVATE void n_ary(pEC ec, const char *name, void (*paramcount)(pEC, const char*), pNode& (*top)(pEC)) {
    paramcount(ec, name);
    hasOneQuote(ec, name);
    saveStack(ec);
    pop(ec->stk);
    exeterm(ec, saved1(ec)->u.lis);
    if (ec->stk == NULL)
      execerror(ec, "value to push", name);
    ec->stk = newnode(ec, ec->stk->op, ec->stk->u, top(ec));
    pop(ec->dump);
}

PRIVATE void nullary_(pEC ec) {
  n_ary(ec, "nullary", &hasOneParam, &saved2);
}
PRIVATE void unary_(pEC ec) {
  n_ary(ec, "unary", &hasTwoParams, &saved3);
}
PRIVATE void binary_(pEC ec) {
  n_ary(ec, "binary", &hasThreeParams, &saved4);
}
PRIVATE void ternary_(pEC ec) {
  n_ary(ec, "ternary", &hasFourParams, &saved5);
}

#define N_ARY(PROCEDURE,NAME,PARAMCOUNT,TOP)			\
PRIVATE void PROCEDURE(pEC ec)	{				\
    PARAMCOUNT(NAME);						\
    hasOneQuote(ec, NAME);						\
    saveStack(ec);							\
    pop(ec->stk);							\
    exeterm(ec, SAVED1->u.lis);					\
    if (ec->stk == NULL) execerror(ec, "value to push",NAME);		\
    ec->stk = newnode(ec, ec->stk->op, ec->stk->u,TOP);				\
    pop(ec->dump);							\
}
// N_ARY(nullary_,"nullary",HASONEPARAM,SAVED2)
// N_ARY(unary_,"unary",HASTWOPARAMS,SAVED3)
// N_ARY(binary_,"binary",HASTHREEPARAMS,SAVED4)
// N_ARY(ternary_,"ternary",HASFOURPARAMS,SAVED5)

/*
PRIVATE void nullary_(pEC ec) {
  HASONEPARAM("nullary");
  saveStack(ec);
  pop(ec->stk);
  exeterm(ec, SAVED1->u.lis);
  ec->stk->next = SAVED2;
  pop(ec->dump);
}
*/

PRIVATE void times_(pEC ec) {
  int i,n;
  hasTwoParams(ec, "times");
  hasOneQuote(ec, "times");
  hasInteger2(ec, "times");
  saveStack(ec);
  ec->stk = ec->stk->next->next;
  n = saved2(ec)->u.num;
  for (i = 1; i <= n; i++)
    exeterm(ec, saved1(ec)->u.lis);
  pop(ec->dump);
}

PRIVATE void infra_(pEC ec) {
  hasTwoParams(ec, "infra");
  hasOneQuote(ec, "infra");
  hasList2(ec, "infra");
  saveStack(ec);
  ec->stk = saved2(ec)->u.lis;
  exeterm(ec, saved1(ec)->u.lis);
  ec->stk = listNewnode(ec, ec->stk, saved3(ec));
  pop(ec->dump);
}

PRIVATE void app1_(pEC ec) {
  hasTwoParams(ec, "app1");
  hasOneQuote(ec, "app1");
  saveStack(ec);
  pop(ec->stk);
  exeterm(ec, saved1(ec)->u.lis);
  pop(ec->dump);
}

PRIVATE void cleave_(pEC ec) {
  /*  X [P1] [P2] cleave ==>  X1 X2	*/
  hasThreeParams(ec, "cleave");
  hasTwoQuotes(ec, "cleave");
  saveStack(ec);
  ec->stk = saved3(ec);
  exeterm(ec, saved2(ec)->u.lis);			/* [P1]		*/
  ec->dump1 = newnode(ec, ec->stk->op,ec->stk->u,ec->dump1);	/*  X1		*/
  ec->stk = saved3(ec);
  exeterm(ec, saved1(ec)->u.lis);			/* [P2]		*/
  ec->dump1 = newnode(ec, ec->stk->op,ec->stk->u,ec->dump1);	/*  X2		*/
  ec->stk = ec->dump1;
  ec->dump1 = ec->dump1->next->next;
  ec->stk->next->next = saved4(ec);
  pop(ec->dump);
}

PRIVATE void app11_(pEC ec) {
  hasThreeParams(ec, "app11");
  hasOneQuote(ec, "app11");
  app1_(ec);
  ec->stk->next = ec->stk->next->next;
}

PRIVATE void unary2_(pEC ec) {
  /*   Y  Z  [P]  unary2     ==>  Y'  Z'  */
  hasThreeParams(ec, "unary2");
  hasOneQuote(ec, "unary2");
  saveStack(ec);
  ec->stk = saved2(ec)->next;				/* just Y on top */
  exeterm(ec, saved1(ec)->u.lis);			/* execute P */
  ec->dump1 = newnode(ec, ec->stk->op,ec->stk->u,ec->dump1);	/* save P(Y) */
  ec->stk = newnode(ec, saved2(ec)->op, saved2(ec)->u, saved3(ec)->next);	/* just Z on top */
  exeterm(ec, saved1(ec)->u.lis);			/* execute P */
  ec->dump1 = newnode(ec, ec->stk->op,ec->stk->u,ec->dump1);	/* save P(Z) */
  ec->stk = ec->dump1;
  ec->dump1 = ec->dump1->next->next;
  ec->stk->next->next = saved4(ec);
  pop(ec->dump);
}

PRIVATE void unary3_(pEC ec) {
  /*  X Y Z [P]  unary3    ==>  X' Y' Z'	*/
  hasFourParams(ec, "unary3");
  hasOneQuote(ec, "unary3");
  saveStack(ec);
  ec->stk = saved3(ec)->next;				/* just X on top */
  exeterm(ec, saved1(ec)->u.lis);			/* execute P */
  ec->dump1 = newnode(ec, ec->stk->op,ec->stk->u,ec->dump1);	/* save p(X) */
  ec->stk = newnode(ec, saved3(ec)->op,saved3(ec)->u,
      saved4(ec)->next);			/* just Y on top */
  exeterm(ec, saved1(ec)->u.lis);			/* execute P */
  ec->dump1 = newnode(ec, ec->stk->op,ec->stk->u,ec->dump1);	/* save P(Y) */
  ec->stk = newnode(ec, saved2(ec)->op, saved2(ec)->u,
      saved4(ec)->next);			/* just Z on top */
  exeterm(ec, saved1(ec)->u.lis);			/* execute P */
  ec->dump1 = newnode(ec, ec->stk->op,ec->stk->u,ec->dump1);	/* save P(Z) */
  ec->stk = ec->dump1;
  ec->dump1 = ec->dump1->next->next->next;
  ec->stk->next->next->next = saved5(ec);
  pop(ec->dump);
}

PRIVATE void unary4_(pEC ec) {
  /*  X Y Z W [P]  unary4    ==>  X' Y' Z' W'	*/
  hasFiveParams(ec, "unary4");
  hasOneQuote(ec, "unary4");
  saveStack(ec);
  ec->stk = saved4(ec)->next;				/* just X on top */
  exeterm(ec, saved1(ec)->u.lis);			/* execute P */
  ec->dump1 = newnode(ec, ec->stk->op,ec->stk->u,ec->dump1);	/* save p(X) */
  ec->stk = newnode(ec, saved4(ec)->op, saved4(ec)->u,
      saved5(ec)->next);			/* just Y on top */
  exeterm(ec, saved1(ec)->u.lis);			/* execute P */
  ec->dump1 = newnode(ec, ec->stk->op,ec->stk->u,ec->dump1);	/* save P(Y) */
  ec->stk = newnode(ec, saved3(ec)->op, saved3(ec)->u,
      saved5(ec)->next);			/* just Z on top */
  exeterm(ec, saved1(ec)->u.lis);			/* execute P */
  ec->dump1 = newnode(ec, ec->stk->op,ec->stk->u,ec->dump1);	/* save P(Z) */
  ec->stk = newnode(ec, saved2(ec)->op, saved2(ec)->u,
      saved5(ec)->next);			/* just W on top */
  exeterm(ec, saved1(ec)->u.lis);			/* execute P */
  ec->dump1 = newnode(ec, ec->stk->op,ec->stk->u,ec->dump1);	/* save P(W) */
  ec->stk = ec->dump1; 
  ec->dump1 = ec->dump1->next->next->next->next;
  ec->stk->next->next->next->next = saved6(ec);
  pop(ec->dump);
}

PRIVATE void app12_(pEC ec) {
  /*   X  Y  Z  [P]  app12  */
  hasThreeParams(ec, "app12");
  unary2_(ec);
  ec->stk->next->next = ec->stk->next->next->next;	/* delete X */
}

PRIVATE void map_(pEC ec) {
  hasTwoParams(ec, "map");
  hasOneQuote(ec, "map");
  saveStack(ec);
  switch(saved2(ec)->op) {
    case LIST_:
      {
        ec->dump1 = newnode(ec, LIST_, saved2(ec)->u, ec->dump1);	/* step old */
        ec->dump2 = listNewnode(ec, 0L, ec->dump2);		/* head new */
        ec->dump3 = listNewnode(ec, 0L, ec->dump3);		/* last new */
        while (dmp1(ec) != NULL) {
          ec->stk = newnode(ec, dmp1(ec)->op, dmp1(ec)->u,saved3(ec));
          exeterm(ec, saved1(ec)->u.lis);
          D(printf("map: "); writefactor(ec->stk, stdout); printf("\n"));
          if (dmp2(ec) == NULL)	{		/* first */
            dmp2(ec) = newnode(ec, ec->stk->op,ec->stk->u,NULL);
            dmp3(ec) = dmp2(ec);
          }
          else {					/* further */
            dmp3(ec)->next = newnode(ec, ec->stk->op,ec->stk->u,NULL);
            dmp3(ec) = dmp3(ec)->next;
          }
          dmp1(ec) = dmp1(ec)->next;
        }
        ec->stk = listNewnode(ec, dmp2(ec), saved3(ec));
        pop(ec->dump3);
        pop(ec->dump2);
        pop(ec->dump1);
        break;
      }
    case STRING_:
      { 
        const char *s;
        char *resultstring;
        int j = 0;
        resultstring = (char *) malloc(strlen(saved2(ec)->u.str) + 1);
        for (s = saved2(ec)->u.str; *s != '\0'; s++) {
          ec->stk = charNewnode(ec, (long)*s, saved3(ec));
          exeterm(ec, saved1(ec)->u.lis);
          resultstring[j++] = ec->stk->u.num;
        }
        ec->stk = stringNewnode(ec, resultstring, saved3(ec));
        break;
      }
    case SET_:
      {
        long i; long resultset = 0;
        for (i = 0; i < SETSIZE; i++)
          if (saved2(ec)->u.set & (1 << i))
          {
            ec->stk = integerNewnode(ec, i, saved3(ec));
            exeterm(ec, saved1(ec)->u.lis);
            resultset = resultset | (1 << ec->stk->u.num);
          }
        ec->stk = setNewnode(ec, resultset, saved3(ec));
        break;
      }
    default:
      badAggregate(ec, "map");
  }
  pop(ec->dump);
}

PRIVATE void step_(pEC ec) {
  hasTwoParams(ec, "step");
  hasOneQuote(ec, "step");
  saveStack(ec);
  ec->stk = ec->stk->next->next;
  switch(saved2(ec)->op) {
    case LIST_:
      {
        ec->dump1 = newnode(ec, LIST_, saved2(ec)->u, ec->dump1);
        while (dmp1(ec) != NULL) {
          GNULLARY(dmp1(ec)->op,dmp1(ec)->u);
          exeterm(ec, saved1(ec)->u.lis);
          dmp1(ec) = dmp1(ec)->next;
        }
        pop(ec->dump1);
        break;
      }
    case STRING_:
      {
        const char *s;
        for (s = saved2(ec)->u.str; *s != '\0'; s++) {
          ec->stk = charNewnode(ec, (long)*s,ec->stk);
          exeterm(ec, saved1(ec)->u.lis);
        }
        break;
      }
    case SET_:
      {
        long i;
        for (i = 0; i < SETSIZE; i++)
          if (saved2(ec)->u.set & (1 << i)) {
            ec->stk = integerNewnode(ec, i,ec->stk);
            exeterm(ec, saved1(ec)->u.lis);
          }
        break;
      }
    default:
      badAggregate(ec, "step");
  }
  pop(ec->dump);
}

PRIVATE void fold_(pEC ec) {
  hasThreeParams(ec, "fold");
  swapd_(ec);
  step_(ec);
}

PRIVATE void cond_(pEC ec) {
  int result = 0;
  hasOneParam(ec, "cond");
  /* must check for QUOTES in list */
  hasList(ec, "cond");
  checkEmptyList(ec, ec->stk->u.lis,"cond");
  saveStack(ec);
  ec->dump1 = newnode(ec, LIST_,ec->stk->u,ec->dump1);
  while ( result == 0 &&
      dmp1(ec) != NULL &&
      dmp1(ec)->next != NULL ) {
    ec->stk = saved2(ec);
    exeterm(ec, dmp1(ec)->u.lis->u.lis);
    result = ec->stk->u.num;
    if (!result)
      dmp1(ec) = dmp1(ec)->next;
  }
  ec->stk = saved2(ec);
  if (result)
    exeterm(ec, dmp1(ec)->u.lis->next);
  else
    exeterm(ec, dmp1(ec)->u.lis); /* default */
  pop(ec->dump1);
  pop(ec->dump);
}

#define IF_TYPE(PROCEDURE,NAME,TYP)				\
    PRIVATE void PROCEDURE(pEC ec)					\
    {   hasTwoParams(ec, NAME);					\
	hasTwoQuotes(ec, NAME);					\
        saveStack(ec);						\
	ec->stk = saved3(ec);						\
	exeterm(ec, ec->stk->op == TYP ? saved2(ec)->u.lis : saved1(ec)->u.lis);\
	pop(ec->dump); }
IF_TYPE(ifinteger_,"ifinteger",INTEGER_)
IF_TYPE(ifchar_,"ifchar",CHAR_)
IF_TYPE(iflogical_,"iflogical",BOOLEAN_)
IF_TYPE(ifstring_,"ifstring",STRING_)
IF_TYPE(ifset_,"ifset",SET_)
IF_TYPE(iffloat_,"iffloat",FLOAT_)
IF_TYPE(iffile_,"iffile",FILE_)
IF_TYPE(iflist_,"iflist",LIST_)

PRIVATE void filter_(pEC ec) {
  hasTwoParams(ec, "filter");
  hasOneQuote(ec, "filter");
  saveStack(ec);
  switch (saved2(ec)->op) {
    case SET_ :
      {
        long j; long resultset = 0;
        for (j = 0; j < SETSIZE; j++) {
          if (saved2(ec)->u.set & (1 << j)) {
            ec->stk = integerNewnode(ec, j, saved3(ec));
            exeterm(ec, saved1(ec)->u.lis);
            if (ec->stk->u.num)
              resultset = resultset | (1 << j); 
          } 
        }
        ec->stk = setNewnode(ec, resultset, saved3(ec));
        break; 
      }
    case STRING_ :
      {
        const char *s;
        char *resultstring;
        int j = 0;
        resultstring = (char *) malloc(strlen(saved2(ec)->u.str) + 1);
        for (s = saved2(ec)->u.str; *s != '\0'; s++) {
          ec->stk = charNewnode(ec, (long)*s, saved3(ec));
          exeterm(ec, saved1(ec)->u.lis);
          if (ec->stk->u.num)
            resultstring[j++] = *s; 
        }
        resultstring[j] = '\0';
        ec->stk = stringNewnode(ec, resultstring, saved3(ec));
        break;
      }
    case LIST_:
      {
        ec->dump1 = newnode(ec, LIST_,saved2(ec)->u,ec->dump1);	/* step old */
        ec->dump2 = listNewnode(ec, 0L, ec->dump2);		/* head new */
        ec->dump3 = listNewnode(ec, 0L, ec->dump3);		/* last new */
        while (dmp1(ec) != NULL) {
          ec->stk = newnode(ec, dmp1(ec)->op,dmp1(ec)->u,saved3(ec));
          exeterm(ec, saved1(ec)->u.lis);
          D(printf("filter: "); writefactor(ec->stk, stdout); printf("\n"));
            if (ec->stk->u.num) {				/* test */
              if (dmp2(ec) == NULL) {		/* first */
                dmp2(ec) = newnode(ec, dmp1(ec)->op, dmp1(ec)->u,NULL);
                dmp3(ec) = dmp2(ec);
              } 
              else {				/* further */
                dmp3(ec)->next = newnode(ec, dmp1(ec)->op, dmp1(ec)->u,NULL);
                dmp3(ec) = dmp3(ec)->next;
              }
            }
          dmp1(ec) = dmp1(ec)->next;
        }
        ec->stk = listNewnode(ec, dmp2(ec), saved3(ec));
        pop(ec->dump3);
        pop(ec->dump2);
        pop(ec->dump1);
        break;
      }
    default :
      badAggregate(ec, "filter");
  }
  pop(ec->dump);
}

PRIVATE void split_(pEC ec) {
  hasTwoParams(ec, "split");
  saveStack(ec);
  switch (saved2(ec)->op) {
    case SET_ :
      {
        long j; long yes_set = 0, no_set = 0;
        for (j = 0; j < SETSIZE; j++) {
          if (saved2(ec)->u.set & (1 << j)) {
            ec->stk = integerNewnode(ec, j,saved3(ec));
            exeterm(ec, saved1(ec)->u.lis);
            if (ec->stk->u.num)
              yes_set = yes_set | (1 << j);
            else  
              no_set = no_set | (1 << j);
          }
        }
        ec->stk = setNewnode(ec, yes_set,saved3(ec));
        nullary<long>(ec, &setNewnode,no_set);
        break;
      }
    case STRING_ :
      {
        const char *s;
        char *yesstring;
        char *nostring;
        int yesptr = 0;
        int noptr = 0;
        yesstring = (char *) malloc(strlen(saved2(ec)->u.str) + 1);
        nostring = (char *) malloc(strlen(saved2(ec)->u.str) + 1);
        for (s = saved2(ec)->u.str; *s != '\0'; s++)
        {
          ec->stk = charNewnode(ec, (long) *s, saved3(ec));
          exeterm(ec, saved1(ec)->u.lis);
          if (ec->stk->u.num)
            yesstring[yesptr++] = *s;
          else
            nostring[noptr++] = *s;
        }
        yesstring[yesptr] = '\0';
        nostring[noptr] = '\0';
        ec->stk = stringNewnode(ec, yesstring,saved3(ec));
        nullary<const char *>(ec, &stringNewnode,nostring);
        break;
      }
    case LIST_:
      {
        ec->dump1 = newnode(ec, LIST_, saved2(ec)->u, ec->dump1);	/* step old */
        ec->dump2 = listNewnode(ec, 0L, ec->dump2);		/* head true */
        ec->dump3 = listNewnode(ec, 0L, ec->dump3);		/* last true */
        ec->dump4 = listNewnode(ec, 0L, ec->dump4);		/* head false */
        ec->dump5 = listNewnode(ec, 0L, ec->dump5);		/* last false */
        while (dmp1(ec) != NULL)
        {
          ec->stk = newnode(ec, dmp1(ec)->op,dmp1(ec)->u, saved3(ec));
          exeterm(ec, saved1(ec)->u.lis);
          D(printf("split: "); writefactor(ec->stk, stdout); printf("\n"));
          if (ec->stk->u.num)				/* pass */
            if (dmp2(ec) == NULL) {		/* first */
              dmp2(ec) =
                newnode(ec, dmp1(ec)->op,
                    dmp1(ec)->u,NULL);
              dmp3(ec) = dmp2(ec);
            }
            else {				/* further */
              dmp3(ec)->next =
                newnode(ec, dmp1(ec)->op,
                    dmp1(ec)->u,NULL);
              dmp3(ec) = dmp3(ec)->next;
            }
          else					/* fail */
            if (dmp4(ec) == NULL)	{	/* first */
              dmp4(ec) =
                newnode(ec, dmp1(ec)->op,
                    dmp1(ec)->u,NULL);
              dmp5(ec) = dmp4(ec);
            }
            else { 				/* further */
              dmp5(ec)->next =
                newnode(ec, dmp1(ec)->op,
                    dmp1(ec)->u,NULL);
              dmp5(ec) = dmp5(ec)->next;
            }
          dmp1(ec) = dmp1(ec)->next;
        }
        ec->stk = listNewnode(ec, dmp2(ec), saved3(ec));
        nullary<pNode>(ec, &listNewnode, dmp4(ec));
        pop(ec->dump5);
        pop(ec->dump4);
        pop(ec->dump3);
        pop(ec->dump2);
        pop(ec->dump1);
        break;
      }
    default :
      badAggregate(ec, "split");
  }
  pop(ec->dump);
}

#define SOMEALL(PROCEDURE,NAME,INITIAL)				\
PRIVATE void PROCEDURE(pEC ec)	{				\
    long result = INITIAL;					\
    hasTwoParams(ec, NAME);						\
    hasOneQuote(ec, NAME);						\
    saveStack(ec);							\
    switch (saved2(ec)->op)						\
      { case SET_ :						\
	  { long j;						\
	    for (j = 0; j < SETSIZE && result == INITIAL; j++)	\
	      { if (saved2(ec)->u.set & (1 << j))			\
		  { ec->stk = integerNewnode(ec, j, saved3(ec));		\
		    exeterm(ec, saved1(ec)->u.lis);			\
		    if (ec->stk->u.num != INITIAL)			\
			result = 1 - INITIAL; } }		\
	    break; }						\
	case STRING_ :						\
	  { const char *s;						\
	    for (s = saved2(ec)->u.str;				\
		 *s != '\0' && result == INITIAL; s++)		\
	      { ec->stk = charNewnode(ec, (long)*s, saved3(ec));			\
		exeterm(ec, saved1(ec)->u.lis);				\
		if (ec->stk->u.num != INITIAL)			\
		    result = 1 - INITIAL; }			\
	    break; }						\
	case LIST_ :						\
	  { ec->dump1 = newnode(ec, LIST_, saved2(ec)->u, ec->dump1);		\
	    while (dmp1(ec) != NULL && result == INITIAL)	\
	      { ec->stk = newnode(ec, dmp1(ec)->op,			\
			dmp1(ec)->u, saved3(ec));		\
		exeterm(ec, saved1(ec)->u.lis);				\
		if (ec->stk->u.num != INITIAL)			\
		     result = 1 - INITIAL; 			\
		dmp1(ec) = dmp1(ec)->next; }		\
	    pop(ec->dump1);				\
	    break; }						\
	default :						\
	    badAggregate(ec, NAME); }				\
    ec->stk = booleanNewnode(ec, result, saved3(ec));			\
    pop(ec->dump);							\
}
SOMEALL(some_,"some",0L)
SOMEALL(all_,"all",1L)

PRIVATE void primrec_(pEC ec) {
  int n = 0; int i;
  hasThreeParams(ec, "primrec");
  saveStack(ec);
  ec->stk = ec->stk->next->next->next;
  switch (saved3(ec)->op) {
    case LIST_:
      {
        Node *current = saved3(ec)->u.lis;
        while (current != NULL) {
          ec->stk = newnode(ec, current->op,current->u,ec->stk);
          current = current->next;
          n++;
        }
        break;
      }
    case STRING_:
      {
        const char *s;
        for (s = saved3(ec)->u.str; *s != '\0'; s++) {
          ec->stk = charNewnode(ec, (long) *s, ec->stk);
          n++;
        }
        break;
      }
    case SET_:
      {
        long j; long set = saved3(ec)->u.set;
        for (j = 0; j < SETSIZE; j++)
          if (set & (1 << j)) {
            ec->stk = integerNewnode(ec, j,ec->stk);
            n++;
          }
        break;
      }
    case INTEGER_:
      {
        long j;
        for (j = saved3(ec)->u.num; j > 0; j--) {
          ec->stk = integerNewnode(ec, j, ec->stk);
          n++;
        }
        break;
      }
    default:
      badData(ec, "primrec");
  }
  exeterm(ec, saved2(ec)->u.lis);
  for (i = 1; i <= n; i++)
    exeterm(ec, saved1(ec)->u.lis);
  pop(ec->dump);
}

PRIVATE void tailrecaux(pEC ec) {
  int result;
tailrec:
  ec->dump1 = listNewnode(ec, ec->stk, ec->dump1);
  exeterm(ec, saved3(ec)->u.lis);
  result = ec->stk->u.num;
  ec->stk = dmp1(ec); pop(ec->dump1);
  if (result)
    exeterm(ec, saved2(ec)->u.lis)
    ; 
  else {
    exeterm(ec, saved1(ec)->u.lis);
    goto tailrec; 
  }
}

PRIVATE void tailrec_(pEC ec) {
  hasThreeParams(ec, "tailrec");
  saveStack(ec);
  ec->stk = saved4(ec);
  tailrecaux(ec);
  pop(ec->dump);
}

PRIVATE void construct_(pEC ec) {
  /* [P] [[P1] [P2] ..] -> X1 X2 ..	*/
  hasTwoParams(ec, "construct");
  hasTwoQuotes(ec, "construct");
  saveStack(ec);
  ec->stk = saved3(ec);			/* pop progs		*/
  ec->dump1 = listNewnode(ec, ec->dump2, ec->dump1);	/* save dump2		*/
  ec->dump2 = ec->stk;			/* save old stack	*/
  exeterm(ec, saved2(ec)->u.lis);		/* [P]			*/
  ec->dump3 = listNewnode(ec, ec->stk, ec->dump3);	/* save current stack	*/
  ec->dump4 = newnode(ec, LIST_,saved1(ec)->u,ec->dump4);	/* step [..]	*/
  while (dmp4(ec) != NULL) {
    ec->stk = dmp3(ec);			/* restore new stack	*/
    exeterm(ec, dmp4(ec)->u.lis);
    ec->dump2 = newnode(ec, ec->stk->op,ec->stk->u,ec->dump2); /* result	*/
    dmp4(ec) = dmp4(ec)->next;
  }
  pop(ec->dump4);
  pop(ec->dump3);
  ec->stk = ec->dump2; 
  ec->dump2 = ec->dump1->u.lis;	/* restore dump2	*/
  pop(ec->dump1);
  pop(ec->dump);
}

PRIVATE void branch_(pEC ec) {
  hasThreeParams(ec, "branch");
  hasTwoQuotes(ec, "branch");
  saveStack(ec);
  ec->stk = saved4(ec);
  exeterm(ec, saved3(ec)->u.num ? saved2(ec)->u.lis : saved1(ec)->u.lis);
  pop(ec->dump);
}

PRIVATE void while_(pEC ec)
{
  hasTwoParams(ec, "while");
  hasTwoQuotes(ec, "while");
  saveStack(ec);
  do {
    ec->stk = saved3(ec);
    exeterm(ec, saved2(ec)->u.lis);	/* TEST */
    if (! ec->stk->u.num)
      break;
    ec->stk = saved3(ec);
    exeterm(ec, saved1(ec)->u.lis);		/* DO */
    saved3(ec) = ec->stk;
  }
  while (1)
    ;
  ec->stk = saved3(ec);
  pop(ec->dump);
}

PRIVATE void ifte_(pEC ec) {
  int result;
  hasThreeParams(ec, "ifte");
  hasThreeQuotes(ec, "ifte");
  saveStack(ec);
  ec->stk = saved4(ec);
  exeterm(ec, saved3(ec)->u.lis);
  result = ec->stk->u.num;
  ec->stk = saved4(ec);
  exeterm(ec, result ? saved2(ec)->u.lis : saved1(ec)->u.lis);
  pop(ec->dump);
}

PRIVATE void condlinrecaux(pEC ec)
{
  int result = 0;
  ec->dump1 = newnode(ec, LIST_,saved1(ec)->u,ec->dump1);
  ec->dump2 = listNewnode(ec, ec->stk, ec->dump2);
  while ( result == 0 &&
      dmp1(ec) != NULL && dmp1(ec)->next != NULL ) {
    ec->stk = dmp2(ec);
    exeterm(ec, dmp1(ec)->u.lis->u.lis);
    result = ec->stk->u.num;
    if (!result) dmp1(ec) = dmp1(ec)->next;
  }
  ec->stk = dmp2(ec);
  if (result) {
    exeterm(ec, dmp1(ec)->u.lis->next->u.lis);
    if (dmp1(ec)->u.lis->next->next != NULL) {
      condlinrecaux(ec);
      exeterm(ec, dmp1(ec)->u.lis->next->next->u.lis);
    }
  }
  else {
    exeterm(ec, dmp1(ec)->u.lis->u.lis);
    if (dmp1(ec)->u.lis->next != NULL) {
      condlinrecaux(ec);
      exeterm(ec, dmp1(ec)->u.lis->next->u.lis);
    }
  }
  pop(ec->dump2);
  pop(ec->dump1);
}

PRIVATE void condlinrec_(pEC ec) {
  hasOneParam(ec, "condlinrec");
  hasList(ec, "condlinrec");
  checkEmptyList(ec, ec->stk->u.lis,"condlinrec");
  saveStack(ec);
  ec->stk = saved2(ec);
  condlinrecaux(ec);
  pop(ec->dump);
}

PRIVATE void condnestrecaux(pEC ec) {
  int result = 0;
  ec->dump1 = newnode(ec, LIST_,saved1(ec)->u,ec->dump1);
  ec->dump2 = listNewnode(ec, ec->stk, ec->dump2);
  while ( result == 0 &&
      dmp1(ec) != NULL && dmp1(ec)->next != NULL )
  {
    ec->stk = dmp2(ec);
    exeterm(ec, dmp1(ec)->u.lis->u.lis);
    result = ec->stk->u.num;
    if (!result)
      dmp1(ec) = dmp1(ec)->next;
  }
  ec->stk = dmp2(ec);
  ec->dump3 = listNewnode(ec, (result ? dmp1(ec)->u.lis->next : dmp1(ec)->u.lis), ec->dump3);
  exeterm(ec, dmp3(ec)->u.lis);
  dmp3(ec) = dmp3(ec)->next;
  while (dmp3(ec) != NULL) {
    condnestrecaux(ec);
    exeterm(ec, dmp3(ec)->u.lis);
    dmp3(ec) = dmp3(ec)->next;
  }
  pop(ec->dump3);
  /*
  if (result) {
    exeterm(ec, dmp1(ec)->u.lis->next->u.lis);
    if (dmp1(ec)->u.lis->next->next != NULL) {
      condnestrecaux(ec);
      exeterm(ec, dmp1(ec)->u.lis->next->next->u.lis);
    }
  }
  else {
    exeterm(ec, dmp1(ec)->u.lis->u.lis);
    if (dmp1(ec)->u.lis->next != NULL) {
      condnestrecaux(ec);
      exeterm(ec, dmp1(ec)->u.lis->next->u.lis);
    }
  }
  */
  pop(ec->dump2);
  pop(ec->dump1);
}

PRIVATE void condnestrec_(pEC ec) {
  hasOneParam(ec, "condnestrec");
  hasList(ec, "condnestrec");
  checkEmptyList(ec, ec->stk->u.lis,"condnestrec");
  saveStack(ec);
  ec->stk = saved2(ec);
  condnestrecaux(ec);
  pop(ec->dump);
}

PRIVATE void linrecaux(pEC ec) {
  int result;
  ec->dump1 = listNewnode(ec, ec->stk,ec->dump1);
  exeterm(ec, saved4(ec)->u.lis);
  result = ec->stk->u.num;
  ec->stk = dmp1(ec); pop(ec->dump1);
  if (result)
    exeterm(ec, saved3(ec)->u.lis);
  else {
    exeterm(ec, saved2(ec)->u.lis);
    linrecaux(ec);
    exeterm(ec, saved1(ec)->u.lis);
  }
}

PRIVATE void linrec_(pEC ec) {
  hasFourParams(ec, "linrec");
  hasFourQuotes(ec, "linrec");
  saveStack(ec);
  ec->stk = saved5(ec);
  linrecaux(ec);
  pop(ec->dump);
}

PRIVATE void binrecaux(pEC ec) {
  int result;
  ec->dump1 = listNewnode(ec, ec->stk,ec->dump1);
  exeterm(ec, saved4(ec)->u.lis);
  result = ec->stk->u.num;
  ec->stk = dmp1(ec);
  pop(ec->dump1);
  if (result) 
    exeterm(ec, saved3(ec)->u.lis);
  else
  {
    exeterm(ec, saved2(ec)->u.lis);		/* split */
    ec->dump2 = newnode(ec, ec->stk->op,ec->stk->u,ec->dump2);
    pop(ec->stk);
    binrecaux(ec);			/* first */
    GNULLARY(ec->dump2->op,ec->dump2->u);
    pop(ec->dump2);
    binrecaux(ec);			/* second */
    exeterm(ec, saved1(ec)->u.lis);		/* combine */
  }
}

PRIVATE void binrec_(pEC ec)
{
    hasFourParams(ec, "binrec");
    hasFourQuotes(ec, "binrec");
    saveStack(ec);
    ec->stk = saved5(ec);
    binrecaux(ec);
    pop(ec->dump);
}

PRIVATE void treestepaux(pEC ec, pNode item) {
  if (item->op != LIST_) {
    GNULLARY(item->op,item->u);
    exeterm(ec, saved1(ec)->u.lis);
  }
  else {
    ec->dump1 = newnode(ec, LIST_,item->u,ec->dump1);
    while (dmp1(ec) != NULL) {
      treestepaux(ec, dmp1(ec));
      dmp1(ec) = dmp1(ec)->next;
    }
    pop(ec->dump1);
  }
}

PRIVATE void treestep_(pEC ec) {
    hasTwoParams(ec, "treestep");
    hasOneQuote(ec, "treestep");
    saveStack(ec);
    ec->stk = saved3(ec);
    treestepaux(ec, saved2(ec));
    pop(ec->dump);
}

PRIVATE void treerecaux(pEC ec) {
  if (ec->stk->next->op == LIST_) {
    NULLARY(LIST_NEWNODE,ANON_FUNCT_NEWNODE(treerecaux,NULL));
    cons_(ec);		/*  D  [[[O] C] ANON_FUNCT_]	*/
    D(printf("treerecaux: stack = "));
    D(writeterm(ec->stk, stdout); printf("\n"));
    exeterm(ec, ec->stk->u.lis->u.lis->next);
  }
  else {
    Node *n = ec->stk;
    pop(ec->stk);
    exeterm(ec, n->u.lis->u.lis);
  }
}

PRIVATE void treerec_(pEC ec) {
  hasThreeParams(ec, "treerec");
  cons_(ec);
  D(printf("deep: stack = "); writeterm(ec->stk, stdout); printf("\n"));
  treerecaux(ec);
}

PRIVATE void genrecaux(pEC ec) {
  int result;
  D(printf("genrecaux: stack = "));
  D(writeterm(ec->stk, stdout); printf("\n"));
  saveStack(ec);
  pop(ec->stk);
  exeterm(ec, saved1(ec)->u.lis->u.lis);			/*	[I]	*/
  result = ec->stk->u.num;
  ec->stk = saved2(ec);
  if (result)
    exeterm(ec, saved1(ec)->u.lis->next->u.lis);	/*	[T]	*/
  else {
    exeterm(ec, saved1(ec)->u.lis->next->next->u.lis);	/*	[R1]	*/
    NULLARY(LIST_NEWNODE,saved1(ec)->u.lis);
    NULLARY(LIST_NEWNODE,ANON_FUNCT_NEWNODE(genrecaux,NULL));
    cons_(ec);
    exeterm(ec, saved1(ec)->u.lis->next->next->next);	/*	[R2]	*/
  }
  pop(ec->dump);
}

PRIVATE void genrec_(pEC ec) {
  hasFourParams(ec, "genrec");
  hasFourQuotes(ec, "genrec");
  cons_(ec);
  cons_(ec);
  cons_(ec);
  genrecaux(ec);
}

PRIVATE void treegenrecaux(pEC ec)
{
  D(printf("treegenrecaux: stack = "));
  D(writeterm(ec->stk, stdout); printf("\n"));
  if (ec->stk->next->op == LIST_) {
    saveStack(ec);				/* begin DIP	*/
    pop(ec->stk);
    exeterm(ec, saved1(ec)->u.lis->next->u.lis);	/*	[O2]	*/
    GNULLARY(saved1(ec)->op,saved1(ec)->u);
    pop(ec->dump);				/*   end DIP	*/
    NULLARY(LIST_NEWNODE,ANON_FUNCT_NEWNODE(treegenrecaux,NULL));
    cons_(ec);
    exeterm(ec, ec->stk->u.lis->u.lis->next->next); /*	[C]	*/
  }
  else {
    Node *n = ec->stk;
    pop(ec->stk);
    exeterm(ec, n->u.lis->u.lis); 		/*	[O1]	*/
  }
}

PRIVATE void treegenrec_(pEC ec) {
  /* T [O1] [O2] [C]	*/
  hasFourParams(ec, "treegenrec");
  cons_(ec); cons_(ec);
  D(printf("treegenrec: stack = "); writeterm(ec->stk, stdout); printf("\n"));
  treegenrecaux(ec);
}


PRIVATE void plain_manual_(pEC ec) {
  make_manual(ec, 0);
}

PRIVATE void html_manual_(pEC ec) {
  make_manual(ec, 1);
}

PRIVATE void latex_manual_(pEC ec) {
  make_manual(ec, 2);
}

PRIVATE void manual_list_aux_(pEC ec) {
  manual_list_(ec);
}


/* - - - - -   I N I T I A L I S A T I O N   - - - - - */

static struct {const char *name; void (*proc) (pEC ec); const char *messg1, *messg2; }
    optable[] =
	/* THESE MUST BE DEFINED IN THE ORDER OF THEIR VALUES */
{

{"__ILLEGAL",		dummy_,			"->",
"internal error, cannot happen - supposedly."},

{"__COPIED",		dummy_,			"->",
"no message ever, used for gc."},

{"__USR",		dummy_,			"usg",
"user node."},

{"__ANON_FUNCT",	dummy_,			"->",
"op for anonymous function call."},

/* LITERALS */

{" truth value type",		dummy_,		"->  B",
"The logical type, or the type of truth values.\nIt has just two literals: true and false."},

{" character type",		dummy_,		"->  C",
"The type of characters. Literals are written with a single quote.\nExamples:  'A  '7  ';  and so on. Unix style escapes are allowed."},

{" integer type",		dummy_,		"->  I",
"The type of negative, zero or positive integers.\nLiterals are written in decimal notation. Examples:  -123   0   42."},

{" set type",			dummy_,		"->  {...}",
"The type of sets of small non-negative integers.\nThe maximum is platform dependent, typically the range is 0..31.\nLiterals are written inside curly braces.\nExamples:  {}  {0}  {1 3 5}  {19 18 17}."},

{" string type",		dummy_,		"->  \"...\" ",
"The type of strings of characters. Literals are written inside double quotes.\nExamples: \"\"  \"A\"  \"hello world\" \"123\".\nUnix style escapes are accepted."},

{" list type",			dummy_,		"->  [...]",
"The type of lists of values of any type (including lists),\nor the type of quoted programs which may contain operators or combinators.\nLiterals of this type are written inside square brackets.\nExamples: []  [3 512 -7]  [john mary]  ['A 'C ['B]]  [dup *]."},

{" float type",			dummy_,		"->  F",
"The type of floating-point numbers.\nLiterals of this type are written with embedded decimal points (like 1.2)\nand optional exponent specifiers (like 1.5E2)"},

{" file type",			dummy_,		"->  FILE:",
"The type of references to open I/O streams,\ntypically but not necessarily files.\nThe only literals of this type are stdin, stdout, and stderr."},

/* OPERANDS */

{"false",		false_,		"->  false",
"Pushes the value false."},

{"true",		true_,		"->  true",
"Pushes the value true."},

{"maxint",		maxint_,	"->  maxint",
"Pushes largest integer (platform dependent). Typically it is 32 bits."},

{"setsize",		setsize_,	"->  setsize",
"Pushes the maximum number of elements in a set (platform dependent).\nTypically it is 32, and set members are in the range 0..31."},

{"stack",		stack_,		".. X Y Z  ->  .. X Y Z [Z Y X ..]",
"Pushes the stack as a list."},

{"__symtabmax",		symtabmax_,	"->",
"Pushes value of maximum size of the symbol table."},

{"__symtabindex",	symtabindex_,	"->",
"Pushes current size of the symbol table."},

{"__dump",		dump_,		"->",
"debugging only: pushes the dump as a list."},

{"conts",		conts_,		"->  [[P] [Q] ..]",
"Pushes current continuations. Buggy, do not use."},

{"autoput",		autoput_,	"->  I",
"Pushes current value of flag  for automatic output, I = 0..2."},

{"undeferror",		undeferror_,	"->  I",
"Pushes current value of undefined-is-error flag."},

{"undefs",		undefs_,	"->",
"Push a list of all undefined symbols in the current symbol table."},

{"echo",			echo_,		"->  I",
"Pushes value of echo flag, I = 0..3."},

{"clock",		clock_,		"->  I",
"Pushes the integer value of current CPU usage in hundreds of a second."},

{"time",		time_,		"->  I",
"Pushes the current time (in seconds since the Epoch)."},

{"rand",		rand_,		"  -> I",
"I is a random integer."},

{"__memorymax",		memorymax_,	"->",
"Pushes value of total size of memory."},

{"stdin",		stdin_,		"->  S",
"Pushes the standard input stream."},

{"stdout",		stdout_,		"->  S",
"Pushes the standard output stream."},

{"stderr",		stderr_,		"->  S",
"Pushes the standard error stream."},


/* OPERATORS */

{"id",			id_,		"->",
"Identity function, does nothing.\nAny program of the form  P id Q  is equivalent to just  P Q."},

{"dup",			dup_,		" X  ->   X X",
"Pushes an extra copy of X onto stack."},

{"swap",		swap_,		" X Y  ->   Y X",
"Interchanges X and Y on top of the stack."},

{"rollup",              rollup_,        "X Y Z  ->  Z X Y",
"Moves X and Y up, moves Z down"},

{"rolldown",            rolldown_,      "X Y Z  ->  Y Z X",
"Moves Y and Z down, moves X up"},

{"rotate",              rotate_,        "X Y Z  ->  Z Y X",
"Interchanges X and Z"},

{"popd",                popd_,          "Y Z  ->  Z",
"As if defined by:   popd  ==  [pop] dip "},

{"dupd",                dupd_,          "Y Z  ->  Y Y Z",
"As if defined by:   dupd  ==  [dup] dip"},

{"swapd",               swapd_,         "X Y Z  ->  Y X Z",
"As if defined by:   swapd  ==  [swap] dip"},

{"rollupd",             rollupd_,       "X Y Z W  ->  Z X Y W",
"As if defined by:   rollupd  ==  [rollup] dip"},

{"rolldownd",           rolldownd_,     "X Y Z W  ->  Y Z X W",
"As if defined by:   rolldownd  ==  [rolldown] dip "},

{"rotated",             rotated_,       "X Y Z W  ->  Z Y X W",
"As if defined by:   rotated  ==  [rotate] dip"},

{"pop",			pop_,		" X  ->",
"Removes X from top of the stack."},

{"choice",		choice_,	"B T F  ->  X",
"If B is true, then X = T else X = F."},

{"or",			or_,		"X Y  ->  Z",
"Z is the union of sets X and Y, logical disjunction for truth values."},

{"xor",			xor_,		"X Y  ->  Z",
"Z is the symmetric difference of sets X and Y,\nlogical exclusive disjunction for truth values."},

{"and",			and_,		"X Y  ->  Z",
"Z is the intersection of sets X and Y, logical conjunction for truth values."},

{"not",			not_,		"X  ->  Y",
"Y is the complement of set X, logical negation for truth values."},

{"+",			plus_,		"M I  ->  N",
"Numeric N is the result of adding integer I to numeric M.\nAlso supports float."},

{"-",			minus_,		"M I  ->  N",
"Numeric N is the result of subtracting integer I from numeric M.\nAlso supports float."},

{"*",			mul_,		"I J  ->  K",
"Integer K is the product of integers I and J.  Also supports float."},

{"/",			divide_,	"I J  ->  K",
"Integer K is the (rounded) ratio of integers I and J.  Also supports float."},

{"rem",			rem_,		"I J  ->  K",
"Integer K is the remainder of dividing I by J.  Also supports float."},

{"div",			div_,		"I J  ->  K L",
"Integers K and L are the quotient and remainder of dividing I by J."},

{"sign",		sign_,		"N1  ->  N2",
"Integer N2 is the sign (-1 or 0 or +1) of integer N1,\nor float N2 is the sign (-1.0 or 0.0 or 1.0) of float N1."},

{"neg",			neg_,		"I  ->  J",
"Integer J is the negative of integer I.  Also supports float."},

{"ord",			ord_,		"C  ->  I",
"Integer I is the Ascii value of character C (or logical or integer)."},

{"chr",			chr_,		"I  ->  C",
"C is the character whose Ascii value is integer I (or logical or character)."},

{"abs",			abs_,		"N1  ->  N2",
"Integer N2 is the absolute value (0,1,2..) of integer N1,\nor float N2 is the absolute value (0.0 ..) of float N1"},

{"acos",		acos_,		"F  ->  G",
"G is the arc cosine of F."},

{"asin",		asin_,		"F  ->  G",
"G is the arc sine of F."},

{"atan",		atan_,		"F  ->  G",
"G is the arc tangent of F."},

{"atan2",		atan2_,		"F G  ->  H",
"H is the arc tangent of F / G."},

{"ceil",		ceil_,		"F  ->  G",
"G is the float ceiling of F."},

{"cos",		cos_,		"F  ->  G",
"G is the cosine of F."},

{"cosh",		cosh_,		"F  ->  G",
"G is the hyperbolic cosine of F."},

{"exp",			exp_,		"F  ->  G",
"G is e (2.718281828...) raised to the Fth power."},

{"floor",		floor_,		"F  ->  G",
"G is the floor of F."},

{"frexp",		frexp_,		"F  ->  G I",
"G is the mantissa and I is the exponent of F.\nUnless F = 0, 0.5 <= abs(G) < 1.0."},

{"ldexp",		ldexp_,		"F I  -> G",
"G is F times 2 to the Ith power."},

{"log",		log_,		"F  ->  G",
"G is the natural logarithm of F."},

{"log10",		log10_,		"F  ->  G",
"G is the common logarithm of F."},

{"modf",		modf_,		"F  ->  G H",
"G is the fractional part and H is the integer part\n(but expressed as a float) of F."},

{"pow",			pow_,		"F G  ->  H",
"H is F raised to the Gth power."},

{"sin",		sin_,		"F  ->  G",
"G is the sine of F."},

{"sinh",		sinh_,		"F  ->  G",
"G is the hyperbolic sine of F."},

{"sqrt",		sqrt_,		"F  ->  G",
"G is the square root of F."},

{"tan",		tan_,		"F  ->  G",
"G is the tangent of F."},

{"tanh",		tanh_,		"F  ->  G",
"G is the hyperbolic tangent of F."},

{"trunc",		trunc_,		"F  ->  I",
"I is an integer equal to the float F truncated toward zero."},

{"localtime",		localtime_,	"I  ->  T",
"Converts a time I into a list T representing local time:\n[year month day hour minute second isdst yearday weekday].\nMonth is 1 = January ... 12 = December;\nisdst is a Boolean flagging daylight savings/summer time;\nweekday is 0 = Monday ... 7 = Sunday."},

{"gmtime",		gmtime_,	"I  ->  T",
"Converts a time I into a list T representing universal time:\n[year month day hour minute second isdst yearday weekday].\nMonth is 1 = January ... 12 = December;\nisdst is false; weekday is 0 = Monday ... 7 = Sunday."},

{"mktime",		mktime_,	"T  ->  I",
"Converts a list T representing local time into a time I.\nT is in the format generated by localtime."},

{"strftime",		strftime_,	"T S1  ->  S2",
"Formats a list T in the format of localtime or gmtime\nusing string S1 and pushes the result S2."},

{"strtol",		strtol_,	"S I  ->  J",
"String S is converted to the integer J using base I.\nIf I = 0, assumes base 10,\nbut leading \"0\" means base 8 and leading \"0x\" means base 16."},

{"strtod",		strtod_,	"S  ->  R",
"String S is converted to the float R."},

{"format",		format_,	"N C I J  ->  S",
"S is the formatted version of N in mode C\n('d or 'i = decimal, 'o = octal, 'x or\n'X = hex with lower or upper case letters)\nwith maximum width I and minimum width J."},

{"formatf",		formatf_,	"F C I J  ->  S",
"S is the formatted version of F in mode C\n('e or 'E = exponential, 'f = fractional,\n'g or G = general with lower or upper case letters)\nwith maximum width I and precision J."},

{"srand",		srand_,		"I  ->  ",
"Sets the random integer seed to integer I."},

{"pred",		pred_,		"M  ->  N",
"Numeric N is the predecessor of numeric M."},

{"succ",		succ_,		"M  ->  N",
"Numeric N is the successor of numeric M."},

{"max",			max_,		"N1 N2  ->  N",
"N is the maximum of numeric values N1 and N2.  Also supports float."},

{"min",			min_,		"N1 N2  ->  N",
"N is the minimum of numeric values N1 and N2.  Also supports float."},

{"fclose",		fclose_,	"S  ->  ",
"Stream S is closed and removed from the stack."},

{"feof",		feof_,		"S  ->  S B",
"B is the end-of-file status of stream S."},

{"ferror",		ferror_,	"S  ->  S B",
"B is the error status of stream S."},

{"fflush",		fflush_,	"S  ->  S",
"Flush stream S, forcing all buffered output to be written."},

{"fgetch",		fgetch_,	"S  ->  S C",
"C is the next available character from stream S."},

{"fgets",		fgets_,		"S  ->  S L",
"L is the next available line (as a string) from stream S."},

{"fopen",		fopen_,		"P M  ->  S",
"The file system object with pathname P is opened with mode M (r, w, a, etc.)\nand stream object S is pushed; if the open fails, file:NULL is pushed."},

{"fread",		fread_,		"S I  ->  S L",
"I bytes are read from the current position of stream S\nand returned as a list of I integers."},

{"fwrite",		fwrite_,	"S L  ->  S",
"A list of integers are written as bytes to the current position of stream S."},

{"fremove",		fremove_,	"P  ->  B",
"The file system object with pathname P is removed from the file system.\n is a boolean indicating success or failure."},

{"frename",		frename_,	"P1 P2  ->  B",
"The file system object with pathname P1 is renamed to P2.\nB is a boolean indicating success or failure."},

{"fput",		fput_,		"S X  ->  S",
"Writes X to stream S, pops X off stack."},

{"fputch",		fputch_,	"S C  ->  S",
"The character C is written to the current position of stream S."},

{"fputchars",		fputchars_,	"S \"abc..\"  ->  S",
"The string abc.. (no quotes) is written to the current position of stream S."},

{"fputstring",		fputchars_,	"S \"abc..\"  ->  S",
"== fputchars, as a temporary alternative."},

{"fseek",		fseek_,		"S P W  ->  S",
"Stream S is repositioned to position P relative to whence-point W,\nwhere W = 0, 1, 2 for beginning, current position, end respectively."},

{"ftell",		ftell_,		"S  ->  S I",
"I is the current position of stream S."},

{"unstack",		unstack_,	"[X Y ..]  ->  ..Y X",
"The list [X Y ..] becomes the new stack."},

{"cons",		cons_,		"X A  ->  B",
"Aggregate B is A with a new member X (first member for sequences)."},

{"swons",		swons_,		"A X  ->  B",
"Aggregate B is A with a new member X (first member for sequences)."},

{"first",		first_,		"A  ->  F",
"F is the first member of the non-empty aggregate A."},

{"rest",		rest_,		"A  ->  R",
"R is the non-empty aggregate A with its first member removed."},

{"compare",		compare_,	"A B  ->  I",
"I (=-1,0,+1) is the comparison of aggregates A and B.\nThe values correspond to the predicates <=, =, >=."},

{"at",			at_,		"A I  ->  X",
"X (= A[I]) is the member of A at position I."},

{"of",			of_,		"I A  ->  X",
"X (= A[I]) is the I-th member of aggregate A."},

{"size",		size_,		"A  ->  I",
"Integer I is the number of elements of aggregate A."},

{"opcase",		opcase_,	"X [..[X Xs]..]  ->  [Xs]",
"Indexing on type of X, returns the list [Xs]."},

{"case",		case_,		"X [..[X Y]..]  ->  Y i",
"Indexing on the value of X, execute the matching Y."},

{"uncons",		uncons_,	"A  ->  F R",
"F and R are the first and the rest of non-empty aggregate A."},

{"unswons",		unswons_,	"A  ->  R F",
"R and F are the rest and the first of non-empty aggregate A."},

{"drop",		drop_,		"A N  ->  B",
"Aggregate B is the result of deleting the first N elements of A."},

{"take",		take_,		"A N  ->  B",
"Aggregate B is the result of retaining just the first N elements of A."},

{"concat",		concat_,	"S T  ->  U",
"Sequence U is the concatenation of sequences S and T."},

{"enconcat",		enconcat_,	"X S T  ->  U",
"Sequence U is the concatenation of sequences S and T\nwith X inserted between S and T (== swapd cons concat)"},

{"name",		name_,		"sym  ->  \"sym\"",
"For operators and combinators, the string \"sym\" is the name of item sym,\nfor literals sym the result string is its type."},

{"intern",		intern_,	"\"sym\"  -> sym",
"Pushes the item whose name is \"sym\"."},

{"body",		body_,		"U  ->  [P]",
"Quotation [P] is the body of user-defined symbol U."},

/* PREDICATES */

{"null",			null_,		"X  ->  B",
"Tests for empty aggregate X or zero numeric."},

{"small",		small_,		"X  ->  B",
"Tests whether aggregate X has 0 or 1 members, or numeric 0 or 1."},

{">=",			geql_,		"X Y  ->  B",
"Either both X and Y are numeric or both are strings or symbols.\nTests whether X greater than or equal to Y.  Also supports float."},

{">",			greater_,		"X Y  ->  B",
"Either both X and Y are numeric or both are strings or symbols.\nTests whether X greater than Y.  Also supports float."},

{"<=",			leql_,			"X Y  ->  B",
"Either both X and Y are numeric or both are strings or symbols.\nTests whether X less than or equal to Y.  Also supports float."},

{"<",			less_,			"X Y  ->  B",
"Either both X and Y are numeric or both are strings or symbols.\nTests whether X less than Y.  Also supports float."},

{"!=",			neql_,			"X Y  ->  B",
"Either both X and Y are numeric or both are strings or symbols.\nTests whether X not equal to Y.  Also supports float."},

{"=",			eql_,			"X Y  ->  B",
"Either both X and Y are numeric or both are strings or symbols.\nTests whether X equal to Y.  Also supports float."},

{"equal",		equal_,			"T U  ->  B",
"(Recursively) tests whether trees T and U are identical."},

{"has",			has_,			"A X  ->  B",
"Tests whether aggregate A has X as a member."},

{"in",			in_,			"X A  ->  B",
"Tests whether X is a member of aggregate A."},

{"integer",		integer_,	"X  ->  B",
"Tests whether X is an integer."},

{"char",			char_,		"X  ->  B",
"Tests whether X is a character."},

{"logical",		logical_,	"X  ->  B",
"Tests whether X is a logical."},

{"set",			set_,		"X  ->  B",
"Tests whether X is a set."},

{"string",		string_,	"X  ->  B",
"Tests whether X is a string."},

{"list",			list_,		"X  ->  B",
"Tests whether X is a list."},

{"leaf",		leaf_,		"X  ->  B",
"Tests whether X is not a list."},

{"user",		user_,		"X  ->  B",
"Tests whether X is a user-defined symbol."},

{"float",		float_,		"R  ->  B",
"Tests whether R is a float."},

{"file",		file_,		"F  ->  B",
"Tests whether F is a file."},

/* COMBINATORS */

{"i",			i_,		"[P]  ->  ...",
"Executes P. So, [P] i  ==  P."},

{"x",			x_,		"[P]i  ->  ...",
"Executes P without popping [P]. So, [P] x  ==  [P] P."},

{"dip", 		dip_,		"X [P]  ->  ... X",
"Saves X, executes P, pushes X back."},

{"app1",		app1_,		"X [P]  ->  R",
"Executes P, pushes result R on stack without X."},

{"app11",		app11_,		"X Y [P]  ->  R",
"Executes P, pushes result R on stack."},

{"app12",		app12_,		"X Y1 Y2 [P]  ->  R1 R2",
"Executes P twice, with Y1 and Y2, returns R1 and R2."},

{"construct",		construct_,	"[P] [[P1] [P2] ..]  ->  R1 R2 ..",
"Saves state of stack and then executes [P].\nThen executes each [Pi] to give Ri pushed onto saved stack."},

{"nullary",		nullary_,	"[P]  ->  R",
"Executes P, which leaves R on top of the stack.\nNo matter how many parameters this consumes, none are removed from the stack."},

{"unary",		unary_,		"X [P]  ->  R",
"Executes P, which leaves R on top of the stack.\nNo matter how many parameters this consumes,\nexactly one is removed from the stack."},

{"unary2",		unary2_,	"X1 X2 [P]  ->  R1 R2",
"Executes P twice, with X1 and X2 on top of the stack.\nReturns the two values R1 and R2."},

{"unary3",		unary3_,	"X1 X2 X3 [P]  ->  R1 R2 R3",
"Executes P three times, with Xi, returns Ri (i = 1..3)."},

{"unary4",		unary4_,	"X1 X2 X3 X4 [P]  ->  R1 R2 R3 R4",
"Executes P four times, with Xi, returns Ri (i = 1..4)."},

{"app2",		unary2_,	"X1 X2 [P]  ->  R1 R2",
"Obsolescent.  == unary2"},

{"app3",		unary3_,	"X1 X2 X3 [P]  ->  R1 R2 R3",
"Obsolescent.  == unary3"},

{"app4",		unary4_,	"X1 X2 X3 X4 [P]  ->  R1 R2 R3 R4",
"Obsolescent.  == unary4"},

{"binary",		binary_,	"X Y [P]  ->  R",
"Executes P, which leaves R on top of the stack.\nNo matter how many parameters this consumes,\nexactly two are removed from the stack."},

{"ternary",		ternary_,	"X Y Z [P]  ->  R",
"Executes P, which leaves R on top of the stack.\nNo matter how many parameters this consumes,\nexactly three are removed from the stack."},

{"cleave",		cleave_,	"X [P1] [P2]  ->  R1 R2",
"Executes P1 and P2, each with X on top, producing two results."},

{"branch",		branch_,	"B [T] [F]  ->  ...",
"If B is true, then executes T else executes F."},

{"ifte",			ifte_,		"[B] [T] [F]  ->  ...",
"Executes B. If that yields true, then executes T else executes F."},

{"ifinteger",		ifinteger_,	"X [T] [E]  ->  ...",
"If X is an integer, executes T else executes E."},

{"ifchar",		ifchar_,	"X [T] [E]  ->  ...",
"If X is a character, executes T else executes E."},

{"iflogical",		iflogical_,	"X [T] [E]  ->  ...",
"If X is a logical or truth value, executes T else executes E."},

{"ifset",		ifset_,		"X [T] [E]  ->  ...",
"If X is a set, executes T else executes E."},

{"ifstring",		ifstring_,	"X [T] [E]  ->  ...",
"If X is a string, executes T else executes E."},

{"iflist",		iflist_,	"X [T] [E]  ->  ...",
"If X is a list, executes T else executes E."},

{"iffloat",		iffloat_,	"X [T] [E]  ->  ...",
"If X is a float, executes T else executes E."},

{"iffile",		iffile_,	"X [T] [E]  ->  ...",
"If X is a file, executes T else executes E."},

{"cond",		cond_,		"[..[[Bi] Ti]..[D]]  ->  ...",
"Tries each Bi. If that yields true, then executes Ti and exits.\nIf no Bi yields true, executes default D."},

{"while",		while_,		"[B] [D]  ->  ...",
"While executing B yields true executes D."},

{"linrec",		linrec_,	"[P] [T] [R1] [R2]  ->  ...",
"Executes P. If that yields true, executes T.\nElse executes R1, recurses, executes R2."},

{"tailrec",		tailrec_,	"[P] [T] [R1]  ->  ...",
"Executes P. If that yields true, executes T.\nElse executes R1, recurses."},

{"binrec",		binrec_,	"[B] [T] [R1] [R2]  ->  ...",
"Executes P. If that yields true, executes T.\nElse uses R1 to produce two intermediates, recurses on both,\nthen executes R2 to combines their results."},

{"genrec",		genrec_,	"[B] [T] [R1] [R2]  ->  ...",
"Executes B, if that yields true executes T.\nElse executes R1 and then [[B] [T] [R1] [R2] genrec] R2."},

{"condnestrec",	condnestrec_,	"[ [C1] [C2] .. [D] ]  ->  ...",
"A generalisation of condlinrec. Each [Ci] is of the form [[B] [R1] [R2] .. [Rn]] and [D] is of the form [[R1] [R2] .. [Rn]]. Tries each B, or if all fail, takes the default [D]. For the case taken, executes each [Ri] but recurses between any two consecutive [Ri]. (n > 3 would be exceptional.)"},

{"condlinrec",		condlinrec_,	"[ [C1] [C2] .. [D] ]  ->  ...",
"Each [Ci] is of the forms [[B] [T]] or [[B] [R1] [R2]].\nTries each B. If that yields true and there is just a [T], executes T and exit.\nIf there are [R1] and [R2], executes R1, recurses, executes R2.\nSubsequent case are ignored. If no B yields true, then [D] is used.\nIt is then of the forms [[T]] or [[R1] [R2]]. For the former, executes T.\nFor the latter executes R1, recurses, executes R2."},

{"step",			step_,		"A  [P]  ->  ...",
"Sequentially putting members of aggregate A onto stack,\nexecutes P for each member of A."},

{"fold",			fold_,		"A V0 [P]  ->  V",
"Starting with value V0, sequentially pushes members of aggregate A\nand combines with binary operator P to produce value V."},

{"map",			map_,		"A [P]  ->  B",
"Executes P on each member of aggregate A,\ncollects results in sametype aggregate B."},

{"times",		times_,		"N [P]  ->  ...",
"N times executes P."},

{"infra",		infra_,		"L1 [P]  ->  L2",
"Using list L1 as stack, executes P and returns a new list L2.\nThe first element of L1 is used as the top of stack,\nand after execution of P the top of stack becomes the first element of L2."},

{"primrec",		primrec_,	"X [I] [C]  ->  R",
"Executes I to obtain an initial value R0.\nFor integer X uses increasing positive integers to X, combines by C for new R.\nFor aggregate X uses successive members and combines by C for new R."},

{"filter",		filter_,	"A [B]  ->  A1",
"Uses test B to filter aggregate A producing sametype aggregate A1."},

{"split",		split_,		"A [B]  ->  A1 A2",
"Uses test B to split aggregate A into sametype aggregates A1 and A2 ."},

{"some",			some_,		"A  [B]  ->  X",
"Applies test B to members of aggregate A, X = true if some pass."},

{"all",			all_,		"A [B]  ->  X",
"Applies test B to members of aggregate A, X = true if all pass."},

{"treestep",		treestep_,	"T [P]  ->  ...",
"Recursively traverses leaves of tree T, executes P for each leaf."},

{"treerec",		treerec_,	"T [O] [C]  ->  ...",
"T is a tree. If T is a leaf, executes O. Else executes [[O] [C] treerec] C."},

{"treegenrec",		treegenrec_,	"T [O1] [O2] [C]  ->  ...",
"T is a tree. If T is a leaf, executes O1.\nElse executes O2 and then [[O1] [O2] [C] treegenrec] C."},

/* MISCELLANEOUS */

{"help",		help1_,			"->",
"Lists all defined symbols, including those from library files.\nThen lists all primitives of raw Joy\n(There is a variant: \"_help\" which lists hidden symbols)."},

{"_help",		h_help1_,		"->",
"Lists all hidden symbols in library and then all hidden inbuilt symbols."},

{"helpdetail",			helpdetail_,		"[ S1  S2  .. ]",
"Gives brief help on each symbol S in the list."},

{"manual",		plain_manual_,		"->",
"Writes this manual of all Joy primitives to output file."},

{"__html_manual",	html_manual_,		"->",
"Writes this manual of all Joy primitives to output file in HTML style."},

{"__latex_manual",	latex_manual_,	"->",
"Writes this manual of all Joy primitives to output file in Latex style but without the head and tail."},

{"__manual_list",	manual_list_aux_,   "->  L",
"Pushes a list L of lists (one per operator) of three documentation strings"},

{"__settracegc",		settracegc_,	"I  ->",
"Sets value of flag for tracing garbage collection to I (= 0..5)."},

{"setautoput",		setautoput_,	"I  ->",
"Sets value of flag for automatic put to I (if I = 0, none;\nif I = 1, put; if I = 2, stack."},

{"setundeferror",	setundeferror_,	"I  ->",
"Sets flag that controls behavior of undefined functions\n(0 = no error, 1 = error)."},

{"setecho",		setecho_,	"I ->",
"Sets value of echo flag for listing.\nI = 0: no echo, 1: echo, 2: with tab, 3: and linenumber."},

{"gc",			gc_,	"->",
"Initiates garbage collection."},

{"system",		system_,	"\"command\"  ->",
"Escapes to shell, executes string \"command\".\nThe string may cause execution of another program.\nWhen that has finished, the process returns to Joy."},

{"getenv",		getenv_,	"\"variable\"  ->  \"value\"",
"Retrieves the value of the environment variable \"variable\"."},

{"argv",		argv_,		"-> A",
"Creates an aggregate A containing the interpreter's command line arguments."},

{"argc",		argc_,		"-> I",
"Pushes the number of command line arguments. This is quivalent to 'argv size'."},

{"__memoryindex",	memoryindex_,	"->",
"Pushes current value of memory."},

{"get",			get_,		"->  F",
"Reads a factor from input and pushes it onto stack."},

{"put",			put_,		"X  ->",
"Writes X to output, pops X off stack."},

{"putch",		putch_,		"N  ->",
"N : numeric, writes character whose ASCII is N."},

{"putchars",		putchars_,	"\"abc..\"  ->",
"Writes  abc.. (without quotes)"},

{"include",		include_,	"\"filnam.ext\"  ->",
"Transfers input to file whose name is \"filnam.ext\".\nOn end-of-file returns to previous input file."},

{"abort",		abortexecution_,	 "->",
"Aborts execution of current Joy program, returns to Joy main cycle."},

{"quit",		quit_,			"->",
"Exit from Joy."},

{0, dummy_, "->","->"}
};

PUBLIC void inisymboltable(pEC ec) {		/* initialise			*/
  int i; const char *s;
  ec->symtabindex = ec->symtab;
  for (i = 0; i < HASHSIZE; ec->hashentry[i++] = ec->symtab)
    ;
  ec->localentry = ec->symtab;
  for (i = 0; optable[i].name; i++) {
    s = optable[i].name;
    /* ensure same algorithm in getsym */
    for (ec->hashvalue = 0; *s != '\0';)
      ec->hashvalue += *s++;
    ec->hashvalue %= HASHSIZE;
    ec->symtabindex->name = optable[i].name;
    ec->symtabindex->u.proc = optable[i].proc;
    ec->symtabindex->next = ec->hashentry[ec->hashvalue];
    ec->hashentry[ec->hashvalue] = ec->symtabindex;
    D(printf("entered %s in symbol table at %ld = %ld\n", \
          ec->symtabindex->name, (long)ec->symtabindex, \
          LOC2INT(ec->symtabindex)));
    ec->symtabindex++;
  }
  ec->firstlibra = ec->symtabindex;
}

PRIVATE void helpdetail_(pEC ec) {
  Node *n;
  hasOneParam(ec, "HELP");
  hasList(ec, "HELP");
  printf("\n");
  n = ec->stk->u.lis;
  while (n != NULL) {
    if (n->op == USR_) {
      printf("%s  ==\n    ",n->u.ent->name);
      writeterm(ec, n->u.ent->u.body, stdout);
      printf("\n");
      break;
    }
    else
      printf("%s        :   %s.\n%s\n",
          optable[ (int) n->op].name,
          optable[ (int) n->op].messg1,
          optable[ (int) n->op].messg2);
    printf("\n");
    n = n->next;
  }
  pop(ec->stk);
}

#define PLAIN (style == 0)
#define HTML (style == 1)
#define LATEX (style == 2)
#define HEADER(N,NAME,HEAD)					\
    if (strcmp(N,NAME) == 0)					\
      { printf("\n\n");						\
        if (LATEX) printf("\\item[--- \\BX{");				\
	printf("%s",HEAD);					\
	if (LATEX) printf("} ---] \\verb# #");				\
	printf("\n\n"); }

PRIVATE void make_manual(pEC, int style /* 0=plain, 1=HTML, 2=Latex */) {
  int i;
  const char * n;
  if (HTML) printf("<HTML>\n<DL>\n");
  for (i = BOOLEAN_; optable[i].name != 0; i++) {
    n = optable[i].name;
    HEADER(n," truth value type","literal") else
      HEADER(n,"false","operand") else
      HEADER(n,"id","operator") else
      HEADER(n,"null","predicate") else
      HEADER(n,"i","combinator") else
      HEADER(n,"help","miscellaneous commands")
      if (n[0] != '_') {
        if (HTML) printf("\n<DT>");
        else 
          if (LATEX) { 
            if (n[0] == ' ') {
              n++;
              printf("\\item[\\BX{");
            }
            else printf("\\item[\\JX{");
          }
        if (HTML && strcmp(n,"<=")==0)
          printf("&lt;=");
        else
          printf("%s",n);
        if (LATEX)
          printf("}]  \\verb#");
        if (HTML)
          printf(" <CODE>      :  </CODE> ");
        /* the above line does not produce the spaces around ":" */
        else
          printf("      :  ");
        printf("%s", optable[i].messg1);
        if (HTML)
          printf("\n<DD>");
        else
          if (LATEX)
            printf("# \\\\ \n {\\small\\verb#");
          else
            printf("\n");
        printf("%s", optable[i].messg2);
        if (LATEX)
          printf("#}");
        printf("\n\n");
      }
  }
  if (HTML)
    printf("\n</DL>\n</HTML>\n");
}

PRIVATE void manual_list_(pEC ec) {
  int i = -1;
  Node *tmp;
  Node *n = NULL;
  while (optable[++i].name)  /* find end */
    ;
  --i; /* overshot */
  while (i) {
    tmp = stringNewnode(ec, optable[i].messg2, NULL);
    tmp = stringNewnode(ec, optable[i].messg1, tmp);
    tmp = stringNewnode(ec, optable[i].name, tmp);
    n   = listNewnode(ec, tmp, n);
    --i;
  }
  ec->stk = listNewnode(ec, n,ec->stk);
}

PUBLIC const char *opername(pEC ec, int o) {
  return optable[(short)o].name;
}

/* END of INTERP.C */
