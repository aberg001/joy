#include <stdio.h>
#include <time.h>
#include <string.h>
#include <setjmp.h>
#include "globals.h"
# ifdef GC_BDW
#    include "gc/gc.h"
# endif

# ifndef GC_BDW
#define MEM_HIGH (MEMORYMAX-1)
# endif

PUBLIC void inimem0(pEC ec) {
  ec->util.memoryindex = ec->util.memory;
  ec->util.mem_low = ec->util.memory;
  ec->util.direction = +1;
}

PUBLIC void inimem1(pEC ec) {
  ec->stk = ec->conts = ec->dump = ec->dump1 = ec->dump2 = ec->dump3 = ec->dump4 = ec->dump5 = NULL;
# ifndef GC_BDW
  ec->util.direction = +1;
  ec->util.memoryindex = ec->util.mem_low;
# endif
}

PUBLIC void inimem2(pEC ec) {
# ifndef GC_BDW
  ec->util.mem_low = ec->util.memoryindex;
  ec->util.mem_mid = ec->util.mem_low + (MEM_HIGH)/2;
  if (ec->tracegc > 1) {
    printf("memory = %ld : %ld\n",
        (long)ec->util.memory, MEM2INT(ec->util.memory));
    printf("memoryindex = %ld : %ld\n",
        (long)ec->util.memoryindex, MEM2INT(ec->util.memoryindex));
    printf("mem_low = %ld : %ld\n",
        (long)ec->util.mem_low, MEM2INT(ec->util.mem_low));
    printf("top of mem = %ld : %ld\n",
        (long)(&ec->util.memory[MEM_HIGH]), MEM2INT((&ec->util.memory[MEM_HIGH])));
    printf("mem_mid = %ld : %ld\n",
        (long)ec->util.mem_mid, MEM2INT(ec->util.mem_mid));
  }
# endif
}

PUBLIC void printnode(pEC ec, pNode p) {
# ifndef GC_BDW
  printf("%10ld:        %-10s %10ld %10ld\n",
      MEM2INT(p),
      ec->symtab[(short) p->op].name,
      p->op == LIST_ ? MEM2INT(p->u.lis) : p->u.num,
      MEM2INT(p->next));
# endif
}

# ifndef GC_BDW
PRIVATE Node *copy(pEC ec, pNode n) {
    Node *temp;
    ec->util.nodesinspected++;
    if (ec->tracegc > 4)
      printf("copy ..\n");
    if (n == NULL)
      return NULL;
    if (n < ec->util.mem_low)
      return n; /* later: combine with previous line */
    if (n->op == ILLEGAL_) {
      printf("copy: illegal node  ");
      printnode(ec, n);
      return(NULL);
    }
    if (n->op == COPIED_)
      return n->u.lis;
    temp = ec->util.memoryindex;
    ec->util.memoryindex += ec->util.direction;
    temp->op = n->op;
/* Nick Forde recommmended replacing this line
    temp->u.num = n->op == LIST_ ? (long)copy(n->u.lis) : n->u.num;
  with the following case statement: */
    switch(n->op) {
      case INTEGER_:
	    temp->u.num = n->u.num;
            break;
	case SET_:
	    temp->u.set = n->u.set;
            break;
	case STRING_:
	    temp->u.str = n->u.str;
            break;
	case FLOAT_:
	    temp->u.dbl = n->u.dbl;
            break;
	case FILE_:
	    temp->u.fil = n->u.fil;
            break;
	case LIST_:
	    temp->u.num = (long)copy(ec, n->u.lis);
            break;
	default:
	    temp->u.num = n->u.num;
    }
/* end of replacement */
    temp->next = copy(ec, n->next);
    n->op = COPIED_;
    n->u.lis = temp;
    ec->util.nodescopied++;
    if (ec->tracegc > 3) {
      printf("%5d -    ", ec->util.nodescopied);
      printnode(ec, temp);
    }
    return temp;
}
# endif

# ifndef GC_BDW

PRIVATE void cop(pEC ec, pNode *pos, const char *name) {
  if (*pos != NULL) {
    if (ec->tracegc > 2) {
      printf("old %s = ", name);
      writeterm(ec, *pos, stdout);
      printf("\n");
    }
    *pos = copy(ec, *pos);
    if (ec->tracegc > 2) {
      printf("new %s = ", name);
      writeterm(ec, *pos, stdout);
      printf("\n");
    }
  }
}

PRIVATE void gc1(pEC ec, const char *mess) {
  ec->util.start_gc_clock = clock();
  if (ec->tracegc > 1)
    printf("begin %s garbage collection\n", mess);
  ec->util.direction = - ec->util.direction;
  ec->util.memoryindex = (ec->util.direction == 1) ? ec->util.mem_low : &ec->util.memory[MEM_HIGH];
  /*
     if (ec->tracegc > 1) {
     printf("ec->util.direction = %d\n", ec->util.direction);
     printf("memoryindex = %d : %d\n",
     (long)ec->util.memoryindex, MEM2INT(ec->util.memoryindex));
     }
     */
 ec->util.nodesinspected = ec->util.nodescopied = 0;

// #define COP(X, NAME)						\
//   if (X != NULL) {						\
//     if (ec->tracegc > 2)					\
//     { printf("old %s = ", NAME);				\
//       writeterm(ec, X, stdout); printf("\n"); }			\
//     X = copy(ec, X);						\
//     if (ec->tracegc > 2)					\
//     { printf("new %s = ", NAME);				\
//       writeterm(ec, X, stdout); printf("\n"); } }

  cop(ec, &(ec->stk), "stk");
  cop(ec, &(ec->prog), "prog");
  cop(ec, &(ec->conts), "conts");
  cop(ec, &(ec->dump), "dump");
  cop(ec, &(ec->dump1), "dump1");
  cop(ec, &(ec->dump2), "dump2");
  cop(ec, &(ec->dump3), "dump3");
  cop(ec, &(ec->dump4), "dump4");
  cop(ec, &(ec->dump5), "dump5");
}

PRIVATE void gc2(pEC ec, const char *mess) {
  int this_gc_clock;
  this_gc_clock = clock() - ec->util.start_gc_clock;
  if (this_gc_clock == 0)
    this_gc_clock = 1; /* correction */
  if (ec->tracegc > 0)
    printf("gc - %d nodes inspected, %d nodes copied, clock: %d\n",
        ec->util.nodesinspected, ec->util.nodescopied, this_gc_clock);
  if (ec->tracegc > 1)
    printf("end %s garbage collection\n", mess);
  ec->gc_clock += this_gc_clock;
}
# endif

PUBLIC void gc_(pEC ec) {
# ifndef GC_BDW
  gc1(ec, "user requested");
  gc2(ec, "user requested");
# else
  GC_gcollect();
# endif
}

PUBLIC Node *newnode(pEC ec, Operator o, Types u, Node *r) {
  Node *p;
# ifndef GC_BDW
  if (ec->util.memoryindex == ec->util.mem_mid) {
    gc1(ec, "automatic");
    if (o == LIST_) u.lis = copy(ec, u.lis);
    r = copy(ec, r);
    gc2(ec, "automatic");
  }
  p = ec->util.memoryindex;
  ec->util.memoryindex += ec->util.direction;
# else
  p = GC_malloc(sizeof(Node));
  if (!p) execerror(ec, "memory", "allocator");
# endif
  p->op = o;
  p->u = u;
  p->next = r;
  D(printnode(p));
  return p;
}

PUBLIC void memoryindex_(pEC ec) {
# ifndef GC_BDW
  ec->stk = INTEGER_NEWNODE((long)MEM2INT(ec->util.memoryindex), ec->stk);
# else
  ec->stk = INTEGER_NEWNODE(0L, ec->stk);
# endif
}

PRIVATE void readmodule_field(pEC ec) {
  Entry *p;
  D(printf("Module %s at %d\n", ec->location->name, (long)ec->location));
  D(p = ec->location->u.module_fields);
  D(while (p) { printf("%s\n", p->name); p = p->next; });
  p = ec->location->u.module_fields;
  getsym(ec);
  if (ec->sym != PERIOD)
    error(ec, "period '.' expected after module name");
  else getsym(ec);
  if (ec->sym != ATOM) {
    error(ec, "atom expected as module field");
    return;
  }
  lookup(ec);
  D(printf("looking for field %s\n", ec->id));
  while (p && strcmp(ec->id, p->name) != 0)
    p = p->next;
  if (p == NULL)
    error(ec, "no such field in module");
  D(printf("found field: %s\n", p->name));
  ec->stk = USR_NEWNODE(p, ec->stk);
  return;
}

PUBLIC void readfactor(pEC ec) {	/* read a JOY factor		*/
  switch (ec->sym) {
    case ATOM:
      lookup(ec);
      D(printf("readfactor: location = %ld\n", (long)ec->location));
      /* replace the following two lines:
         if (ec->location->is_module)
         { readmodule_field(); return; }
         with the following block */
      while (ec->location->is_module) {
        Entry *mod_fields;
        mod_fields = ec->location->u.module_fields;
        getsym(ec);
        if (ec->sym != PERIOD)
          error(ec, "period '.' expected after module name");
        else
          getsym(ec);
        if (ec->sym != ATOM) {
          error(ec, "atom expected as module field");
          return;
        }
        lookup(ec);
        D(printf("looking for field %s\n", ec->id));
        while (mod_fields && strcmp(ec->id, mod_fields->name) != 0)
          mod_fields = mod_fields->next;
        if (mod_fields == NULL) {
          error(ec, "no such field in module");
          abortexecution_(ec);
        }
        D(printf("found field: %s\n", mod_fields->name));
        ec->location = mod_fields;
      }

      /* end of replacement */
      if (ec->location < ec->firstlibra) {
        ec->bucket.proc = ec->location->u.proc;
        ec->stk = newnode(ec, LOC2INT(ec->location), ec->bucket, ec->stk);
      }
      else
        ec->stk =  USR_NEWNODE(ec->location, ec->stk);
      return;
    case BOOLEAN_: case INTEGER_: case CHAR_: case STRING_:
      ec->bucket.num = ec->num;
      ec->stk = newnode(ec, ec->sym, ec->bucket, ec->stk);
      return;
    case FLOAT_:
      ec->stk = FLOAT_NEWNODE(ec->dbl, ec->stk);
      return;
    case LBRACE:
      {
        long set = 0;
        getsym(ec);
        while (ec->sym != RBRACE) {
          if (ec->sym == CHAR_ || ec->sym == INTEGER_)
            set = set | (1 << ec->num);
          else
            error(ec, "numeric expected in set");
          getsym(ec);
        }
        ec->stk = SET_NEWNODE(set, ec->stk);
      }
      return;
    case LBRACK:
      {
        void readterm(pEC);
        getsym(ec);
        readterm(ec);
        if (ec->sym != RBRACK)
          error(ec, "']' expected");
        return;
      }
    case LPAREN:
      error(ec, "'(' not implemented");
      getsym(ec);
      return;
    default:
      error(ec, "a factor cannot begin with this symbol");
      return;
  }
}

PUBLIC void readterm(pEC ec) {
  ec->stk = LIST_NEWNODE(0L, ec->stk);
  if (ec->sym <= ATOM) {
    readfactor(ec);
    ec->stk->next->u.lis = ec->stk;
    ec->stk = ec->stk->next;
    ec->stk->u.lis->next = NULL;
    ec->dump = newnode(ec, LIST_, ec->stk->u, ec->dump);
    getsym(ec);
    while (ec->sym <= ATOM)
    {
      readfactor(ec);
      ec->dump->u.lis->next = ec->stk;
      ec->stk = ec->stk->next;
      ec->dump->u.lis->next->next = NULL;
      ec->dump->u.lis = ec->dump->u.lis->next;
      getsym(ec);
    }
    ec->dump = ec->dump->next;
  }
}

PUBLIC void writefactor(pEC ec, Node *n, FILE *stm) {
  if (n == NULL)
    execerror(ec, "non-empty stack","print");
  switch (n->op) {
    case BOOLEAN_:
      fprintf(stm, "%s", n->u.num ? "true" : "false");
      return;
    case INTEGER_:
      fprintf(stm, "%ld", n->u.num);
      return;
    case FLOAT_:
      fprintf(stm, "%g", n->u.dbl);
      return;
    case SET_:
      {
        int i;
        long set = n->u.set;
        fprintf(stm, "{");
        for (i = 0; i < SETSIZE; i++)
          if (set & (1 << i)) {
            fprintf(stm, "%d", i);
            set = set & ~(1 << i);
            if (set != 0)
              fprintf(stm, " ");
          }
        fprintf(stm, "}");
        return;
      }
    case CHAR_:
      fprintf(stm, "'%c", (char) n->u.num);
      return;
    case STRING_:
      fprintf(stm, "\"%s\"", n->u.str);
      return;
    case LIST_:
      fprintf(stm, "%s","[");
      writeterm(ec, n->u.lis, stm);
      fprintf(stm, "%s","]");
      return;
    case USR_:
      fprintf(stm, "%s", n->u.ent->name ); return;
    case FILE_:
      if (n->u.fil == NULL)
        fprintf(stm, "file:NULL");
      else
        if (n->u.fil == stdin)
          fprintf(stm, "file:stdin");
        else
          if (n->u.fil == stdout)
            fprintf(stm, "file:stdout");
          else
            if (n->u.fil == stderr)
              fprintf(stm, "file:stderr");
            else
              fprintf(stm, "file:%p", (void *)n->u.fil);
      return;
    default:
      fprintf(stm, "%s", ec->symtab[(int) n->op].name);
      return;
  }
}

PUBLIC void writeterm(pEC ec, pNode n, FILE *stm) {
  while (n != NULL) {
    writefactor(ec, n, stm);
    n = n->next;
    if (n != NULL)
      fprintf(stm, " ");
  }
}
