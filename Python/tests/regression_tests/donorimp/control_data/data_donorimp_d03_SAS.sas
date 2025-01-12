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
%let out_dir_path=C:\git\banff_redesign\Banff\BanffProcedures\Python\banff\tests\regression_tests\donorimp\control_data\;
%let test_proc=donorimp;
%let test_case=d03;
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
 /* To verify that more than one BY variable can be used. */
options nodate nonumber ls=90 ps=100;
data indata;
   infile cards;
   input ident $ area $ total Q1 Q2 Q3 Q4 staff toexcl $ ;
   cards;
REC01 A1  500  120  150   80  150   50   .
REC02 A1  750  200  170  130  250  100   .
REC04 A1 1000  150  250  350  250  100   .
REC05 A2 1050  200  225  325  300  100   E
;
data instat;
   infile cards;
   input ident $ fieldid $ status $;
   cards;
REC01 Q3 IPR 
REC04 Q2 FTE 
REC04 Q3 FTI 
REC04 Q4 FTI 
;
proc donorimputation
   data=indata
   instatus=instat
   out=outdata
   outstatus=outstat
   donormap=map
   edits="Q1 + Q2 + Q3 + Q4 = total;"
   postedits="Q1 + Q2 + Q3 + Q4 - total <= 0;"
   mindonors=1
   pcentdonors=1
   n=3
   matchfieldstat
   eligdon=any
   ;
   id ident;
   dataexclvar toexcl;
   by area staff;
run;
%export_ds_donorimp(
    data=indata
    ,instatus=instat
    ,out=outdata
    ,outstatus=outstat
    ,donormap=map
)
proc print data=indata;
title "BANFF donor imputation input data";
run;
proc print data=instat;
title "BANFF donor imputation input status";
run;
proc print data=outdata;
title "BANFF donor imputation output data";
run;
proc print data=outstat;
title "BANFF donor imputation output status";
run;
proc print data=map;
title "BANFF donor imputation donor map";
run;
