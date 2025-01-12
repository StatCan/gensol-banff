#ifndef IO_MESSAGES_FR_H
#define IO_MESSAGES_FR_H

#define IO_TEST_MESSAGE "MESSAGE DE TEST pour la prise en charge de la langue maternelle: forêt, naïf, français, crème brûlée"

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
#define MsgNumericVarNotInDataSet "La variable numérique %s n'est pas dans la table %s."
#define MsgCharacterVarNotInDataSet "La variable caractère %s n'est pas dans la table %s."

#define MsgVarNotNumericInDataSet "La variable %s n'est pas numérique dans la table %s."
#define MsgVarNotCharacterInDataSet "La variable %s n'est pas caractère dans la table %s."

#define MsgStatementNotSpecified "%s non spécifié."
#define MsgStatementSpecifiedWithoutVariableList "Le paramètre %s a été spécifié sans qu'une variable ou une liste de variables ne soit donnée."
#define M10001 "--- Système %s %s développé à Statistique Canada ---"
#define M10002 "PROCÉDURE %s Version %s"
#define M10003 "Créée le %s à %s"
#define M10004 "Courriel: %s"
#define M10005 "%s" /* specific for printing header message */
#define MsgByVariableNotSameType "La variable %*s a été définie comme alphanumérique et numérique."
#define MsgByVariableNotInDataSet "Variable BY %*s inexistante dans la table d'entrée %8b.%*s."
/* END: copied from MessageSAS_en.h */

// `JUM_PARM_`: parameter type validation
#define JUM_PARM_INVALID "paramètre '%s' type incorrect, attendu %s, mais reçu\n"
#define JUM_PARM_INVALID_INT "une constante INTEGER"
#define JUM_PARM_INVALID_REAL "une constante numérique RÉEL"
#define JUM_PARM_INVALID_QS "une chaîne entre guillemets"
#define JUM_PARM_INVALID_NAME "une courte chaîne entre guillemets"
#define JUM_PARM_INVALID_FLAG "une valeur booléenne"
#define JUM_PARM_INVALID_NAME_MSG "la longueur du paramètre %s (%zu) dépasse la limite de %d: %s\n"
#define JUM_PARM_TYPE_UNKNOWN "Type de paramètre inconnu reçu (%d)\n"

/* Varlist Names: used in some error messages
    used during varlist initialization (`VL_init*()`)*/
    // DO NOT TRANSLATE the following identifiers: `instatus`, `inestimator`, `inalgorithm`, `unit_id`, or `by`
#define VL_NAME_in_var          "variables d'entrée"
#define VL_NAME_instatus        "instatus variables"
#define VL_NAME_instatus_hist   "instatus_hist variables"
#define VL_NAME_inestimator     "inestimator variables"
#define VL_NAME_inalgorithm     "inalgorithm variables"
#define VL_NAME_unit_id         "unit_id"
#define VL_NAME_by_var          "by"

/* related to datasets and their values */
#define JUM_SINGLE_VARLIST_COUNT_INVALID "La liste '%s' accepte au plus 1 variable %s.\n"

#define JUM_VARLIST_WRONG_TYPE "La variable '%s' dans la liste '%s' (ensemble de données %s) doit être %s, mais elle est %s.\n"

#define JUM_VARLIST_MEMBER_MISSING "Variable '%s' dans la liste '%s' (ensemble de données %s) introuvable.\n"

#define JUM_TYPE_MISMATCH_CHAR "données non valides détectées lors de la lecture de la variable de caractère '%s' sur la ligne %d\n"
#define JUM_TYPE_MISMATCH_NUM "données non valides détectées lors de la lecture de la variable numérique '%s' sur la ligne %d\n"

#define JUM_DATA_NOT_SORTED_P1 "L'ensemble de données %s n'est pas trié par ordre croissant."
#define JUM_DATA_NOT_SORTED_P2 "Le groupe actuel a "
#define JUM_DATA_NOT_SORTED_P3 " et le groupe suivant a "

#define JUM_PRINT_INPUT_DATASET_METADATA "%s: %d colonne(s), %d rangée(s)\n"
#define JUM_DATASET_NOT_SPECIFIED "%s: non spécifié\n"
#define JUM_DATASET_ENABLED "%s: activé\n"
#define JUM_DATASET_DISABLED "%s: désactivé\n"

/* JSON related */
#define JUM_ERROR_PREFIX_LENGTH 7   // length of string 'ERROR: '
#define JUM_JSON_PRINT_OBJECT "Objet JSON\n"
#define JUM_JSON_PRINT_ARRAY "Tableau JSON de %lld élément(s):\n"
#define JUM_JSON_PRINT_STRING "chaîne: \"%s\"\n"
#define JUM_JSON_PRINT_INTEGER "entier: \"%" JSON_INTEGER_FORMAT "\"\n"
#define JUM_JSON_PRINT_REAL "réel: %f\n"
#define JUM_JSON_PRINT_BOOL_TRUE "booléen: vrai\n"
#define JUM_JSON_PRINT_BOOL_FALSE "booléen: faux\n"
#define JUM_JSON_PRINT_NULL "NULL\n"
#define JUM_JSON_PRINT_UNKNOWN "type JSON non reconnu %d\n"

#define JUM_JSON_PARSE_ERROR_INDATA "impossible d'analyser l'entrée JSON pour le jeu de données INDATA\n"
#define JUM_JSON_DECODE_ERROR "erreur json sur la ligne %d, position %d :% s\n"

/* Apache Arrow related */
#define MSG_ARROW_ERROR_MESSAGE "erreur inattendue de nanoarrow"
#define MSG_ARROW_OOB_REF_COL "essayer de référencer l'index de colonne hors limites"
#define MSG_ARROW_OOB_REF_ROW "essayer de référencer un index de ligne hors limites"
#define MSG_ARROW_INVALID_REF_DS "référence d'ensemble de données invalide"
#define MSG_ARROW_FAIL_DS_INIT "échec de l'initialisation de l'ensemble de données de sortie"
#define MSG_ARROW_FAIL_DS_WRITE "échec de l'écriture de la valeur dans l'ensemble de données de sortie"

/* IOUtil related */
#define MSG_IO_OUT_COL_TYPE_INVALID "essayer de définir un type de colonne de sortie non valide"
#define MSG_IO_VAR_NAME_TOO_LONG "le nom de la variable dépasse la longueur maximale  (%d) dans la liste '%s' (longueur %zd, nom '%s')\n"

#endif