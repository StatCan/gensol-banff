%let tmpData = C:\git\banff_redesign\Banff\BanffProcedures\Python\banff\tests\ptest\CEAG\control_data;
libname tmpLib "&tmpData"; 

proc prorate
   data=tmpLib.prorate_ceag_a_indata
   instatus=tmpLib.prorate_ceag_a_instatus
   out=WORK.outdata
   outstatus=WORK.outstatus
   outreject=WORK.rejected
   edits=
   "B17026_Y8T130 + B17026_Y9_SR31T130 + B17026_Y9_SR32T130 + B17026_Y9_SR33T130 + B17026_Y9_SR34T130 = B17026_Y91T130;
   B17026_Y101_SR85 + B17026_Y10_SR85 = B17026_Y8T130;
   B17026_Y101_SR31 + B17026_Y10_SR31 = B17026_Y9_SR31T130;
   B17026_Y101_SR32 + B17026_Y10_SR32 = B17026_Y9_SR32T130;
   B17026_Y101_SR33 + B17026_Y10_SR33 = B17026_Y9_SR33T130;
   B17026_Y101_SR34 + B17026_Y10_SR34 = B17026_Y9_SR34T130;"
   decimal=9
   lowerbound=0
   method=BASIC
   modifier=ALWAYS
   rejectnegative
   nobystats
   ;
   id I43600;
run;
     