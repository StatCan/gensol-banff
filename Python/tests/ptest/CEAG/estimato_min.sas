%let tmpData = C:\git\banff_redesign\Banff\BanffProcedures\Python\banff\tests\ptest\CEAG\control_data;
libname tmpLib "&tmpData"; 

proc estimator
   seed=1
   data=tmpLib.estimato_ceag_a_indata
   hist=tmpLib.estimato_ceag_a_indata_hist
   datastatus=tmpLib.estimato_ceag_a_instatus
   estimator=tmpLib.estimato_ceag_a_inestimator
   out=WORK.outdata
   outstatus=WORK.outstatus
   outestparms=WORK.estparms
   acceptnegative
   nobystats
   ;
   id I43600;
   by 
   P41_10_3_CURR
   P51_24_3_CURR
   ;
run;
