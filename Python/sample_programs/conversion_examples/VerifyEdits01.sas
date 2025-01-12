/* Implicit addition of positivity edits: REJECTNEGATIVE */
proc VERIFYEDITS
edits = "
HEN_LT20+HEN_GE20+HEN_OTH=HEN_TOT;
2*EGG_LAID<=HEN_GE20;
HEN_GE20<=4*EGG_LAID;
EGG_SOLD<=EGG_LAID;
EGG_VALU<=2.75*EGG_SOLD;
0.9*EGG_SOLD<=EGG_VALU;
fail:HEN_GE20>300000;"
imply = 50
extremal = 10
;run;
