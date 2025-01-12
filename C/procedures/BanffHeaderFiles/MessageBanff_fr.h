#ifndef MESSAGEBANFF_FR_H
#define MESSAGEBANFF_FR_H

#include "BanffIdentifiers.h"

/********************************************************************************************/
/* Introduced during redesign conversion                                                    */
/********************************************************************************************/
/* return code descriptions */
#define RC_DESC_SUCCESS "exécution réussie"
#define RC_DESC_FAIL_UNHANDLED "erreur inattendue"
#define RC_DESC_FAIL_INIT_IN_DATASET "échec d'initialisation du jeu de données d'entrée"
#define RC_DESC_FAIL_READ_PARMS_LEGACY "échec d'obtention des paramètres"
#define RC_DESC_FAIL_MISSING_DATASET "jeu de données obligatoire manquant"
#define RC_DESC_FAIL_VARLIST_NOT_FOUND "variable dans la liste de variables non trouvée"
#define RC_DESC_FAIL_VARLIST_INVALID_COUNT "nombre de variables non valide dans la liste de variables"
#define RC_DESC_FAIL_VARLIST_SYSTEM_GENERATED "liste de variables générée par le système non valide"
#define RC_DESC_FAIL_SETUP_DATASET_IN "échec de configuration du jeu de données d'entrée"
#define RC_DESC_FAIL_SETUP_OTHER "erreur de configuration inattendue"
#define RC_DESC_FAIL_ALLOCATE_MEMORY "échec d'allocation de mémoire"
#define RC_DESC_FAIL_LPI_INIT "échec d'initialisation LPI"
#define RC_DESC_FAIL_EDITS_PARSE "échec d'analyse des modifications"
#define RC_DESC_FAIL_VALIDATION_LEGACY "échec de validation de l'héritage"
#define RC_DESC_FAIL_VALIDATION_NEW "échec de validation"
#define RC_DESC_FAIL_EDITS_OTHER "erreur inattendue liée aux modifications"
#define RC_DESC_FAIL_EDITS_CONSISTENCY "échec de la vérification de cohérence des modifications"
#define RC_DESC_FAIL_EDITS_REDUNDANCY "échec des modifications Vérification de redondance"
#define RC_DESC_FAIL_NAME_TOO_LONG "le nom de la variable dépasse la longueur maximale"
#define RC_DESC_FAIL_READ_GENERIC "Erreur inattendue lors de la lecture du jeu de données"
#define RC_DESC_FAIL_WRONG_SORT_ORDER "Ordre de tri non valide"
#define RC_DESC_FAIL_READ_SYNC "Échec de la synchronisation des jeux de données"
#define RC_DESC_FAIL_READ_DUPLICATE_DATA "Données en double détectées"
#define RC_DESC_FAIL_WRITE_GENERIC "échec de l'écriture dans l'ensemble de données de sortie"
#define RC_DESC_FAIL_PROCESSING_GENERIC "Erreur de traitement inattendue"
#define RC_DESC_EIE_TRANSFORM_FAIL "Échec de la transformation"
#define RC_DESC_EIE_KDTREE_FAIL "Échec KDTREE"
#define RC_DESC_EIE_MATCHFIELDS_FAIL "Erreur de correspondance des champs"

/********************************************************************************************/
/* Generic to Banff                                                                         */
/********************************************************************************************/
#define MsgParmMandatory "%s est obligatoire."
#define MsgNumericVarNotInDataSet "La variable numérique %s n'est pas dans la table %s."
#define MsgVarNotNumericInDataSet "La variable %s n'est pas numérique dans la table %s."
#define MsgCharacterVarNotInDataSet "La variable caractère %s n'est pas dans la table %s."
#define MsgVarNotCharacterInDataSet "La variable %s n'est pas caractère dans la table %s."
#define MsgParmNotSpecified "%s non spécifié."

#define MsgNumberObs "Nombre d'observations dans la table %s %.*s: %*d"
#define MsgNumberValidObs "Nombre d'observations valides dans la table %s %.*s: %*d"
#define MsgNumberObs_LPH       9  /* Paired message for Length of PlaceHolders (LPH) */
#define MsgNumberValidObs_LPH  9

/********************************************************************************************/
/* Generic to Banff procedures                                                              */
/********************************************************************************************/
/* All procedures */
#define MsgDefault " (défaut)"
#define MsgNotUsed " (pas utilisé)"
#define MsgFunctionFailure "%s a échoué."
#define MsgParmInvalid "%s invalide."
#define MsgPositivityOptionInvalid "Les options " BPN_REJECT_NEGATIVE " et " BPN_ACCEPT_NEGATIVE " ne peuvent pas être spécifiées en même temps."                           
#define MsgStatementMandatory "L'énoncé %s est obligatoire."
#define MsgParmGreater0 "La valeur doit être supérieure à 0."
#define MsgParmGreaterEqual0 "La valeur doit être supérieure ou égale à 0."

/* Flag type parameters */
#define MsgAcceptNegative BPN_ACCEPT_NEGATIVE " = True"
#define MsgRejectNegative BPN_ACCEPT_NEGATIVE " = False"
#define MsgRejectNegativeDefault BPN_ACCEPT_NEGATIVE " = False" MsgDefault
#define MsgAcceptZero BPN_ACCEPT_ZERO " = True"
#define MsgAcceptZeroDefault BPN_ACCEPT_ZERO " = True" MsgDefault
#define MsgRejectZero BPN_ACCEPT_ZERO " = False"
#define MsgRejectZeroDefault BPN_ACCEPT_ZERO " = False" MsgDefault

/* LP solver */
#define MsgInitLPFail "Incapable d'initialiser le solutionneur de programmation linéaire %s."
#define MsgInitLPSuccess "Le solutionneur de programmation linéaire %s, version %s, a été initialisé."

/* Seed */
#define MsgSeedEqualIntegerChosenBySystem "" BPN_SEED " = %d (valeur choisie par le système)"
#define MsgSeedOutOfRangeGeneratedNumberTaken "La valeur spécifiée pour l'option " BPN_SEED " excède le maximum acceptable par la plateforme et ne peut pas être retenue. Cette valeur a été remplacée par un nombre aléatoire."
#define MsgSeedNegativeGeneratedNumberTaken "La valeur spécifiée pour l'option " BPN_SEED " est négative et ne peut pas être retenue. Cette valeur a été remplacée par un nombre aléatoire."
#define MsgSeedZeroGeneratedNumberTaken "La valeur spécifiée pour l'option " BPN_SEED " est 0 et ne peut pas être retenue. Cette valeur a été remplacée par un nombre aléatoire."

/* Edits */
#define MsgAddingPositivityEditsFail "L'option " BPN_REJECT_NEGATIVE " est active mais le système échoue lorsqu'il essaie d'ajouter les règles de positivité à l'ensemble des règles originales."
#define MsgEditsVarNotInDataSet "La variable %s spécifiée dans les règles (" BPN_EDITS ") n'est pas dans la table %s."
#define MsgEditsVarNotNumericInDataSet "La variable %s spécifiée dans les règles (" BPN_EDITS ") n'est pas numérique dans la table %s."
#define MsgHeaderEdits "Ensemble des règles de vérification (" BPN_EDITS "): "
#define MsgHeaderReducedEdits "Ensemble MINIMAL des règles de vérification (" BPN_EDITS "): "
#define MsgInconsistentEdits "Les règles de vérification sont incohérentes. S'il vous plaît, vérifiez vos règles (" BPN_EDITS ")."
#define MsgPositivityEditsAdded "(Les règles de 'positivité' ont été ajoutées car l'option " BPN_REJECT_NEGATIVE " est active.)"
#define MsgReducedEditsEquivalent "L'ensemble minimal des règles de vérification est identique à l'ensemble des règles originales spécifiées."

/* Miscellaneous (found in more than one procedure)*/
#define MsgDataSetWithTwoKeysDuplicate "Les clés de la table %s ne sont pas uniques. La clé courante est %s et la clé précédente était %s."
#define MsgDataSetWithTwoKeysNotSorted "La table %s n'est pas triée en ordre ascendant. La clé courante est %s et la clé précédente était %s."
#define MsgByVarsNotFoundInDataSet "Une ou plusieurs variables " BPN_BY " ne sont pas présentes sur la table %s." /* HIST or AUX */
#define MsgFasterPerformanceIfByVarsInDataSet "L'exécution de cette procédure pourrait être plus rapide si la table %s contenait les variables " BPN_BY "."
#define MsgKeyNameWithDuplicateValueInDataSet "%s est non unique pour la valeur '%s' dans la table %s."
#define MsgNoObservationsInDataSet "Aucune observation dans la table %s."
#define MsgNumberDroppedMissingKeyNameInDataSet "%d observations ont été rejetées de la table %s parce que la variable %s est manquante."
#define MsgNumberDroppedMissingKeyNameOrFieldidInDataSet "%d observations ont été rejetées de la table %s parce que la variable %s ou FIELDID est manquante."
#define MsgNumberDroppedMissingValuesInDataSet1NotFTIInDataSet2 "%d observations ont été rejetées de la table %s parce qu'une valeur est manquante et il n'y a aucune observation correspondante dans la table %s avec un statut FTI."
#define MsgNumberDroppedNegativeValuesInDataSet1NotFTIInDataSet2 "%d observations ont été rejetées de la table %s parce qu'une valeur est négative et il n'y a aucune observation correspondante dans la table %s avec un statut FTI."
#define MsgTwoStatementsExclusive "Les listes de variables des énoncés %s et %s sont mutuellemement exclusives."
#define MsgVarNameInTwoStatementsExclusive "La variable %s fait partie des énoncés %s et %s. Ces énoncés sont mutuellement exclusifs."

/********************************************************************************************/
/* Verifyedits                                                                              */
/********************************************************************************************/
#define MsgNoExtremalPrinted "L'option " BPN_EXTREMAL " n'est pas disponible lorsque l'option " BPN_ACCEPT_NEGATIVE " est spécifiée. Aucun point extrémal ne sera calculé."

/* Parm: " BPN_IMPLY " and " BPN_EXTREMAL " */
#define MsgParmInvalidNegativeValueReplacedByDefault "La valeur spécifiée pour %s est négative; cette valeur est invalide. La valeur par défaut a été prise comme valeur de remplacement."

/********************************************************************************************/
/* Editstats                                                                              */
/********************************************************************************************/
#define MsgAllOutDataSetsNULL "Aucune analyse n'est faite par la procédure. _NULL_ a été spécifié pour chacun des ensembles de données de sortie."
#define MsgEditSizeExceededMax "L'espace nécessaire pour définir la variable caractère EDIT dans la table de sortie %s dépasse la longueur maximum acceptable."

/********************************************************************************************/
/* Outlier                                                                                  */
/********************************************************************************************/
#define MsgAtLeastMIIorMEIMustBeSpecified "Les multiplicateurs " BPN_MEI " et " BPN_MII ", ou au moins l'un d'entre eux, doivent être spécifiés."
#define MsgAtLeastBETAIorBETAEMustBeSpecified "Les multiplicateurs " BPN_BETA_E " et " BPN_BETA_I ", ou au moins l'un d'entre eux, doivent être spécifiés."
#define MsgMIIMustBeGreaterThanMEI "" BPN_MII " doit avoir une valeur plus grande que celle du " BPN_MEI "."
#define MsgBETAIMustBeGreaterThanBETAE "" BPN_BETA_I " doit avoir une valeur plus grande que celle du " BPN_BETA_E "."

#define MsgHeaderForByGroup "Les messages suivants concernent le regroupement " BPN_BY ":%-2n%*s" /* %-2n: specific to SAS_XP routines */
#define MsgHeaderForByGroup_SAS_FREE "Les messages suivants concernent le regroupement " BPN_BY ":" "\n" MSG_INDENT_NOTE
#define MsgHeaderForByGroupAbove_SAS_FREE "Le message ci-dessus concernait le groupe " BPN_BY "  suivant :" "\n" MSG_INDENT_NOTE
#define MsgHeaderForVariable "Les messages suivants concernent la variable:%-2n%*s"       /* %-2n: specific to SAS_XP routines */
#define MsgHeaderForVariable_SAS_FREE "Les messages suivants concernent la variable:\n      %*s"

#define MsgIdValueDuplicatedInDataSet "La clé '%s' est non unique dans la table %s."
#define MsgMinimumValueOfValidObsNotReached "Nombre d'observations valides dans la table " DSN_INDATA " est inférieur au " BPN_MIN_OBS " %d. Aucune identification de valeurs aberrantes."
#define MsgVarNameNotInDataSet "La variable %s n'est pas présente sur la table %s."
#define MsgStatementEmpty "Au moins une variable doit être spécifiée avec l'énoncé %s."
#define MsgMINOBSInvalidValue "Valeur invalide pour " BPN_MIN_OBS ". La valeur doit être plus grande ou égale à %d."
#define MsgMINOBSWarningValue "La valeur de " BPN_MIN_OBS " est plus petite que %d. La méthode est plus efficace avec un minimum de %d observations."
#define MsgInvalidExponent "La valeur doit être supérieure ou égale à 0 et inférieure ou égale à 1."
#define MsgInvalidMethod "Les mots-clés acceptables sont: CURRENT, HISTORIC, RATIO, SIGMAGAP."
#define MsgInvalidSigma "Les mots-clés acceptables sont: STD, MAD."
#define MsgInvalidSide "Les mots-clés acceptables sont: LEFT, BOTH, RIGHT."
#define MsgInvalidStartCentile "La valeur doit être supérieure ou égale à %.0f et inférieure à %.0f."
#define MsgMultipleStatements "Plusieurs énoncés %s. Seulement une est permise."

/* CU method */
#define MsgHISTDataSetIgnored "La table %s a été spécifiée avec la méthode des données courantes. Cette table ne sera pas utilisée." /* HIST or AUX */
#define MsgExponentIgnored "L'option " BPN_EXPONENT " a été spécifiée avec la méthode des données courantes ou SIGMAGAP. Cette option ne sera pas utilisée."
#define MsgNoDefaultForVar "Il est impossible de construire une liste par défaut pour l'énoncé " BPN_VAR " car aucune variable numérique n'est présente sur la table " DSN_INDATA "."
#define MsgWithIgnored "Un énoncé " BPN_WITH " a été spécifié avec la méthode des données courantes. Cet énoncé sera ignoré."

/* HT/Ratio method */
#define MsgHTRatioMethodAcceptNegativeIgnored "" BPN_ACCEPT_NEGATIVE " ne devrait pas être spécifiée avec la méthode des ratios ou historique. Pour des raisons méthodologiques, les valeurs 0 et négatives ne seront jamais retenues."
#define MsgHTRatioMethodAcceptZeroIgnored "" BPN_ACCEPT_ZERO " ne devrait pas être spécifiée avec la méthode des ratios ou historique. Pour des raisons méthodologiques, les valeurs 0 et négatives ne seront jamais retenues."
#define MsgHTRatioMethodBadSpecifications "La méthode des ratios ou historique ne peut pas être utilisée avec les spécifications fournies."
#define MsgHTRatioMethodDifferentSizeForVarAndWith "Les énoncés " BPN_VAR " et " BPN_WITH " comprennent un nombre différent de variables."

/* SIGMAGAP method */
/*
#define MsgSigmagapMethodAcceptNegativeIgnored "" BPN_ACCEPT_NEGATIVE " ne devrait pas être spécifiée avec la méthode sigmagap à deux variables. Pour des raisons méthodologiques, les valeurs 0 et négatives ne seront jamais retenues."
#define MsgSigmagapMethodAcceptZeroIgnored "" BPN_ACCEPT_ZERO " ne devrait pas être spécifiée avec la méthode sigmagap à deux variables. Pour des raisons méthodologiques, les valeurs 0 et négatives ne seront jamais retenues."
*/
/* Print count of "bad data" by " BPN_VAR ". */
#define MsgNumberDroppedInDataSetMissingValueForVar "%d observations ont été rejetées de la table %s parce que la variable %*s est manquante."
#define MsgNumberDroppedInDataSetNegativeValueForVar "%d observations ont été rejetées de la table %s parce que la variable %*s est négative."
#define MsgNumberDroppedInDataSetNegativeValueForWeightedVar "%d observations ont été rejetées de la table %s parce que la variable pondérée %*s est négative."
#define MsgNumberDroppedInDataSetZeroValueForVar "%d observations ont été rejetées de la table %s parce que la variable %*s est 0."
#define MsgNumberDroppedInDataSetZeroValueForWeightedVar "%d observations ont été rejetées de la table %s parce que la variable pondérée %*s est 0."

/* Print weight data quality warnings. */
#define MsgNegativeWeightWarning "Au moins une variable " BPN_WEIGHT " est négative."
#define MsgZeroWeightWarning "Au moins une variable " BPN_WEIGHT " est égale à zéro."

/* Print Statistics -
   Format: %.*s --> field width of string "" to output
           %*s  --> field width to extract from a line of 'dot' string
   Subtotal: count only (heading spaces: 1 tab) */
#define MsgNumberDroppedIDOnlyInDataSet "%.*srejetées parce que " BPN_UNIT_ID " est présente uniquement sur %s %.*s: %*d"
#define MsgNumberDroppedMissingID "%.*srejetées parce que " BPN_UNIT_ID " est manquante %.*s: %*d"
#define MsgNumberDroppedMissingWeight "%.*srejetées parce que " BPN_WEIGHT " est manquant %.*s: %*d"
#define MsgNumberDroppedIDOnlyInDataSet_LPH  13  /* Paired Msg for Length of PlaceHolders (LPH) */
#define MsgNumberDroppedMissingID_LPH        11
#define MsgNumberDroppedMissingWeight_LPH    11

/********************************************************************************************/
/* Errorloc                                                                                 */
/********************************************************************************************/
#define MsgNotConstrainedByCardinality "Le résultat ne sera pas limité par la cardinalité."
#define MsgRedundantEditsFail "Incapable de déceler les règles redondantes (" BPN_EDITS ")."
#define MsgTimePerObsAdjustedToMinValue "La valeur de " BPN_TIME_PER_OBS " est invalide et a été ajustée à %.2f secondes, soit le temps minimum par observation."
#define MsgVarNameUnbounded "La variable %s n'est pas bornée. Une borne inférieure ou supérieure DOIT exister."
#define MsgWeightVarDefault "Un poids de 1 sera attribué à chaque variable spécifiée dans les règles de vérification."
/*this message is used for both errorloc and donorimp when both the " BPN_SEED " and " BPN_RAND_NUM_VAR " parameters have been specified*/
#define MsgRanndumvarSeedSpecified "Les paramètres " BPN_SEED " et " BPN_RAND_NUM_VAR " ont tous les deux été spécifiés. Le paramètre " BPN_SEED " sera ignoré."
/*the following messages are used for validating the value of " BPN_RAND_NUM_VAR "*/
#define MsgRandnumvarErrorlocLessThanZero_main "ERRORLOC: Le nombre aléatoire de l'enregistrement %f est plus petit que 0. Veuillez corriger le nombre aléatoire et relancez la procédure."
#define MsgRandnumvarErrorlocGreaterThanOne_main "ERRORLOC: Le nombre aléatoire de l'enregistrement %f est plus grand que 1. Veuillez corriger le nombre aléatoire et relancez la procédure."

/* Print Statistics
   Format: %.*s --> field width of string "" to output
           %*s  --> field width to extract from a line of 'dot' string  */
/* Total: count and percent */
#define MsgNoValidDataInGroup "Aucune observation valide dans le regoupement traité."
#define MsgNumberPercentObs "Nombre d'observations %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentObsDroppedMissingKey "Nombre d'observations rejetées car la clé est manquante %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentObsFailedEdits "Nombre d'observations ayant échoué les règles %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentObsPassedEdits "Nombre d'observations ayant réussi les règles %.*s: %*d %.*s %*.*f%%"
/* Subtotal: count and percent (heading spaces: 1 tab) */
#define MsgNumberPercentObsNoSolution "%.*sNombre d'observations sans solution %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentObsWithVarsToImpute "%.*sNombre d'observations ayant des variables à imputer %.*s: %*d %.*s %*.*f%%"
/* Subtotal: count and percent (heading spaces: 2 tabs) */
#define MsgNumberPercentObsNoSolutionCardinality "%.*sdû à une contrainte de cardinalité %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentObsNoSolutionSpace "%.*sdû à une contrainte d'espace mémoire %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentObsNoSolutionTime "%.*sdû à une contrainte de temps %.*s: %*d %.*s %*.*f%%"
/* Subtotal: count only (heading spaces: 2 tabs) */
#define MsgNumberVarsToImpute "%.*sNombre de variables à imputer %.*s: %*d"

#define MsgNumberPercentObs_LPH                   18  /* Paired Msg for Length of PlaceHolders (LPH) */
#define MsgNumberPercentObsDroppedMissingKey_LPH  18
#define MsgNumberPercentObsFailedEdits_LPH        18
#define MsgNumberPercentObsPassedEdits_LPH        18
#define MsgNumberPercentObsNoSolution_LPH         22
#define MsgNumberPercentObsWithVarsToImpute_LPH   22
#define MsgNumberPercentObsNoSolutionCardinality_LPH  22
#define MsgNumberPercentObsNoSolutionSpace_LPH    22
#define MsgNumberPercentObsNoSolutionTime_LPH     22
#define MsgNumberVarsToImpute_LPH                 11

/********************************************************************************************/
/* Deterministic                                                                            */
/********************************************************************************************/
/* Print Statistics
   Format: %.*s --> field width of string "" to output
           %*s  --> field width to extract from a line of 'dot' string  */
/* Subtotal: count only (heading spaces: 1 tab) */
#define MsgNumberValidObsWithFTI "%.*sayant des variables FTI %.*s: %*d"
#define MsgNumberValidObsWithoutFTI "%.*ssans variables FTI %.*s: %*d"
/* Subtotal: count only (heading spaces: 2 tabs) */
#define MsgNumberValidObsImputed "%.*simputées %.*s: %*d"
#define MsgNumberValidObsNotImputed "%.*snon imputées %.*s: %*d"

/* Paired message for Length of PlaceHolders (LPH) */
#define MsgNumberValidObsWithFTI_LPH     11
#define MsgNumberValidObsWithoutFTI_LPH  11
#define MsgNumberValidObsImputed_LPH     11
#define MsgNumberValidObsNotImputed_LPH  11

/********************************************************************************************/
/* Donorimputation and Massimputation                                                       */
/********************************************************************************************/
#define MsgFooterAllByGroup "Le message cité plus haut était valable pour le total de tous les regroupements 'by'."
#define MsgInvalidPcentdonors "" BPN_PERCENT_DONORS " doit être plus grand ou égal à 1 et moins de 100."
#define MsgNoImputationDoneNoRecipient "Il n'y a pas de receveurs dans le regroupement traité. Aucune imputation ne sera faite."
#define MsgNoImputationDoneNotEnoughDonors "Le nombre de donneurs est insuffisant dans le regroupement traité. Aucune imputation ne sera faite."
#define MsgNoImputationDoneNoValidObs "Il n'y a pas d'observations valides dans le regroupement traité. Aucune imputation ne sera faite."
#define MsgNoRandomSearch "%s non spécifié (aucune recherche aléatoire de donneurs ne sera faite)" /* for print parameters */
#define MsgNoLimit "Pas de limite"
#define MsgParmIntegerGE1 "%s doit être un entier plus grand ou égal à 1."
#define MsgParmOutOfRange "La valeur spécifiée pour l'option %s est hors des valeurs acceptables par la plateforme."
#define MsgParmWithDuplicateVariable "Une variable est répétée dans l'option %s."
#define MsgInvalidNLimit "Paramètre " BPN_N_LIMIT " invalide. " BPN_N_LIMIT " doit être un entier plus grand ou égal à 1."
#define MsgInvalidMrl "Paramètre " BPN_MRL " invalide. " BPN_MRL " doit être plus grand que 0."
#define MsgDonorLimitation  "Les paramètres de DONORLIMIT pourraient empêcher certains receveurs d'être imputés."
#define MsgPcentdonorsEqualDoublePcentSymbol "" BPN_PERCENT_DONORS " = %.*f%%"
#define MsgPcentdonorsEqualDoublePcentSymbolDefault "" BPN_PERCENT_DONORS " = %.*f%% (défaut)"

/* Print Statistics
   Format: %.*s --> field width of string "" to output
           %*s  --> field width to extract from a line of 'dot' string  */
/* Total: count only */
#define MsgNumberObservations "Nombre d'observations %.*s: %*d"
#define MsgNumberObservationsDropped "Nombre d'observations rejetées %.*s: %*d"
/* Total: count and percent */
#define MsgNumberPercentDonors "Nombre de donneurs %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentReachedDonorLimit "Nombre de donneurs ayant atteint DONORLIMIT %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentRecipients "Nombre de receveurs %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentValidObs "Nombre d'observations valides %.*s: %*d %.*s %*.*f%%"
/* Subtotal: count only (heading spaces: 1 tab) */
#define MsgNumberObsDroppedExcludedDonors "%.*sdonneurs exclus %.*s: %*d" /* only for Donorimputation */
#define MsgNumberObsDroppedNegativeData "%.*svaleurs négatives %.*s: %*d"
#define MsgNumberObsDroppedNonEligibleDonors "%.*sdonneurs non éligibles %.*s: %*d" /* only for Donorimputation */
#define MsgNumberObsDroppedMissingData "%.*svaleurs manquantes %.*s: %*d"
#define MsgNumberObsDroppedMissingKey "%.*sclé manquante %.*s: %*d"
#define MsgNumberObsDroppedMixed "%.*sobservations mixtes %.*s: %*d"
/* Subtotal: count and percent (heading spaces: 1 tab) */
#define MsgNumberPercentRecipientsWithoutEnoughDonors "%.*snon imputés (nombre insuffisant de donneurs) %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentRecipientsWithoutMatchingFields "%.*ssans champs d'appariement %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentRecipientsWithMatchingFields "%.*savec champs d'appariement %.*s: %*d %.*s %*.*f%%"
/* Subtotal: count and percent (heading spaces: 2 tabs) */
#define MsgNumberPercentRecipientsImputed "%.*simputés %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentRecipientsNotImputed "%.*snon imputés %.*s: %*d %.*s %*.*f%%"

#define MsgNumberObservations_LPH                  7  /* Paired message for Length of PlaceHolders (LPH) */
#define MsgNumberObservationsDropped_LPH           7
#define MsgNumberPercentDonors_LPH                18
#define MsgNumberPercentReachedDonorLimit_LPH     18
#define MsgNumberPercentRecipients_LPH            18
#define MsgNumberPercentValidObs_LPH              18
#define MsgNumberObsDroppedExcludedDonors_LPH     11
#define MsgNumberObsDroppedNegativeData_LPH       11
#define MsgNumberObsDroppedNonEligibleDonors_LPH  11
#define MsgNumberObsDroppedMissingData_LPH        11
#define MsgNumberObsDroppedMissingKey_LPH         11
#define MsgNumberObsDroppedMixed_LPH              11
#define MsgNumberPercentRecipientsWithoutEnoughDonors_LPH    22
#define MsgNumberPercentRecipientsWithoutMatchingFields_LPH  22
#define MsgNumberPercentRecipientsWithMatchingFields_LPH     22
#define MsgNumberPercentRecipientsImputed_LPH     22
#define MsgNumberPercentRecipientsNotImputed_LPH  22

/********************************************************************************************/
/* Donorimputation                                                                          */
/********************************************************************************************/
#define MsgInvalidEligdon "Les mots-clés acceptables pour l'option " BPN_ELIGDON " sont ORIGINAL/O ou ANY/A."
#define MsgParmGenericProblem "Il y a un problème avec l'option %s."
/*the following messages are added for addition of " BPN_RAND_NUM_VAR "*/
#define MsgRandnumvarRandkeylistNotFound "Dans la fonction ReadData(), impossible de trouver la position dans RANDKEY_LIST pour insérer la valeur de la clé du fichier d'entrée " DSN_INDATA "=."
#define MsgRandnumvarDuplicateKey "Dans la fonction ReadData(), une valeur non unique %s a été trouvé dans la liste de nombres aléatoires."
#define MsgRandnumvarKeyInsertFailed "La fonction ReadData() n'a pas pu insérer la valeur de la clé de l'enregistrement courant du fichier d'entrée " DSN_INDATA " dans RANDKEY_LIST."
#define MsgRandumvarInvalidValues "ERRORLOC: La variable spécifiée par l'énoncé " BPN_RAND_NUM_VAR " contient une ou plusieurs valeurs manquantes. Veuillez vérifier le contenu de la variable " BPN_RAND_NUM_VAR "."

#define MsgRandnumvarDonorImputationLessThanZero "DONOR: Le nombre aléatoire du receveur %f est plus petit que 0. Veuillez corriger le nombre aléatoire et relancez la procédure."
#define MsgRandnumvarDonorImputationGreaterThanOne "DONOR: Le nombre aléatoire du receveur %f est plus grand que 1. Veuillez corriger le nombre aléatoire et relancez la procédure." 

/* PostEdits */
#define MsgAddingPositivityPostEditsFail "L'option " BPN_REJECT_NEGATIVE " a été spécifée mais le système échoue lorsqu'il essaie d'ajouter les règles de positivité pour " BPN_POST_EDITS "."
#define MsgDifferentSetVarsInEditsPostEdits "Les règles de vérification " BPN_EDITS " et " BPN_POST_EDITS " ne font pas référence au même ensemble de variables. S'il vous plaît, vérifiez vos règles " BPN_EDITS " et " BPN_POST_EDITS "."
#define MsgEditsTakenAsPostEdits "Aucune règle de vérification après imputation (" BPN_POST_EDITS ") n'a été spécifiée. Les règles de vérification " BPN_EDITS " seront prises comme " BPN_POST_EDITS "."
#define MsgHeaderPostEdits "Ensemble des règles de vérification après imputation (" BPN_POST_EDITS "): "
#define MsgHeaderReducedPostEdits "Ensemble MINIMAL des règles de vérification après imputation (" BPN_POST_EDITS "): "
#define MsgInconsistentPostEdits "Les règles de vérification après imputation sont incohérentes. S'il vous plaît, vérifiez vos règles " BPN_POST_EDITS "."
#define MsgPositivityPostEditsAdded "(Les règles de 'positivité' ont été ajoutées car l'option " BPN_REJECT_NEGATIVE " a été spécifiée.)"
#define MsgReducedPostEditsEquivalent "L'ensemble minimal des règles de vérification après imputation est identique à l'ensemble des règles originales spécifiées."

/* Print info counters */
#define MsgNumberDroppedExcludedDonors "%d observations ont été rejetées de la table " DSN_INDATA " parce que ce sont des donneurs potentiels mais la valeur de %s est 'E' pour ces observations."
#define MsgNumberDroppedMissingValuesInEditsMustmatchNotFTI "%d observations ont été rejetées de la table " DSN_INDATA " parce que une/plusieurs variables des règles de vérification " BPN_EDITS " ou une/plusieurs variables d'appariement " BPN_MUST_MATCH " (autres que celles se retrouvant dans les " BPN_EDITS ") sont manquantes dans la table " DSN_INDATA " et il n'y a pas de statut FTI pour ces variables dans la table " DSN_INSTATUS "."
#define MsgNumberDroppedMixedObs "%d observations mixtes ont été rejetées de la table " DSN_INDATA " parce que une ou plusieurs variables d'appariement " BPN_MUST_MATCH ", autres que celles se retrouvant dans les règles de vérification " BPN_EDITS ", ont un statut FTI dans la table " DSN_INSTATUS "."
#define MsgNumberDroppedNegativeValuesInEditsMustmatchNotFTI "%d observations ont été rejetées de la table " DSN_INDATA " parce que une/plusieurs variables des règles de vérification " BPN_EDITS " ou une/plusieurs variables d'appariement " BPN_MUST_MATCH " (autres que celles se retrouvant dans les " BPN_EDITS ") sont négatives dans la table " DSN_INDATA " et il n'y a pas de statut FTI pour ces variables dans la table " DSN_INSTATUS "."
#define MsgNumberDroppedNonEligibleDonors "%d observations ont été rejetées de la table " DSN_INDATA " parce que ce sont des donneurs potentiels mais ils ne répondent pas au critère spécifié comme " BPN_ELIGDON "."

/********************************************************************************************/
/* Massimputation                                                                           */
/********************************************************************************************/
#define MsgNoImputationInvalidOptions "Options invalides. Au moins l'une des options suivantes doit être spécifiée: " BPN_MUST_MATCH " ou " BPN_RANDOM "."

/* Print info counters */
#define MsgNumberDroppedSomeMissingButNotAllMustimpute "%d observations ont été rejetées de la table " DSN_INDATA " parce qu'une ou plusieurs variables de l'énoncé " BPN_MUST_IMPUTE " sont manquantes, mais non toutes ces variables."
#define MsgNumberDroppedNegativeMustimputeOrMustmatch "%d observations ont été rejetées de la table " DSN_INDATA " parce qu'une ou plusieurs variables des énoncés " BPN_MUST_IMPUTE " ou " BPN_MUST_MATCH " ont une valeur négative."
#define MsgNumberDroppedMixedMissingMustmatch "%d observations mixtes ont été rejetées de la table " DSN_INDATA " parce qu'une ou plusieurs variables de l'énoncé " BPN_MUST_MATCH " sont manquantes pour des donneurs potentiels non encore rejetés."

/********************************************************************************************/
/* Estimator                                                                                */
/********************************************************************************************/
#define MsgAlgorithmWithHistVarAndNoHistDataSet "Au moins un algorithme fait référence à une variable historique et aucune table " DSN_INDATA_HIST " n'a été spécifiée."
#define MsgCountCriteriaDoubleReplacedByInteger "Erreur de conversion pour la variable COUNTCRITERIA dans la table " DSN_INESTIMATOR ". COUNTCRITERIA devrait être un entier. %f est la valeur qui a été spécifiée mais la valeur %d sera celle retenue."
#define MsgExclVarStatementIgnored "L'énoncé %s ne devrait pas être spécifié lorsqu'il n'y a aucune moyenne à calculer, aucune estimation par régression linéaire à traiter ou aucune erreur aléatoire à calculer."
#define MsgHeaderEstimator "Estimateur"
#define MsgHeaderPrintStatistics "Statistiques sur les observations:"
#define MsgHistOptionIgnored "Une table " DSN_INDATA_HIST " a été spécifiée bien qu'aucune variable historique ne fasse partie des spécifications d'estimation. "
#define MsgHistStatusOptionIgnored "Une table " DSN_INSTATUS_HIST " a été spécifiée bien qu'aucune table " DSN_INDATA_HIST " n'ait été spécifiée."
#define MsgMustSpecifyEstimatorSpecs "Aucune spécification d'estimation n'a été fournie."
#define MsgNotEnoughValidObsToProcessEstimators "Le nombre d'observations valides est insuffisant pour traiter les spécifications d'estimation."
#define MsgNumberDroppedMissingKeyNameOneDataSet "%d observations ont été rejetées parce que la variable %s est manquante dans la table %s."
#define MsgOutAcceptableOptionIgnored "Une table " DSN_OUTACCEPTABLE " a été spécifiée alors qu'il n'y a aucune moyenne à calculer et aucune estimation par régression linéaire à traiter."
#define MsgOutEstEFOptionIgnored "Une table " DSN_OUTEST_EF " a été spécifiée alors qu'aucune fonction d'estimation ayant au moins un paramètre n'a été demandée."
#define MsgOutEstLROptionIgnored "Une table " DSN_OUTEST_LR " a été spécifiée alors qu'aucune estimation par régression linéaire n'a été demandée."
#define MsgOutRandomErrorOptionIgnored "Une table " DSN_OUTRAND_ERROR " a été spécifiée alors qu'il n'y a aucune erreur aléatoire à calculer."
#define MsgRedefinitionOfPredefinedAlgorithmName "L'algorithme prédéfini '%s' a été redéfini."
#define MsgVarNameInByAndCurrentEstimatorSpecs "L'énoncé " BPN_BY " et les spécifications d'estimation ayant trait à la période courante font tous deux référence à la variable %s."
#define MsgVarNameInByAndHistoricalEstimatorSpecs "L'énoncé " BPN_BY " et les spécifications d'estimation ayant trait à la période historique font tous deux référence à la variable %s."

/* Print Estimator Specifications
   Format: %.*s --> field width of string "" to output */
#define MsgHeaderPrintEstimatorSpecifications "Ensemble des spécifications d'estimation:"
#define MsgEstimatorNumber "Estimateur %d."
#define MsgDescriptorNameStringValue "%s %.*s: %s"   /* 1st string: see List of DescriptorName */ // removed %m
/* List of DescriptorName */
#define MsgDescriptorAlgorithmName "Nom de l'algorithme"
#define MsgDescriptorAuxiliaryVariables "Variables auxiliaires"
#define MsgDescriptorCountCriteria "Nombre minimal"
#define MsgDescriptorDescription "Description"
#define MsgDescriptorExcludeImputed "Exclure obs. imputées"
#define MsgDescriptorExcludeOutliers "Exclure obs. aberrantes"
#define MsgDescriptorFieldid "Variable"
#define MsgDescriptorFormula "Formule"
#define MsgDescriptorPercentCriteria "Pourcentage minimal"
#define MsgDescriptorRandomError "Erreur aléatoire"
#define MsgDescriptorStatus "Statut"
#define MsgDescriptorType "Type"
#define MsgDescriptorVarianceExponent "Exposant (variance)"
#define MsgDescriptorVariancePeriod "Période (variance)"
#define MsgDescriptorVarianceVariable "Variable de variance"
#define MsgDescriptorWeightVariable "Variable de pondération"
/* Others */
#define MsgTypeInitInvalid "TYPE INVALIDE"
#define MsgAlgorithmNameNotFound "%.*s Nom d'algorithme inexistant."

/* Print Formulas. */
#define MsgHeaderPrintFormulas "Formule pour les spécifications d'estimation:"
#define MsgFieldidEqualFormula "%s = %s"
#define MsgFieldidEqualFormulaRandomError "%s = %s + Erreur aléatoire"

/* Print Imputation Statistics. */
#define MsgHeaderPrintImputationStatistics "Statistiques sur l'imputation:"
#define MsgColumnNegative "Nég"
#define MsgEstimatorNotActive "non actif"

/* Print Parameters Statistics. */
#define MsgHeaderPrintParameters "Statistiques sur les estimateurs:"
#define MsgAcceptableObsCountPercentForEF "Total = %d Pourcentage = %.*f %spondéré." /* String: MsgIndicatorNotWeighted or "" */
#define MsgAcceptableObsCountPercentForLR "Total = %d Pourcentage = %.*f %spondéré et %s variable de variance." /* 1st string: MsgIndicatorNotWeighted or "" / 2nd string: MsgIndicatorHasNotVariance or MsgIndicatorHasVariance*/
#define MsgHeaderAverage "Moyenne"
#define MsgHeaderExponent "Exposant"
#define MsgHeaderPeriod "Période"
#define MsgIndicatorHasNotVariance "sans"
#define MsgIndicatorNotWeighted "non "
#define MsgIndicatorHasVariance "ayant une"
#define MsgIntercept "Ordonnée à l'origine"
#define MsgNoParameters "Aucun paramètre."
#define MsgPeriodCurr "COUR"

/********************************************************************************************/
/* Prorate                                                                                  */
/********************************************************************************************/
#define MsgBasicMethodLowerBoundAndPositivityInconsistent "Avec la méthode de base du calcul proportionnel, la limite inférieure ne peut pas être négative lorsque l'option " BPN_REJECT_NEGATIVE " est active."
#define MsgDataSetInstatusIgnored "La table " DSN_INSTATUS " ne sera pas utilisée: aucun des modificateurs n'est ORIGINAL ou IMPUTED."
#define MsgDataSetInstatusMandatory "La table " DSN_INSTATUS " est obligatoire: au moins un modificateur est ORIGINAL ou IMPUTED."
#define MsgDecimalMustBeIntegerBetweenTwoValues "" BPN_DECIMAL " doit être un entier compris entre %d et %d inclusivement."
#define MsgInvalidMethodStringPlusValidKeywords "" BPN_METHOD " invalide: %s. Les mots-clés acceptables sont: %c, %s, %c, %s."
#define MsgInvalidModifierStringPlusValidKeywords "" BPN_MODIFIER " invalide: %s. Les mots-clés acceptables sont: %c, %s, %c, %s, %c, %s, %c, %s."
#define MsgNoProratingAllModifiersAreString "Tous les modificateurs ont la valeur %s. Il n'y a aucune variable à imputer au moyen du calcul proportionnel."
#define MsgNotConstrainedByUpperBound "Le résultat ne sera pas restreint par une limite supérieure."
#define MsgScalingMethodLowerBoundGEDouble "" BPN_LOWER_BOUND " invalide. " BPN_LOWER_BOUND " doit être plus grand ou égal à %.0f avec la méthode dite 'scaling' du calcul proportionnel."
#define MsgScalingMethodUpperBoundGEDouble "" BPN_UPPER_BOUND " invalide. " BPN_UPPER_BOUND " doit être plus grand ou égal à %.0f avec la méthode dite 'scaling' du calcul proportionnel."
#define MsgUpperBoundGTLowerBound "" BPN_UPPER_BOUND " doit être supérieur à " BPN_LOWER_BOUND "."
#define MsgVarInTwoStatementsExclusive "Une variable fait partie simultanément des énoncés %s et %s. Ces énoncés sont mutuellement exclusifs."

/* Print Edits */
#define MsgHeaderProrateEdits "" BPN_EDITS " analysées par le parseur pour la procédure PRORATE "
#define MsgHeaderEditsModifier "(si aucun modificateur n'est spécifié pour une variable, la valeur de l'option " BPN_MODIFIER " sera utilisée):"
#define MsgHeaderGroup "--- GROUPE ---"
#define MsgEditLevelEqualInteger "Niveau de la règle: %d"

/* Print Statistics */
#define MsgNoImputationDoneNoValidObsByGroup "Aucune observation valide dans le regroupement traité. Aucune imputation n'a été faite."
#define MsgNumberObsDroppedNegativeValueAndPositivityInconsistent "%d observations ont été rejetées de la table " DSN_INDATA " parce qu'une valeur est négative et l'option " BPN_ACCEPT_NEGATIVE " n'a pas été spécifiée."


#endif