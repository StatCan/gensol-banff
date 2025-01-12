/* LPInterface.c:
   Interface between systems and solvers: GLPK */

#include "lpx.h"

#include <float.h>

#include "EI_EditsStruct.h"
#include "LPInterface.h"

#include "MessageLPI.h"

/* FLT_EPSILON is defined in float.h */
#define EIM_DBL_EQ(a,b)      (fabs((a)-(b)) < FLT_EPSILON)

/* these max sizes include NULL terminator */
#define ROW_NAME_MAX_SIZE 8    /* Row name max size */
#define PB_NAME_MAX_SIZE 15    /* Problem name max size */
#define LP_VERSION_MAX_SIZE 20 /* LP package version max size */
#define LP_NAME_MAX_SIZE 10    /* LP package name max size */

#define INITIAL_PB_TABLE_SIZE 10 /*Initial size of problem table must be >=2*/


/******************** Static functions declaration: start ********************/

static int check_number_edits_and_cols(EIT_EDITS *Edits, int NumEdits, char *errmsg);
static int countnonzeroelements(EIT_EDITS *Edits, int NumEdits);
static int GetFirstProblemTableNullEntry (void);
static int optimize(int PbNb, int iterationlimit, int timelimit, int optsense, char *errmsg);
static int callback_print_glpk(void *info, const char *msg);
static void callback_error_glpk(void *info);

/********************* Static functions declaration: end *********************/

/****************** Global variables declarations: start *********************/

/* global variables that are common to all solvers */

/*Counts lp problems created and not yet deleted*/
static int gActiveProblemCount;
/* allocated size of pbtab */
static int gProblemTableSize;

/* global variables that are specific to each solver */

/*Array of pointers to glpk problem objects*/
static LPX **glpkpbtab = NULL;

/****************** Global variables declarations: end *********************/


/*-----------------------------------------------------------------------------
  Functions published by this interface are listed below in alphabetical order.
  Callback functions (callback_xxxx_yyyy()) are particular in that they are
  called by the optimizers, not by the usual user of the interface
  Are called from optimizers to perform specific actions, especially print out
  message concerning the progress of the optimization process.
  ---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  For use with glpk.
  Prints any output from glpk.
  Usage:
  glp_term_hook(callback_print_glpk, NULL);
  ---------------------------------------------------------------------------*/
static int callback_print_glpk(void *info, const char *msg)
{
	FILE * fp;
    info = info; /* for lint */
	fp = fopen ("c:\\callback_print_glpk.txt", "a+");  // nosemgrep  // ignore this non-security code
    fprintf (fp, "%s", msg);
	fclose (fp);
    /* make sure to fflush the output buffer! */
    fflush (NULL);
    return 1;
}

/*-----------------------------------------------------------------------------
  For use with glpk.
  Prints any error from glpk.
  Usage:
  glp_error_hook(callback_error_glpk, NULL);
  ---------------------------------------------------------------------------*/
static void callback_error_glpk(void *info)
{
	FILE * fp;
    info = info; /* for lint */
	fp = fopen ("c:\\callback_error_glpk.txt", "a+");  // nosemgrep  // ignore this non-security code
    fprintf (fp, "%s", "callback_error_glpk was called");
	fclose (fp);
    /* make sure to fflush the output buffer! */
    fflush (NULL);
}

#define PrintGlobal(X)
#ifdef NEVERTOBEDEFINED
static void PrintGlobal (char * msg) {
    int i;
	static char s[1001];
	static char t[101];

    sprintf (s, "%s Size=%d Count=%d\n", msg, gProblemTableSize, gActiveProblemCount);

	if (glpkpbtab == NULL) {
        sprintf (t, "glpkpbtab is null");
		strcat (s, t);
    }
    else {
        for (i = 1; i < gProblemTableSize; i++) {
            if (glpkpbtab[i] != NULL) {
                sprintf (t, "%d ", i);
				strcat (s, t);
            }
        }
    }
    //sprintf (t, "\n");
	//strcat (s, t);
	callback_print_glpk (NULL, s);
}
#endif

/*-----------------------------------------------------------------------------
  Add empty rows and columns in a problem object.
  - Only the first NumEdits edits equations are used
  - Row bounds are set
  - Columns (variables) bounds are set to 'free'
  - Sense (LPI_EQ_TYPE_EQUAL or LPI_EQ_TYPE_LT) is set
  - Row names are set
  - Columns names are set
  - Objective function is set to zero
  - ONLY MATRIX COEFFICIENTS ARE NOT SET
  ---------------------------------------------------------------------------*/
int LPI_AddEmptyRowsAndCols(int PbNb, EIT_EDITS *Edits, int NumEdits,
    char *errmsg)
{
    int rc = LPI_SUCCESS;
    int i;

    if (check_number_edits_and_cols(Edits, NumEdits, errmsg) != LPI_SUCCESS){
        rc = LPI_FAIL;
        goto TERMINATE;
    }

    glp_add_rows(glpkpbtab[PbNb], NumEdits);

    /*sets the 'b' of Ax <= b */
    for (i = 0; i < Edits->NumberofInequalities; i++){
        if (i > NumEdits - 1)
            break;
        glp_set_row_bnds(glpkpbtab[PbNb], i+1, LPX_UP - LPX_FR + GLP_FR, 0.0,
            Edits->Coefficient[i][Edits->NumberofColumns - 1]);
    }

    /* sets the 'b' of Ax = b */
    for (i = Edits->NumberofInequalities; i < Edits->NumberofEquations; i++){
        if (i > NumEdits - 1)
            break;
        glp_set_row_bnds(glpkpbtab[PbNb], i+1, LPX_FX - LPX_FR + GLP_FR,
            Edits->Coefficient[i][Edits->NumberofColumns - 1],
            Edits->Coefficient[i][Edits->NumberofColumns - 1]);
    }

    glp_add_cols(glpkpbtab[PbNb], Edits->NumberofColumns - 1);

    /*Since the actual bounds are specified in the Edits,
        let the variables be 'free'. i.e. unbounded above and below.
        Otherwise, it seems the default ub AND lb == 0 */
    for (i = 0; i < Edits->NumberofColumns - 1; i++){
        glp_set_col_bnds(glpkpbtab[PbNb], i+1, LPX_FR - LPX_FR + GLP_FR, 0.0, 0.0);
    }

TERMINATE:

	return rc;
}

/*-----------------------------------------------------------------------------
  Prints any output from the optimizer.
  Arguments: - Problem object number
             - String for eventual error message to return
  Returns zero if success, non-zero if fails
  ---------------------------------------------------------------------------*/
int LPI_CallbackPrint(char *errmsg)
{
    errmsg[0] = '\0';
    glp_term_hook(callback_print_glpk, NULL);
    glp_error_hook(callback_error_glpk, NULL);
	return LPI_SUCCESS;
}

/*-----------------------------------------------------------------------------
  Silences any output from the optimizer.
  Arguments: - String for eventual error message to return (not used)
  Returns zero if success, non-zero if fails
  ---------------------------------------------------------------------------*/
int LPI_CallbackSilence(char *errmsg)
{
    errmsg[0] = '\0';
	glp_term_out (GLP_OFF);
    //glp_term_hook(NULL, NULL);
    //glp_error_hook(NULL, NULL);
    return LPI_SUCCESS;
}

/*-----------------------------------------------------------------------------
  Calls the appropriate solver to create a problem, and
  returns the ordinal number (>= 1) of the created problem. Returns zero if
  no problem could be created.
  arguments:
  errmsg: to return an error message
  ---------------------------------------------------------------------------*/
int LPI_CreateProb(char *errmsg)
{
    int i;
    int PbNb = 0; /* Will contain the number of the problem object created*/

    PbNb = GetFirstProblemTableNullEntry ();

    if (PbNb > (gProblemTableSize - 1)){
        /* pb table is full, reallocate it */
        gProblemTableSize = gProblemTableSize + INITIAL_PB_TABLE_SIZE;
        glpkpbtab = realloc(glpkpbtab, gProblemTableSize * sizeof *glpkpbtab);
        if (glpkpbtab == NULL){
            sprintf(errmsg, "LPI_CreateProb: " MsgMemoryError "\n");
            return 0;
        }
        /* make sure to initialize the added space */
        for (i=gProblemTableSize-INITIAL_PB_TABLE_SIZE;i<gProblemTableSize;i++)
            glpkpbtab[i] = NULL;
   }

    glpkpbtab[PbNb] = lpx_create_prob();
    if (glpkpbtab[PbNb] == NULL){
        sprintf(errmsg, "LPI_CreateProb: " MsgFailure "\n", "lpx_create_prob");
        return 0;
    }

    /* Increment the active problems count*/
    gActiveProblemCount++;

	PrintGlobal ("out CreateProb");
    return PbNb;
}

/*-----------------------------------------------------------------------------
  Calls the appropriate solver to delete a problem, previously
  created by LPI_CreateProb.
  Returns zero if successs, and non-zero if fails.
  ---------------------------------------------------------------------------*/
int LPI_DeleteProb(int PbNb, char *errmsg)
{
    errmsg[0] = '\0';
    lpx_delete_prob(glpkpbtab[PbNb]);
    glpkpbtab[PbNb] = NULL;
    gActiveProblemCount--;
	PrintGlobal ("out DeleteProb");
    return LPI_SUCCESS;
}

/*-----------------------------------------------------------------------------
  Deletes a row from the problem object.
  Arguments: - Problem object number
             - row is the indice (zero-based) of the row to delete
             - String for eventual error message to return
  Returns zero if success, non-zero if fails
  ---------------------------------------------------------------------------*/
int LPI_DeleteRow(int PbNb, int row, char *errmsg)
{
    int num[2];
    errmsg[0] = '\0';
    num[1] = row+1; /* glpk indices are 1-based */
    glp_del_rows(glpkpbtab[PbNb], 1, num);
    return LPI_SUCCESS;
}

/*-----------------------------------------------------------------------------
  Returns 1 if glpkpbtab == NULL, 0 otherwise.
  ---------------------------------------------------------------------------*/
int LPI_glpkpbtab_is_NULL(void)
{
    return (glpkpbtab == NULL);
}

/*-----------------------------------------------------------------------------
  Calls the appropriate solver to free the environment
  for the LP problem. Returns 0 if success. If fails, returns a non-zero
  writes an error message in the string errmsg.
  ---------------------------------------------------------------------------*/
int LPI_FreeLpEnv(char *errmsg)
{
    free(glpkpbtab);
    glpkpbtab = NULL;
    gProblemTableSize = 0;
    gActiveProblemCount = 0;

    errmsg[0] = '\0';
    glp_free_env();
	PrintGlobal ("out  FreeLpEnv");
    return LPI_SUCCESS;
}

/*-----------------------------------------------------------------------------
  Gets the non-zero elements of the row row (zero-based).
  nz: will contain the number of non-zeros elements for the row
  colind: will contein the column indices
  val: will contain the values
  Arrays colind and val must have at least a size of nz+1.
  (thus ncols+1 if there is no zero-coefficient in the row)
  Returns zero if success and non-zero if fails.
  ---------------------------------------------------------------------------*/
int LPI_GetMatRow(int PbNb, int row, int *nz, int *colind, double *val,
    char *errmsg)
{
    int rc = LPI_SUCCESS;
    int j;
    errmsg[0] = '\0';
    *nz = glp_get_mat_row(glpkpbtab[PbNb], row+1, colind, val);
    for (j = 0; j < *nz; j++){
       colind[j] = colind[j+1];
       val[j] = val[j+1];
    }
    return rc;
}

/*-----------------------------------------------------------------------------
  Gets the number of columns (variables) in the problem object.
  If success, returns the number of columns;
  If fails, writes an error message and return a non-zero negative integer
  ---------------------------------------------------------------------------*/
int LPI_GetNumCols(int PbNb, char *errmsg)
{
    int ncols;
    /* glpk API directely returns the number of columns.
       No way to test for error*/
    errmsg[0] = '\0';
    ncols = glp_get_num_cols(glpkpbtab[PbNb]);
    return ncols;
}

/*-----------------------------------------------------------------------------
  Gets the number of rows (excluding objective fcn) in the problem object.
  If success, returns the number of rows;
  If fails, writes an error message and return a non-zero negative integer
  ---------------------------------------------------------------------------*/
int LPI_GetNumRows(int PbNb, char *errmsg)
{
    int nrows;
    errmsg[0] = '\0';
    nrows = glp_get_num_rows(glpkpbtab[PbNb]);
    return nrows;
}

/*-----------------------------------------------------------------------------
  Gets the current value of the objective function of the problem object.
  If success, returns zero;
  If fails, writes an error message and return a non-zero negative integer
  ---------------------------------------------------------------------------*/
int LPI_GetObjVal(int PbNb, double *objvalue, char *errmsg)
{
    /* glpk API directly returns the objective value.
       No way to test for error*/
    errmsg[0] = '\0';
    *objvalue = glp_get_obj_val(glpkpbtab[PbNb]);
    return LPI_SUCCESS;
}

/*-----------------------------------------------------------------------------
  Gets the right-hand side value of a single constraint.
  Arguments: pb number, row indice (zero-based), pointer to contain
  the right-hand side value.
  If success, returns zero;
  If fails, writes an error message and return a non-zero negative integer
  ---------------------------------------------------------------------------*/
int LPI_GetRhsVal(int PbNb, int row, double *rhsvalue, char *errmsg)
{
    /* glpk API directely returns the ub value. No way to test for error*/
    errmsg[0] = '\0';
    *rhsvalue = glp_get_row_ub(glpkpbtab[PbNb], row+1);
    if (*rhsvalue == +DBL_MAX) *rhsvalue = 0.0;
    
    return LPI_SUCCESS;
}

/*-----------------------------------------------------------------------------
  Change an equality to equality, for the constraint having the
  index row (zero-based).
 - sense is set LPI_EQ_TYPE_LT for <= or LPI_EQ_TYPE_EQUAL for =
  If success, returns zero;
  If fails, writes an error message and return a non-zero negative integer
  ---------------------------------------------------------------------------*/
int LPI_InequalityToEquality (int PbNb, int row, char *errmsg)
{
    /* glp_set_row_bnds is a void function, so no way to know if it fails.
       we just return success */
	double ub;
    ub = glp_get_row_ub(glpkpbtab[PbNb], row+1);
	if (ub == +DBL_MAX) ub = 0.0;
    glp_set_row_bnds(glpkpbtab[PbNb], row+1, LPX_FX - LPX_FR + GLP_FR, ub, ub);
    return LPI_SUCCESS;
}

/*-----------------------------------------------------------------------------
  Calls the appropriate solver to initilize the environment for the LP problem.
  Returns:
  If success, returns zero and pointers to name and version of the LP package
  If fails, returns non-zero and pointers to name and version
    of the LP package, if available (or NULL pointer(s) if not available)
  ---------------------------------------------------------------------------*/
int LPI_InitLpEnv(char **lpname, char **lpversion)
{
    static char namestr[LP_NAME_MAX_SIZE];
    static char versionstr[LP_VERSION_MAX_SIZE];

    int i;

    /* explicitely initialize the glpk environment */

    //rene:lib_set_ptr(NULL);

    strcpy(namestr, "GLPK");
    *lpname = namestr;

    strcpy(versionstr, GLPKVERSION);
    *lpversion = versionstr;

    gActiveProblemCount = 0;
    gProblemTableSize = INITIAL_PB_TABLE_SIZE;
    glpkpbtab = malloc (gProblemTableSize * sizeof *glpkpbtab);
    if (glpkpbtab == NULL){
        return LPI_FAIL;
    }
    /* make sure to initialize the new space */
    for (i = 0; i < gProblemTableSize; i++)
        glpkpbtab[i] = NULL;
	PrintGlobal ("out  InitLpEnv");
    return LPI_SUCCESS;
}

/*-----------------------------------------------------------------------------
  Makes the Edits in the EIT_EDITS structure the constraints in the
  lp problem. Uses the first 'NumEdits'.
  Return: zero if success, non-zero if fails.
  ---------------------------------------------------------------------------*/
int LPI_MakeMatrix(int PbNb, EIT_EDITS *Edits, int NumEdits, char *errmsg)
{
    int    j;        /*counter for columns */
    int    nz = 0;   /*counter for non-zero matrix elements*/
    int    nztotal;  /*total number of non-zero matrix elements,
                      exclude constants and objective function coefficients*/
    int    rc = LPI_SUCCESS;
    /*Note: 0 indices not allowed in GLPK*/
    double *a = NULL;
    int    *cn = NULL;
    int    k;
    int    *rn = NULL;

    if (LPI_AddEmptyRowsAndCols(PbNb, Edits, NumEdits, errmsg) != LPI_SUCCESS){
        return LPI_FAIL;
    }

    nztotal = countnonzeroelements(Edits, NumEdits);
    if (nztotal < 1){
        sprintf(errmsg, "LPI_MakeMatrix: " MsgNoNonZeroElements "\n");
        return LPI_FAIL;
    }

    rn = malloc ((nztotal+1) * sizeof *rn);
    cn = malloc ((nztotal+1) * sizeof *cn);
    a = malloc ((nztotal+1) * sizeof *a);
    if ((rn == NULL) || (cn == NULL) || (a == NULL)){
        sprintf(errmsg, "LPI_MakeMatrix: " MsgMemoryError "\n");
        rc = LPI_FAIL;
        goto TERMINATE;
    }
/* This is the old code
    for (j = 0; j < NumEdits; j++){
        for (k = 0; k < Edits->NumberofColumns - 1; k++){
            if (!EIM_DBL_EQ(Edits->Coefficient[j][Edits->NumberofColumns-2-k],
                    0.0)){
                rn[nz+1] = j+1;
                cn[nz+1] = k+1;
                a[nz+1] = Edits->Coefficient[j][Edits->NumberofColumns-2-k];
                nz++;
            }
        }
    }
end of old code */

/* This is the new code */
    for (j = 0; j < NumEdits; j++){
        for (k = 0; k < Edits->NumberofColumns - 1; k++){
            if (!EIM_DBL_EQ(Edits->Coefficient[j][k], 0.0)){
                rn[nz+1] = j+1;
                cn[nz+1] = k+1;
                a[nz+1] = Edits->Coefficient[j][k];
                nz++;
            }
        }
    }
/* end of new code */


    glp_load_matrix(glpkpbtab[PbNb], nz, rn, cn, a);

TERMINATE:
    if (rn != NULL)
        free(rn);
    if (cn != NULL)
        free(cn);
    if (a != NULL)
        free(a);

    return rc;
}

/*-----------------------------------------------------------------------------
  Maximizes the objective function for problem number PbNb.
  Arguments:
    iterationlimit: Max number of iterations (negative if no limit)
    timelimit: time limit in seconds (negative if no limit)
    errmsg: string that will contain an error message if any
NOTE: iterationlimit and timelimit are not currently used (default values
of each LP package are used).
  Return values are:
    LPI_LP_OPT        optimal solution found
    LPI_LP_UNBND      solution is unbounded
    LPI_LP_FEAS       solution is feasible
    LPI_LP_IT_LIM     premature end, max number of iteraions exceeded
    LPI_LP_TIME_LIM   premature end, time exceeded
    LPI_LP_OTHER_ERR  abnormal end, for other reason
    LPI_LP_NOOPT_NOR_FEAS_NOR_UNBND not optimum nor feasible nor unbounded
  ---------------------------------------------------------------------------*/
int LPI_MaximizeObj(int PbNb, int iterationlimit, int timelimit, char *errmsg)
{
    glp_set_obj_dir(glpkpbtab[PbNb], LPX_MAX - LPX_MIN + GLP_MIN);
    return optimize(PbNb, iterationlimit, timelimit, LPI_OPT_SENSE_MAX, errmsg);
}

/*-----------------------------------------------------------------------------
  Minimizes the objective function for problem number PbNb.
  Arguments:
    iterationlimit: Max number of iterations (negative if no limit)
    timelimit: time limit in seconds (negative if no limit)
    errmsg: string that will contain an error message if any
NOTE: iterationlimit and timelimit are not currently used (default values
of each LP package are used).
  Return values are:
    LPI_LP_OPT        optimal solution found
    LPI_LP_UNBND      solution is unbounded
    LPI_LP_FEAS       solution is feasible
    LPI_LP_IT_LIM     premature end, max number of iteraions exceeded
    LPI_LP_TIME_LIM   premature end, time exceeded
    LPI_LP_OTHER_ERR  abnormal end, for other reason
    LPI_LP_NOOPT_NOR_FEAS_NOR_UNBND not optimum nor feasible nor unbounded
  ---------------------------------------------------------------------------*/
int LPI_MinimizeObj(int PbNb, int iterationlimit, int timelimit, char *errmsg)
{
    glp_set_obj_dir(glpkpbtab[PbNb], LPX_MIN - LPX_MIN + GLP_MIN);
    return optimize(PbNb, iterationlimit, timelimit, LPI_OPT_SENSE_MIN, errmsg);
}

/*-----------------------------------------------------------------------------
  Reads the problem contained in the file fname, to populate the problem object
  number PbNb. The problem object PbNb must have been already created using
  LPI_CreateProb.
  If success, returns zero; If fail, writes an error message and return non-zero
  The problem file must be in CPLEX LP format.
  ---------------------------------------------------------------------------*/
int LPI_ReadProbLpt(int PbNb, char* fname, char *errmsg)
{
	glpkpbtab[PbNb] = lpx_create_prob();
	if (glp_read_lp(glpkpbtab[PbNb], NULL, fname))
         lpx_delete_prob(glpkpbtab[PbNb]), glpkpbtab[PbNb] = NULL;
    if (glpkpbtab[PbNb] == NULL){
        sprintf(errmsg, "LPI_ReadProbLpt: " MsgFailure "\n", "glp_read_lp");
        return LPI_FAIL;
    }
    return LPI_SUCCESS;
}

/*-----------------------------------------------------------------------------
  Scale a LP problem data. To call if one thinks there may be numerical 
  difficulties, ex. matrix coef. too big with other too small
  Arguments: - Problem object number
             - String for eventual error message to return
  Returns zero if success, non-zero if fails
  ---------------------------------------------------------------------------*/
int LPI_ScaleProb(int PbNb, char *errmsg)
{
    int rc = LPI_SUCCESS;
    lpx_scale_prob(glpkpbtab[PbNb]);
    return rc;
}



/*-----------------------------------------------------------------------------
  Sets or changes a real control parameter.
  Arguments: - Problem object number
             - control type (currently supported types are:
               Optimality tolerence LPI_OPT_TOL_TYPE and feasibility
               tolerance LPI_FEAS_TOL_TYPE
             - control value (currently supported:
               Optimality tolerence LPI_OPT_TOL_VAL and feasibility
               tolerance LPI_FEAS_TOL_VAL)
             - String for eventual error message to return
  Returns zero if success, non-zero if fails
  ---------------------------------------------------------------------------*/
int LPI_SetControlParmReal(int PbNb, int controltype,
    double controlvalue, char *errmsg)
{
    /* nothing to do */
    int rc = LPI_SUCCESS;
    PbNb = PbNb;
    controltype = controltype;
    controlvalue = controlvalue;
    errmsg[0] = '\0';
    return rc;
}

/*-----------------------------------------------------------------------------
  Set or change a complete row in the problem object..
 - row is the row indice, and is zero-based
 - nz is the total number of non-zero coefficients in the row
 - ndx stores the column indices, starting from ndz[0], and these
   indices are zero-based.
 - val stores the coefficient values, starting from val[0]
 - sense is set LPI_EQ_TYPE_LT for <= or LPI_EQ_TYPE_EQUAL for =
 - rhs is the value of the row bound (right hand side value of equation)
 - ilperrmsg will return and eventual error message
 - Returns zero if success or non-zero if fails
  ---------------------------------------------------------------------------*/
int LPI_SetMatRow(int PbNb, int row, int nz, int *ndx, double *val,
   char sense, double rhs, char *errmsg)
{
    int *collist = NULL;
    double *vallist = NULL;
    int rc = LPI_SUCCESS;
    int j;
    collist = malloc((nz+1)* sizeof *collist);
    vallist = malloc((nz+1)* sizeof *vallist);
    if ((collist == NULL) || (vallist == NULL)){
        sprintf(errmsg, "LPI_SetMatRow: " MsgMemoryError "\n");
        rc = LPI_FAIL;
        goto TERMINATE;
    }
     /* Translate data because glpk uses one-based indices*/
    for (j = 0; j < nz; j++){
         collist[j+1] = ndx[j]+1;
         vallist[j+1] = val[j];
    }

    /* Sets the matrix row */
    glp_set_mat_row(glpkpbtab[PbNb], row+1, nz, collist, vallist);
    if (sense == LPI_EQ_TYPE_LT){
        glp_set_row_bnds(glpkpbtab[PbNb], row+1, LPX_UP - LPX_FR + GLP_FR, 0.0, rhs);
    }
    else if (sense == LPI_EQ_TYPE_EQUAL){
        glp_set_row_bnds(glpkpbtab[PbNb], row+1, LPX_FX - LPX_FR + GLP_FR, rhs, rhs);
    }
    else {
        sprintf(errmsg, "LPI_SetMatRow: " MsgInvalidConstraintSense "\n",
            LPI_EQ_TYPE_LT, LPI_EQ_TYPE_EQUAL);
        rc = LPI_FAIL;
        goto TERMINATE;
    }
TERMINATE:
    if (collist != NULL)
        free(collist);
    if (vallist != NULL)
        free(vallist);
    return rc;
}

/*-----------------------------------------------------------------------------
  Sets or changes the objective function coefficients.
  Arguments: - Problem object number
             - number of coefficients to set or change
             - Array containing the column indices of the coefficients
               (Column indices starts from zero. For GLPK, we should add 1 to
                these indices sinces GLPK indices start at 1).
             - Array containing the values of the coefficients
             - String for eventual error message to return
  Note: fixed part of objective function is not changed by this function.
  Returns zero if success, non-zero if fails
  ---------------------------------------------------------------------------*/
int LPI_SetObjCoef(int PbNb, int cnt, int *cindices,
                     double *values, char *errmsg)
{
    int j;
    errmsg[0] = '\0';
    for (j = 0; j < cnt; j++){
        glp_set_obj_coef(glpkpbtab[PbNb], cindices[j]+1, values[j]);
    }
    return LPI_SUCCESS;
}

/*-----------------------------------------------------------------------------
  Sets or changes a single objective function coefficient.
  Arguments: - Problem object number
             - col: zero-base colunm indice, (-1 for constant part of obj fcn.
             But for GLPK, Constant part has zero as indice)
             - Value of the objective function coefficient to set
             - String for eventual error message to return
  Returns zero if success, non-zero if fails
  ---------------------------------------------------------------------------*/
int LPI_SetObjCoefSingle(int PbNb, int col, double value, char *errmsg)
{
     if ((col < -1) || (col > glp_get_num_cols(glpkpbtab[PbNb]) -1)){
        sprintf(errmsg, "LPI_SetObjCoefSingle: " MsgColumnIndexOutOfRange "\n");
        return LPI_FAIL;
     }
     /*col + 1 because GLPK indexes start at 1, and 0 corresponds to constant
      part of the objective function*/
     glp_set_obj_coef(glpkpbtab[PbNb], col+1, value);
    return LPI_SUCCESS;
}

/*-----------------------------------------------------------------------------
  Extracts a linear constraint from the problem object
  (but not the constant) then makes it the objective function.
  constraint row index is row (zero-based).
  If success, returns zero;
  If fails, writes an error message and return a non-zero negative integer
  ---------------------------------------------------------------------------*/
int LPI_SetObjRow(int PbNb, int row, char *errmsg)
{
    int ncols = 0;
    int nzcnt;
    int *rmatind = NULL; /* coulumn index */
    double *rmatval = NULL; /* values */
    int rc = LPI_SUCCESS;

    int j;
    /* First extract the row*/
    ncols = glp_get_num_cols(glpkpbtab[PbNb]);
    rmatind = malloc((ncols+1)* sizeof *rmatind);
    rmatval = malloc((ncols+1)* sizeof *rmatval);
    if ((rmatind == NULL) ||(rmatval == NULL)){
        sprintf(errmsg, "LPI_SetObjRow: " MsgMemoryError "\n");
        rc = LPI_FAIL;
        goto TERMINATE;
    }

    nzcnt = glp_get_mat_row(glpkpbtab[PbNb], row+1, rmatind, rmatval);

    /* Initializes the objective function to zero*/
    if (LPI_SetObjZero(PbNb, errmsg) != LPI_SUCCESS){
        sprintf(errmsg, "LPI_SetObjRow: " MsgFailure "\n", "LPI_SetObjZero");
        rc = LPI_FAIL;
        goto TERMINATE;
    }

    /* Now make the extracted row the objective function*/
    for (j = 1; j <= nzcnt; j++){
        glp_set_obj_coef(glpkpbtab[PbNb], rmatind[j], rmatval[j]);
    }

TERMINATE:

	if (rmatind != NULL)
        free(rmatind);
    if (rmatval != NULL)
        free(rmatval);
    return rc;
}

/*-----------------------------------------------------------------------------
  Sets the objective function to zero
  Returns zero if success, non-zero if fails
  ---------------------------------------------------------------------------*/
int LPI_SetObjZero(int PbNb, char *errmsg)
{
    int j;
    int cur_numcols;
    int rc = LPI_SUCCESS;
    errmsg[0] = '\0';
    cur_numcols = glp_get_num_cols(glpkpbtab[PbNb]);
    for (j = 0; j <= cur_numcols; j++){
        /* Constant part is set to zero when j = 0, in GLPK */
        glp_set_obj_coef(glpkpbtab[PbNb], j, 0.0);
    }
    return rc;
}

/*-----------------------------------------------------------------------------
  Sets the sense of optimization: minimization or maximization.
  sense is 1 for maximization and 2 for minimization.
  Returns zero if success, non-zero if fails.
  ---------------------------------------------------------------------------*/

int LPI_SetOptimizationSense(int PbNb, int optsense, char *errmsg)
{
    if (optsense == LPI_OPT_SENSE_MAX)
        glp_set_obj_dir(glpkpbtab[PbNb], LPX_MAX - LPX_MIN + GLP_MIN);
    else if (optsense == LPI_OPT_SENSE_MIN)
        glp_set_obj_dir(glpkpbtab[PbNb], LPX_MIN - LPX_MIN + GLP_MIN);
    else {
        sprintf(errmsg,
            "LPI_SetOptimizationSense: " MsgInvalidOptimizationSense "\n",
            optsense, LPI_OPT_SENSE_MAX, LPI_OPT_SENSE_MIN);
        return LPI_FAIL;
    }
    return LPI_SUCCESS;
}

/*-----------------------------------------------------------------------------
  Write a problem object data, in CPLEX LP format. If success,
  returns zero; If fail, writes an error message and return non-zero
  ---------------------------------------------------------------------------*/
int LPI_WriteLpt(int PbNb, char* fname, char *errmsg)
{
    if (glp_write_lp(glpkpbtab[PbNb], NULL, fname) != 0){
        sprintf(errmsg, "LPI_WriteLpt: " MsgFailure "\n", "glp_write_lp");
        return LPI_FAIL;
    }
    return LPI_SUCCESS;
}

/*************** End of functions published by the interface  ****************/




/***** Static functions used in this module: start (alphabetical order) *****/


/*-----------------------------------------------------------------------------
  Checks if the number of edits and columns (variables) are valid.
  Returns 0 if valid, non-zero and an error message if not.
  This check is performed before attemting to build a problem with the edits
  supplied by the user of the interface.
  ---------------------------------------------------------------------------*/
static int check_number_edits_and_cols(EIT_EDITS *Edits, int NumEdits, char *errmsg)
{
    if (NumEdits > Edits->NumberofEquations){
        sprintf(errmsg,
            "LPI_AddEmptyRowsAndCols: " MsgTooManyEditsRequested "\n",
            NumEdits, Edits->NumberofEquations);
        return LPI_FAIL;
    }

    if (NumEdits < 1){
        sprintf(errmsg, "LPI_AddEmptyRowsAndCols: " MsgNoEdits "\n", NumEdits);
        return LPI_FAIL;
    }

    if ((Edits->NumberofColumns - 1) < 1){
        sprintf(errmsg, "LPI_AddEmptyRowsAndCols: " MsgNoVariables "\n",
            Edits->NumberofColumns - 1);
        return LPI_FAIL;
    }

    return LPI_SUCCESS;
}

/*-----------------------------------------------------------------------------
  Determines the total number of nonzeros matrix elements in the
  equations (righ-hand sides and objective function not included).
  ---------------------------------------------------------------------------*/
static int countnonzeroelements(EIT_EDITS *Edits, int NumEdits)
{
    int j;
    int k;
    int nzcount = 0;
    for (j = 0; j < NumEdits; j++){
        for (k = 0; k < Edits->NumberofColumns - 1; k++){
            if (Edits->Coefficient[j][k] != 0.0)
                nzcount++;
        }
    }
    return nzcount;
}

/*-----------------------------------------------------------------------------
  Find the index of the first null entry in pbtab.
  ---------------------------------------------------------------------------*/
static int GetFirstProblemTableNullEntry (void) {
    int i;

    for (i = 1; i < gProblemTableSize; i++) {
        if (glpkpbtab[i] == NULL) return i;
    }
    return i;
}

/*-----------------------------------------------------------------------------
  Performs the optimization for the lp problem
  (For CPLEX and GLPK, a unique routine is called for
  maximization and minimization, after the optimization sense has been set.
  However for XPRESS, there are 2 distincts routines: XPRSmaxim and XPRSminim).
NOTE: iterationlimit and timelimit are not currently used (default values
of each LP package are used).
  ---------------------------------------------------------------------------*/
static int optimize(int PbNb, int iterationlimit, int timelimit,
                       int optsense, char *errmsg)
{
    int solverrc;
    int solstatus;
	glp_smcp parm;

	iterationlimit = iterationlimit; /* for lint */
    timelimit = timelimit; /* for lint */
    optsense = optsense; /* for lint */

    /* Added to solve a problem where under certain conditions
    optimize would loop infinitely because of 'numerical instability' */

    lpx_set_int_parm(glpkpbtab[PbNb], LPX_K_SCALE, 2);

    lpx_scale_prob(glpkpbtab[PbNb]);

    /* for debug purposes */
    /* glp_write_lp(glpkpbtab[PbNb], "Lastprob.txt");*/
	
    fill_smcp(glpkpbtab[PbNb], &parm);
    solverrc = glp_simplex(glpkpbtab[PbNb], &parm);

    /* Checks if the simplex failed because of invalid initial basis
       (LPX_E_BADB) or unable to start the search (LPX_E_FAULT).
       This may be the case if for example the constraint matrix
       is changed (between 2 calls to lpx_simplex) so that it becomes singular.
       In this case, Andrew Makhorin recommands to provide the solver with
       a valid initial basis by using the API routines lpx_std_basis or
       lpx_adv_basis, then calls the simplex again. Here we use lpx_std_basis.*/
    if ((solverrc == GLP_EBADB) || (solverrc == GLP_ESING) || (solverrc == GLP_ECOND) || (solverrc == GLP_EBOUND)){
        glp_std_basis(glpkpbtab[PbNb]);
        fill_smcp(glpkpbtab[PbNb], &parm);
        solverrc = glp_simplex(glpkpbtab[PbNb], &parm);
    }

	/*sprintf(errmsg, "Return code from glp_simplex: %d \n", solverrc);*/

    switch (solverrc){
    case 0:
    case GLP_ENOPFS:
    case GLP_ENODFS:
         break;
    case GLP_EBOUND:
    case GLP_EOBJLL:
    case GLP_EOBJUL:
    case GLP_EFAIL:
    case GLP_EBADB:
        sprintf(errmsg, "GLPK: " MsgUnableToSolve "\n");
        return LPI_LP_OTHER_ERR;
    case GLP_EITLIM:
        sprintf(errmsg, "GLPK: " MsgIterationsLimitExceeded "\n");
        return LPI_LP_IT_LIM;
    case GLP_ETMLIM:
        sprintf(errmsg, "GLPK: " MsgTimeLimitExceeded "\n");
        return LPI_LP_TIME_LIM;
    default:
        break;
    } /*switch*/

    solstatus = glp_get_status(glpkpbtab[PbNb]);

    switch (solstatus){
    case GLP_OPT:
		sprintf(errmsg, "GLPK:Optimal Solution " MsgBlank "\n");
        return LPI_LP_OPT;
    case GLP_UNBND:
		sprintf(errmsg, "GLPK:Unbounded Solution " MsgBlank "\n");
        return LPI_LP_UNBND;
    case GLP_FEAS:
		sprintf(errmsg, "GLPK:Feasible Solution " MsgBlank "\n");
        return LPI_LP_FEAS;
    default:
		sprintf(errmsg, "GLPK:NO_OPT_NOR_FEAS_NOR_UNBND " MsgBlank "\n");
        return LPI_LP_COMPLETED_NO_OPT_NOR_FEAS_NOR_UNBND;
    } /*switch*/
}
