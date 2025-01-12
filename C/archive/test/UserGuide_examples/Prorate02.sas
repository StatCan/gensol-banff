options linesize=80 pagesize=32000 nodate;
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
if mod(n,3)=0 then ZONE=1; else ZONE=2;
if mod(n,10)=0 then Q1=-Q1; /* negative data */
output;
n = n + 1;
end;
run;
data proratestatus;
infile datalines delimiter=',';
input ident $ Fieldid $ Status $;
datalines;
R01,M1,IDN
R01,M3,IDN
R02,M1,IPV
R05,M1,IDN
R05,Q3,IDN
R07,M3,IPV
R08,M1,IPV
R09,M2,IDN
R10,M1,IPV
R10,Q1,IPV
;
run;
proc sort data=proratedata; by zone ident; run;
proc prorate
data=proratedata
instatus=proratestatus
out=pro_outdata
outstatus=pro_outstatus
outreject=pro_rejected
edits=&proratingedits
method = scaling
decimal = 1
lowerbound = 0.1
upperbound = 1.1
modifier = imputed
rejectnegative
;
id ident;
by zone;
run;
