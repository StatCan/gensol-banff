libname tutorial "C:\tfs\generalizedSystems\Banff\Test\Outlier\OutBounds\data";  


/***********************************
 * Lesson 3 - PROC Outlier         *
 ***********************************/
 
/* Run query to select only data with variable STRAT>1. Proc Outlier will not be run
   for units in STRAT=1 according to the user's survey specifications. */ 
proc sql;
   create table work.outlier_indata as 
   select IDENT, 
          AREA, 
          STRAT, 
          HEN_LT20, 
          HEN_GE20, 
          HEN_OTH, 
          HEN_TOT, 
          EGG_LAID, 
          EGG_SOLD, 
          EGG_VALU, 
          QR_REV, 
          QR_EXP, 
          QR_PROF 
  from tutorial.current
    where STRAT > 1;
quit; 
data work.indata;
set work.outlier_indata;
run;
data work.hist;
set tutorial.historical;
if IDENT='R005' then HEN_TOT=-HEN_TOT;
if IDENT='R007' then DELETE;
if IDENT='R042' then HEN_TOT=.;

run;

/* First execution of Proc Outlier for variables with only positive values. */ 
proc outlier  
  data = work.indata
  hist = work.hist
  outstatus = work.outlier_outstatus1 
  outsummary=outsum1
  method = historic 
  mei = 4 
  mii = 7 
  mdm = 0.05 
  exponent = 0
  minobs = 10
  outlierstat
  ; 
  id IDENT; 
  var HEN_TOT HEN_LT20; 
  by AREA; 
run; 
proc print data=outsum1;
title "outsum1";
run;
