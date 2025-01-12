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
%let out_dir_path=C:\git\banff_redesign\Banff\BanffProcedures\Python\banff\tests\misc_tests\outlier\control_data\;
%let test_proc=outlier;
%let test_case=FD0x;
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
 /* Program:  FD_one_variable_examples_for_HB&SG.sas 
   By:  Danny Finch, BSMD, July 15, 2013 

   This program contrasts the Hidiroglou-Berthelot with Sigma-Gap Methods using 2 examples of
   1-Dataset and 1-Variable from pages 4-3 and 4-13 of the Functional Description.

   On Apr.16/14, a new Sigma-Gap example was added to the end of this program using a new
   1-Dataset and 1-Variable to serve as an additional example on page 4-14 of the Functional Description.

*/

options nodate nonumber;

data indata; 
   infile cards;
   input ident $ V1 V2;
   cards;
R01 -1 60
R02 4 15
R03 7 150
R04 7 130
R05 8 40
R06 8 160
R07 8 70
R08 8 70
R09 9 40
R10 9 62
R11 9 75
R12 9 100
R13 9 180
R14 9 200
R15 10 90
R16 10 100
R17 11 165
R18 11 30
R19 11 300
R20 12 100
R21 13 195
R22 13 160
R23 15 .
R24 19 .
;
run;

proc print data=indata; 
title "Test input data - 2 variables:  V1 and V2";
run;

/* This part of the program contrasts the Hidiroglou-Berthelot with Sigma-Gap Methods using
   1-Dataset and 1-Variable based on the example on page 4-3 of the Functional Description */

/* HB test - 1 variable:  V1 */
proc outlier
   data=indata
   outstatus=outstata
   method=C
   MII=6
   MEI=4
   outlierstat
   acceptnegative
   ;
   id ident;
   var V1;
run;

%let subtest=a;
%export_ds_outlier(
   data=indata
   ,outstatus=outstata
)


proc print data=outstata;
title "HB test OutStatus - 1 variable:  V1";
run;

/* SG test - 1 variable:  V1 */
proc outlier
   data=indata
   outstatus=outstatb
   method=S
   BETAI=3.0
   BETAE=1.5
   /*startcentile=75*/
   outlierstat
   acceptnegative
   ;  
   id ident;
   var V1;
run;

%let subtest=b;
%export_ds_outlier(
   data=indata
   ,outstatus=outstatb
)

proc print data=outstatb;
title "SG test OutStatus - 1 variable:  V1";
run;

/* This part of the program contrasts the Interquartile with Sigma-Gap Methods using
   1-Dataset and 1-Variable based on the example on page 4-9 of the Functional Description */

/* HB test - 1 variable:  V2 */
proc outlier
   data=indata
   outstatus=outstatc
   method=C
   MII=6
   MEI=3
   outlierstat
   acceptnegative
   ;
   id ident;
   var V2;
run;

%let subtest=c;
%export_ds_outlier(
   data=indata
   ,outstatus=outstatc
)

proc print data=outstatc;
title "HB test OutStatus - 1 variable:  V2";
run;

/* SG test - 1 variable:  V2 */
proc outlier
   data=indata
   outstatus=outstatd
   method=S
   BETAI=3.0
   BETAE=1.0
   startcentile=75
   outlierstat
   acceptnegative
   ;  
   id ident;
   var V2;
run;

%let subtest=d;
%export_ds_outlier(
   data=indata
   ,outstatus=outstatd
)

proc print data=outstatd;
title "SG test OutStatus - 1 variable:  V2";
run;


/* This part of the program creates a brand new Sigma-Gap example for the 
Functional Description using 1-Dataset and 1-variable */

data indata2; 
   infile cards;
   input ident $ V1;
   cards;
R01 5
R02 5
R03 5
R04 6
R05 6
R06 6
R07 6
R08 6
R09 7
R10 7
R11 7
R12 24
R13 24
R14 25
R15 25
R16 25
R17 25
R18 27
R19 28
R20 100
;
run;

proc print data=indata2; 
title "NEW test input data - 1 variable:  V1";
run;

/* SG test - 1 variable:  V1 */
proc outlier
   data=indata2
   outstatus=outstate
   method=S
   side=RIGHT
   BETAI=3.0
   BETAE=1.5
   startcentile=80
   outlierstat
   acceptnegative
   ;  
   id ident;
   var V1;
run;

%let subtest=e;
%export_ds_outlier(
   data=indata2
   ,outstatus=outstate
)

proc print data=outstate;
title "NEW SG test OutStatus - 1 variable:  V1";
run;
