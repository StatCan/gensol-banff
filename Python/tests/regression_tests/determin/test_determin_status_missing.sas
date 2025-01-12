/* this file complements `test_determin_status_missing.py` 
It investigates how SAS handles 
- missing BY values
- unordered BY values on status 

In some situations SAS doesn't detect improperly sorted status datasets and doesn't end up
reading status codes */

data determdata;
    infile cards;
    input ident $ TOTAL Q1 Q2 Q3 Q4 staff prov;
    cards;
    REC01 500 100 125 125 150 2000 24
    REC02 750 200 170 130 250 2500 24
    REC03 400 80 90 100 130 1500 24
    REC04 1000 150 250 350 250 3500 24
    REC05 3000 500 500 1000 1000 5000 24
    REC06 50 10 15 500 20 100 24
    REC07 600 110 140 230 45 2400 30
    REC08 900 175 999 999 300 3000 30
    REC09 2500 400 555 600 5000 89 30
    REC10 800 11 12 13 14 2800 30
    REC11 -25 -10 -5 -5 -10 3000 30
    ;
    data determstat;
    infile cards;
    input fieldid $ status $ ident $ prov;
    cards;
    Q3 FTI REC06 24
    Q4 FTI REC12 .
    Q4 FTI REC07 30
    Q2 FTI REC08 30
    Q3 FTI REC08 30
    Q4 FTI REC09 30
    staff FTI REC09 30
    Q1 FTI REC10 30
    Q2 FTI REC10 30
    Q3 FTI REC10 30
    Q4 FTI REC10 30
    Q4 FTI REC11 30
    ;

	/* 'REC12' in status but not indata
		'prov' is numeric, missing
	Result: C detects out of order BY groups in status
	ERROR: Data set WORK.DETERMSTAT is not sorted in ascending sequence. The current BY group has
       prov = 24 and the next BY group has prov = ..
*/
    proc DETERMINISTIC
    data=determdata
    instatus=determstat
    out=outdata
    outstatus=outstatus
    edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;"
    acceptnegative;
    id ident;
    by prov;
    run;
    

	data determdata;
    infile cards;
    input ident $ TOTAL Q1 Q2 Q3 Q4 staff prov $;
    cards;
    REC01 500 100 125 125 150 2000 24
    REC02 750 200 170 130 250 2500 24
    REC03 400 80 90 100 130 1500 24
    REC04 1000 150 250 350 250 3500 24
    REC05 3000 500 500 1000 1000 5000 24
    REC06 50 10 15 500 20 100 24
    REC07 600 110 140 230 45 2400 30
    REC08 900 175 999 999 300 3000 30
    REC09 2500 400 555 600 5000 89 30
    REC10 800 11 12 13 14 2800 30
    REC11 -25 -10 -5 -5 -10 3000 30
    ;
    data determstat;
    infile cards;
    input fieldid $ status $ ident $ prov $;
    cards;
    Q3 FTI REC06 24
    Q4 FTI REC12 .
    Q4 FTI REC07 30
    Q2 FTI REC08 30
    Q3 FTI REC08 30
    Q4 FTI REC09 30
    staff FTI REC09 30
    Q1 FTI REC10 30
    Q2 FTI REC10 30
    Q3 FTI REC10 30
    Q4 FTI REC10 30
    Q4 FTI REC11 30
    ;
	/* 'REC12' in status but not indata
		'prov' is character, missing
	Result: C detects out of order BY groups in status
	ERROR: Data set WORK.DETERMSTAT is not sorted in ascending sequence. The current BY group has
       prov = 24 and the next BY group has prov = ''.
*/
    proc DETERMINISTIC
    data=determdata
    instatus=determstat
    out=outdata
    outstatus=outstatus
    edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;"
    acceptnegative;
    id ident;
    by prov;
    run;
    

    data determstat;
    infile cards;
    input fieldid $ status $ ident $ prov $;
    cards;
    Q4 FTI REC12 40
    Q3 FTI REC06 24
    Q4 FTI REC07 30
    Q2 FTI REC08 30
    Q3 FTI REC08 30
    Q4 FTI REC09 30
    staff FTI REC09 30
    Q1 FTI REC10 30
    Q2 FTI REC10 30
    Q3 FTI REC10 30
    Q4 FTI REC10 30
    Q4 FTI REC11 30
    ;
	/* 'REC12' in status but not indata
		'prov' 40 only exists in status
		- its value exceeds that of all other BY values
		- it appears FIRST when it should appear LAST
	Result: SAS Thinks that status doesn't contain any data for
			prov 24 and 30 */
    proc DETERMINISTIC
    data=determdata
    instatus=determstat
    out=outdata
    outstatus=outstatus
    edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;"
    acceptnegative;
    id ident;
    by prov;
    run;
    

    data determstat;
    infile cards;
    input fieldid $ status $ ident $ prov $;
    cards;
    Q3 FTI REC06 24
    Q4 FTI REC12 40
    Q4 FTI REC07 30
    Q2 FTI REC08 30
    Q3 FTI REC08 30
    Q4 FTI REC09 30
    staff FTI REC09 30
    Q1 FTI REC10 30
    Q2 FTI REC10 30
    Q3 FTI REC10 30
    Q4 FTI REC10 30
    Q4 FTI REC11 30
    ;
	/* 'REC12' in status but not indata
		'prov' 40 only exists in status
		- its value exceeds that of all other BY values
		- it appears SECOND when it should appear LAST
	Result: SAS Thinks that status doesn't contain any data for
			prov 24 and 30 PAST RECORD 12 */
    proc DETERMINISTIC
    data=determdata
    instatus=determstat
    out=outdata
    outstatus=outstatus
    edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;"
    acceptnegative;
    id ident;
    by prov;
    run;
    

    data determstat;
    infile cards;
    input fieldid $ status $ ident $ prov $;
    cards;
    Q3 FTI REC06 24
    Q4 FTI REC12 23
    Q4 FTI REC07 30
    Q2 FTI REC08 30
    Q3 FTI REC08 30
    Q4 FTI REC09 30
    staff FTI REC09 30
    Q1 FTI REC10 30
    Q2 FTI REC10 30
    Q3 FTI REC10 30
    Q4 FTI REC10 30
    Q4 FTI REC11 30
    ;
	/* 'REC12' in status but not indata
		'prov' 23 only exists in status
		- it should appear FIRST but appears second (after 24)
	Result: SAS detects this and stops with an error */
    proc DETERMINISTIC
    data=determdata
    instatus=determstat
    out=outdata
    outstatus=outstatus
    edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;"
    acceptnegative;
    id ident;
    by prov;
    run;
    
    data determstat;
    infile cards;
    input fieldid $ status $ ident $ prov $;
    cards;
    Q3 FTI REC06 24
    Q4 FTI REC13 25
    Q4 FTI REC12 23
    Q4 FTI REC07 30
    Q2 FTI REC08 30
    Q3 FTI REC08 30
    Q4 FTI REC09 30
    staff FTI REC09 30
    Q1 FTI REC10 30
    Q2 FTI REC10 30
    Q3 FTI REC10 30
    Q4 FTI REC10 30
    Q4 FTI REC11 30
    ;
	/* 'REC12' in status but not indata
		its 'prov' 23 only exists in status
	   'REC13' in status but not indata
	    its 'prov' 25 only exists in status
		
	Result: SAS detects this and stops with an error */
    proc DETERMINISTIC
    data=determdata
    instatus=determstat
    out=outdata
    outstatus=outstatus
    edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;"
    acceptnegative;
    id ident;
    by prov;
    run;