

/* Create a dataset with var1 + var2 + var3 = total */
/* Add a few missing values, and a few errors		*/
%let seed=3212024;

data input;
call streaminit(&seed);  
do i=1 to 10;
	RecordID = cats("R",put(i,z4.));
	var1= round(100*rand('uniform'));
	var2= round(100*rand('uniform'));
	var3= round(100*rand('uniform'));
	total=sum(of var1-var3);
	if i=1 then total=.;				/* Total is missing so will be flagged for FTI */
	if i=2 then total=total+1;			/* Inconsistency so either Var1 or Var 2 will be flagged for FTI */
	if i=3 then total=total+1;			/* Inconsistency so either Var1 or Var 2 will be flagged for FTI */
	if i=4 then var1=-1;				/* Negative so Var1 will be flagged for FTI */
	output;
	end;
run;

/* Run ErrorLoc without an InStatus												*/
/* Give Var3 weight=3 so it is less likely to be selected then Var1 and Var2 	*/
/* Give Total weight=10 so it is never going to be chosen over Var1, Var2, Var3 */

proc errorloc
	data=input
	edits="Var1+Var2+Var3=Total;"
	weights="Var3=3; Total=10;"
	outstatus=outstatus1
	outreject=outreject1
	rejectnegative
	seed=&seed
	;
	id RecordID;
	run;

proc print data=outstatus1;
	run;
/* Expected results */


/* Create an InStatus file with a few FTI flags */
data instatus;
	length FieldID $32.;
	set input;
	Status="FTI";
	if i=2 then do;
		FieldID="Var3";			/* This should cause Var3 to be flagged in ErrorLoc instead of Var1 or Var2 */
		output;
		end;
	if i=3 then do;
		FieldID="Total";		/* This should cause Total to be flagged in ErrorLoc instead of Var1 or Var2 */
		output;
		end;
	keep RecordID FieldID Status;
run;	

/* As programmed, nothing will change in ErrorLoc */
/* We can generate expected results by removing values that are flagged as FTI */

data input2;
	set input;
	if i=2 then Var3=.;
	if i=3 then Total=.;
	run;
proc print data=input2; run;



proc errorloc
	data=input2
	edits="Var1+Var2+Var3=Total;"
	weights="Var3=3; Total=10;"
	outstatus=outstatus2
	outreject=outreject2
	rejectnegative
	seed=&seed
	;
	id RecordID;
	run;


title "Current output";
proc print data=outstatus1;
	run;
title "Desired output when InStatus specified";
proc print data=outstatus2;
	run;

/* As desired, Var3 is flagged as FTI instead of Var1 or Var2, for Record 2 */
/* Total is flagged as FTI instead of Var1 or Var2, for record 3 */
/* Other records are unaffected	*/
