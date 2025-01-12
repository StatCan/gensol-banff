options linesize=80 pagesize=32000; 
data outlierdata (drop=n);
  n = 1;
  do while (n <= 30);
    IDENT = 'R' || put (n, Z7.);
    X01 = round (ranuni (1) * 20);
    X02 = round (ranuni (1) * 20);
    if mod(n,2)=0 then Prov=10; else Prov=11;
    if mod(n,5)=0 then do; X01=-X01; X02=-X02; end;
    output;
    n = n + 1;
  end;
run;
/* sort the data in order of the BY variable */
proc sort; by prov; run; 
data outlierhist (drop=n);
n = 1;
do while (n <= 30);
IDENT = 'R' || put (n, Z7.);
X01 = round (ranuni (1) * 30);
X02 = round (ranuni (1) * 30);
if mod(n,2)=0 then Prov=10; else Prov=11;
if mod(n,7)=0 then do; X01=-X01; X02=-X02; end;
output;
n = n + 1;
end;
run;
/* sort the data in order of the BY variable */
proc sort; by prov; run;
proc outlier
data=outlierdata
hist=outlierhist
outstatus=outlierstatus3
method=historic
mii=1.5
mei=1.3
mdm=.05
outlierstat
;
id ident;
var X01 X02;
with X01 X02; /* the resulting 2 ratios will be:  X01_data/X01_hist and X02_data/X02_hist */
by prov;
run;
proc print data=outlierstatus3;
title "Example 3 - Outlier ratios detected by the HB method with a historical trend using 2 data sets";run;