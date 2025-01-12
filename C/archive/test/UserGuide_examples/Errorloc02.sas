DATA data;
   INPUT REC $ X Y randkey;
   CARDS;
A 3 4 0.1
B 2 3 0.6
C 4 1 0.3
D 5 6 0.4
;
RUN;

PROC ERRORLOC
   DATA=data
   OUTSTATUS=outstatus1
   OUTREJECT=outreject1 
   EDITS= "X+Y>=6; X<=4; 
           Y<=5;"
   ;
   ID REC;
   randnumvar randkey;
RUN;
