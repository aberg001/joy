
operator

id      :  ->
Identity function, does nothing.
Any program of the form  P id Q  is equivalent to just  P Q.

dup      :   X  ->   X X
Pushes an extra copy of X onto stack.

swap      :   X Y  ->   Y X
Interchanges X and Y on top of the stack.

rollup      :  X Y Z  ->  Z X Y
Moves X and Y up, moves Z down

rolldown      :  X Y Z  ->  Y Z X
Moves Y and Z down, moves X up

rotate      :  X Y Z  ->  Z Y X
Interchanges X and Z

popd      :  Y Z  ->  Z
As if defined by:   popd  ==  [pop] dip 

dupd      :  Y Z  ->  Y Y Z
As if defined by:   dupd  ==  [dup] dip

swapd      :  X Y Z  ->  Y X Z
As if defined by:   swapd  ==  [swap] dip

rollupd      :  X Y Z W  ->  Z X Y W
As if defined by:   rollupd  ==  [rollup] dip

rolldownd      :  X Y Z W  ->  Y Z X W
As if defined by:   rolldownd  ==  [rolldown] dip 

rotated      :  X Y Z W  ->  Z Y X W
As if defined by:   rotated  ==  [rotate] dip

pop      :   X  ->
Removes X from top of the stack.

choice      :  B T F  ->  X
If B is true, then X = T else X = F.

