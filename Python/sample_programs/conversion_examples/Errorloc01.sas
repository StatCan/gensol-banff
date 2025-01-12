options ls=80 ps=80 nodate;
data example;
input IDENT $ X1 X2 ZONE $1.;
cards;
R03 10 40 B
R02 -4 49 A
R04 4 49 A
R01 16 49 A
R05 15 51 B
R07 -4 29 B
R06 30 70 B
;
run;

/* sort the data in order of the BY variable */

proc sort data=example; by ZONE IDENT;run;

 

proc errorloc
data=example
outstatus=outstatus
outreject=outreject
edits="x1>=-5; x1<=15; x2>=30; x1+x2<=50;"
weights="x1=1.5;"
cardinality=2
timeperobs=.1
;
id IDENT;
by ZONE;
run;
