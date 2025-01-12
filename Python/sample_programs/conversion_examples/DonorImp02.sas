/*******************************************************/
/* PROC DONORIMPUTATION example #2                     */
/* Changing the RANDKEY from 0.2 to 0.8 for recipient  */
/* Record B will pick a different donor between A and  */
/* C, which are both equidistant donors.               */
/*******************************************************/


DATA data;
   INPUT REC $ X Y randkey;
   CARDS;
A 1 2 0.1
B 2 3 0.8
C 3 4 0.3
; 
RUN;

DATA instatus;
   INPUT REC $ FIELDID $ STATUS $;
   cards;
B X FTI
; 
RUN;

PROC DONORIMPUTATION
   DATA = data
   INSTATUS = instatus
   OUT = outdata2
   OUTSTATUS = outstatus2
   DONORMAP = outmap2
   EDITS = "x=y;"
   POSTEDITS = "x<=y;"
   MINDONORS = 1
   PCENTDONORS = 1
   N = 1
   displaylevel=1
   NLIMIT = 1
   MATCHFIELDSTAT
   ;
   ID REC;
   randnumvar randkey;
RUN;
