options linesize=80 pagesize=32000 nodate;
%let proratingedits="
Q1+4Q2+2Q3+Q4=YEAR;
";
data proratedata;
infile datalines delimiter=',';
input ident $ Q1 Q2 Q3 Q4 YEAR;
datalines;
R01,-25,42,27,25,40
R02,-25,44,20,20,40
R03,-18,44,15,5,40
R04,-18,42,15,5,40
R05,12,30,15,5,40
R06,12,20,10,5,40
run;
proc prorate
data=proratedata
out=pro_outdata
outstatus=pro_outstatus
outreject=pro_rejected
edits=&proratingedits
method = BASIC
decimal = 1
lowerbound = -100.0 /* set to 0.0 for method = SCALING */
upperbound = 100.0
modifier = always
acceptnegative
;
id ident;
run;
