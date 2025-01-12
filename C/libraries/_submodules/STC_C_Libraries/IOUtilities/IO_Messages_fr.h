#ifndef IO_MESSAGES_FR_H
#define IO_MESSAGES_FR_H

#define IO_TEST_MESSAGE "MESSAGE DE TEST pour la prise en charge de la langue maternelle: for�t, na�f, fran�ais, cr�me br�l�e"

#define JUM__NUMERIC "numeric"
#define JUM__CHARACTER "character"
#define JUM_MISSING "<missing>"
// translate these prefixes? Double check for "lph" length place holders in MessageBanff.h if doing that.  
#define MSG_PREFIX_ERROR "ERROR: "
#define MSG_PREFIX_WARN "WARNING: "
#define MSG_PREFIX_NOTE "NOTE: "
#define MSG_INDENT_ERROR "       "
#define MSG_INDENT_WARN "         "
#define MSG_INDENT_NOTE "      "

/* taken from MessageBanff_*.h */
#define MsgDataSetMandatory "La table %s est obligatoire."

/* START: copied from MessageSAS_en.h */
#define MsgNumericVarNotInDataSet "La variable num�rique %s n'est pas dans la table %s."
#define MsgCharacterVarNotInDataSet "La variable caract�re %s n'est pas dans la table %s."

#define MsgVarNotNumericInDataSet "La variable %s n'est pas num�rique dans la table %s."
#define MsgVarNotCharacterInDataSet "La variable %s n'est pas caract�re dans la table %s."

#define MsgStatementNotSpecified "%s non sp�cifi�."
#define MsgStatementSpecifiedWithoutVariableList "Le param�tre %s a �t� sp�cifi� sans qu'une variable ou une liste de variables ne soit donn�e."
#define M10001 "--- Syst�me %s %s d�velopp� � Statistique Canada ---"
#define M10002 "PROC�DURE %s Version %s"
#define M10003 "Cr��e le %s � %s"
#define M10004 "Courriel: %s"
#define M10005 "%s" /* specific for printing header message */
#define MsgByVariableNotSameType "La variable %*s a �t� d�finie comme alphanum�rique et num�rique."
#define MsgByVariableNotInDataSet "Variable BY %*s inexistante dans la table d'entr�e %8b.%*s."
/* END: copied from MessageSAS_en.h */

// `JUM_PARM_`: parameter type validation
#define JUM_PARM_INVALID "param�tre '%s' type incorrect, attendu %s, mais re�u\n"
#define JUM_PARM_INVALID_INT "une constante INTEGER"
#define JUM_PARM_INVALID_REAL "une constante num�rique R�EL"
#define JUM_PARM_INVALID_QS "une cha�ne entre guillemets"
#define JUM_PARM_INVALID_NAME "une courte cha�ne entre guillemets"
#define JUM_PARM_INVALID_FLAG "une valeur bool�enne"
#define JUM_PARM_INVALID_NAME_MSG "la longueur du param�tre %s (%zu) d�passe la limite de %d: %s\n"
#define JUM_PARM_TYPE_UNKNOWN "Type de param�tre inconnu re�u (%d)\n"

/* Varlist Names: used in some error messages
    used during varlist initialization (`VL_init*()`)*/
    // DO NOT TRANSLATE the following identifiers: `instatus`, `inestimator`, `inalgorithm`, `unit_id`, or `by`
#define VL_NAME_in_var          "variables d'entr�e"
#define VL_NAME_instatus        "instatus variables"
#define VL_NAME_instatus_hist   "instatus_hist variables"
#define VL_NAME_inestimator     "inestimator variables"
#define VL_NAME_inalgorithm     "inalgorithm variables"
#define VL_NAME_unit_id         "unit_id"
#define VL_NAME_by_var          "by"

/* related to datasets and their values */
#define JUM_SINGLE_VARLIST_COUNT_INVALID "La liste '%s' accepte au plus 1 variable %s.\n"

#define JUM_VARLIST_WRONG_TYPE "La variable '%s' dans la liste '%s' (ensemble de donn�es %s) doit �tre %s, mais elle est %s.\n"

#define JUM_VARLIST_MEMBER_MISSING "Variable '%s' dans la liste '%s' (ensemble de donn�es %s) introuvable.\n"

#define JUM_TYPE_MISMATCH_CHAR "donn�es non valides d�tect�es lors de la lecture de la variable de caract�re '%s' sur la ligne %d\n"
#define JUM_TYPE_MISMATCH_NUM "donn�es non valides d�tect�es lors de la lecture de la variable num�rique '%s' sur la ligne %d\n"

#define JUM_DATA_NOT_SORTED_P1 "L'ensemble de donn�es %s n'est pas tri� par ordre croissant."
#define JUM_DATA_NOT_SORTED_P2 "Le groupe actuel a "
#define JUM_DATA_NOT_SORTED_P3 " et le groupe suivant a "

#define JUM_PRINT_INPUT_DATASET_METADATA "%s: %d colonne(s), %d rang�e(s)\n"
#define JUM_DATASET_NOT_SPECIFIED "%s: non sp�cifi�\n"
#define JUM_DATASET_ENABLED "%s: activ�\n"
#define JUM_DATASET_DISABLED "%s: d�sactiv�\n"

/* JSON related */
#define JUM_ERROR_PREFIX_LENGTH 7   // length of string 'ERROR: '
#define JUM_JSON_PRINT_OBJECT "Objet JSON\n"
#define JUM_JSON_PRINT_ARRAY "Tableau JSON de %lld �l�ment(s):\n"
#define JUM_JSON_PRINT_STRING "cha�ne: \"%s\"\n"
#define JUM_JSON_PRINT_INTEGER "entier: \"%" JSON_INTEGER_FORMAT "\"\n"
#define JUM_JSON_PRINT_REAL "r�el: %f\n"
#define JUM_JSON_PRINT_BOOL_TRUE "bool�en: vrai\n"
#define JUM_JSON_PRINT_BOOL_FALSE "bool�en: faux\n"
#define JUM_JSON_PRINT_NULL "NULL\n"
#define JUM_JSON_PRINT_UNKNOWN "type JSON non reconnu %d\n"

#define JUM_JSON_PARSE_ERROR_INDATA "impossible d'analyser l'entr�e JSON pour le jeu de donn�es INDATA\n"
#define JUM_JSON_DECODE_ERROR "erreur json sur la ligne %d, position %d :% s\n"

/* Apache Arrow related */
#define MSG_ARROW_ERROR_MESSAGE "erreur inattendue de nanoarrow"
#define MSG_ARROW_OOB_REF_COL "essayer de r�f�rencer l'index de colonne hors limites"
#define MSG_ARROW_OOB_REF_ROW "essayer de r�f�rencer un index de ligne hors limites"
#define MSG_ARROW_INVALID_REF_DS "r�f�rence d'ensemble de donn�es invalide"
#define MSG_ARROW_FAIL_DS_INIT "�chec de l'initialisation de l'ensemble de donn�es de sortie"
#define MSG_ARROW_FAIL_DS_WRITE "�chec de l'�criture de la valeur dans l'ensemble de donn�es de sortie"

/* IOUtil related */
#define MSG_IO_OUT_COL_TYPE_INVALID "essayer de d�finir un type de colonne de sortie non valide"
#define MSG_IO_VAR_NAME_TOO_LONG "le nom de la variable d�passe la longueur maximale  (%d) dans la liste '%s' (longueur %zd, nom '%s')\n"

#endif