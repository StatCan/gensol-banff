data massimpdata;
infile cards;
input IDENT $ TOTAL Q1 Q2 Q3 Q4 STAFF;
cards;
REC01 500 100 125 125 150 1000
REC02 750 200 170 130 250 2000
REC03 400 80 90 100 130 1000
REC04 1000 150 250 350 250 2000
REC05 3000 500 500 1000 1000 1000
REC06 800 200 225 200 175 2000
REC07 600 125 150 175 150 1000
REC08 900 175 200 225 300 2000
REC09 2500 500 650 600 750 1000
REC10 800 150 175 225 250 2000
REC21 3000 -45 -50 75 -234 2000
REC11 575 . . . . 1000
REC12 850 . . . . 2000
REC13 375 . . . . 1000
REC14 1100 . . . . 2000
REC15 3100 . . . . 1000
REC16 750 . . . . 2000
REC17 675 . . . . 1000
REC18 875 . . . . 2000
REC19 4000 . . . . 1000
REC20 . . . . . 2000
;
/* sort the input data set by the STAFF and IDENT variables.*/

proc sort data=massimpdata;
by STAFF IDENT;
run;

proc massimputation
data=massimpdata
out=outdata
donormap=donormap
mindonors=1
pcentdonors=1
random
acceptnegative
;
id IDENT;
mustimpute Q1 Q2 Q3 Q4;
mustmatch TOTAL;
by STAFF;
run;
