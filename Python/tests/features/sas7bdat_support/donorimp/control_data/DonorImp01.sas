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
%let out_dir_path=C:\git\banff_redesign\Banff\BanffProcedures\Python\banff\tests\misc_tests\donorimp\control_data\;
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
 /* create the data= data set */
data donordata;
infile cards;
input IDENT $ TOTAL Q1 Q2 Q3 Q4 STAFF PROV;
cards;
REC01 500 120 150 80 150 50 24
REC02 750 200 170 130 250 75 24
REC03 400 80 90 100 130 40 24
REC04 1000 150 250 350 250 100 24
REC05 1050 200 225 325 300 100 24
REC06 500 100 125 5000 130 45 24
REC07 400 80 90 100 15 40 30
REC08 950 150 999 999 200 90 30
REC09 1025 200 225 300 10 10 30
REC10 800 11 12 13 14 80 30
REC11 -25 -10 -5 -5 -10 3000 30
REC12 1000 150 250 350 250 100 30
REC13 999 200 225 325 300 100 30
REC14 -25 -10 -5 -10 -5 3000 30
;

/* create the exclusion variable */
data donordata;
set donordata;
if (TOTAL > 1000) then TOEXCL='E';
else TOEXCL='';
run;
 
/* create the instatus data set */
data donorstat;
infile cards;
input FIELDID $ STATUS $ IDENT $;
cards;
Q3 IPR REC01
Q4 FTE REC04
Q3 FTI REC06
Q2 FTI REC07
Q2 FTI REC08
Q3 FTI REC08
Q4 FTI REC09
STAFF FTI REC09
Q1 FTI REC10
Q2 FTI REC10
Q3 FTI REC10
Q4 FTI REC10
Q1 FTI REC11
Q2 FTI REC11
Q3 FTI REC11
Q4 FTI REC11
Q2 FTI REC13
Q3 FTI REC13
;
/* call the donorimputation procedure */
proc DONORIMPUTATION
data=donordata
instatus=donorstat
out=donorout
outstatus=outstat
donormap=donormap
edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;"
postedits="Q1 + Q2 + Q3 + Q4 - TOTAL <= 0;"
mindonors=1
pcentdonors=1
n=1
nlimit=1
mrl=0.5
random
eligdon=original
matchfieldstat
acceptnegative
;
id IDENT;
mustmatch STAFF;
dataexclvar TOEXCL;
by prov;
run;

%export_ds_donorimp(
   data=donordata
   ,instatus=donorstat
   ,out=donorout
   ,outstatus=outstat
   ,donormap=donormap
)