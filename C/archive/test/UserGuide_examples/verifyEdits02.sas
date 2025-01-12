/* With rules specified, value of variables NETINC1, NETINC2
and NETINCYEAR could be negative. */
proc VERIFYEDITS 
edits = "
NETINC1 + NETINC2 = NETINCYEAR;
INC1 - EXP1 = NETINC1;
INC2 - EXP2 = NETINC2;
INC1 >= 0; INC2 >= 0;
EXP1 >= 0; EXP2 >= 0;"
imply = 50
acceptnegative
;
run;
