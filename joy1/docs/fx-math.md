
# Math operators

### +      :  M I  ->  N
Numeric N is the result of adding integer I to numeric M.
Also supports float.

### -      :  M I  ->  N
Numeric N is the result of subtracting integer I from numeric M.
Also supports float.

### *      :  I J  ->  K
Integer K is the product of integers I and J.  Also supports float.

### /      :  I J  ->  K
Integer K is the (rounded) ratio of integers I and J.  Also supports float.

### rem      :  I J  ->  K
Integer K is the remainder of dividing I by J.  Also supports float.

### div      :  I J  ->  K L
Integers K and L are the quotient and remainder of dividing I by J.

### sign      :  N1  ->  N2
Integer N2 is the sign (-1 or 0 or +1) of integer N1,
or float N2 is the sign (-1.0 or 0.0 or 1.0) of float N1.

### neg      :  I  ->  J
Integer J is the negative of integer I.  Also supports float.

### ord      :  C  ->  I
Integer I is the Ascii value of character C (or logical or integer).

### chr      :  I  ->  C
C is the character whose Ascii value is integer I (or logical or character).

### abs      :  N1  ->  N2
Integer N2 is the absolute value (0,1,2..) of integer N1,
or float N2 is the absolute value (0.0 ..) of float N1

### acos      :  F  ->  G
G is the arc cosine of F.

### asin      :  F  ->  G
G is the arc sine of F.

### atan      :  F  ->  G
G is the arc tangent of F.

### atan2      :  F G  ->  H
H is the arc tangent of F / G.

### ceil      :  F  ->  G
G is the float ceiling of F.

### cos      :  F  ->  G
G is the cosine of F.

### cosh      :  F  ->  G
G is the hyperbolic cosine of F.

### exp      :  F  ->  G
G is e (2.718281828...) raised to the Fth power.

### floor      :  F  ->  G
G is the floor of F.

### frexp      :  F  ->  G I
G is the mantissa and I is the exponent of F.
Unless F = 0, 0.5 <= abs(G) < 1.0.

### ldexp      :  F I  -> G
G is F times 2 to the Ith power.

### log      :  F  ->  G
G is the natural logarithm of F.

### log10      :  F  ->  G
G is the common logarithm of F.

### modf      :  F  ->  G H
G is the fractional part and H is the integer part
(but expressed as a float) of F.

### pow      :  F G  ->  H
H is F raised to the Gth power.

### sin      :  F  ->  G
G is the sine of F.

### sinh      :  F  ->  G
G is the hyperbolic sine of F.

### sqrt      :  F  ->  G
G is the square root of F.

### tan      :  F  ->  G
G is the tangent of F.

### tanh      :  F  ->  G
G is the hyperbolic tangent of F.


