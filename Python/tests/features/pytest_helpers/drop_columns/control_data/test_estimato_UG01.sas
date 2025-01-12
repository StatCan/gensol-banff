/* SAS_EXPORT: START
   Use this code to export consistently named and formatted datasets for later use by Python.  
   Insert this entire file at the top of any SAS program and update the variables as needed.  

   To export datasets from a test case, insert a call to `%export_ds_<proc-name>(...)` 
   immediately following the procedure call.  Only specify the dataset references
   you are interested in.  Datasets will be exported to the `&out_dir_path` folder and follow
   the implemented naming convention.  
   See the bottom of this template file for an example call.  
*/

/* !!!DEVELOPER MUST DEFINE the following variables !!!
   they are used to define the save location and dataset names
   datasets are saved according to the name genreated by `%generate_testcase_name()` */
options dlcreatedir;
%let out_dir_path=C:\git\banff_redesign\Banff\BanffProcedures\Python\banff\tests\misc_tests\pytest_helpers\drop_columns\control_data\;
%let test_proc=donorimp;
%let test_case=ug01;
%let subtest=a; /* when multiple proc calls, update this prior to export so datasets don't overwrite */

/* setup: build testcase name, open output folder */
libname out_dir "&out_dir_path";

/****** define helper macros ******/
/* uses `&test_proc.`, `&test_case.`, and `&subtest.` to 
   generate a test case name, `&testcase_name.`*/
%macro generate_testcase_name();
   %global testcase_name;
   %let testcase_name=&test_proc._&test_case._&subtest;
%mend;

/* saves dataset `ds_ref` to libref `out_dir` */
%macro export_dataset(ds_ref, ds_name);
    %if ("&ds_ref." ne ".") %then %do;
    %generate_testcase_name() 
        /* copy dataset to control data folder */
        data out_dir.&testcase_name._&ds_name.;
            set &ds_ref.;
        run;
    %end;
%mend;

/* convenience function maps procedure's SAS dataset names to its Python dataset names */
%macro export_ds_donorimp(data=., instatus=., out=., outstatus=., donormap=.);
    %export_dataset(&data, indata)
    %export_dataset(&instatus, instatus)
    %export_dataset(&out, outdata)
    %export_dataset(&outstatus, outstatus)
    %export_dataset(&donormap, donormap)
%mend;

/* convenience function maps procedure's SAS dataset names to its Python dataset names */
%macro export_ds_outlier(data=., hist=., outstatus=., outsummary=.);
    %export_dataset(&data, indata)
    %export_dataset(&hist, indata_hist)
    %export_dataset(&outstatus, outstatus)
    %export_dataset(&outsummary, outsummary)
%mend;

/* convenience function maps procedure's SAS dataset names to its Python dataset names */
%macro export_ds_estimato(ALGORITHM=., DATA=., HIST=., ESTIMATOR=., DATASTATUS=., HISTSTATUS=., OUTACCEPTABLE=., OUT=., OUTESTEF=., OUTESTLR=., OUTESTPARMS=., OUTRANDOMERROR=., OUTSTATUS=.);
    %export_dataset(&ALGORITHM, inalgorithm)
    %export_dataset(&DATA, indata)
    %export_dataset(&HIST, indata_hist)
    %export_dataset(&ESTIMATOR, inestimator)
    %export_dataset(&DATASTATUS, instatus)
    %export_dataset(&HISTSTATUS, instatus_hist)
    %export_dataset(&OUTACCEPTABLE, outacceptable)
    %export_dataset(&OUT, outdata)
    %export_dataset(&OUTESTEF, outest_ef)
    %export_dataset(&OUTESTLR, outest_lr)
    %export_dataset(&OUTESTPARMS, outest_parm)
    %export_dataset(&OUTRANDOMERROR, outrand_err)
    %export_dataset(&OUTSTATUS, outstatus)

%mend;

/* example calls

%export_dataset(work.data, indata)

or 

%export_ds_donorimp(
   data=indata
   ,instatus=instat
   ,out=outdata
   ,outstatus=outstat
   ,donormap=map
)

%export_ds_outlier(
   data=indata
   ,hist=inhist
   ,outstatus=outstat
   ,outsummary=outsum
)

 SAS_EXPORT: END */
 
data currentdata;
infile cards;
input ident $ x prov;
cards;
REC01 -500 24
REC02 750 24
REC03 -400 24
REC04 1000 24
REC05 1050 24
REC06 500 24
REC07 400 30
REC08 950 30
REC09 1025 30
REC10 -800 30
REC12 10000 30
REC13 500 24
REC14 750 24
REC15 400 24
REC16 -1000 24
REC17 1050 24
REC18 500 24
REC19 -400 30
REC20 950 30
REC21 -1025 30
REC22 800 30
REC23 10000 30
;
run;

/* Create the historical data set*/
data histdata;
infile cards;
input IDENT $ X prov;
cards;
REC01 500 24
REC02 -750 24
REC03 400 24
REC05 870 24
REC06 500 24
REC07 400 30
REC08 950 30
REC09 950 30
REC10 -800 30
REC11 800 30
REC12 500 24
REC13 750 24
REC14 -400 24
REC15 870 24
REC16 500 24
REC17 -400 30
REC18 950 30
REC19 950 30
REC20 800 30
REC21 800 30
;
run;

/* Create the status file for the current data set*/
/* This data set can also be created by running */
/* PROC ERRORLOC on the current data set with the */
/* edit x>0 */
data currstatus;
input ident $ fieldid $ status $;
cards;
REC01 x FTI
REC03 x FTI
REC10 x FTI
REC16 x FTI
REC19 x FTI
REC21 x FTI
;
run;

/* Create the status file for the historical data set*/
/* This data set can also be created by running */
/* PROC ERRORLOC on the historical data set with the */
/* edit x>0 */
data histstatus;
input ident $ fieldid $ status $;
cards;
REC02 X FTI
REC10 X FTI
REC14 X FTI
REC17 X FTI
;
run;

/* Create an exclusion variable called CURRDATAEXCL */
/* to be used with the DATAEXCLVAR statement */
/* of PROC ESTIMATOR */
data currentdata;
set currentdata;
if (x > 2000) then
CURRDATAEXCL='E';
else
CURRDATAEXCL='';
run;

/* Create an exclusion variable called HISTDATAEXCL */
/* to be used with the HISTEXCLVAR statement */
/* of PROC ESTIMATOR */
data histdata;
set histdata;
if (x > 1000) then
HISTDATAEXCL='E';
else
HISTDATAEXCL='';
run;

/* Here we define our own algorithm which redefines */
/* a predefined algorithm */
data algorithm;
length algorithmname $ 8;
length type $ 2;
length status $ 2;
length formula $ 30;
length formula $ 30;
algorithmname='prevalue';
type='ef';
status='vp';
formula='fieldid(h,v)';
description='Previous value is imputed.';
run;

/* Here we create the data set that contains the imputation */
/* strategy.  The estimators will be processed in the order in which */
/* they appear in the data set. */
data estimator;
length est_setid $ 4;
length algorithmname $ 8;
length excludeimputed $ 1;
length excludeimputed $ 1;
length randomerror $ 1;
length countcriteria 8;
length percentcriteria 8;
length weightvariable $ 15;
length variancevariable $ 15;
length varianceperiod $ 1;
length varianceexponent 8;
length fieldid $ 15;
length auxvariables $ 45;
est_setid='set1';
excludeimputed='y';
excludeoutliers='y';
randomerror='y';
algorithmname='prevalue';
fieldid='x';
output;
est_setid='set1';
excludeimputed='y';
excludeoutliers='y';
randomerror='y';
countcriteria=1;
percentcriteria=1;
algorithmname='curmean';
fieldid='x';
output;
est_setid='set2';
excludeimputed='n';
excludeoutliers='n';
randomerror='y';
countcriteria=1;
percentcriteria=1;
algorithmname='diftrend';
fieldid='x';
output;
est_setid='set2';
excludeimputed='y';
excludeoutliers='y';
randomerror='y';
countcriteria=1;
percentcriteria=1;
algorithmname='histreg';
fieldid='x';
output;
run;

/* Sort the data set and call PROC ESTIMATOR */
proc sort data=currentdata; by prov ident; run;
proc sort data=histdata; by prov ident; run;

proc estimator
data=currentdata
datastatus=currstatus
hist=histdata
OUTESTPARMS=OUTESTPARMS
algorithm=algorithm
estimator=estimator (where=(est_setid='set1'))
out=outdata
outstatus=outstatus
seed=1
outrandomerror=outrandomerror
acceptnegative;
id ident;
dataexclvar CURRDATAEXCL;
histexclvar HISTDATAEXCL;
by prov;
run;


%export_ds_estimato(
    data=currentdata
    ,datastatus=currstatus
    ,hist=histdata
    ,OUTESTPARMS=OUTESTPARMS
    ,algorithm=algorithm
    ,estimator=estimator (where=(est_setid='set1'))
    ,out=outdata
    ,outstatus=outstatus
    ,outrandomerror=outrandomerror
)