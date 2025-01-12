#ifndef SUTIL_H
#define SUTIL_H
/* implements redesigned versions of legacy "SASUtilities" functions */

#include "IOUtil.h"
#include "EI_Edits.h"

/* START: copied from uwproc.h */

/*
type definition of some functions of the SAS/TOOLKIT
*/
#define SUTIL_VARIABLE_TYPE_NUMERIC   1
#define SUTIL_VARIABLE_TYPE_CHARACTER 2

/*
Name of the variables of the FIELDSTAT data set.
where should it be define? here?
*/
#define SUTIL_FIELDSTAT_FIELDID   "FIELDID"
#define SUTIL_FIELDSTAT_STATUS    "STATUS"

#define SUtil_PrintSystemInfo(Sys,SysVer,Proc,ProcVersion,Email,Msg) \
    Hidden_SUtil_PrintSystemInfo(Sys,SysVer,Proc,ProcVersion,Email,Msg,__DATE__,__TIME__)

IO_BOOLEAN SUtil_AreAllByVariablesInDataSet(
    DSR_generic* LookUp,
    DSR_generic* DataSet);

/*
Returns the presence of duplicate in a list of "variables position".
Retourne la présence de duplicata dans une liste de "position de variables".
*/
extern int SUtil_AreDuplicateInListPosition(
    /* Number of variables. Can be found with a call to SFN().

    Nombre de variables. Trouvé par un appel a SFN(). */
    int   NumberofVars,
    /* List of positions. Can be found with a call to SFLP ().

    Liste de positions. Trouvé par un appel a SFLP (). */
    int* VarsPositionList)
    ;

IO_BOOLEAN SUtil_GetOriginalName(DSR_generic* dsr, char* var_name);
extern int SUtil_GetVariablePosition(DSR_generic* dsr, char* VariableName);

void SUtil_PrintInputDataSetInfo(DSR_generic* dsr);
void SUtil_PrintOutputDataSetInfo(DSW_generic* dsw);

extern void SUtil_PrintfAllocate(void);
extern void SUtil_PrintfFree(void);
extern int SUtil_PrintfToLog(const char* Format, ...);
/*
Print the API messages / Imprime les messages des API.
*/
extern void SUtil_PrintMessages(void);

/*
Prints statement name and string of list of variables in it.
Ex.: In calling program,
        parameter NoStatement -> BY_GRM_NB
        parameter StatementName ->  "BY = "
     and will print in log: "BY = VAR1 VAR2 ... VARn"
*/
extern void SUtil_PrintStatementVars(
    PARM_generic* pg,
    DSR_generic* dsr,
    DS_varlist* vl,
    char* StatementName
);


extern void Hidden_SUtil_PrintSystemInfo(char*, char*, char*, char*,
    char*, char*, char*, char*);

/* copy a string to another and pad with blanks */
extern char* SUtil_CopyPad(char*, char*, size_t);

extern void SUtil_NullTerminate(char*, int);
/* pad a string with blanks */
extern char* SUtil_Pad(char*, size_t);

#ifndef EDITS_DISABLED
extern void SUtil_PrintEdits(EIT_EDITS*);
#endif
void* SUtil_AllocateMemory(size_t Quantity);

extern void* SUtil_ReallocateMemory(size_t, size_t, void*);
extern void SUtil_FreeMemory(void*);

#endif