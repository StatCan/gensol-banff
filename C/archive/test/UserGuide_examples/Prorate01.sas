options linesize=80 pagesize=32000;
%let proratingedits="
Q1:A+Q2:I+Q3:n+Q4:o=YEAR;
M1+M2+M3=Q1;
";
data proratedata(drop=n);
n = 1;
do while (n <= 10);
IDENT = 'R' || put (n, Z2.);
M1 = round ((10 + rannor (1)) / 3);
M2 = round ((10 + rannor (1)) / 3);
M3 = round ((10 + rannor (1)) / 3);
Q1 = round (10 + rannor (1) / 4);
Q2 = round (10 + rannor (1) / 4);
Q3 = round (10 + rannor (1) / 4);
Q4 = round (10 + rannor (1) / 4);
YEAR = 40;
output;
n = n + 1;
end;
run;
proc prorate
data=proratedata
edits=&proratingedits
verifyedits
;
id ident;
run;
