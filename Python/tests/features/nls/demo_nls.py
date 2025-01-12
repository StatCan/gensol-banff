"""
Demonstrate the Native Language Support feature
    Runs the same procedure call multiple times, switching the language between calls
    using `banff.nls.set_language()`

    Language can be switched at any time.  
"""
# import necessary Python packages
import banff
import pandas as pd
from io import StringIO

# create indata dataframe
indata = pd.read_csv(StringIO("""
    ident TOTAL Q1 Q2 Q3 Q4 staff prov
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
"""), sep=r'\s+')


# create donorstat dataset
determstat = pd.read_csv(StringIO("""
    fieldid status ident
    Q3 FTI REC06
    Q4 FTI REC07
    Q2 FTI REC08
    Q3 FTI REC08
    Q4 FTI REC09
    staff FTI REC09
    Q1 FTI REC10
    Q2 FTI REC10
    Q3 FTI REC10
    Q4 FTI REC10
    Q4 FTI REC11
"""), sep=r'\s+')

print("\n\n~~~~~setting language to English~~~~~\n\n")
banff.nls.set_language(banff.SupportedLanguage.en)

# run Banff call
banff_call= banff.determin(
                indata=indata,
                instatus=determstat,
                edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;",
                accept_negative=True,
                unit_id="ident",
                by="prov",trace=True
                )

print("\n\n~~~~~setting language to French~~~~~\n\n")
banff.nls.set_language(banff.SupportedLanguage.fr)
# run Banff call
banff_call= banff.determin(
                indata=indata,
                instatus=determstat,
                edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;",
                accept_negative=True,
                unit_id="ident",
                by="prov",trace=True
                )
print("\n\n~~~~~setting procedure language to French~~~~~\n\n")
banff_call._reload_c(lang=banff.nls.get_language())
banff_call.rerun()


print("\n\n~~~~~setting language back to English~~~~~\n\n")
banff.nls.set_language(banff.SupportedLanguage.en)
banff.determin._reload_c(lang=banff.SupportedLanguage.en)

banff_call.rerun()