
# Literal

## truth value type      :  ->  B
The logical type, or the type of truth values.
It has just two literals: `true` and `false`.

## character type      :  ->  C
The type of characters. Literals are written with a single quote.
Examples:  `'A`  `'7`  `';`  and so on. Unix style escapes are allowed.

## integer type      :  ->  I
The type of negative, zero or positive integers.
Literals are written in decimal notation. Examples:  `-123`   `0`   `42`.

## set type      :  ->  {...}
The type of sets of small non-negative integers.
The maximum is platform dependent, typically the range is 0..31.
Literals are written inside curly braces.
Examples:  `{}`  `{0}`  `{1 3 5}`  `{19 18 17}`.

Note: Set types will probably be removed in a future version, replaced with
a library implementation..

## string type      :  ->  "..."
The type of strings of characters. Literals are written inside double
quotes.  Examples: `""`  `"A"`  `"hello world"` `"123"`.  Unix style
escapes are accepted.

## list type      :  ->  [...]
The type of lists of values of any type (including lists),
or the type of quoted programs which may contain operators or combinators.
Literals of this type are written inside square brackets.
Examples: `[]`  `[3 512 -7]`  `[john mary]`  `['A 'C ['B]]`  `[dup *]`.

## float type      :  ->  F
The type of floating-point numbers.
Literals of this type are written with embedded decimal points (like
`1.2`) and optional exponent specifiers (like `1.5E2`)

## file type      :  ->  FILE:
The type of references to open I/O streams,
typically but not necessarily files.
The only literals of this type are `stdin`, `stdout`, and `stderr`.

