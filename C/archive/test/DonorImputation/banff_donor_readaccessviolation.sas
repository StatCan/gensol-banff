%let num_donors = 500;
%let num_recipients = 10;

/* Generate random test data */
data oom_test_data(drop=i N);
   do i=1 to %eval(&num_donors+&num_recipients);
      N + 1;
      match_var = ranuni(0);
      if N <= &num_donors then dummy = 1;
      else dummy = 0;
      output;
   end;
run;

data
      oom_test_data(keep=uid match_var dummy)
      oom_test_status(keep=uid fieldid status);
   set oom_test_data;

   length uid $ 100;
   uid = compress(put(_N_, best.));
   fieldid = "dummy";
   if dummy = 1 then status = "IDE";
   else status = "FTI";
   run;

/* success */
proc donorimputation
      data = oom_test_data
      instatus = oom_test_status
      out = oom_test_out_data
      outstatus = oom_test_outstatus
      donormap = oom_test_donormap
      edits = "dummy > 0;"
      postedits = "dummy > 0;"
      mindonors = 1
      pcentdonors = 1
      acceptnegative
      n = 1
      displaylevel = 1
      ;
   id uid;
   mustmatch match_var;
run;
/* failure: when validation fails, something goes wrong */
proc donorimputation
      data = oom_test_data
      out = oom_test_out_data
      instatus = oom_test_status
      outstatus = oom_test_outstatus
      donormap = oom_test_donormap
      edits = "dummy > 0;"
      postedits = "dummy > 0;"
      mindonors = 1
      pcentdonors = 1
      acceptnegative
      /*n = 1*/
      DISPLAYLEVEL=1
      ;
   id uid;
   mustmatch match_var;
run;
