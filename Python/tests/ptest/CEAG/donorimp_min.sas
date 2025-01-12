%let tmpData = C:\git\banff_redesign\Banff\BanffProcedures\Python\banff\tests\ptest\CEAG\control_data;
libname tmpLib "&tmpData"; 

proc donorimputation
   seed=1
   data=tmpLib.donorimp_ceag_a_indata
   instatus=tmpLib.donorimp_ceag_a_instatus
      (where=(editGroupId="EGR3668" 
         or (status IN("FTE","FTI") and OutlierStatus NE "")))
   out=WORK.outdata
   outstatus=WORK.outstatus
   donormap=WORK.donormap
   edits=
   "B17026_Y10_SR33 >= 0;
    B17026_Y10_SR34 >= 0;
    B17026_Y10_SR32 >= 0;
    B17026_Y10_SR31 >= 0;
    B17026_Y10_SR85 >= 0;
    B17026_Y101_SR34 >= 0;
    B17026_Y101_SR33 >= 0;
    B17026_Y101_SR32 >= 0;
    B17026_Y101_SR31 >= 0;
    B17026_Y101_SR85 >= 0;
    B17026_Y91T130 >= 0;
    B17026_Y9_SR34T130 >= 0;
    B17026_Y9_SR33T130 >= 0;
    B17026_Y9_SR32T130 >= 0;
    B17026_Y9_SR31T130 >= 0;
    B17026_Y8T130 >= 0;
    B17026_Y101T130 >= 0;
    B17026_Y10T130 >= 0;"
   postedits=
   "B17026_Y10_SR33 >= 0;
    B17026_Y10_SR34 >= 0;
    B17026_Y10_SR32 >= 0;
    B17026_Y10_SR31 >= 0;
    B17026_Y10_SR85 >= 0;
    B17026_Y101_SR34 >= 0;
    B17026_Y101_SR33 >= 0;
    B17026_Y101_SR32 >= 0;
    B17026_Y101_SR31 >= 0;
    B17026_Y101_SR85 >= 0;
    B17026_Y91T130 >= 0;
    B17026_Y9_SR34T130 >= 0;
    B17026_Y9_SR33T130 >= 0;
    B17026_Y9_SR32T130 >= 0;
    B17026_Y9_SR31T130 >= 0;
    B17026_Y8T130 >= 0;
    B17026_Y10T130 >= 0;
    B17026_Y101T130 >= 0.1;
    B17026_Y91T130 >= 0.1;
    B17026_Y8T130 + B17026_Y9_SR31T130 + B17026_Y9_SR32T130 + B17026_Y9_SR33T130 + B17026_Y9_SR34T130 >= 0.1;"
   mindonors=3
   pcentdonors=1
   n=7000
   eligdon=ANY
   rejectnegative
   matchfieldstat
   nobystats
   ;
   id I43600;
   mustmatch
   B17026_y101T130
   ;
   randnumvar RandomNbr_rand;
   dataexclvar _EXCLUDE;
   by 
   P54_08_3
   P51_21_3
   ;
run;