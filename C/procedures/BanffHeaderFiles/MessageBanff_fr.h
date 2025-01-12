#ifndef MESSAGEBANFF_FR_H
#define MESSAGEBANFF_FR_H

#include "BanffIdentifiers.h"

/********************************************************************************************/
/* Introduced during redesign conversion                                                    */
/********************************************************************************************/
/* return code descriptions */
#define RC_DESC_SUCCESS "ex�cution r�ussie"
#define RC_DESC_FAIL_UNHANDLED "erreur inattendue"
#define RC_DESC_FAIL_INIT_IN_DATASET "�chec d'initialisation du jeu de donn�es d'entr�e"
#define RC_DESC_FAIL_READ_PARMS_LEGACY "�chec d'obtention des param�tres"
#define RC_DESC_FAIL_MISSING_DATASET "jeu de donn�es obligatoire manquant"
#define RC_DESC_FAIL_VARLIST_NOT_FOUND "variable dans la liste de variables non trouv�e"
#define RC_DESC_FAIL_VARLIST_INVALID_COUNT "nombre de variables non valide dans la liste de variables"
#define RC_DESC_FAIL_VARLIST_SYSTEM_GENERATED "liste de variables g�n�r�e par le syst�me non valide"
#define RC_DESC_FAIL_SETUP_DATASET_IN "�chec de configuration du jeu de donn�es d'entr�e"
#define RC_DESC_FAIL_SETUP_OTHER "erreur de configuration inattendue"
#define RC_DESC_FAIL_ALLOCATE_MEMORY "�chec d'allocation de m�moire"
#define RC_DESC_FAIL_LPI_INIT "�chec d'initialisation LPI"
#define RC_DESC_FAIL_EDITS_PARSE "�chec d'analyse des modifications"
#define RC_DESC_FAIL_VALIDATION_LEGACY "�chec de validation de l'h�ritage"
#define RC_DESC_FAIL_VALIDATION_NEW "�chec de validation"
#define RC_DESC_FAIL_EDITS_OTHER "erreur inattendue li�e aux modifications"
#define RC_DESC_FAIL_EDITS_CONSISTENCY "�chec de la v�rification de coh�rence des modifications"
#define RC_DESC_FAIL_EDITS_REDUNDANCY "�chec des modifications V�rification de redondance"
#define RC_DESC_FAIL_NAME_TOO_LONG "le nom de la variable d�passe la longueur maximale"
#define RC_DESC_FAIL_READ_GENERIC "Erreur inattendue lors de la lecture du jeu de donn�es"
#define RC_DESC_FAIL_WRONG_SORT_ORDER "Ordre de tri non valide"
#define RC_DESC_FAIL_READ_SYNC "�chec de la synchronisation des jeux de donn�es"
#define RC_DESC_FAIL_READ_DUPLICATE_DATA "Donn�es en double d�tect�es"
#define RC_DESC_FAIL_WRITE_GENERIC "�chec de l'�criture dans l'ensemble de donn�es de sortie"
#define RC_DESC_FAIL_PROCESSING_GENERIC "Erreur de traitement inattendue"
#define RC_DESC_EIE_TRANSFORM_FAIL "�chec de la transformation"
#define RC_DESC_EIE_KDTREE_FAIL "�chec KDTREE"
#define RC_DESC_EIE_MATCHFIELDS_FAIL "Erreur de correspondance des champs"

/********************************************************************************************/
/* Generic to Banff                                                                         */
/********************************************************************************************/
#define MsgParmMandatory "%s est obligatoire."
#define MsgNumericVarNotInDataSet "La variable num�rique %s n'est pas dans la table %s."
#define MsgVarNotNumericInDataSet "La variable %s n'est pas num�rique dans la table %s."
#define MsgCharacterVarNotInDataSet "La variable caract�re %s n'est pas dans la table %s."
#define MsgVarNotCharacterInDataSet "La variable %s n'est pas caract�re dans la table %s."
#define MsgParmNotSpecified "%s non sp�cifi�."

#define MsgNumberObs "Nombre d'observations dans la table %s %.*s: %*d"
#define MsgNumberValidObs "Nombre d'observations valides dans la table %s %.*s: %*d"
#define MsgNumberObs_LPH       9  /* Paired message for Length of PlaceHolders (LPH) */
#define MsgNumberValidObs_LPH  9

/********************************************************************************************/
/* Generic to Banff procedures                                                              */
/********************************************************************************************/
/* All procedures */
#define MsgDefault " (d�faut)"
#define MsgNotUsed " (pas utilis�)"
#define MsgFunctionFailure "%s a �chou�."
#define MsgParmInvalid "%s invalide."
#define MsgPositivityOptionInvalid "Les options " BPN_REJECT_NEGATIVE " et " BPN_ACCEPT_NEGATIVE " ne peuvent pas �tre sp�cifi�es en m�me temps."                           
#define MsgStatementMandatory "L'�nonc� %s est obligatoire."
#define MsgParmGreater0 "La valeur doit �tre sup�rieure � 0."
#define MsgParmGreaterEqual0 "La valeur doit �tre sup�rieure ou �gale � 0."

/* Flag type parameters */
#define MsgAcceptNegative BPN_ACCEPT_NEGATIVE " = True"
#define MsgRejectNegative BPN_ACCEPT_NEGATIVE " = False"
#define MsgRejectNegativeDefault BPN_ACCEPT_NEGATIVE " = False" MsgDefault
#define MsgAcceptZero BPN_ACCEPT_ZERO " = True"
#define MsgAcceptZeroDefault BPN_ACCEPT_ZERO " = True" MsgDefault
#define MsgRejectZero BPN_ACCEPT_ZERO " = False"
#define MsgRejectZeroDefault BPN_ACCEPT_ZERO " = False" MsgDefault

/* LP solver */
#define MsgInitLPFail "Incapable d'initialiser le solutionneur de programmation lin�aire %s."
#define MsgInitLPSuccess "Le solutionneur de programmation lin�aire %s, version %s, a �t� initialis�."

/* Seed */
#define MsgSeedEqualIntegerChosenBySystem "" BPN_SEED " = %d (valeur choisie par le syst�me)"
#define MsgSeedOutOfRangeGeneratedNumberTaken "La valeur sp�cifi�e pour l'option " BPN_SEED " exc�de le maximum acceptable par la plateforme et ne peut pas �tre retenue. Cette valeur a �t� remplac�e par un nombre al�atoire."
#define MsgSeedNegativeGeneratedNumberTaken "La valeur sp�cifi�e pour l'option " BPN_SEED " est n�gative et ne peut pas �tre retenue. Cette valeur a �t� remplac�e par un nombre al�atoire."
#define MsgSeedZeroGeneratedNumberTaken "La valeur sp�cifi�e pour l'option " BPN_SEED " est 0 et ne peut pas �tre retenue. Cette valeur a �t� remplac�e par un nombre al�atoire."

/* Edits */
#define MsgAddingPositivityEditsFail "L'option " BPN_REJECT_NEGATIVE " est active mais le syst�me �choue lorsqu'il essaie d'ajouter les r�gles de positivit� � l'ensemble des r�gles originales."
#define MsgEditsVarNotInDataSet "La variable %s sp�cifi�e dans les r�gles (" BPN_EDITS ") n'est pas dans la table %s."
#define MsgEditsVarNotNumericInDataSet "La variable %s sp�cifi�e dans les r�gles (" BPN_EDITS ") n'est pas num�rique dans la table %s."
#define MsgHeaderEdits "Ensemble des r�gles de v�rification (" BPN_EDITS "): "
#define MsgHeaderReducedEdits "Ensemble MINIMAL des r�gles de v�rification (" BPN_EDITS "): "
#define MsgInconsistentEdits "Les r�gles de v�rification sont incoh�rentes. S'il vous pla�t, v�rifiez vos r�gles (" BPN_EDITS ")."
#define MsgPositivityEditsAdded "(Les r�gles de 'positivit�' ont �t� ajout�es car l'option " BPN_REJECT_NEGATIVE " est active.)"
#define MsgReducedEditsEquivalent "L'ensemble minimal des r�gles de v�rification est identique � l'ensemble des r�gles originales sp�cifi�es."

/* Miscellaneous (found in more than one procedure)*/
#define MsgDataSetWithTwoKeysDuplicate "Les cl�s de la table %s ne sont pas uniques. La cl� courante est %s et la cl� pr�c�dente �tait %s."
#define MsgDataSetWithTwoKeysNotSorted "La table %s n'est pas tri�e en ordre ascendant. La cl� courante est %s et la cl� pr�c�dente �tait %s."
#define MsgByVarsNotFoundInDataSet "Une ou plusieurs variables " BPN_BY " ne sont pas pr�sentes sur la table %s." /* HIST or AUX */
#define MsgFasterPerformanceIfByVarsInDataSet "L'ex�cution de cette proc�dure pourrait �tre plus rapide si la table %s contenait les variables " BPN_BY "."
#define MsgKeyNameWithDuplicateValueInDataSet "%s est non unique pour la valeur '%s' dans la table %s."
#define MsgNoObservationsInDataSet "Aucune observation dans la table %s."
#define MsgNumberDroppedMissingKeyNameInDataSet "%d observations ont �t� rejet�es de la table %s parce que la variable %s est manquante."
#define MsgNumberDroppedMissingKeyNameOrFieldidInDataSet "%d observations ont �t� rejet�es de la table %s parce que la variable %s ou FIELDID est manquante."
#define MsgNumberDroppedMissingValuesInDataSet1NotFTIInDataSet2 "%d observations ont �t� rejet�es de la table %s parce qu'une valeur est manquante et il n'y a aucune observation correspondante dans la table %s avec un statut FTI."
#define MsgNumberDroppedNegativeValuesInDataSet1NotFTIInDataSet2 "%d observations ont �t� rejet�es de la table %s parce qu'une valeur est n�gative et il n'y a aucune observation correspondante dans la table %s avec un statut FTI."
#define MsgTwoStatementsExclusive "Les listes de variables des �nonc�s %s et %s sont mutuellemement exclusives."
#define MsgVarNameInTwoStatementsExclusive "La variable %s fait partie des �nonc�s %s et %s. Ces �nonc�s sont mutuellement exclusifs."

/********************************************************************************************/
/* Verifyedits                                                                              */
/********************************************************************************************/
#define MsgNoExtremalPrinted "L'option " BPN_EXTREMAL " n'est pas disponible lorsque l'option " BPN_ACCEPT_NEGATIVE " est sp�cifi�e. Aucun point extr�mal ne sera calcul�."

/* Parm: " BPN_IMPLY " and " BPN_EXTREMAL " */
#define MsgParmInvalidNegativeValueReplacedByDefault "La valeur sp�cifi�e pour %s est n�gative; cette valeur est invalide. La valeur par d�faut a �t� prise comme valeur de remplacement."

/********************************************************************************************/
/* Editstats                                                                              */
/********************************************************************************************/
#define MsgAllOutDataSetsNULL "Aucune analyse n'est faite par la proc�dure. _NULL_ a �t� sp�cifi� pour chacun des ensembles de donn�es de sortie."
#define MsgEditSizeExceededMax "L'espace n�cessaire pour d�finir la variable caract�re EDIT dans la table de sortie %s d�passe la longueur maximum acceptable."

/********************************************************************************************/
/* Outlier                                                                                  */
/********************************************************************************************/
#define MsgAtLeastMIIorMEIMustBeSpecified "Les multiplicateurs " BPN_MEI " et " BPN_MII ", ou au moins l'un d'entre eux, doivent �tre sp�cifi�s."
#define MsgAtLeastBETAIorBETAEMustBeSpecified "Les multiplicateurs " BPN_BETA_E " et " BPN_BETA_I ", ou au moins l'un d'entre eux, doivent �tre sp�cifi�s."
#define MsgMIIMustBeGreaterThanMEI "" BPN_MII " doit avoir une valeur plus grande que celle du " BPN_MEI "."
#define MsgBETAIMustBeGreaterThanBETAE "" BPN_BETA_I " doit avoir une valeur plus grande que celle du " BPN_BETA_E "."

#define MsgHeaderForByGroup "Les messages suivants concernent le regroupement " BPN_BY ":%-2n%*s" /* %-2n: specific to SAS_XP routines */
#define MsgHeaderForByGroup_SAS_FREE "Les messages suivants concernent le regroupement " BPN_BY ":" "\n" MSG_INDENT_NOTE
#define MsgHeaderForByGroupAbove_SAS_FREE "Le message ci-dessus concernait le groupe " BPN_BY "  suivant :" "\n" MSG_INDENT_NOTE
#define MsgHeaderForVariable "Les messages suivants concernent la variable:%-2n%*s"       /* %-2n: specific to SAS_XP routines */
#define MsgHeaderForVariable_SAS_FREE "Les messages suivants concernent la variable:\n      %*s"

#define MsgIdValueDuplicatedInDataSet "La cl� '%s' est non unique dans la table %s."
#define MsgMinimumValueOfValidObsNotReached "Nombre d'observations valides dans la table " DSN_INDATA " est inf�rieur au " BPN_MIN_OBS " %d. Aucune identification de valeurs aberrantes."
#define MsgVarNameNotInDataSet "La variable %s n'est pas pr�sente sur la table %s."
#define MsgStatementEmpty "Au moins une variable doit �tre sp�cifi�e avec l'�nonc� %s."
#define MsgMINOBSInvalidValue "Valeur invalide pour " BPN_MIN_OBS ". La valeur doit �tre plus grande ou �gale � %d."
#define MsgMINOBSWarningValue "La valeur de " BPN_MIN_OBS " est plus petite que %d. La m�thode est plus efficace avec un minimum de %d observations."
#define MsgInvalidExponent "La valeur doit �tre sup�rieure ou �gale � 0 et inf�rieure ou �gale � 1."
#define MsgInvalidMethod "Les mots-cl�s acceptables sont: CURRENT, HISTORIC, RATIO, SIGMAGAP."
#define MsgInvalidSigma "Les mots-cl�s acceptables sont: STD, MAD."
#define MsgInvalidSide "Les mots-cl�s acceptables sont: LEFT, BOTH, RIGHT."
#define MsgInvalidStartCentile "La valeur doit �tre sup�rieure ou �gale � %.0f et inf�rieure � %.0f."
#define MsgMultipleStatements "Plusieurs �nonc�s %s. Seulement une est permise."

/* CU method */
#define MsgHISTDataSetIgnored "La table %s a �t� sp�cifi�e avec la m�thode des donn�es courantes. Cette table ne sera pas utilis�e." /* HIST or AUX */
#define MsgExponentIgnored "L'option " BPN_EXPONENT " a �t� sp�cifi�e avec la m�thode des donn�es courantes ou SIGMAGAP. Cette option ne sera pas utilis�e."
#define MsgNoDefaultForVar "Il est impossible de construire une liste par d�faut pour l'�nonc� " BPN_VAR " car aucune variable num�rique n'est pr�sente sur la table " DSN_INDATA "."
#define MsgWithIgnored "Un �nonc� " BPN_WITH " a �t� sp�cifi� avec la m�thode des donn�es courantes. Cet �nonc� sera ignor�."

/* HT/Ratio method */
#define MsgHTRatioMethodAcceptNegativeIgnored "" BPN_ACCEPT_NEGATIVE " ne devrait pas �tre sp�cifi�e avec la m�thode des ratios ou historique. Pour des raisons m�thodologiques, les valeurs 0 et n�gatives ne seront jamais retenues."
#define MsgHTRatioMethodAcceptZeroIgnored "" BPN_ACCEPT_ZERO " ne devrait pas �tre sp�cifi�e avec la m�thode des ratios ou historique. Pour des raisons m�thodologiques, les valeurs 0 et n�gatives ne seront jamais retenues."
#define MsgHTRatioMethodBadSpecifications "La m�thode des ratios ou historique ne peut pas �tre utilis�e avec les sp�cifications fournies."
#define MsgHTRatioMethodDifferentSizeForVarAndWith "Les �nonc�s " BPN_VAR " et " BPN_WITH " comprennent un nombre diff�rent de variables."

/* SIGMAGAP method */
/*
#define MsgSigmagapMethodAcceptNegativeIgnored "" BPN_ACCEPT_NEGATIVE " ne devrait pas �tre sp�cifi�e avec la m�thode sigmagap � deux variables. Pour des raisons m�thodologiques, les valeurs 0 et n�gatives ne seront jamais retenues."
#define MsgSigmagapMethodAcceptZeroIgnored "" BPN_ACCEPT_ZERO " ne devrait pas �tre sp�cifi�e avec la m�thode sigmagap � deux variables. Pour des raisons m�thodologiques, les valeurs 0 et n�gatives ne seront jamais retenues."
*/
/* Print count of "bad data" by " BPN_VAR ". */
#define MsgNumberDroppedInDataSetMissingValueForVar "%d observations ont �t� rejet�es de la table %s parce que la variable %*s est manquante."
#define MsgNumberDroppedInDataSetNegativeValueForVar "%d observations ont �t� rejet�es de la table %s parce que la variable %*s est n�gative."
#define MsgNumberDroppedInDataSetNegativeValueForWeightedVar "%d observations ont �t� rejet�es de la table %s parce que la variable pond�r�e %*s est n�gative."
#define MsgNumberDroppedInDataSetZeroValueForVar "%d observations ont �t� rejet�es de la table %s parce que la variable %*s est 0."
#define MsgNumberDroppedInDataSetZeroValueForWeightedVar "%d observations ont �t� rejet�es de la table %s parce que la variable pond�r�e %*s est 0."

/* Print weight data quality warnings. */
#define MsgNegativeWeightWarning "Au moins une variable " BPN_WEIGHT " est n�gative."
#define MsgZeroWeightWarning "Au moins une variable " BPN_WEIGHT " est �gale � z�ro."

/* Print Statistics -
   Format: %.*s --> field width of string "" to output
           %*s  --> field width to extract from a line of 'dot' string
   Subtotal: count only (heading spaces: 1 tab) */
#define MsgNumberDroppedIDOnlyInDataSet "%.*srejet�es parce que " BPN_UNIT_ID " est pr�sente uniquement sur %s %.*s: %*d"
#define MsgNumberDroppedMissingID "%.*srejet�es parce que " BPN_UNIT_ID " est manquante %.*s: %*d"
#define MsgNumberDroppedMissingWeight "%.*srejet�es parce que " BPN_WEIGHT " est manquant %.*s: %*d"
#define MsgNumberDroppedIDOnlyInDataSet_LPH  13  /* Paired Msg for Length of PlaceHolders (LPH) */
#define MsgNumberDroppedMissingID_LPH        11
#define MsgNumberDroppedMissingWeight_LPH    11

/********************************************************************************************/
/* Errorloc                                                                                 */
/********************************************************************************************/
#define MsgNotConstrainedByCardinality "Le r�sultat ne sera pas limit� par la cardinalit�."
#define MsgRedundantEditsFail "Incapable de d�celer les r�gles redondantes (" BPN_EDITS ")."
#define MsgTimePerObsAdjustedToMinValue "La valeur de " BPN_TIME_PER_OBS " est invalide et a �t� ajust�e � %.2f secondes, soit le temps minimum par observation."
#define MsgVarNameUnbounded "La variable %s n'est pas born�e. Une borne inf�rieure ou sup�rieure DOIT exister."
#define MsgWeightVarDefault "Un poids de 1 sera attribu� � chaque variable sp�cifi�e dans les r�gles de v�rification."
/*this message is used for both errorloc and donorimp when both the " BPN_SEED " and " BPN_RAND_NUM_VAR " parameters have been specified*/
#define MsgRanndumvarSeedSpecified "Les param�tres " BPN_SEED " et " BPN_RAND_NUM_VAR " ont tous les deux �t� sp�cifi�s. Le param�tre " BPN_SEED " sera ignor�."
/*the following messages are used for validating the value of " BPN_RAND_NUM_VAR "*/
#define MsgRandnumvarErrorlocLessThanZero_main "ERRORLOC: Le nombre al�atoire de l'enregistrement %f est plus petit que 0. Veuillez corriger le nombre al�atoire et relancez la proc�dure."
#define MsgRandnumvarErrorlocGreaterThanOne_main "ERRORLOC: Le nombre al�atoire de l'enregistrement %f est plus grand que 1. Veuillez corriger le nombre al�atoire et relancez la proc�dure."

/* Print Statistics
   Format: %.*s --> field width of string "" to output
           %*s  --> field width to extract from a line of 'dot' string  */
/* Total: count and percent */
#define MsgNoValidDataInGroup "Aucune observation valide dans le regoupement trait�."
#define MsgNumberPercentObs "Nombre d'observations %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentObsDroppedMissingKey "Nombre d'observations rejet�es car la cl� est manquante %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentObsFailedEdits "Nombre d'observations ayant �chou� les r�gles %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentObsPassedEdits "Nombre d'observations ayant r�ussi les r�gles %.*s: %*d %.*s %*.*f%%"
/* Subtotal: count and percent (heading spaces: 1 tab) */
#define MsgNumberPercentObsNoSolution "%.*sNombre d'observations sans solution %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentObsWithVarsToImpute "%.*sNombre d'observations ayant des variables � imputer %.*s: %*d %.*s %*.*f%%"
/* Subtotal: count and percent (heading spaces: 2 tabs) */
#define MsgNumberPercentObsNoSolutionCardinality "%.*sd� � une contrainte de cardinalit� %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentObsNoSolutionSpace "%.*sd� � une contrainte d'espace m�moire %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentObsNoSolutionTime "%.*sd� � une contrainte de temps %.*s: %*d %.*s %*.*f%%"
/* Subtotal: count only (heading spaces: 2 tabs) */
#define MsgNumberVarsToImpute "%.*sNombre de variables � imputer %.*s: %*d"

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
#define MsgNumberValidObsImputed "%.*simput�es %.*s: %*d"
#define MsgNumberValidObsNotImputed "%.*snon imput�es %.*s: %*d"

/* Paired message for Length of PlaceHolders (LPH) */
#define MsgNumberValidObsWithFTI_LPH     11
#define MsgNumberValidObsWithoutFTI_LPH  11
#define MsgNumberValidObsImputed_LPH     11
#define MsgNumberValidObsNotImputed_LPH  11

/********************************************************************************************/
/* Donorimputation and Massimputation                                                       */
/********************************************************************************************/
#define MsgFooterAllByGroup "Le message cit� plus haut �tait valable pour le total de tous les regroupements 'by'."
#define MsgInvalidPcentdonors "" BPN_PERCENT_DONORS " doit �tre plus grand ou �gal � 1 et moins de 100."
#define MsgNoImputationDoneNoRecipient "Il n'y a pas de receveurs dans le regroupement trait�. Aucune imputation ne sera faite."
#define MsgNoImputationDoneNotEnoughDonors "Le nombre de donneurs est insuffisant dans le regroupement trait�. Aucune imputation ne sera faite."
#define MsgNoImputationDoneNoValidObs "Il n'y a pas d'observations valides dans le regroupement trait�. Aucune imputation ne sera faite."
#define MsgNoRandomSearch "%s non sp�cifi� (aucune recherche al�atoire de donneurs ne sera faite)" /* for print parameters */
#define MsgNoLimit "Pas de limite"
#define MsgParmIntegerGE1 "%s doit �tre un entier plus grand ou �gal � 1."
#define MsgParmOutOfRange "La valeur sp�cifi�e pour l'option %s est hors des valeurs acceptables par la plateforme."
#define MsgParmWithDuplicateVariable "Une variable est r�p�t�e dans l'option %s."
#define MsgInvalidNLimit "Param�tre " BPN_N_LIMIT " invalide. " BPN_N_LIMIT " doit �tre un entier plus grand ou �gal � 1."
#define MsgInvalidMrl "Param�tre " BPN_MRL " invalide. " BPN_MRL " doit �tre plus grand que 0."
#define MsgDonorLimitation  "Les param�tres de DONORLIMIT pourraient emp�cher certains receveurs d'�tre imput�s."
#define MsgPcentdonorsEqualDoublePcentSymbol "" BPN_PERCENT_DONORS " = %.*f%%"
#define MsgPcentdonorsEqualDoublePcentSymbolDefault "" BPN_PERCENT_DONORS " = %.*f%% (d�faut)"

/* Print Statistics
   Format: %.*s --> field width of string "" to output
           %*s  --> field width to extract from a line of 'dot' string  */
/* Total: count only */
#define MsgNumberObservations "Nombre d'observations %.*s: %*d"
#define MsgNumberObservationsDropped "Nombre d'observations rejet�es %.*s: %*d"
/* Total: count and percent */
#define MsgNumberPercentDonors "Nombre de donneurs %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentReachedDonorLimit "Nombre de donneurs ayant atteint DONORLIMIT %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentRecipients "Nombre de receveurs %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentValidObs "Nombre d'observations valides %.*s: %*d %.*s %*.*f%%"
/* Subtotal: count only (heading spaces: 1 tab) */
#define MsgNumberObsDroppedExcludedDonors "%.*sdonneurs exclus %.*s: %*d" /* only for Donorimputation */
#define MsgNumberObsDroppedNegativeData "%.*svaleurs n�gatives %.*s: %*d"
#define MsgNumberObsDroppedNonEligibleDonors "%.*sdonneurs non �ligibles %.*s: %*d" /* only for Donorimputation */
#define MsgNumberObsDroppedMissingData "%.*svaleurs manquantes %.*s: %*d"
#define MsgNumberObsDroppedMissingKey "%.*scl� manquante %.*s: %*d"
#define MsgNumberObsDroppedMixed "%.*sobservations mixtes %.*s: %*d"
/* Subtotal: count and percent (heading spaces: 1 tab) */
#define MsgNumberPercentRecipientsWithoutEnoughDonors "%.*snon imput�s (nombre insuffisant de donneurs) %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentRecipientsWithoutMatchingFields "%.*ssans champs d'appariement %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentRecipientsWithMatchingFields "%.*savec champs d'appariement %.*s: %*d %.*s %*.*f%%"
/* Subtotal: count and percent (heading spaces: 2 tabs) */
#define MsgNumberPercentRecipientsImputed "%.*simput�s %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentRecipientsNotImputed "%.*snon imput�s %.*s: %*d %.*s %*.*f%%"

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
#define MsgInvalidEligdon "Les mots-cl�s acceptables pour l'option " BPN_ELIGDON " sont ORIGINAL/O ou ANY/A."
#define MsgParmGenericProblem "Il y a un probl�me avec l'option %s."
/*the following messages are added for addition of " BPN_RAND_NUM_VAR "*/
#define MsgRandnumvarRandkeylistNotFound "Dans la fonction ReadData(), impossible de trouver la position dans RANDKEY_LIST pour ins�rer la valeur de la cl� du fichier d'entr�e " DSN_INDATA "=."
#define MsgRandnumvarDuplicateKey "Dans la fonction ReadData(), une valeur non unique %s a �t� trouv� dans la liste de nombres al�atoires."
#define MsgRandnumvarKeyInsertFailed "La fonction ReadData() n'a pas pu ins�rer la valeur de la cl� de l'enregistrement courant du fichier d'entr�e " DSN_INDATA " dans RANDKEY_LIST."
#define MsgRandumvarInvalidValues "ERRORLOC: La variable sp�cifi�e par l'�nonc� " BPN_RAND_NUM_VAR " contient une ou plusieurs valeurs manquantes. Veuillez v�rifier le contenu de la variable " BPN_RAND_NUM_VAR "."

#define MsgRandnumvarDonorImputationLessThanZero "DONOR: Le nombre al�atoire du receveur %f est plus petit que 0. Veuillez corriger le nombre al�atoire et relancez la proc�dure."
#define MsgRandnumvarDonorImputationGreaterThanOne "DONOR: Le nombre al�atoire du receveur %f est plus grand que 1. Veuillez corriger le nombre al�atoire et relancez la proc�dure." 

/* PostEdits */
#define MsgAddingPositivityPostEditsFail "L'option " BPN_REJECT_NEGATIVE " a �t� sp�cif�e mais le syst�me �choue lorsqu'il essaie d'ajouter les r�gles de positivit� pour " BPN_POST_EDITS "."
#define MsgDifferentSetVarsInEditsPostEdits "Les r�gles de v�rification " BPN_EDITS " et " BPN_POST_EDITS " ne font pas r�f�rence au m�me ensemble de variables. S'il vous pla�t, v�rifiez vos r�gles " BPN_EDITS " et " BPN_POST_EDITS "."
#define MsgEditsTakenAsPostEdits "Aucune r�gle de v�rification apr�s imputation (" BPN_POST_EDITS ") n'a �t� sp�cifi�e. Les r�gles de v�rification " BPN_EDITS " seront prises comme " BPN_POST_EDITS "."
#define MsgHeaderPostEdits "Ensemble des r�gles de v�rification apr�s imputation (" BPN_POST_EDITS "): "
#define MsgHeaderReducedPostEdits "Ensemble MINIMAL des r�gles de v�rification apr�s imputation (" BPN_POST_EDITS "): "
#define MsgInconsistentPostEdits "Les r�gles de v�rification apr�s imputation sont incoh�rentes. S'il vous pla�t, v�rifiez vos r�gles " BPN_POST_EDITS "."
#define MsgPositivityPostEditsAdded "(Les r�gles de 'positivit�' ont �t� ajout�es car l'option " BPN_REJECT_NEGATIVE " a �t� sp�cifi�e.)"
#define MsgReducedPostEditsEquivalent "L'ensemble minimal des r�gles de v�rification apr�s imputation est identique � l'ensemble des r�gles originales sp�cifi�es."

/* Print info counters */
#define MsgNumberDroppedExcludedDonors "%d observations ont �t� rejet�es de la table " DSN_INDATA " parce que ce sont des donneurs potentiels mais la valeur de %s est 'E' pour ces observations."
#define MsgNumberDroppedMissingValuesInEditsMustmatchNotFTI "%d observations ont �t� rejet�es de la table " DSN_INDATA " parce que une/plusieurs variables des r�gles de v�rification " BPN_EDITS " ou une/plusieurs variables d'appariement " BPN_MUST_MATCH " (autres que celles se retrouvant dans les " BPN_EDITS ") sont manquantes dans la table " DSN_INDATA " et il n'y a pas de statut FTI pour ces variables dans la table " DSN_INSTATUS "."
#define MsgNumberDroppedMixedObs "%d observations mixtes ont �t� rejet�es de la table " DSN_INDATA " parce que une ou plusieurs variables d'appariement " BPN_MUST_MATCH ", autres que celles se retrouvant dans les r�gles de v�rification " BPN_EDITS ", ont un statut FTI dans la table " DSN_INSTATUS "."
#define MsgNumberDroppedNegativeValuesInEditsMustmatchNotFTI "%d observations ont �t� rejet�es de la table " DSN_INDATA " parce que une/plusieurs variables des r�gles de v�rification " BPN_EDITS " ou une/plusieurs variables d'appariement " BPN_MUST_MATCH " (autres que celles se retrouvant dans les " BPN_EDITS ") sont n�gatives dans la table " DSN_INDATA " et il n'y a pas de statut FTI pour ces variables dans la table " DSN_INSTATUS "."
#define MsgNumberDroppedNonEligibleDonors "%d observations ont �t� rejet�es de la table " DSN_INDATA " parce que ce sont des donneurs potentiels mais ils ne r�pondent pas au crit�re sp�cifi� comme " BPN_ELIGDON "."

/********************************************************************************************/
/* Massimputation                                                                           */
/********************************************************************************************/
#define MsgNoImputationInvalidOptions "Options invalides. Au moins l'une des options suivantes doit �tre sp�cifi�e: " BPN_MUST_MATCH " ou " BPN_RANDOM "."

/* Print info counters */
#define MsgNumberDroppedSomeMissingButNotAllMustimpute "%d observations ont �t� rejet�es de la table " DSN_INDATA " parce qu'une ou plusieurs variables de l'�nonc� " BPN_MUST_IMPUTE " sont manquantes, mais non toutes ces variables."
#define MsgNumberDroppedNegativeMustimputeOrMustmatch "%d observations ont �t� rejet�es de la table " DSN_INDATA " parce qu'une ou plusieurs variables des �nonc�s " BPN_MUST_IMPUTE " ou " BPN_MUST_MATCH " ont une valeur n�gative."
#define MsgNumberDroppedMixedMissingMustmatch "%d observations mixtes ont �t� rejet�es de la table " DSN_INDATA " parce qu'une ou plusieurs variables de l'�nonc� " BPN_MUST_MATCH " sont manquantes pour des donneurs potentiels non encore rejet�s."

/********************************************************************************************/
/* Estimator                                                                                */
/********************************************************************************************/
#define MsgAlgorithmWithHistVarAndNoHistDataSet "Au moins un algorithme fait r�f�rence � une variable historique et aucune table " DSN_INDATA_HIST " n'a �t� sp�cifi�e."
#define MsgCountCriteriaDoubleReplacedByInteger "Erreur de conversion pour la variable COUNTCRITERIA dans la table " DSN_INESTIMATOR ". COUNTCRITERIA devrait �tre un entier. %f est la valeur qui a �t� sp�cifi�e mais la valeur %d sera celle retenue."
#define MsgExclVarStatementIgnored "L'�nonc� %s ne devrait pas �tre sp�cifi� lorsqu'il n'y a aucune moyenne � calculer, aucune estimation par r�gression lin�aire � traiter ou aucune erreur al�atoire � calculer."
#define MsgHeaderEstimator "Estimateur"
#define MsgHeaderPrintStatistics "Statistiques sur les observations:"
#define MsgHistOptionIgnored "Une table " DSN_INDATA_HIST " a �t� sp�cifi�e bien qu'aucune variable historique ne fasse partie des sp�cifications d'estimation. "
#define MsgHistStatusOptionIgnored "Une table " DSN_INSTATUS_HIST " a �t� sp�cifi�e bien qu'aucune table " DSN_INDATA_HIST " n'ait �t� sp�cifi�e."
#define MsgMustSpecifyEstimatorSpecs "Aucune sp�cification d'estimation n'a �t� fournie."
#define MsgNotEnoughValidObsToProcessEstimators "Le nombre d'observations valides est insuffisant pour traiter les sp�cifications d'estimation."
#define MsgNumberDroppedMissingKeyNameOneDataSet "%d observations ont �t� rejet�es parce que la variable %s est manquante dans la table %s."
#define MsgOutAcceptableOptionIgnored "Une table " DSN_OUTACCEPTABLE " a �t� sp�cifi�e alors qu'il n'y a aucune moyenne � calculer et aucune estimation par r�gression lin�aire � traiter."
#define MsgOutEstEFOptionIgnored "Une table " DSN_OUTEST_EF " a �t� sp�cifi�e alors qu'aucune fonction d'estimation ayant au moins un param�tre n'a �t� demand�e."
#define MsgOutEstLROptionIgnored "Une table " DSN_OUTEST_LR " a �t� sp�cifi�e alors qu'aucune estimation par r�gression lin�aire n'a �t� demand�e."
#define MsgOutRandomErrorOptionIgnored "Une table " DSN_OUTRAND_ERROR " a �t� sp�cifi�e alors qu'il n'y a aucune erreur al�atoire � calculer."
#define MsgRedefinitionOfPredefinedAlgorithmName "L'algorithme pr�d�fini '%s' a �t� red�fini."
#define MsgVarNameInByAndCurrentEstimatorSpecs "L'�nonc� " BPN_BY " et les sp�cifications d'estimation ayant trait � la p�riode courante font tous deux r�f�rence � la variable %s."
#define MsgVarNameInByAndHistoricalEstimatorSpecs "L'�nonc� " BPN_BY " et les sp�cifications d'estimation ayant trait � la p�riode historique font tous deux r�f�rence � la variable %s."

/* Print Estimator Specifications
   Format: %.*s --> field width of string "" to output */
#define MsgHeaderPrintEstimatorSpecifications "Ensemble des sp�cifications d'estimation:"
#define MsgEstimatorNumber "Estimateur %d."
#define MsgDescriptorNameStringValue "%s %.*s: %s"   /* 1st string: see List of DescriptorName */ // removed %m
/* List of DescriptorName */
#define MsgDescriptorAlgorithmName "Nom de l'algorithme"
#define MsgDescriptorAuxiliaryVariables "Variables auxiliaires"
#define MsgDescriptorCountCriteria "Nombre minimal"
#define MsgDescriptorDescription "Description"
#define MsgDescriptorExcludeImputed "Exclure obs. imput�es"
#define MsgDescriptorExcludeOutliers "Exclure obs. aberrantes"
#define MsgDescriptorFieldid "Variable"
#define MsgDescriptorFormula "Formule"
#define MsgDescriptorPercentCriteria "Pourcentage minimal"
#define MsgDescriptorRandomError "Erreur al�atoire"
#define MsgDescriptorStatus "Statut"
#define MsgDescriptorType "Type"
#define MsgDescriptorVarianceExponent "Exposant (variance)"
#define MsgDescriptorVariancePeriod "P�riode (variance)"
#define MsgDescriptorVarianceVariable "Variable de variance"
#define MsgDescriptorWeightVariable "Variable de pond�ration"
/* Others */
#define MsgTypeInitInvalid "TYPE INVALIDE"
#define MsgAlgorithmNameNotFound "%.*s Nom d'algorithme inexistant."

/* Print Formulas. */
#define MsgHeaderPrintFormulas "Formule pour les sp�cifications d'estimation:"
#define MsgFieldidEqualFormula "%s = %s"
#define MsgFieldidEqualFormulaRandomError "%s = %s + Erreur al�atoire"

/* Print Imputation Statistics. */
#define MsgHeaderPrintImputationStatistics "Statistiques sur l'imputation:"
#define MsgColumnNegative "N�g"
#define MsgEstimatorNotActive "non actif"

/* Print Parameters Statistics. */
#define MsgHeaderPrintParameters "Statistiques sur les estimateurs:"
#define MsgAcceptableObsCountPercentForEF "Total = %d Pourcentage = %.*f %spond�r�." /* String: MsgIndicatorNotWeighted or "" */
#define MsgAcceptableObsCountPercentForLR "Total = %d Pourcentage = %.*f %spond�r� et %s variable de variance." /* 1st string: MsgIndicatorNotWeighted or "" / 2nd string: MsgIndicatorHasNotVariance or MsgIndicatorHasVariance*/
#define MsgHeaderAverage "Moyenne"
#define MsgHeaderExponent "Exposant"
#define MsgHeaderPeriod "P�riode"
#define MsgIndicatorHasNotVariance "sans"
#define MsgIndicatorNotWeighted "non "
#define MsgIndicatorHasVariance "ayant une"
#define MsgIntercept "Ordonn�e � l'origine"
#define MsgNoParameters "Aucun param�tre."
#define MsgPeriodCurr "COUR"

/********************************************************************************************/
/* Prorate                                                                                  */
/********************************************************************************************/
#define MsgBasicMethodLowerBoundAndPositivityInconsistent "Avec la m�thode de base du calcul proportionnel, la limite inf�rieure ne peut pas �tre n�gative lorsque l'option " BPN_REJECT_NEGATIVE " est active."
#define MsgDataSetInstatusIgnored "La table " DSN_INSTATUS " ne sera pas utilis�e: aucun des modificateurs n'est ORIGINAL ou IMPUTED."
#define MsgDataSetInstatusMandatory "La table " DSN_INSTATUS " est obligatoire: au moins un modificateur est ORIGINAL ou IMPUTED."
#define MsgDecimalMustBeIntegerBetweenTwoValues "" BPN_DECIMAL " doit �tre un entier compris entre %d et %d inclusivement."
#define MsgInvalidMethodStringPlusValidKeywords "" BPN_METHOD " invalide: %s. Les mots-cl�s acceptables sont: %c, %s, %c, %s."
#define MsgInvalidModifierStringPlusValidKeywords "" BPN_MODIFIER " invalide: %s. Les mots-cl�s acceptables sont: %c, %s, %c, %s, %c, %s, %c, %s."
#define MsgNoProratingAllModifiersAreString "Tous les modificateurs ont la valeur %s. Il n'y a aucune variable � imputer au moyen du calcul proportionnel."
#define MsgNotConstrainedByUpperBound "Le r�sultat ne sera pas restreint par une limite sup�rieure."
#define MsgScalingMethodLowerBoundGEDouble "" BPN_LOWER_BOUND " invalide. " BPN_LOWER_BOUND " doit �tre plus grand ou �gal � %.0f avec la m�thode dite 'scaling' du calcul proportionnel."
#define MsgScalingMethodUpperBoundGEDouble "" BPN_UPPER_BOUND " invalide. " BPN_UPPER_BOUND " doit �tre plus grand ou �gal � %.0f avec la m�thode dite 'scaling' du calcul proportionnel."
#define MsgUpperBoundGTLowerBound "" BPN_UPPER_BOUND " doit �tre sup�rieur � " BPN_LOWER_BOUND "."
#define MsgVarInTwoStatementsExclusive "Une variable fait partie simultan�ment des �nonc�s %s et %s. Ces �nonc�s sont mutuellement exclusifs."

/* Print Edits */
#define MsgHeaderProrateEdits "" BPN_EDITS " analys�es par le parseur pour la proc�dure PRORATE "
#define MsgHeaderEditsModifier "(si aucun modificateur n'est sp�cifi� pour une variable, la valeur de l'option " BPN_MODIFIER " sera utilis�e):"
#define MsgHeaderGroup "--- GROUPE ---"
#define MsgEditLevelEqualInteger "Niveau de la r�gle: %d"

/* Print Statistics */
#define MsgNoImputationDoneNoValidObsByGroup "Aucune observation valide dans le regroupement trait�. Aucune imputation n'a �t� faite."
#define MsgNumberObsDroppedNegativeValueAndPositivityInconsistent "%d observations ont �t� rejet�es de la table " DSN_INDATA " parce qu'une valeur est n�gative et l'option " BPN_ACCEPT_NEGATIVE " n'a pas �t� sp�cifi�e."


#endif