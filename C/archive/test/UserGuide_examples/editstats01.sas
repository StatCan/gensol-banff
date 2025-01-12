data data;
input x1 x2 x3 Group $1.;
cards;
4 3 2 A
-4 3 2 A
6 3 2 B
4 3 . A
6 3 . B
;
run;
proc sort data=data; by Group; run;
proc editstats
data=data
outreducededits=min_edits
outeditstatus=table11
outkeditsstatus=table12
outglobalstatus=table13
outeditapplic=table21
outvarsrole=table22
edits="
x1+1>=x2;
x1<=5;
x2>=x3;
x1+x2+x3<=9;"
acceptnegative
;
by Group;
run;
