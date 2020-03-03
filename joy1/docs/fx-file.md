
# File operations

## fclose      :  S  ->  
Stream S is closed and removed from the stack.

## feof      :  S  ->  S B
B is the end-of-file status of stream S.

## ferror      :  S  ->  S B
B is the error status of stream S.

## fflush      :  S  ->  S
Flush stream S, forcing all buffered output to be written.

## fgetch      :  S  ->  S C
C is the next available character from stream S.

## fgets      :  S  ->  S L
L is the next available line (as a string) from stream S.

## fopen      :  P M  ->  S
The file system object with pathname P is opened with mode M (r, w, a, etc.)
and stream object S is pushed; if the open fails, file:NULL is pushed.

## fread      :  S I  ->  S L
I bytes are read from the current position of stream S
and returned as a list of I integers.

## fwrite      :  S L  ->  S
A list of integers are written as bytes to the current position of stream S.

## fremove      :  P  ->  B
The file system object with pathname P is removed from the file system.
 is a boolean indicating success or failure.

## frename      :  P1 P2  ->  B
The file system object with pathname P1 is renamed to P2.
B is a boolean indicating success or failure.

## fput      :  S X  ->  S
Writes X to stream S, pops X off stack.

## fputch      :  S C  ->  S
The character C is written to the current position of stream S.

## fputchars      :  S "abc.."  ->  S
The string abc.. (no quotes) is written to the current position of stream S.

## fputstring      :  S "abc.."  ->  S
Same as fputchars, as a temporary alternative.

## fseek      :  S P W  ->  S
Stream S is repositioned to position P relative to whence-point W,
where W = 0, 1, 2 for beginning, current position, end respectively.

## ftell      :  S  ->  S I
I is the current position of stream S.
