/* create the data= data set */
data donordata;
infile cards;
input IDENT $ TOTAL Q1 Q2 Q3 Q4 STAFF PROV;
cards;
REC01 500 120 150 80 150 50 24
REC02 750 200 170 130 250 75 24
REC03 400 80 90 100 130 40 24
REC04 1000 150 250 350 250 100 24
REC05 1050 200 225 325 300 100 24
REC06 500 100 125 5000 130 45 24
REC07 400 80 90 100 15 40 30
REC08 950 150 999 999 200 90 30
REC09 1025 200 225 300 10 10 30
REC10 800 11 12 13 14 80 30
REC11 -25 -10 -5 -5 -10 3000 30
REC12 1000 150 250 350 250 100 30
REC13 999 200 225 325 300 100 30
REC14 -25 -10 -5 -10 -5 3000 30
;

/* create the exclusion variable */
data donordata;
set donordata;
if (TOTAL > 1000) then TOEXCL='E';
else TOEXCL='';
run;
 
/* create the instatus data set */
data donorstat;
infile cards;
input FIELDID $ STATUS $ IDENT $;
cards;
Q3 IPR REC01
Q4 FTE REC04
Q3 FTI REC06
Q2 FTI REC07
Q2 FTI REC08
Q3 FTI REC08
Q4 FTI REC09
STAFF FTI REC09
Q1 FTI REC10
Q2 FTI REC10
Q3 FTI REC10
Q4 FTI REC10
Q1 FTI REC11
Q2 FTI REC11
Q3 FTI REC11
Q4 FTI REC11
Q2 FTI REC13
Q3 FTI REC13
;
/* call the donorimputation procedure */
proc DONORIMPUTATION
data=donordata
instatus=donorstat
out=donorout
outstatus=outstat
donormap=donormap
edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;"
postedits="Q1 + Q2 + Q3 + Q4 - TOTAL <= 0;"
mindonors=1
pcentdonors=1
n=1
nlimit=1
mrl=0.5
random
eligdon=original
matchfieldstat
acceptnegative
;
id IDENT;
mustmatch STAFF;
dataexclvar TOEXCL;
by prov;
run;
