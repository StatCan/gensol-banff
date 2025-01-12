%macro apply_list_affixes(list, prefix=, suffix=, separator=%str( ));
    %if %length(%trim(&list)) = 0 %then %return;
    %unquote(&prefix.%qsysfunc(tranwrd(%sysfunc(compbl(&list)), %str( ), &suffix.&separator.&prefix.))&suffix.)
%mend apply_list_affixes;

%macro TEST_get_block_params(gifi_source=, block_num=);
    %local found;
    %global special mindonor pcentdonor allow_negative_details matching;
    %let found = 0;
    proc sql noprint;
        select 1, special, mindonor, pcentdonor, allow_negative_details, matching
        into :found, :special trimmed, :mindonor trimmed, :pcentdonor trimmed, 
             :allow_negative_details trimmed, :matching trimmed
        from block_parameters
        where generic="&block_num" and source="%upcase(&gifi_source)";
    %if &found ^= 1 %then %do; 
        /* Use default parameters if nothing specific is defined for this block */
        proc sql noprint;
            select 1, special, mindonor, pcentdonor, allow_negative_details, matching
            into :found, :special trimmed, :mindonor trimmed, :pcentdonor trimmed, 
                 :allow_negative_details trimmed, :matching trimmed
            from block_parameters
            where generic="0000" and source="%upcase(&gifi_source)";
        %if &found ^= 1 %then %do;             
            data _null_;
                file print;
                put "Error processing level 3 block %trim(&block_num): the block parameters file contains neither a block-specific nor a default (%trim(&gifi_source)) definition.";
            run;
        %end;
    %end;
%mend TEST_get_block_params;

%macro get_gifi_for_level(out=, level=, gifi_source=);
    data &out;
        infile "&gifi_file_loc" truncover;
        input @1  source         $2. 
              @4  level          $1.
              @6  from           $4.
              @11 to             $4.
              @16 no_allocation   1.
              @18 amort_src_from $4.
              @23 amort_src_to   $4.
              ;
        if upcase(source) = "%upcase(%trim(&gifi_source))";
        if level = "%trim(&level)";

        drop source level;
    run;   
%mend get_gifi_for_level;


%macro TEST_lv3_donor_select(year=, block_num=, matching=, generic_code=, detail_codes=,
                            mindonor=, pcentdonor=, nlimit=, allow_negative_details=,
                            verbose_log=0, verbose_output=0);

    %let countDetails = %sysfunc(countw(&detail_codes));
    %let generic = L%trim(&generic_code);
    %let details = %apply_list_affixes(&detail_codes, prefix=L);
    %let details_ratio = %apply_list_affixes(&details, prefix=ratio_); 
    %let details_reported = %apply_list_affixes(&details, suffix=r);

    %let details_rep_comma = %apply_list_affixes(&details_reported, separator=%str(, ));
    %let details_rep_name = %apply_list_affixes(&details_reported, prefix=%str(%"), suffix=%str(%"));    
    %let details_rep_edits = %apply_list_affixes(&details_reported, suffix=%str( >= 0;)); 
    %let details_rep_sum = %apply_list_affixes(&details_reported, separator=%str( + ));

    %let strict_edits     = %str(detail_counts >= 0; block_total > 0; donor_status = 1;);
    %let loose_edits      = %str(detail_counts >= 0; donor_status = 1;);
    %let strict_postedits = %str(detail_counts >= 0; block_total > 0; donor_status = 1;);
    %let loose_postedits  = %str(detail_counts >= 0; donor_status = 1;);
    %let details_needed_edits = %str(&details_rep_edits);
    %let details_needed_postedits = %str(&details_rep_sum >= detail_counts + 1;);   
    
    proc sort data=input.pre_banff_b&block_num._&year out=to_impute; by unique_id; run;

    /* We start with a blank set and then append to it with each run. */
    data donormap; 
        set _null_; 
        /* Need to define all these variables at the outset so that they'll be there on the off chance 
           that we don't end up running BANFF at all. */
        length recipient donor $31. NAICS $6. NAICS5 $5. NAICS4 $4. NAICS3 $3. NAICS2 CANADA PROVINCE $2. 
               NUMBER_OF_ATTEMPTS DONORLIMIT REQUIRE_NEW_DETAILS 8. IMP_LEVEL $2.; 
        call missing(recipient, donor, NAICS, NAICS5, NAICS4, NAICS3, NAICS2, CANADA, PROVINCE,
                     NUMBER_OF_ATTEMPTS, DONORLIMIT, REQUIRE_NEW_DETAILS, IMP_LEVEL);
    run;

    %TEST_perform_lv3_runs(require_new_details=0, block_num=&block_num, details=&details, 
                          mindonor=&mindonor, pcentdonor=&pcentdonor, nlimit=&nlimit, 
                          matching=&matching, skip_optional_runs=0, verbose_log=&verbose_log, 
                          verbose_output=&verbose_output);

%mend TEST_lv3_donor_select;



%macro TEST_perform_lv3_runs(require_new_details=0, block_num= , details= , mindonor= , pcentdonor= , 
                            nlimit= , matching=, skip_optional_runs=0, verbose_log=0, verbose_output=0);
    /*  &skip_optional_runs, when nonzero, directs the process to only perform runs marked as 
        ALWAYS_TRY in the run parameter file. When it is set, there will be more records that
        fall through to default imputation because they were unable to find a donor. */
 
    /*  When &require_new_details is 0, no meaningful postedits will be applied, and thus 
        we can use fewer variables and set n (the max # attempts per recipient) to 1.

        Otherwise, postedits are applied to ensure that we do not select a donor with only
        details that are already reported on the recipient. In this case we set n to a fairly
        large number, because it can happen that most donors have the same reporting pattern
        and thus we need to search for a while to locate one that passes.

        The runtime in this situation increases proportionately to n, so it is important to 
        find a balance where it does not take too long and yet few records are failing because
        they reach n without finding a suitable donor. For now, it is set to 1000.  */
    
    %if &require_new_details = 0 %then %do;
        %let n_param = 1;
        %let drop_detail_clause = %str(drop = &details_reported);
        %let extra_edits =; 
        %let extra_postedits =;
    %end;
    %if &require_new_details ^= 0 %then %do;
        %let n_param = 1000; 
        %let drop_detail_clause =;
        %let extra_edits = &details_needed_edits;
        %let extra_postedits = &details_needed_postedits;
    %end;

    %do run = 1 %to &total_runs;
        proc sql noprint;
            select trim(categories),            
                   case when strict_edit then 
                       trim("%bquote(&strict_edits &extra_edits)") 
                   else 
                       trim("%bquote(&loose_edits &extra_edits)") 
                   end,

                   case when strict_postedit then 
                       trim("%bquote(&strict_postedits &extra_postedits)") 
                   else 
                       trim("%bquote(&loose_postedits &extra_postedits)")
                   end,

                   case when always_try = 0 or missing(always_try) then 0 else 1 end
            into :domain, :edits, :postedits, :mandatory
            from runs where run = &run;
            quit;

        /* If the domain key contains more than one variable, we need its final component alone 
           for some later BY-group logic. */
        %let last_domain_var = %trim(%scan(&domain, %sysfunc(countw(&domain))));

        /* We make an _temp table and then replace to_impute with that rather than replacing 
           to_impute directly as a workaround for an issue we were having with replacing datasets
           too quickly. This is a bit of a kludge and it should be obsoleted by SAS 9.4 M5 which
           has a hotfix, so it can be unrolled once we're sure everyone is using that version or 
           later. */

    
        data to_impute_temp;
            set to_impute;
            length missing_class 3.;
            
            array domain[*] $ &domain;
            missing_class = cmiss(of domain[*]) > 0; /* assumes the domain is all character vars */
        run; 

        proc datasets nolist; delete to_impute; run;

        proc sql noprint;
            select count(1), sum(donor_status=1), sum(donor_status=0), sum(missing(donor_status))
                   into :total_rec_count, :donor_rec_count, :excl_rec_count, :recip_rec_count
            from to_impute_temp where not missing_class;
            select count(1) into :total_missing_class
            from to_impute_temp where missing_class;
        
        proc datasets nolist; change to_impute_temp=to_impute; run;
        
        /* Log statistics about the run. If turned on, this is output for _every_ run (more than 20 times 
           per block), so I don't advise turning it on unless you're just running one or two blocks 
           in isolation. */
        %if &verbose_output > 0 and (&skip_optional_runs = 0 or &mandatory > 0) %then %do; 
        
            title "Block %trim(&block_num), run %trim(&run): &domain";

            %if &require_new_details ^= 0 %then %do;
                title2 "Resolving leftover cases where naïve donor choice overlapped completely in details.";
            %end;

            data _null_;
                file print;            
                put "Edits: &edits";
                put; 
                put "Postedits: &postedits";
                put; 
                put "----------------------------------"; 
                put;
                put "Skipping %trim(&total_missing_class) records with missing values in the imputation class.";
                put "%trim(&total_rec_count) records being processed:";
                put "    %trim(&recip_rec_count) to be imputed";
                put "    %trim(&donor_rec_count) donors";
                put "    %trim(&excl_rec_count) records excluded";
            run;
            title;

        %end;
        
        /* We skip non-mandatory runs when &skip_optional_runs > 0, and we skip all remaining runs
           once all recipients have been imputed. Otherwise, do the imputation run here: */
        %if &recip_rec_count > 0 and (&skip_optional_runs = 0 or &mandatory > 0) %then %do;

            proc sort data = to_impute; by &domain unique_id; run;
                   
            data instatus (keep = unique_id &domain status fieldid);
                set to_impute(where=(not missing_class));

                length fieldid $14.;

                array details_rep_name[&countDetails] $ (&details_rep_name);
                array details_rep[&countDetails] &details_reported;

                %if &require_new_details ^= 0 %then %do;
                do i = 1 to &countDetails;                
                    if missing(donor_status) and not details_rep[i] then status = 'FTI';
                    else status = 'IDE';
                    fieldid = details_rep_name[i];
                    output instatus;
                end;
                %end;
                
                if missing(donor_status) then status = 'FTI'; /* to be imputed */
                else status = 'IDE'; /* value valid for imputation */
                fieldid = "donor_status";
                output instatus;
            run;
            
            data imputed; set _null_; run;            


            /*options fullstimer;*/  /* track BANFF's memory usage */
            data midput.indata_&block_num._&run.;
                set to_impute(where=(not missing_class) 
                                     &drop_detail_clause);
            run;

            data midput.instatus_&block_num._&run.;
                set instatus;
            run;
            options nomlogic;
            %put SA_RUN_block_num=&block_num
            SA_RUN_run=&run
            SA_RUN_drop_clause=&drop_detail_clause
            SA_RUN_edits = "&edits"
            SA_RUN_postedits = "&postedits"
            SA_RUN_mindonors = &mindonor
            SA_RUN_pcentdonors = &pcentdonor
            SA_RUN_n = &n_param
            SA_RUN_displaylevel=&verbose_log
            SA_RUN_mustmatch &matching
            SA_RUN_by &domain;
            %if %length(%trim(&nlimit)) > 0 %then %put nlimit = &nlimit; 
            proc donorimputation 
                    data = to_impute(where=(not missing_class) 
                                     &drop_detail_clause)
                    out = imputed_cur
                    instatus = instatus
                    outstatus = outstatus
                    donormap = donormap_cur
                    edits = "&edits"
                    postedits = "&postedits"
                    mindonors = &mindonor                
                    pcentdonors = &pcentdonor
                    %if %length(%trim(&nlimit)) > 0 %then nlimit = &nlimit; 
                    n = &n_param
                    displaylevel=&verbose_log
                    acceptnegative;
                id unique_id; 
                mustmatch &matching;
                by &domain;
                dataexclvar exclude;
                randnumvar random;  /* requires banff v2.07 or higher */
            run;
            %let err_code = &syserr;
            /*options nofullstimer;*/

            %if %length(&err_code)>0 and &err_code ^= 0 and &err_code ^= 4 %then %do;
                data _null_;
                    file print;
                    put;
                    put "BANFF encountered an unexpected internal error in proc donorimputation for block %trim(&block_num) during run %trim(&run).";
                    put;
                run;
            %end;               

            data midput.outstatus_&block_num._&run.;
                set outstatus;
            run;
            
            data midput.donormap_&block_num._&run.;
                set donormap_cur;
            run;
            
            data midput.outdata_&block_num._&run.;
                set imputed_cur;
            run;

            data donormap;
                set donormap donormap_cur(in=cur);
 
                if cur then do;
                    imp_level = put(&run, z2.);
                    require_new_details = (&require_new_details ^= 0);
                end;
            run;

            data imputed;
                set imputed imputed_cur(keep=unique_id);
            run;


            /* contains all donors and the remaining recipients in each run */
            proc sort data = to_impute(drop=missing_class);
                by unique_id; 
            run; 
            proc sort data=imputed; by unique_id; run;

            data to_impute;
                merge to_impute(in=a) imputed(in=b);
                by unique_id;
                            
                if a and not b then output to_impute;
            run;

        %end; /* end IF num recipients > 0 AND (skip optional runs = 0 OR mandatory run > 0) */
    %end; /* end loop over runs */

    /* clean up */
    proc datasets library = work nolist;
        delete imputed imputed_cur donormap_cur instatus outstatus;
    run; quit;


%mend TEST_perform_lv3_runs;


