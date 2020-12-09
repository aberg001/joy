# Input Output
## newline
Prints a newline to stdout.
## putln
Same as [[fx-misc#put]] with a newline at the end.
## space
Prints a space character.
## bell
Prints a bell (^G).  On many systems, causes an audible beep.
## putstrings
Prints each of a list of strings, with no separators.
## ask
Prompts the user for a value, which is left on the stack.
- **BUGBUG**: This is currently not working.  
```
JOY  -  compiled at 10:02:45 on Dec  1 2020 (NOBDW)
Copyright 2001 by Manfred von Thun
usrlib  is loaded
inilib  is loaded
agglib  is loaded
"help me" ask .
Please help me
10
10
10
 ^
	 END or period '.' expected
"help me" ask .
Please help me
10 .
10
10 .
 ^
	 END or period '.' expected
"help me" ask.
Please help me
10.
10
10.
  ^
	 END or period '.' expected

```
# Operators
## dup2
Duplicate the top two items on the stack.
## pop2
Pop off the top two items from the stack.
## newstack
Discard the entire contents of the stack.
## truth
Synonym for [[fx-operand#true]].
## falsity
Synonym for [[fx-operand#false]].
## to-upper
Convert a single character to uppercase.  Currently only works for 7-bit ASCII codes.
## to-lower
Convert a single character to lowercase.  Currently only works for 7-bit ASCII codes.
## boolean
## numerical
## swoncat
"Swap and concat".  Concatenate two sequences, 
# Date and Time
## weekdays
Returns a list of weekday names.  
- **BUGBUG** this should be localized for more than just US-en.
## months
Returns a list of month names.  This returns short names, "JAN", "FEB" &c.
- **BUGBUG** This should be localized for more than just US-en.
## localtime-strings
## today
## now
## popd
## show-todaynow
# Program Operators
## conjoin
## disjoin
## negate
# Combinators
## sequor : `[A] [B]` -> `X`
Sequential OR.  This is the normal "logical OR with shortcut evaluation".  `A` and `B` are programs that give true/false output. `X` is true if the result of running either `A` or `B` is true.  If the result of `A` is true, `B` is not run.
## sequand : `[A] [B]` -> `X`
Sequential AND.  This is the normal "logical AND with shortcut evaluation".  `A` and `B` are programs that give true/false output. `X` is false if the result of running either `A` or `B` is false.  If the result of `A` is false, `B` is not run.
## dipd :  `X Y [P]`  ->  `... X Y`
Like [[fx-combinator#dip]] but it hides two values off the top of the stack.
## dip2 :  `X Y [P]`  ->  `... X Y`
Synanym for [[#dipd]].
## dip3 :  `X Y Z [P]`  ->  `... X Y Z`
Like [[fx-combinator#dip]] but it hides three values off the top of the stack.
## call : `S` -> `...`
Execute the program referenced by symbol S.
## i2
## nullary2
## repeat
## forever
# Library inclusion
## verbose
## libload
## basic-libload
Loads all of the basic libraries:
- [[agglib]]
- [[seqlib]]
- [[numlib]]
## special-libload
- [[mtrlib]]
- [[tutlib]]
- [[lazlib]]
- [[lsplib]]
- [[symlib]] 
## all-libload
Loads all of the system libraries.  