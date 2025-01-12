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
%let test_case=r04;
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
%macro export_ds_errorloc(data=., outstatus=., outreject=.);
    %export_dataset(&data, indata)
    %export_dataset(&outstatus, outstatus)
    %export_dataset(&outreject, outreject)
%mend;

/* convenience function maps procedure's SAS dataset names to its Python dataset names */
%macro export_ds_estimato(algorithm=., data=., aux=., hist=., datastatus=., histstatus=., estimator=., out=., outstatus=., outestparms=., outestef=., outestlr=., outestparms=., outrandomerror=.);
   %export_dataset(&algorithm, inalgorithm)
   %export_dataset(&data, indata)
   %export_dataset(&aux, indata_hist)
   %export_dataset(&hist, indata_hist)
   %export_dataset(&datastatus, instatus)
   %export_dataset(&histstatus, instatus_hist)
   %export_dataset(&estimator, inestimator)
   %export_dataset(&out, outdata)
   %export_dataset(&outestparms, outest_parm)
   %export_dataset(&outestef, outest_ef)
   %export_dataset(&outestlr, outest_lr)
   %export_dataset(&outrandomerror, outrand_err)
   %export_dataset(&outstatus, outstatus)
%mend;


/* convenience function maps procedure's SAS dataset names to its Python dataset names */
%macro export_ds_prorate(data=., instatus=., out=., outstatus=., outreject=.);
    %export_dataset(&data, indata)
    %export_dataset(&instatus, instatus)
    %export_dataset(&out, outdata)
    %export_dataset(&outstatus, outstatus)
    %export_dataset(&outreject, outreject)
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

%export_ds_errorloc(
   data=indata
   ,outstatus=outstat
   ,outreject=outrej
)

%export_ds_estimato(
   data=indata
   ,hist=inhist
   ,outstatus=outstat
   ,outsummary=outsum
)
%export_ds_prorate(
   data=indata
   ,hist=inhist
   ,outstatus=outstat
   ,outsummary=outsum
)

 SAS_EXPORT: END */