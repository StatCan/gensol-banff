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
%let out_dir_path=C:\git\banff_redesign\Banff\BanffProcedures\Python\banff\tests\regression_tests\determin\control_data\;
%let test_proc=determin;
%let test_case=h01;
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
 /* To ensure that the FTI status is recognized and the imputed value is correct. 
VB should be imputed the value 113.95. */
options nodate nonumber ls=80 ps=100;

data indata;
    input REC $ VA VB VC VD TOT1 VE VF VG TOT2 GT;
       cards;
3  16.00 40.23 36.21 10.54 176.7 49.21 88.23 38.95 178.2 390.0
6  76.21 20.54 38.99 33.63 102.1 53.58 16.73 95.63 115.8 470.5
7  50.60 45.24 97.48 45.96 297.0  .    86.40  0.05  89.6 434.2
;
run;

data instat;
   input REC $ FIELDID $ STATUS $;
      cards;
3 VA IPR 
3 VB FTI
3 VC I
3 VD I
3 TOT1 O
3 VE IC2
3 VF O
3 VG IPM
3 TOT2 ICM
6 VB IPV
6 VC IAT
6 VD O
6 TOT1 I
6 VE O
6 VF IDN
6 TOT2 IPV
7 VB IPV
7 VC ODE
7 VD IDE
7 VF O
7 VG I
7 TOT2 ICR
;
run;
proc DETERMINISTIC
   data=indata
   instatus=instat
   out=outdata
   outstatus=outstat
   edits="VA >= 0.001; VC >= 0.0001; VE >= 0.0001; 
      VA + VB + VC + VD = TOT1; VE + VF + VG = TOT2; TOT1 + TOT2 = GT;"
   ;
   id REC;   
run;

proc print data=indata;
title "Deterministic input data";
run;

proc print data=instat;
title "Deterministic input status";
run;

proc print data=outdata;
title "Deterministic output records";
run;

proc print data=outstat;
title "Deterministic output status";
run;

%export_ds_donorimp(
   data=indata
   ,instatus=instat
   ,out=outdata
   ,outstatus=outstat
)