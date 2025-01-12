/* ==== BEGIN DISK PATH PARAMETERS TO EDIT ==== */

%let input_data_loc = G:\Banff_test\td\T2_GDA_BANFF_Test\Data;
%let mid_data_loc = G:\Banff_test\td\T2_GDA_BANFF_Test\Data_mid;

/* Log and macro paths should end in a platform-appropriate directory separator. */
%let log_output_loc = G:\Banff_test\td\T2_GDA_BANFF_Test\Logs\;
%let macro_include_loc = G:\Banff_test\td\T2_GDA_BANFF_Test\Programs\;

%let gifi_file_loc = G:\Banff_test\td\T2_GDA_BANFF_Test\Data\gda_gifi.txt;
%let lv3_param_file_loc = G:\Banff_test\td\T2_GDA_BANFF_Test\Data\level3_block_params.txt;
%let lv3_runs_file_loc = G:\Banff_test\td\T2_GDA_BANFF_Test\Data\level3_run_params.txt;

/* ==== END DISK PATH PARAMETERS TO EDIT ==== */


options mprint mlogic;

%let year = 2023;
%let gifi_source = IS;


%include "&macro_include_loc.BANFF_test_donor_select_macros.sas";

libname input "&input_data_loc.";
libname midput "&mid_data_loc.";

/* First line of this file is assumed to be a cosmetic header */
data block_parameters;
    infile "&lv3_param_file_loc" firstobs=2 lrecl=1000 truncover end=end;
    length source $2. generic $4. special $1. mindonor 8. pcentdonor 8. allow_negative_details 8.
           matching $200.;

    input @1  source $2. 
          @4  generic $4. 
          @9  special $1. 
          @11 mindonor 4. 
          @16 pcentdonor 2. 
          @19 allow_negative_details 1.
          @21 matching $200.;

    source = upcase(source);    

    /* todo here: log invalid inputs on nonblank lines */
    if source not in ("IS","BS") then delete;
    if notdigit(generic) > 0 then delete; /* expect 0000 for defaults */
    if missing(special) then delete;

    if special = "0" then do;
        if missing(mindonor) or mindonor < 0 then delete;
        if missing(pcentdonor) or pcentdonor < 0 then delete;
        if pcentdonor < 1 then pcentdonor = 1; /* this is the minimum BANFF will allow */
        if allow_negative_details not in (0,1) then delete;
    end;
run;

proc sort data=block_parameters; by source generic; run;

data block_parameters;
    set block_parameters;
    by source generic; 
    if not first.generic then delete;
run;

/* First line of this file is assumed to be a cosmetic header */
data runs;
    infile "&lv3_runs_file_loc" firstobs=2 lrecl=1000 truncover end=end;
    length run STRICT_EDIT STRICT_POSTEDIT ALWAYS_TRY 8. CATEGORIES $100.;

    run = _N_;
    input STRICT_EDIT STRICT_POSTEDIT ALWAYS_TRY CATEGORIES $100.;
    if end then call symput("total_runs", compress(put(run, 8.)));
run;

%get_gifi_for_level(out=gifi_groups_&gifi_source, level=3, gifi_source=&gifi_source);

proc sql noprint;
    select distinct from into :generic_codes separated by ' '
    from gifi_groups_&gifi_source where not no_allocation;
quit;

%put _user_;

%macro run_all_blocks();
    %local i count_generics;

    data donormap_full;
        length source $2. block $4.;
        set _null_; 
        call missing(source, block);
    run;

    %if %length(%trim(&generic_codes)) > 0 %then %do;
        %let count_generics = %sysfunc(countw(&generic_codes));
         
        %do i=1 %to &count_generics;
            %let block_num = %scan(&generic_codes, &i);
            %run_block(&block_num);
        %end;
    %end;
%mend run_all_blocks;


%macro run_block(block_num);
    proc printto log = "&log_output_loc.BANFF_test_&gifi_source._&block_num..log" new; run; 

    %TEST_get_block_params(gifi_source=&gifi_source, block_num=&block_num);

    proc sort nodupkey data = gifi_groups_&gifi_source(where=(from="&block_num"))
                       out = details(keep=to no_allocation amort_src_from amort_src_to);
        by to no_allocation;
    run;

    proc sql noprint;
        select to into :L&block_num._details_alloc separated by ' '
        from details where not no_allocation;

        select to into :L&block_num._details_unalloc separated by ' '
        from details where no_allocation;

    /* Check for blocks with no allocations permitted (which will be skipped), and also
       check for amortization blocks (which aren't skipped normally, but will be for this
       test program. */            
        select count(distinct to), count(distinct amort_src_from) 
            into :L&block_num._num_details_alloc trimmed, :amort_blocks trimmed
        from details where not no_allocation;
    quit;

    %if %unquote(%nrstr(&L)&block_num._num_details_alloc) > 0 and &amort_blocks = 0 %then %do;
        %if %unquote(%nrstr(&L)&block_num._num_details_alloc) > 1 %then %do; %put Okay then! &block_num; %end;
    
        %TEST_lv3_donor_select(year=&year, block_num=&block_num, matching=&matching, generic_code=&block_num, 
                               detail_codes=%unquote(%nrstr(&L)&block_num._details_alloc), mindonor=&mindonor, 
                               pcentdonor=&pcentdonor, allow_negative_details=&allow_negative_details, verbose_output=0);    

        data donormap_full;
            set donormap_full donormap(in=new_set);
            if new_set then do;
                source = "&gifi_source";
                block = "&block_num";
            end;
        run;

        proc datasets nolist; delete donormap to_impute; run;
    %end;

    proc printto; run; 
%mend run_block;

%run_all_blocks();

/* donormap_full will now contain all the results of the above processing, 
   and can be output somewhere if you want to save it. */
