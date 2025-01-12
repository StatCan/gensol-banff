options linesize=80 pagesize=32000; 
data outlierdata (drop=n);
  n = 1;
  do while (n <= 30);
    IDENT = 'R' || put (n, Z7.);
    X01 = round (ranuni (1) * 20);
    X02 = round (ranuni (1) * 20);
    if mod(n,2)=0 then Prov=10;
      else Prov=11;
  if mod(n,5)=0 then do; 
    X01=-X01; 
    X02=-X02; 
    end;
  output;
  n = n + 1;
  end;
run;

/* sort the data in order of the BY variable */
proc sort data=outlierdata; by prov; run;
proc outlier
data=outlierdata
outstatus=outlierstatus2
method=sg
betae=1.00
betai=1.75
sigma=MAD
startcentile=60
outlierstat
acceptnegative
;
id ident;
var X01 X02;
by prov;
run;

proc print data=outlierstatus2;
title "Example 2 - Outliers detected by the SG method using 1 data set";
run;
