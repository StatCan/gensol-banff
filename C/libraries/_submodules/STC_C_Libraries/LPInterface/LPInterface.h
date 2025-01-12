#ifndef LPINTERFACE_H
#define LPINTERFACE_H

/*!
@mainpage LPI - Linear Programming Interface Documentation
A set of functions used to do LP.
*/

/*!
@defgroup LPI LPI
@{
*/

/****************************************************************************/
/* This header file contains prototypes and definitions for functions       */
/* that are used by the APIs to interact with linear programming (lp)       */
/* modules.                                                                 */
/****************************************************************************/

#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "EI_EditsStruct.h"

/* The following block is to facilitate the export and import
 of functions in DLL, under windows */

#if defined (_EXPORTING) /* when building the DLL for Windows */
    #define CLASS_DECLSPEC __declspec(dllexport)
#elif defined (_IMPORTING) /* when importing the DLL for Windows */
    #define CLASS_DECLSPEC __declspec(dllimport)
#else  /* Under Unix: use the usual extern keyword for non static functions*/
    #define CLASS_DECLSPEC extern
#endif

/* Optimization senses, Maximization or Minimization*/
#define LPI_OPT_SENSE_MAX -1
#define LPI_OPT_SENSE_MIN 1

#define LPI_EQ_TYPE_EQUAL 'E'
#define LPI_EQ_TYPE_LT 'L'

/* General return codes of the routines of this interface */
#define LPI_FAIL    -2
#define LPI_SUCCESS  0

/* Optimality and feasibility tolerances for the optimizers
  Note: Too small values may lead optimizers to conlude that there is
   no (optimal) solution!*/

#define LPI_OPT_TOL_TYPE 1
#define LPI_OPT_TOL_VAL 1.0e-6
#define LPI_FEAS_TOL_TYPE 2
#define LPI_FEAS_TOL_VAL  1.0e-6


/* Status of the solution returned by this interface
   after the optimizer has been called to solve the problem */
#define LPI_LP_OPT                    1 /* Completed: Optimum found */
#define LPI_LP_UNBND                  2 /* Completed: Unbounded solution */
#define LPI_LP_FEAS                   3 /* Completed: Feaseable solution */
#define LPI_LP_IT_LIM                 4 /* Halted: number of iterations limit*/
#define LPI_LP_TIME_LIM               5 /* Halted: Time limit*/
#define LPI_LP_OTHER_ERR              6 /* Halted: Other error */
#define LPI_LP_COMPLETED_NO_OPT_NOR_FEAS_NOR_UNBND 7 /*Completed: no solution*/

/**********************************************************************/
/* Routine of the interface are listed below, in alphabetical order   */
/**********************************************************************/

/*!
@defgroup LPI_AddEmptyRowsAndCols LPI_AddEmptyRowsAndCols()
@brief Add empty rows and columns in a problem object.

 - Only the first @c NumberOfEdits edits equations are used.
 - Row bounds are set.
 - Columns (variables) bounds are set to 'free'.
 - Sense is set (LPI_EQ_TYPE_LT '<=' or LPI_EQ_TYPE_EQUAL '='.)
 - Row names are set.
 - Columns names are set.
 - Objective function is set to zero.

@note Matrix coefficients are not set.
@return LPI_SUCCESS on success, LPI_FAIL otherwise.
@{
*/
CLASS_DECLSPEC int LPI_AddEmptyRowsAndCols (
        /*! Problem Number. */
    int ProblemNumber,
        /*! The Edits. */
    EIT_EDITS * Edits,
        /*! Number of Edits. */
    int NumberOfEdits,
        /*! Will contain an error message, if any. */
    char * ErrorMessage);
/*@}*/

/*!
@defgroup LPI_CallbackPrint LPI_CallbackPrint()
@brief Prints any output from the optimizer.
@return LPI_SUCCESS on success, LPI_FAIL otherwise.
@{
*/
CLASS_DECLSPEC int LPI_CallbackPrint (
        /*! Will contain an error message, if any. */
    char * ErrorMessage);
/*@}*/

/*!
@defgroup LPI_CallbackSilence LPI_CallbackSilence()
@brief Silences any output from the optimizer.
@return LPI_SUCCESS on success, LPI_FAIL otherwise.
@{
*/
CLASS_DECLSPEC int LPI_CallbackSilence (
        /*! Will contain an error message, if any. */
    char * ErrorMessage);
/*@}*/

/*!
@defgroup LPI_CreateProb LPI_CreateProb()
@brief Calls the appropriate solver to create a problem.
@return The ordinal number of the created problem, zero if no problem could be created.
@{
*/
CLASS_DECLSPEC int LPI_CreateProb (
        /*! Will contain an error message, if any. */
    char * ErrorMessage);
/*@}*/

/*!
@defgroup LPI_DeleteProb LPI_DeleteProb()
@brief Calls the appropriate solver to delete a problem, previously created by
LPI_CreateProb.
@return LPI_SUCCESS on success, LPI_FAIL otherwise.
@{
*/
CLASS_DECLSPEC int LPI_DeleteProb (
        /*! Problem Number. */
    int ProblemNumber,
        /*! Will contain an error message, if any. */
    char * ErrorMessage);
/*@}*/

/*!
@defgroup LPI_DeleteRow LPI_DeleteRow()
@brief Deletes a row from the problem object.
@return LPI_SUCCESS on success, LPI_FAIL otherwise.
@{
*/
CLASS_DECLSPEC int LPI_DeleteRow (
        /*! Problem Number. */
    int ProblemNumber,
        /*! indice (zero-based) of the row to delete. */
    int Row,
        /*! Will contain an error message, if any. */
    char * ErrorMessage);
/*@}*/

/*!
@defgroup LPI_glpkpbtab_is_NULL LPI_glpkpbtab_is_NULL()
@brief Checks whether glpkpbtab is NULL.
@return 1 on success (glpkpbtab is NULL), 0 otherwise (glpkpbtab is not NULL).
@{
*/
CLASS_DECLSPEC int LPI_glpkpbtab_is_NULL (
        /*! Takes no arguments. */
    void);
/*@}*/

/*!
@defgroup LPI_FreeLpEnv LPI_FreeLpEnv()
@brief Calls the appropriate solver to free the environment for the LP problem.
@return LPI_SUCCESS on success, LPI_FAIL otherwise.
@{
*/
CLASS_DECLSPEC int LPI_FreeLpEnv (
        /*! Will contain an error message, if any. */
    char * ErrorMessage);
/*@}*/

/*!
@defgroup LPI_GetMatRow LPI_GetMatRow()
@brief Gets the non-zero coefficients of the row row.
@note Arrays ColumnIndices and Values must have at least a size of nz+1. (thus ncols = +1 if there is no zero-coefficient in the row)
@return LPI_SUCCESS on success, LPI_FAIL otherwise.
@{
*/
CLASS_DECLSPEC int LPI_GetMatRow (
        /*! Problem Number. */
    int ProblemNumber,
        /*! Row. */
    int Row,
        /*! will contain the number of non-zero coefficients for the row. */
    int * nz,
        /*! will contain the column indices of the non-zero coefficients. */
    int * ColumnIndices,
        /*! will contain the values. */
    double * Values,
        /*! Will contain an error message, if any. */
    char * ErrorMessage);
/*@}*/

/*!
@defgroup LPI_GetNumCols LPI_GetNumCols()
@brief Gets the number of columns (variables) in the problem object.
@return the number of columns on success, LPI_FAIL otherwise.
@{
*/
CLASS_DECLSPEC int LPI_GetNumCols (
        /*! Problem Number. */
    int ProblemNumber,
        /*! Will contain an error message, if any. */
    char * ErrorMessage);
/*@}*/

/*!
@defgroup LPI_GetNumRows LPI_GetNumRows()
@brief Gets the number of rows (excluding objective fcn) in the problem object.
@return the number of rows on success, LPI_FAIL otherwise.
@{
*/
CLASS_DECLSPEC int LPI_GetNumRows (
        /*! Problem Number. */
    int ProblemNumber,
        /*! Will contain an error message, if any. */
    char * ErrorMessage);
/*@}*/

/*!
@defgroup LPI_GetObjVal LPI_GetObjVal()
@brief Gets the current value of the objective function of the problem object.
@return LPI_SUCCESS on success, LPI_FAIL otherwise.
@{
*/
CLASS_DECLSPEC int LPI_GetObjVal (
        /*! Problem Number. */
    int ProblemNumber,
        /*! ObjValue. */
    double * ObjValue,
        /*! Will contain an error message, if any. */
    char * ErrorMessage);
/*@}*/

/*!
@defgroup LPI_GetRhsVal LPI_GetRhsVal()
@brief Gets the right-hand side value of a single constraint.
  Arguments: pb number, row indice (zero-based), pointer to contain
  the right-hand side value.
@return LPI_SUCCESS on success, LPI_FAIL otherwise.
@{
*/
CLASS_DECLSPEC int LPI_GetRhsVal (
        /*! Problem Number. */
    int ProblemNumber,
        /*! row indice (zero-based). */
    int Row,
        /*! pointer to contain the right-hand side value. */
    double * RhsValue,
        /*! Will contain an error message, if any. */
    char * ErrorMessage);
/*@}*/

/*!
@defgroup LPI_InequalityToEquality LPI_InequalityToEquality()
@brief Change an equality to equality, for the constraint having the
  index row (zero-based).
@return LPI_SUCCESS on success, LPI_FAIL otherwise.
@{
*/
CLASS_DECLSPEC int LPI_InequalityToEquality (
        /*! Problem Number. */
    int ProblemNumber,
        /*! Row. */
    int Row,
        /*! Will contain an error message, if any. */
    char * ErrorMessage);
/*@}*/

/*!
@defgroup LPI_InitLpEnv LPI_InitLpEnv()
@brief Calls the appropriate solver to initilize the environment for the LP problem.

It sets pointers to name and version of the LP package.
@return LPI_SUCCESS on success, LPI_FAIL otherwise.
@{
*/
CLASS_DECLSPEC int LPI_InitLpEnv (
        /*! The address of a string. */
    char ** LpPackageName,
        /*! The address of a string. */
    char ** LpPackageVersion);
/*@}*/

/*!
@defgroup LPI_MakeMatrix LPI_MakeMatrix()
@brief Makes the Edits in the EIT_EDITS structure the constraints in the
 lp problem. Uses the first 'NumberOfEdits'.
@return LPI_SUCCESS on success, LPI_FAIL otherwise.
@{
*/
CLASS_DECLSPEC int LPI_MakeMatrix (
        /*! Problem Number. */
    int ProblemNumber,
        /*! The Edits. */
    EIT_EDITS * Edits,
        /*! Number of Edits. */
    int NumberOfEdits,
        /*! Will contain an error message, if any. */
    char * ErrorMessage);
/*@}*/

/*!
@defgroup LPI_MaximizeObj LPI_MaximizeObj()
@brief Maximizes the objective function for problem number ProblemNumber.

@note IterationLimit and TimeLimit are not currently used (default values of
each LP package are used).

@return
\verbatim
LPI_LP_OPT          optimal solution found
LPI_LP_UNBND        solution is unbounded
LPI_LP_FEAS         solution is feasible
LPI_LP_IT_LIM       premature end, max number of iterations exceeded
LPI_LP_TIME_LIM     premature end, time exceeded
LPI_LP_OTHER_ERR    abnormal end, for other reason
LPI_LP_COMPLETED_NO_OPT_NOR_FEAS_NOR_UNBND
                    not optimum nor feasible nor unbounded
\endverbatim
@{
*/
CLASS_DECLSPEC int LPI_MaximizeObj (
        /*! Problem Number. */
    int ProblemNumber,
        /*! Maximum number of iterations (negative if no limit). */
    int IterationLimit,
        /*! Time limit in seconds (negative if no limit). */
    int TimeLimit,
        /*! Will contain an error message, if any. */
    char * ErrorMessage);
/*@}*/

/*!
@defgroup LPI_MinimizeObj LPI_MinimizeObj()
@brief Minimizes the objective function for problem number ProblemNumber.

@note IterationLimit and TimeLimit are not currently used (default values of
each LP package are used).

@return
\verbatim
LPI_LP_OPT          optimal solution found
LPI_LP_UNBND        solution is unbounded
LPI_LP_FEAS         solution is feasible
LPI_LP_IT_LIM       premature end, max number of iterations exceeded
LPI_LP_TIME_LIM     premature end, time exceeded
LPI_LP_OTHER_LIM    abnormal end, for other reason
LPI_LP_COMPLETED_NO_OPT_NOR_FEAS_NOR_UNBND
                    not optimum nor feasible nor unbounded
\endverbatim
@{
*/
CLASS_DECLSPEC int LPI_MinimizeObj (
        /*! Problem Number. */
    int ProblemNumber,
        /*! Maximum number of iterations (negative if no limit). */
    int IterationLimit,
        /*! Time limit in seconds (negative if no limit). */
    int TimeLimit,
        /*! Will contain an error message, if any. */
    char * ErrorMessage);
/*@}*/

/*!
@defgroup LPI_ReadProbLpt LPI_ReadProbLpt()
@brief Reads the problem contained in the file FileName, to populate the problem object number ProblemNumber.

The problem object ProblemNumber must have been already created using LPI_CreateProb.
@note The problem file must be in CPLEX LP format.
@return LPI_SUCCESS on success, LPI_FAIL otherwise.
@{
*/
CLASS_DECLSPEC int LPI_ReadProbLpt (
        /*! Problem Number. */
    int ProblemNumber,
        /*! File name. */
    char * FileName,
        /*! Will contain an error message, if any. */
    char * ErrorMessage);
/*@}*/


/*!
@defgroup LPI_ScaleProb LPI_ScaleProb()
@brief Scale a LP problem data. To call if one thinks there may be numerical difficulties, ex. matrix coef. too big with other too small.


@return LPI_SUCCESS on success, LPI_FAIL otherwise.
@{
*/
CLASS_DECLSPEC int LPI_ScaleProb (
        /*! Problem Number. */
    int ProblemNumber,
        /*! Will contain an error message, if any. */
    char * ErrorMessage);
/*@}*/





/*!
@defgroup LPI_SetControlParmReal LPI_SetControlParmReal()
@brief Sets or changes a real control parameter.


@return LPI_SUCCESS on success, LPI_FAIL otherwise.
@{
*/
CLASS_DECLSPEC int LPI_SetControlParmReal (
        /*! Problem Number. */
    int ProblemNumber,
        /*! control type (currently supported types are: Optimality tolerence LPI_OPT_TOL_TYPE and feasibility tolerance LPI_FEAS_TOL_TYPE.) */
    int ControlType,
        /*! control value (currently supported: Optimality tolerence LPI_OPT_TOL_VAL and feasibility tolerance LPI_FEAS_TOL_VAL.) */
    double ControlValue,
        /*! Will contain an error message, if any. */
    char * ErrorMessage);
/*@}*/

/*!
@defgroup LPI_SetMatRow LPI_SetMatRow()
@brief Set or change a complete row in the problem object.
@return LPI_SUCCESS on success, LPI_FAIL otherwise.
@{
*/
CLASS_DECLSPEC int LPI_SetMatRow (
        /*! Problem Number. */
    int ProblemNumber,
        /*! indice (zero-based) of the row to change. */
    int Row,
        /*! nz is the total number of non-zero coefficients. */
    int nz,
        /*! ndx stores the columns indices, starting from ndz[0], and these indices are zero-based. */
    int * ColumnIndices,
        /*! Values stores the coefficient values, starting from val[0]. */
    double * Values,
        /*! Sense is set (LPI_EQ_TYPE_LT '<=' or LPI_EQ_TYPE_EQUAL '='.) */
    char Sense,
        /*! RhsValue is the value of the row bound (right hand side value of equation) */
    double RhsValue,
        /*! Will contain an error message, if any. */
    char * ErrorMessage);
/*@}*/

/*!
@defgroup LPI_SetObjCoef LPI_SetObjCoef()
@brief Sets or changes the objective function coefficients.
@note fixed part of objective function is not changed by this function.
@return LPI_SUCCESS on success, LPI_FAIL otherwise.
@{
*/
CLASS_DECLSPEC int LPI_SetObjCoef (
        /*! Problem Number. */
    int ProblemNumber,
        /*! number of coefficients to set or change. */
    int Count,
        /*! Array containing the column indices of the coefficients (column indices starts from zero. */
    int * ColumnIndices,
        /*! Array containing the values of the coefficients. */
    double * Values,
        /*! Will contain an error message, if any. */
    char * ErrorMessage);
/*@}*/

/*!
@defgroup LPI_SetObjCoefSingle LPI_SetObjCoefSingle()
@brief Sets or changes a single objective function coefficient.
@return LPI_SUCCESS on success, LPI_FAIL otherwise.
@{
*/
CLASS_DECLSPEC int LPI_SetObjCoefSingle (
        /*! Problem Number. */
    int ProblemNumber,
        /*! zero-base column index (-1 for constant part of obj fcn). */
    int ColumnIndex,
        /*! Value of the objective function coefficient to set. */
    double Value,
        /*! Will contain an error message, if any. */
    char * ErrorMessage);
/*@}*/

/*!
@defgroup LPI_SetObjRow LPI_SetObjRow()
@brief Extracts a linear constraint from the problem object (but not the
constant) then makes it the objective function.
@return LPI_SUCCESS on success, LPI_FAIL otherwise.
@{
*/
CLASS_DECLSPEC int LPI_SetObjRow (
        /*! Problem Number. */
    int ProblemNumber,
        /*! constraint row index is row (zero-based). */
    int Row,
        /*! Will contain an error message, if any. */
    char * ErrorMessage);
/*@}*/

/*!
@defgroup LPI_SetObjZero LPI_SetObjZero()
@brief Sets the objective function to zero.
@return LPI_SUCCESS on success, LPI_FAIL otherwise.
@{
*/
CLASS_DECLSPEC int LPI_SetObjZero (
        /*! Problem Number. */
    int ProblemNumber,
        /*! Will contain an error message, if any. */
    char * ErrorMessage);
/*@}*/

/*!
@defgroup LPI_SetOptimizationSense LPI_SetOptimizationSense()
@brief Sets the sense of optimization: minimization or maximization.
@return LPI_SUCCESS on success, LPI_FAIL otherwise.
@{
*/
CLASS_DECLSPEC int LPI_SetOptimizationSense (
        /*! Problem Number. */
    int ProblemNumber,
        /*! sense is 1 for maximization and 2 for minimization. */
    int Sense,
        /*! Will contain an error message, if any. */
    char * ErrorMessage);
/*@}*/

/*!
@defgroup LPI_WriteLpt LPI_WriteLpt()
@brief Write a problem object data, in CPLEX LP format.
@return LPI_SUCCESS on success, LPI_FAIL otherwise.
@{
*/
CLASS_DECLSPEC int LPI_WriteLpt (
        /*! Problem Number. */
    int ProblemNumber,
        /*! File name. */
    char * FileName,
        /*! Will contain an error message, if any. */
    char * ErrorMessage);
/*@}*/

#endif
