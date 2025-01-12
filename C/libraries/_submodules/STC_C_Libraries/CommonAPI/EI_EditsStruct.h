#ifndef EI_EDITSSTRUCT_H
#ifndef EDITS_DISABLED
#define EI_EDITSSTRUCT_H

/*
EI_EDITS - Linear equations/Equations lin�aires
This structure is used to hold the data representing the linear equations.
The edits are sorted. The active inequalities appear first,
than the active equalities, and finally the inactive equations.

Cette structure sert � garder l'information sur les �quations lin�aires.
Les r�gles sont ordonn�es. Les inegalit�s actives sont plac�es en premier,
suivi des �galites actives, puis finalement les �quations inactives.

Structure variables / Variables de la structure:
int    *  EditId;               Identifier of edits
                                Identificateur d'equation

char   ** FieldName;            Names of Edits fields
                                Noms des champs des �quations

double ** Coefficient;          Field Coefficients
                                Coefficients des champs

int       NumberofEquations;    Number of active linear Equations
                                Nombre d'�quations lin�aires actives

int       NumberofInequalities; Number of active Inequalities
                                Nombre d'in�galit�s actives

int       NumberofColumns;      Number of Columns (including constant)
                                Nombre de colonnes (incluant la constante)

OBSOLETE
int       NumberofRows;         Total number of Linear Equations
                                (active + inactive)
                                a row becomes inactive when it is dropped
                                in EI_EditsDropEquation().
                                Nombre total d'�quations lin�aires
                                (actives + inactives)
                                une equation devient inactive lorsqu'elle
                                est "dropp�" dans la fonction
                                EI_EditsDropEquation().
*/
typedef struct {
    int     * EditId;
    char   ** FieldName;
    double ** Coefficient;
    int       NumberofEquations;
    int       NumberofInequalities;
    int       NumberofColumns;
/*    int       NumberofRows;*/
} EIT_EDITS;

#endif
#endif
