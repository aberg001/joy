/* FILE : scan.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include "globals.h"
#ifdef GC_BDW
#    include "gc/gc.h"
#    define malloc GC_malloc_atomic
#endif

#define EOLN '\n'

PUBLIC void inilinebuffer(p_EC ec) {
  ec->scanner.ilevel = 0;
  ec->scanner.infile[ec->scanner.ilevel] = ec->srcfile;
}

PUBLIC void putline(p_EC ec) {
  if (ec->echoflag > 2) printf("%4d", ec->scanner.linenumber);
  if (ec->echoflag > 1) printf("\t");
  printf("%s\n", ec->scanner.linbuf);
}

PRIVATE void getch(p_EC ec) {
  char c;
  if (ec->scanner.currentcolumn == ec->scanner.linelength) {
Again:
    ec->scanner.currentcolumn = 0; ec->scanner.linelength = 0;
    ec->scanner.linenumber++;
    while ((c = getc(ec->scanner.infile[ec->scanner.ilevel])) != EOLN) {
      ec->scanner.linbuf[ec->scanner.linelength++] = c;
      if (feof(ec->scanner.infile[ec->scanner.ilevel])) {
        ec->scanner.ilevel--;
        D(printf("reset to level %d\n", ec->scanner.ilevel));
        if (ec->scanner.ilevel < 0)
          quit_(ec);
      } 
    }
    ec->scanner.linbuf[ec->scanner.linelength++] = ' ';  /* to help getsym for numbers */
    ec->scanner.linbuf[ec->scanner.linelength++] = '\0';
    if (ec->echoflag) putline(ec);
    if (ec->scanner.linbuf[0] == SHELLESCAPE) {
      system(&(ec->scanner.linbuf[1]));
      goto Again;
    }
  }
  ec->ch = ec->scanner.linbuf[ec->scanner.currentcolumn++];
}

PUBLIC int endofbuffer(p_EC ec) {
  return (ec->scanner.currentcolumn == ec->scanner.linelength);
}

PUBLIC void error(p_EC ec, const char *message) {
  int i;
  putline(ec);
  if (ec->echoflag > 1) putchar('\t');
  for (i = 0; i < ec->scanner.currentcolumn-2; i++)
    if (ec->scanner.linbuf[i] <= ' ')
      putchar(ec->scanner.linbuf[i]);
    else
      putchar(' ');
  printf("^\n\t%s\n",message);
  ec->scanner.errorcount++;
}

PUBLIC int doinclude(p_EC ec, const char *filnam) {
  if (ec->scanner.ilevel+1 == INPSTACKMAX)
    execerror(ec, "fewer include files","include");
  if ((ec->scanner.infile[ec->scanner.ilevel+1] = fopen(filnam,"r")) != NULL) {
    ec->scanner.ilevel++;
    return(1);
  }
  execerror(ec, "valid file name","include");
  return 0;
}

PRIVATE char specialchar(p_EC ec) {
  getch(ec);
  switch (ec->ch) {
    case 'n' :
      return '\n';
    case 't' :
      return '\t';
    case 'b' :
      return '\b';
    case 'r' :
      return '\r';
    case 'f' :
      return '\f';
    case '\'':
      return '\'';
    case '\"':
      return '\"';
    default :
      if (ec->ch >= '0' && ec->ch <= '9') {
        int i;
        ec->num = ec->ch - '0';
        for (i = 0; i < 2; i++) {
          getch(ec);
          if (ec->ch < '0' || ec->ch > '9') {
            ec->scanner.currentcolumn++; /* to get pointer OK */
            error(ec, "digit expected");
            ec->scanner.currentcolumn--;
          }
          ec->num = 10 * ec->num + ec->ch - '0';
        }
        return ec->num;
      }
      else
        return ec->ch;
  }
}

PUBLIC void getsym(p_EC ec) {
Start:
  while (ec->ch <= ' ') getch(ec);
  switch (ec->ch) {
    case '(':
      getch(ec);
      if (ec->ch == '*') {
        getch(ec);
        do {
          while (ec->ch != '*') 
            getch(ec); 
          getch(ec);
        }
        while (ec->ch != ')');
        getch(ec);
        goto Start;
      }
      else {
        ec->sym = LPAREN;
        return;
      }
    case '#':
      ec->scanner.currentcolumn = ec->scanner.linelength;
      getch(ec);
      goto Start;
    case ')':
      ec->sym = RPAREN;
      getch(ec);
      return;
    case '[':
      ec->sym = LBRACK;
      getch(ec);
      return;
    case ']':
      ec->sym = RBRACK;
      getch(ec);
      return;
    case '{':
      ec->sym = LBRACE;
      getch(ec);
      return;
    case '}':
      ec->sym = RBRACE;
      getch(ec);
      return;
    case '.': 
      ec->sym = PERIOD;
      getch(ec);
      return;
    case ';':
      ec->sym = SEMICOL;
      getch(ec);
      return;
    case '\'':
      getch(ec);
      if (ec->ch == '\\') 
        ec->ch = specialchar(ec);
      ec->num = ec->ch;
      ec->sym = CHAR_;
      getch(ec);
      return;
    case '"':
      {
        char string[INPLINEMAX];
        /*register*/ int i = 0;
        getch(ec);
        while (ec->ch != '"' && !endofbuffer(ec)) {
          if (ec->ch == '\\') 
            ec->ch = specialchar(ec);
          string[i++] = ec->ch;
          getch(ec);
        }
        string[i] = '\0';
        getch(ec);
        D(printf("getsym: string = %s\n",string));
        ec->num = (long) malloc(strlen(string) + 1);
        strcpy((char *) ec->num, string);
        ec->sym = STRING_;
        return;
      }
    case '-': /* PERHAPS unary minus */
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      {
        char number[25];
        char *p = number;
        if ( isdigit(ec->ch) ||
            ( ec->scanner.currentcolumn < ec->scanner.linelength &&
              isdigit((int)ec->scanner.linbuf[ec->scanner.currentcolumn]) ) ) {
          do {
            *p++ = ec->ch;
            getch(ec);
          }
          while (strchr("0123456789+-Ee.", ec->ch))
            ;
          *p = 0;
          if (strpbrk(number, ".eE")) {
            ec->dbl = strtod(number, NULL);
            ec->sym = FLOAT_;
            return;
          }
          else {
            ec->num = strtol(number, NULL, 0);
            ec->sym = INTEGER_;
            return;
          }
        }
      }
      /* ELSE '-' is not unary minus, fall through */
    default:
      {
        int i = 0;
        ec->hashvalue = 0; /* ensure same algorithm in inisymtab */
        do {
          if (i < ALEN-1) {
            ec->id[i++] = ec->ch;
            ec->hashvalue += ec->ch;
          }
          getch(ec);
        }
        while (isalpha(ec->ch) || isdigit(ec->ch) ||
            ec->ch == '=' || ec->ch == '_' || ec->ch == '-')
          ;
        ec->id[i] = '\0';
        ec->hashvalue %= HASHSIZE;
        if (isupper((int)ec->id[1])) {
          if (strcmp(ec->id,"LIBRA") == 0 || strcmp(ec->id,"DEFINE") == 0) {
            ec->sym = LIBRA;
            return;
          }
          if (strcmp(ec->id,"HIDE") == 0) {
            ec->sym = HIDE;
            return;
          }
          if (strcmp(ec->id,"IN") == 0) {
            ec->sym = IN;
            return;
          }
          if (strcmp(ec->id,"END") == 0) {
            ec->sym = END;
            return;
          }
          if (strcmp(ec->id,"MODULE") == 0) {
            ec->sym = MODULE;
            return;
          }
          if (strcmp(ec->id,"PRIVATE") == 0) {
            ec->sym = JPRIVATE;
            return;
          }
          if (strcmp(ec->id,"PUBLIC") == 0) {
            ec->sym = JPUBLIC;
            return;
          }
          /* possibly other uppers here */
        }
        if (strcmp(ec->id,"!") == 0) { /* should this remain or be deleted ? */
          ec->sym = PERIOD;
          return;
        }
        if (strcmp(ec->id,"==") == 0) {
          ec->sym = EQDEF;
          return;
        }
        if (strcmp(ec->id,"true") == 0) {
          ec->sym = BOOLEAN_;
          ec->num = 1;
          return;
        }
        if (strcmp(ec->id,"false") == 0) {
          ec->sym = BOOLEAN_;
          ec->num = 0;
          return;
        }
        if (strcmp(ec->id,"maxint") == 0) {
          ec->sym = INTEGER_;
          ec->num = MAXINT;
          return;
        }
        ec->sym = ATOM;
        return;
      }
  }
}
