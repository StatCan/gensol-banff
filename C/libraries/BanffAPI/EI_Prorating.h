/*
EI_PRORATING.h
*/

#ifndef EI_PRORATING_H
#define EI_PRORATING_H

#include "EI_Common.h"
#include "slist.h"

/* Prorating Method: 2 letters */
#define EIM_PRORATING_METHOD_SCALING_LETTER     's'
#define EIM_PRORATING_METHOD_BASIC_LETTER       'b'

/* Prorating Method: a string */
#define EIM_PRORATING_METHOD_SCALING_STRING     "SCALING"
#define EIM_PRORATING_METHOD_BASIC_STRING       "BASIC"

/* Modifier: a letter (parser) */
#define EIM_PRORATING_MODIFIER_ALWAYS_LETTER    'a'
#define EIM_PRORATING_MODIFIER_IMPUTED_LETTER   'i'
#define EIM_PRORATING_MODIFIER_NEVER_LETTER     'n'
#define EIM_PRORATING_MODIFIER_ORIGINAL_LETTER  'o'
#define EIM_PRORATING_MODIFIER_NOTSET_LETTER    ' '

/* Modifier: a string */
#define EIM_PRORATING_MODIFIER_ALWAYS_STRING    "ALWAYS"
#define EIM_PRORATING_MODIFIER_IMPUTED_STRING   "IMPUTED"
#define EIM_PRORATING_MODIFIER_NEVER_STRING     "NEVER"
#define EIM_PRORATING_MODIFIER_ORIGINAL_STRING  "ORIGINAL"
#define EIM_PRORATING_MODIFIER_NOTSET_STRING    " "

#define EIM_PRORATING_DEFAULT_MODIFIER   EIE_PRORATING_MODIFIER_ALWAYS
#define EIM_PRORATING_LOWER_DECIMAL      0
#define EIM_PRORATING_UPPER_DECIMAL      9

/* Minimum Lower and upper bounds values for scaling method */
#define EIM_PRORATING_LOWER_LOWERBOUND 0.0
#define EIM_PRORATING_LOWER_UPPERBOUND 0.0

#define EIM_PRORATING_DEFAULT_LOWERBOUND 0.0

/* Prorating Method values / Valeurs pour les méthodes de prorata */
typedef enum {
    EIE_PRORATING_METHOD_NOTSET   =  0,
    EIE_PRORATING_METHOD_SCALING  =  3,
    EIE_PRORATING_METHOD_BASIC    =  5
} EIT_PRORATING_METHOD;


/* Modifier values in edits / Valeurs pour les modificateurs dans les règles*/
typedef enum {
    EIE_PRORATING_MODIFIER_NOTSET   =  0,
    EIE_PRORATING_MODIFIER_ALWAYS   =  3,
    EIE_PRORATING_MODIFIER_IMPUTED  =  5,
    EIE_PRORATING_MODIFIER_NEVER    =  7,
    EIE_PRORATING_MODIFIER_ORIGINAL = 11
} EIT_PRORATING_MODIFIER;


/*  Flag values for raking / Valeurs pour indicateur de suivi pour "raking"*/
typedef enum {
    EIE_RAKING_FLAG_NO_PRORATABLE      = 0,
    EIE_RAKING_FLAG_PRORATABLE         = 1,
    EIE_RAKING_FLAG_PRORATED           = 2,
    EIE_RAKING_FLAG_NOTHING_TO_PRORATE = 3
} EIT_RAKING_FLAG_PRORATING;


/* Return codes for EI_ProratingInitDataGroup function/ Codes de retour pour la fonction EI_ProratingInitDataGroup*/
typedef enum {
    EIE_PRORATING_INITDATAGROUP_FAIL    = EIE_FAIL,
    EIE_PRORATING_INITDATAGROUP_SUCCEED = EIE_SUCCEED
} EIT_PRORATING_INITDATAGROUP_RETURNCODE;


/* Return codes for EI_Raking & EI_Prorating functions/ Codes de retour pour les fonctions EI_Raking & EI_Prorating*/
typedef enum {
    EIE_RAKING_FAIL_NEGATIVE_VALUE     = -17,
    EIE_RAKING_FAIL_K_GT_PLUS_ONE      = -15,
    EIE_RAKING_FAIL_K_LT_MINUS_ONE     = -14,
    EIE_RAKING_FAIL_OUT_OF_BOUNDS      = -13,
    EIE_RAKING_FAIL_SUM_NULL           = -12,
    EIE_RAKING_FAIL_DECIMAL_ERROR      = -11,
    EIE_RAKING_FAIL_NOTHING_TO_PRORATE = -10,
    EIE_RAKING_FAIL                    = EIE_FAIL,
    EIE_RAKING_SUCCEED                 = EIE_SUCCEED,
    EIE_RAKING_SUCCEED_NOTHING_TO_PRORATE = 1
} EIT_RAKING_RETURNCODE;

/*
EIT_PRORATING_EDIT - Edit specifications/Spécifications d'une règle

This structure is used to hold original information on one edit
(verified by the parser).

Cette structure sert à garder l'information originale concernant une règle
(vérifiée par le "parser").

Structure variables / Variables de la structure:

FieldName;      Field name / Nom des champs

TotalName;      Total field name / Nom du champ pour total

Weight;         Field weights / Poids des champs

ProratingFlag;  Proratable flag for fields / Indicateur si champ est modifiable

NumberOfFields; Number of fields / Nombre de champs

Level;          Edit level / Niveau de la règle
*/
typedef struct {
    char   ** FieldName;
    char   *  TotalName;
    double *  Weight;
    int    *  ProratingFlag;
    int       NumberOfFields;
    int       Level;
} EIT_PRORATING_EDIT;

/*
EIT_PRORATING_GROUP - Array of edits / Tableau de règles

This structure is used to describe a group of edits (verified by the parser).

Cette structure sert à décrire un groupe de règles (vérifié  par le "parser").

Structure variables / Variables de la structure:

Edit;          Pointer to one edit / Pointeur vers une règle

NumberOfEdits; Number of edits / Nombre de règles
*/
typedef struct {
     EIT_PRORATING_EDIT *Edit;
     int                 NumberOfEdits;
} EIT_PRORATING_GROUP;

/*
EIT_EDIT_DATA - Edit with characteristics/ Règle de vérification avec caractéristiques
This structure is used to hold original information on one edit
(field names, weights, modifiers and total), record field values
(original and/or imputed) and a flag for the follow-up about raking.
- The component "ProratingFlag" refers to the field proratability (0:No / 1:Yes)
  and does the follow-up about the process (if the field was proratable,
  the raking changes its value for 2:Prorated or 3:Nothing to prorate).

Cette structure sert à garder l'information originale concernant une règle
(nom des champs, poids, modificateurs et nom pour le total), les valeurs des
champs et du total pour un enregistrement (originales et/ou imputées)
et un indicateur de suivi concernant le "raking".
- La composante "ProratingFlag" indique si le champ doit être modifié par
  "raking" (0:Non / 1:Oui) et fait le suivi par rapport au processus
  (si le champ devait être modifié, le processus modifie la valeur du flag
  pour 2:Ajusté ou 3:Rien à ajuster).

Structure variables / Variables de la structure:

FieldName;      Field names / Nom des champs

FieldValue;     Field values / Valeur des champs

Weight;         Field weights / Poids des champs

ProratingFlag;  Follow-up flag / Indicateur de suivi

TotalName;      Total field name / Nom du champ pour Total

TotalValue;     Total field value / Valeur du champ Total

NumberOfFields; Number of fields (excluding Total) / Nombre de champs (excluant Total)
*/
typedef struct {
    char   ** FieldName;
    double *  FieldValue;
    double *  Weight;
    int    *  ProratingFlag;
    char   *  TotalName;
    double    TotalValue;
    int       NumberOfFields;
} EIT_EDIT_DATA;

/*
EIT_GROUP_DATA - Array of edits with data values/ Tableau de règles avec valeurs pour les données
This structure is used to describe a group of edits with the record data.

Cette structure sert à décrire un groupe de règles pour un enregistrement de
données.

Structure variables / Variables de la structure:

Edit;          Pointer to one edit / Pointeur vers une règle

NumberOfEdits; Number of edits / Nombre de règles
*/
typedef struct {
     EIT_EDIT_DATA *Edit;
     int            NumberOfEdits;
} EIT_GROUP_DATA;

/*
EI_ProratingFreeDataGroup - Frees memory allocated for EIT_GROUP_DATA - Libère la mémoire allouée pour EIT_GROUP_DATA
This function frees the memory allocated by EI_ProratingInitDataGroup()

Cette fonction libère la mémoire allouée par EI_ProratingInitDataGroup()
*/
CLASS_DECLSPEC void EI_ProratingFreeDataGroup (
        /* One group of edits data

        Données: groupe de règles */
    EIT_GROUP_DATA * GroupData);

/*
EI_ProratingInitDataGroup - Allocates memory and Initializes EIT_GROUP_DATA - Alloue de la mémoire et initialise EIT_GROUP_DATA

This function allocates memory and initializes partly the group data structure
for edits (1 respondant) with information from the equivalent structure
already filled by the parser
(Initialize: NumberOfEdits/FieldName/Weight/ProratingFlag/TotalName)
It is the programmer's responsability to free that structure.

Cette fonction alloue de la mémoire et initialise partiellement la structure
de données portant sur un groupe de règles (1 répondant), en utilisant
l'information déjà repérée par le parser et entreposée dans la structure
correspondante
(Initialise: NumberOfEdits/FieldName/Weight/ProratingFlag/TotalName)
C'est la responsabilité du programmeur de libérer la mémoire de cette structure
Returns EIT_PRORATING_INITDATAGROUP_RETURNCODE.
*/
CLASS_DECLSPEC EIT_PRORATING_INITDATAGROUP_RETURNCODE EI_ProratingInitDataGroup (
        /* Group of edits read by parser

        Groupe de règles lues avec le parser */
    EIT_PRORATING_GROUP *ParserEdits,
        /* Group of edits for data

        Groupe de règles pour les données*/
    EIT_GROUP_DATA *GroupData);


/* EI_ProratingResetProratingFlag - Reset "ProratingFlag" for all edits-Réinitialise "ProratingFlag" dans les règles*/
/* This function resets the components "ProratingFlag" in the group data structure
   for edits (1 respondant) with original information from the equivalent structure
   already filled by the parser*/
/**/
/**/
/* Cette fonction réinitialise les composantes "ProratingFlag" dans la structure
   de données portant sur un groupe de règles (1 répondant), en utilisant l'information
   originale déjà repérée par le parser et entreposée dans la structure correspondante.*/
/* Returns VOID. */
/**/
CLASS_DECLSPEC void EI_ProratingResetProratingFlag (
        /* Group of edits read by parser

        Groupe de règles lues avec le parser*/
    EIT_PRORATING_GROUP *ParserEdits,
        /* Group of edits for data

        Groupe de règles pour les données*/
    EIT_GROUP_DATA *GroupData);

/*
 EI_ProratingBasic - Processes all edits for 1 respondent (call EI_Raking) - Traite toutes les règles pour 1 répondant (appel EI_Raking).
 This function does the raking for all edits with data for a respondant.
   If one error happens when raking one edit, a follow-up is made (flags)
   and the process ends for that respondant.

Cette fonction exécute le "raking" pour toutes les règles d'un groupe,
   avec les données d'un répondant. Si une erreur se produit lors du
   traitement d'une règle de vérification, un suivi est fait (flags) et
   le traitement se termine pour ce répondant.
Returns EIT_RAKING_RETURNCODE.
*/
CLASS_DECLSPEC EIT_RAKING_RETURNCODE EI_ProratingBasic (
        /* One group of edits data

        Données:groupe de règles*/
    EIT_GROUP_DATA *GroupData,
        /*Precision:Number of decimals

        Précision:Nombre de décimales*/
    int DecimalPlaces,
        /*Precision:Lower bound

        Précision:Limite inférieure*/
    double LowerBound,
        /*Precision:Upper bound

        Précision:Limite supérieure*/
    double UpperBound,
        /* Edit number : first raking error

        Numéro de la règle: première erreur*/
    int * EditNumberFail,
        /* Index of field: ratio out of bounds

        Indice du champ: ratio hors limites*/
    int * IndexFieldOutOfBound,
        /* Erroneous ratio value

        Valeur du ratio erroné*/
    double * ValueRatioOutOfBound);

/*
EI_ProratingScaling is similar to EI_ProratingBasic, but uses the
'Scaling Approach' of prorating, so that the sign of a variable cannot change.
*/
CLASS_DECLSPEC EIT_RAKING_RETURNCODE EI_ProratingScaling (
        /* One group of edits data

        Données:groupe de règles*/
    EIT_GROUP_DATA *GroupData,
        /* Precision:Number of decimals

        Précision:Nombre de décimales*/
    int DecimalPlaces,
        /* Precision:Lower bound

        Précision:Limite inférieure*/
    double LowerBound,
        /* Precision:Upper bound

        Précision:Limite supérieure*/
    double UpperBound,
        /* Edit number : first raking error

        Numéro de la règle: première erreur*/
    int * EditNumberFail,
        /* Index of field: ratio out of bounds

        Indice du champ: ratio hors limites*/
    int * IndexFieldOutOfBound,
       /* Erroneous ratio value

       Valeur du ratio erroné*/
    double * ValueRatioOutOfBound);


/***********************************************************/
/* Function: EI_ProratingSetFlag                           */
/*                                                         */
/* Purpose : Set the prorating flag (field is imputable    */
/*           or not) based on the modifiers in the edits   */
/*           and the field status from input status table. */
/***********************************************************/
CLASS_DECLSPEC void EI_ProratingSetFlag (
    EIT_GROUP_DATA * Edit,
    int FlagReadStatus,
    tSList * FieldStat,
    EIT_PRORATING_MODIFIER DefaultModifier);

/*
EI_RakingBasic - Do raking with rounding for one edit- Exécute "raking" avec arrondissement pour une règle.

This function will balance a summation by distributing the differential with
the expected total across the summation components based on a specific weight
associated with each of those components and only on components which might be
changeable (If "Ratio Out of Bound", keep Field Index and Value ratio for the
first error of this type).

Cette fonction équilibrera une somme en distribuant l'écart avec un total
attendu entre les diverses composantes, tenant compte du poids de chacune de
ces composantes et en ne modifiant que les composantes modifiables (Si "Ratio
hors limite", garde Indice du champ et Valeur du ratio pour la première erreur
de ce type).

Returns EIT_RAKING_RETURNCODE.
*/
CLASS_DECLSPEC EIT_RAKING_RETURNCODE EI_RakingBasic (
        /* One edit data

        Données pour une règle*/
    EIT_EDIT_DATA *Edit,
        /* Precision:Number of decimals

        Précision:Nombre de décimales*/
    int DecimalPlaces,
        /* Precision:Lower bound

        Précision:Limite inférieure*/
    double LowerBound,
        /* Precision:Upper bound

        Précision:Limite supérieure*/
    double UpperBound,
        /* Index of field: ratio out of bounds

        Indice du champ: ratio hors limites*/
    int * IndexFieldOutOfBound,
        /* Erroneous ratio value

        Valeur du ratio erroné*/
    double * ValueRatioOutOfBound);

/*
EI_RakingScaling is similar to EI_RakingBasic, but uses the
'Scaling Approach' of prorating, so that the sign of a variable cannot change.
*/
CLASS_DECLSPEC EIT_RAKING_RETURNCODE EI_RakingScaling (
        /* One edit data

        Données pour une règle*/
    EIT_EDIT_DATA *Edit,
        /* Precision:Number of decimals

        Précision:Nombre de décimales*/
    int DecimalPlaces,
        /* Precision:Lower bound

        Précision:Limite inférieure*/
    double LowerBound,
        /* Precision:Upper bound

        Précision:Limite supérieure*/
    double UpperBound,
        /* Index of field: ratio out of bounds

        Indice du champ: ratio hors limites*/
    int * IndexFieldOutOfBound,
        /* Erroneous ratio value

        Valeur du ratio erroné*/
    double * ValueRatioOutOfBound);

/*
Debug print functions
*/
CLASS_DECLSPEC void EI_ProratingDataGroupPrint (EIT_GROUP_DATA *);

#endif
