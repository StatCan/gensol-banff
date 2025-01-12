#ifndef MessageBanffAPI_FR_H
#define MessageBanffAPI_FR_H

/**************************************************************/
/* M10000 series - Information messages                         */
/* - Severity is EIE_INFORMATION with EI_AddMessage()         */
/* - Other cases: word 'NOTE:' added at the beginning         */
/**************************************************************/
#define M10010 "Le temps minimum par observation est de %.2f. Ce paramètre a été réinitialisé."
#define M10011 "L'estimateur %d ne sera pas traité parce qu'il n'y a pas d'observations acceptables permettant de calculer les moyennes."
#define M10012 "L'estimateur %d ne sera pas traité parce que le dénominateur d'au moins une moyenne vaut zéro."
#define M10013 "L'estimateur %d ne sera pas traité parce qu'il n'y a pas d'observations acceptables permettant de calculer les coefficients betas."
#define M10014 "Estimateur %d: Le nombre de donneurs disponibles pour déduire l'erreur aléatoire est %d."
#define M10015 "L'estimateur %d ne satisfait pas COUNTCRITERIA:   exigence %-4d valeur %-4d"
#define M10016 "L'estimateur %d ne satisfait pas PERCENTCRITERIA:   exigence %-4.1f valeur %-4.1f"
#define M10017 "L'estimateur %d ne sera pas traité."
#define M10018 "Nombre d'observations retenues pour l'identification des valeurs aberrantes: %ld."
#define M10019 "Valeur du 25e centile :"
#define M10020 "Valeur du 50e centile :"
#define M10021 "Valeur du 75e centile :"
#define M10022 "MDM spécifié sera ignoré parce que la médiane vaut 0."
#define M10023 "La médiane des ratios : %.5f"
#define M10024 "MDM spécifié sera ignoré parce que la médiane des effets vaut 0."
#define M10025 "Intervalle d'acceptation : [ %.5f , %.5f ]"
#define M10026 "Intervalles d'exclusion  : [ %.5f , %.5f [ U ] %.5f , %.5f ]"
#define M10027 "Intervalles d'imputation : ] -infini , %.5f [ U ] %.5f , +infini ["
#define M10028 "Intervalles d'exclusion  : ] -infini , %.5f [ U ] %.5f , +infini ["
#define M10029 "L'écart-sigma est 0."
#define M10030 "Déviation"
#define M10031 "Beta d'exclusion"
#define M10032 "Beta d'imputation"
#define M10033 "L'écart-sigma d'exclusion"
#define M10034 "L'écart-sigma d'imputation"
#define M10035 "Borne d'imputation de gauche"
#define M10036 "Borne d'exclusion de gauche"
#define M10037 "Borne d'exclusion de droite"
#define M10038 "Borne d'imputation de droite"
/**************************************************************/
/* M20000 series - Warning messages                            */
/* - Severity is EIE_WARNING with EI_AddMessage()             */
/* - Other cases: word 'AVERTISSEMENT:' added at the beginning*/
/**************************************************************/
#define M20024 "Estimateur %d: Le nombre de donneurs disponibles pour déduire l'erreur aléatoire est inférieur à %d. La variance des valeurs imputées ne sera pas fiable."
#define M20025 "Estimateur %d: %s, division par zéro."
#define M20026 "Le nombre d'observations est inférieur à MINOBS %d. Aucune identification de valeurs aberrantes."
#define M20027 "Un poids a été assigné à la variable %s mais cette variable ne fait pas partie des règles de vérification."
/**************************************************************/
/* M30000 series - Error messages                              */       
/* - Severity is EIE_ERROR with EI_AddMessage()               */
/* - Other cases: word 'ERREUR:' added at the beginning       */
/**************************************************************/
#define M30003 "Il y a trop d'erreurs. La compilation doit s'arrêter."
#define M30007 "Cherchons '%s' mais avons trouvé '%s' à la place."
#define M30009 "Nom de variable trop long!"
#define M30010 "%s a échoué."
#define M30032 "Matrices incompatibles (nombre différent d'observations): X et Y"
#define M30033 "Matrices incompatibles (nombre différent d'observations): D et X, Y"
#define M30034 "Incapable de calculer la matrice Xt, la transposée de la matrice X"
#define M30035 "Incapable de calculer la matrice U = X * Xt"
#define M30036 "Incapable d'allouer la mémoire pour la matrice U1"
#define M30037 "Incapable d'allouer la mémoire pour le vecteur w"
#define M30038 "Incapable d'allouer la mémoire pour la matrice V"
#define M30039 "L'estimateur %d ne sera pas traité parce qu'aucune convergence ne peut être atteinte lors du calcul des coefficients betas."
#define M30040 "Échec à l'étape SVDCMP"
#define M30041 "Incapable d'allouer la mémoire pour la matrice XtY"
#define M30042 "Échec à l'étape SVBKSB."
#define M30043 "Estimateur %d: l'algorithme '%s' est un nom non défini dans la table ALGORITHM ou dans la liste des algorithmes prédéfinis."
#define M30044 "Estimateur %d: les valeurs acceptables pour VARIANCEPERIOD sont 'C', 'H' ou une valeur manquante."
#define M30045 "Estimateur %d: les valeurs acceptables pour EXCLUDEIMPUTED sont 'Y', 'N' ou une valeur manquante."
#define M30046 "Estimateur %d: les valeurs acceptables pour EXCLUDEOUTLIERS sont 'Y', 'N' ou une valeur manquante."
#define M30047 "Estimateur %d: les valeurs acceptables pour RANDOMERROR sont 'Y' ou 'N'."
#define M30048 "Estimateur %d: COUNTCRITERIA doit être un entier plus grand ou égal à 1."
#define M30049 "Estimateur %d: PERCENTCRITERIA doit être un nombre plus grand que 0 ou inférieur à 100."
#define M30050 "Estimateur %d: STATUS (%s) doit comprendre 1 à 3 caractères."
#define M30051 "Estimateur %d: les valeurs acceptables pour TYPE sont '%s' ou '%s'."
#define M30052 "Estimateur %d: le nombre de noms différents fournis comme variables auxiliaires est supérieur à celui exigé dans la formule."
#define M30053 "Estimateur %d: le nombre de noms différents fournis comme variables auxiliaires est inférieur à celui exigé dans la formule."
#define M30054 "Estimateur %d: %s ne doit pas être spécifiée pour un estimateur de type EF n'ayant aucune moyenne à calculer ou aucune erreur aléatoire à déduire." /* %s: EXCLUDEIMPUTED or EXCLUDEOUTLIERS */
#define M30055 "Estimateur %d: %s doit être spécifiée pour un estimateur de type EF ayant une erreur aléatoire à déduire." /* %s: EXCLUDEIMPUTED or EXCLUDEOUTLIERS */
#define M30056 "Estimateur %d: %s ne doit pas être spécifiée pour un estimateur de type EF n'ayant aucune moyenne à calculer." /* %s: COUNTCRITERIA, PERCENTCRITERIA or WEIGHTVARIABLE */
#define M30057 "Estimateur %d: %s doit être spécifiée pour un estimateur de type LR ou un estimateur de type EF ayant au moins une moyenne à calculer." /* %s: EXCLUDEIMPUTED, EXCLUDEOUTLIERS,COUNTCRITERIA or PERCENTCRITERIA */
#define M30058 "Estimateur %d: %s doit être spécifiée pour un estimateur de type LR lorsque VARIANCEVARIABLE est spécifiée." /* %s: VARIANCEPERIOD or VARIANCEEXPONENT */
#define M30059 "Estimateur %d: %s ne doit pas être spécifiée pour un estimateur de type LR lorsque VARIANCEVARIABLE n'est pas spécifiée." /* %s: VARIANCEPERIOD or VARIANCEEXPONENT */
#define M30060 "Estimateur %d: %s ne doit pas être spécifiée pour un estimateur de type EF." /* %s: VARIANCEVARIABLE, VARIANCEPERIOD or VARIANCEEXPONENT */
#define M30061 "Estimateur %d: la valeur courante de FIELDID %s est utilisée comme variable auxiliaire."
#define M30062 "Estimateur %d: WEIGHTVARIABLE et FIELDID ne peuvent pas faire référence à la même variable %s pour un estimateur de type LR."
#define M30063 "Estimateur %d: WEIGHTVARIABLE et FIELDID ne peuvent pas faire référence à la même variable %s pour un estimateur de type EF ayant au moins une moyenne à calculer pour la période courante."
#define M30064 "Estimateur %d: VARIANCEVARIABLE et FIELDID ne peuvent pas faire référence à la même variable %s lorsque VARIANCEPERIOD est la période courante."
#define M30065 "Estimateur %d: WEIGHTVARIABLE et VARIANCEVARIABLE ne peuvent pas faire référence à la même variable %s lorsque VARIANCEPERIOD est la période courante."
#define M30066 "Estimateur %d: WEIGHTVARIABLE %s ne peut pas être utilisée comme variable auxiliaire courante pour un estimateur de type LR."
#define M30067 "Estimateur %d: WEIGHTVARIABLE %s ne peut pas être utilisée comme variable auxiliaire (incluant FIELDID possiblement) pour un estimateur de type EF."
#define M30068 "Estimateur %d: VARIANCEVARIABLE %s ne peut pas être utilisée comme variable auxiliaire pour une période identique à celle de VARIANCEPERIOD."
#define M30069 "Estimateur %d: la clé %s a une valeur %s de 0.0 pour %s."      /* ... une valeur 'VariablePeriod' de 0.0 pour 'VariableType' */
#define M30070 "Estimateur %d: la clé %s a une valeur %s négative pour %s."    /* ... une valeur 'VariablePeriod' négative pour 'VariableType' */
#define M30071 "Estimateur %d: la clé %s a une valeur %s manquante pour %s."   /* ... une valeur 'VariablePeriod' manquante pour 'VariableType' */
#define M30072 "Estimateur %d: la clé %s a un statut pour la valeur %s de %s." /* ... un statut pour la valeur 'VariablePeriod' de 'VariableType' */
#define M30073 "Échec à l'étape de la 'Recherche des champs d'appariement' pour le receveur %s."

#define M30077 "ERREUR: Incohérence de l'ensemble des règles de vérification."
#define M30078 "ERREUR: Espace insuffisant."
#define M30079 "ERREUR: Espace insuffisant. Ancien numéro des colonnes: max %d y_col %d"
#define M30080 "ERREUR: Espace insuffisant. En raison de contraintes d'espace, la cardinalité a été réduite de %d à %d."
#define M30081 "Emit(): Élément inconnu"
#define M30082 "Opération illégale."
#define M30083 "Exponent(): '^' doit être suivi d'un nombre"
#define M30084 "Recherche 'Aux' ou 'FieldId' mais trouve plutôt '%s'."
#define M30085 "Élément trop long."
#define M30086 "AUX non suivi d'un entier."
#define M30087 "Erreur syntaxique."
#define M30088 "La constante zéro n'est pas permise."
#define M30089 "0 n'est pas un nombre valide pour AUX."
#define M30090 "AUX%d dépasse la limite permise."
#define M30091 "AUX%d est manquant."
#define M30092 "FIELDID ne peut pas être spécifié pour la période courante."
#define M30093 "Au moins deux régresseurs, incluant possiblement l'ordonnée à l'origine, sont identiques. Ils font référence à la même combinaison de termes: nom, période et exposant."
#define M30094 "DECIMAL doit être un entier compris entre %d et %d inclusivement."
#define M30095 "UPPERBOUND doit être plus grand que LOWERBOUND."
#define M30096 "LOWERBOUND et UPPERBOUND doivent être tous deux des nombres positifs avec la méthode dite 'scaling' du calcul proportionnel."
#define M30098 "Les règles de vérification font référence à plusieurs groupes de règles d'ajustement au moyen du calcul proportionnel. Il n'est permis de spécifier qu'un seul de ces groupes de règles."
#define M30099 "EditGroupFixHierarchy(): On ne peut pas avoir deux fois la même variable!"
#define M30100 "Nom de(s) variable(s) répété(s):"
#define M30101 "La variable '%s' désigne à deux reprises un total!"
#define M30102 "Le poids doit être positif!"
#define M30103 "On ne peut pas avoir deux fois la même variable dans une règle de vérification!"
#define M30104 "Une variable est sans valeur."
#define M30105 "Le nombre doit être plus grand que 0."
/**************************************************************/
/* M40000 series - Statistics (reports)                         */
/**************************************************************/
/* EI_CHECK(): Edits Analysis Statistics */
#define M40001 "ANALYSE STATISTIQUE DES RÈGLES DE VÉRIFICATION"
        /* Edits read */
#define M40002 "Nombre de règles de vérification lues %.*s:  %d"
#define M40003 "%.*sNombre d'égalités lues %.*s:  %d"        /* one tab */
#define M40004 "%.*sNombre d'inégalités lues %.*s:  %d"      /* one tab */ 
        /* Minimal Set of Edits */
#define M40005 "Nombre de règles de vérification dans l'ensemble minimal %.*s:  %d"
#define M40006 "%.*sNombre d'égalités retenues %.*s:  %d"    /* one tab */ 
#define M40007 "%.*sNombre d'égalités cachées %.*s:  %d"     /* one tab */
#define M40008 "%.*sNombre d'inégalités retenues %.*s:  %d"  /* one tab */
        /* Redundant edits */ 
#define M40009 "Nombre de règles de vérification redondantes %.*s:  %d"
#define M40010 "%.*sNombre d'égalités redondantes %.*s:  %d"    /* one tab */
#define M40011 "%.*sNombre d'inégalités redondantes %.*s:  %d"  /* one tab */
#define M40012 "%.*sNombre d'égalités cachées redondantes %.*s:  %d"   /* one tab */
        /* M40013 and M40014 form one message. */
#define M40013 "%.*sNombre de règles strictes mais non restrictives"   /* one tab */
#define M40014 "%.*squant à la région d'acceptation %.*s:  %d"         /* two tabs*/
        /* About variables in edits */
#define M40015 "Nombre de variables %.*s:  %d"
#define M40016 "Nombre de variables non bornées %.*s:  %d"
#define M40017 "Nombre de variables déterministes %.*s:  %d"
        /* Paired Msg for Length of PlaceHolders: LPH */
#define M40002_LPH   6 
#define M40003_LPH  10
#define M40004_LPH  10
#define M40005_LPH   6
#define M40006_LPH  10
#define M40007_LPH  10
#define M40008_LPH  10
#define M40009_LPH   6
#define M40010_LPH  10
#define M40011_LPH  10
#define M40012_LPH  10
#define M40013_LPH   4
#define M40014_LPH  10
#define M40015_LPH   6
#define M40016_LPH   6
#define M40017_LPH   6

/* EI_CHECK(): Print variable bounds */
#define M40018 "BORNES DES VARIABLES"
#define M40019 "Nom de la variable"
#define M40020 "Inférieure"
#define M40021 "Supérieure"
#define M40022 "non bornée"
#define M40023 "déterministe"

/* EI_IMPLY() */ 
#define M40024 "ANALYSE DIAGNOSTIQUE DES RÈGLES DE VÉRIFICATION IMPLICITES"
#define M40025 "PARTIE 1 - LISTE DES RÈGLES DE VÉRIFICATION IMPLICITES"
#define M40026 "Liste des règles de vérification analysées:"
#define M40027 "Liste des règles de vérification implicites trouvées:"
#define M40028 "Aucune règle de vérification implicite n'a été trouvée."
#define M40029 "PARTIE 2 - STATISTIQUES SUR LES RÈGLES DE VÉRIFICATION IMPLICITES"
#define M40030 "Nombre maximum désigné de règles implicites %.*s:  %s"  /* 2nd string : M40031 */ 
#define M40031 "entier max. (défaut)" /* INT_MAX */
#define M40032 "Nombre maximum de règles implicites à générer %.*s:  %d"
#define M40033 "Nombre de règles implicites générées %.*s:  %d"
#define M40034 "%.*sNombre d'égalités implicites %.*s:  %d"   /* one tab */
#define M40035 "%.*sNombre d'inégalités implicites %.*s:  %d" /* one tab */
       /* Paired Msg for Length of PlaceHolders: LPH */
#define M40030_LPH   6
#define M40032_LPH   6
#define M40033_LPH   6
#define M40034_LPH  10
#define M40035_LPH  10

/* EI_EXTREMAL() */
#define M40036 "ANALYSE DIAGNOSTIQUE DES POINTS EXTRÉMAUX DES RÈGLES DE VÉRIFICATION"
#define M40037 "PARTIE 1 - AUCUN POINT EXTRÉMAL ayant une cardinalité inférieure ou égale à %d n'a été trouvé."
#define M40038 "PARTIE 1 - LISTE DES POINTS EXTRÉMAUX"
#define M40039 "Point extrémal no. %d"
#define M40040 "  Toutes les variables ont une valeur de 0." /* Potential description of one extremal point */
#define M40041 "* Les variables non affichées ont une valeur de 0." /* General note */
#define M40042 "PARTIE 2 - STATISTIQUES SUR LES POINTS EXTRÉMAUX"
#define M40043 "Cardinalité maximale spécififiée %.*s:  %s"  /* 2nd string : M40044 */   
#define M40044 "nombre de variables (défaut)"
#define M40045 "Cardinalité maximale spécifiée %.*s:  %d"
#define M40046 "Cardinalité maximale réelle %.*s:  %d"
#define M40047 "Nombre de points extrémaux %.*s:  %d"
#define M40048 "Nombre de points extrémaux ayant une cardinalité de %3d %.*s:  %d"
        /* Paired Msg for Length of PlaceHolders: LPH */
#define M40043_LPH   6
#define M40045_LPH   6
#define M40046_LPH   6
#define M40047_LPH   6
#define M40048_LPH   6 /* do not count %3d */

/* EI_STATISTICS() */
#define M40049 "STATISTISTIQUES SOMMAIRES SUR LES RÈGLES DE VÉRIFICATION"
/* Table 1-1 */
#define M40050 "Tableau 1.1"
#define M40051 "Répartition des règles de vérification selon le nombre d'observations ayant satisfait, comportant des valeurs manquantes ou ayant échoué chacune des règles."
#define M40052 "No. règle"
#define M40053 "Observations"
#define M40054 "ayant réussi"
#define M40055 "manquantes"
#define M40056 "ayant échoué"
/* Table 1.2 */
#define M40057 "Tableau 1.2"
#define M40058 "Répartition des observations selon qu'elles ont satisfait, comportaient des valeurs manquantes ou ont échoué 'k' règles de vérification."
#define M40059 "Nombre de règles"
#define M40060 "Total (observ.)"
/* Table 1.3 */
#define M40061 "Tableau 1.3"
#define M40062 "Nombre d'observations qui ont satisfait l'ensemble des règles de vérification, ont échoué ou comportaient des valeurs manquantes pour au moins l'une de ces régles."
/* Table 2.1 */
#define M40063 "Tableau 2.1"
#define M40064 "Nombre de fois où chaque variable hérite du code d'état PASS, MISS ou FAIL des observations, suivant que la règle soit concernée* ou non par cette variable." /* see M40075 for the 

asterix note */
#define M40065 "Variable"
#define M40066 "Observ."   /* in english: M40066 <-> M40067 */
#define M40067 "et règles" /* in english: M40067 <-> M40066 */
#define M40068 "réussies"
#define M40069 "manquantes"
#define M40070 "échouées"
#define M40071 "non"
#define M40072 "concernées"
#define M40073 "Nombre"
#define M40074 "de règles"
#define M40075 "Une règle est concernée par la variable j si le j ième coefficient est différent de zéro. Une règle non concernée par la variable j se retrouve sous la colonne 'Observ. et règles non concernées'."
/* Table 2.2 */
#define M40076 "Tableau 2.2"
#define M40077 "Nombre d'observations ayant PASS, MISS, ou FAIL comme code d'état général selon que la variable j contribue* ou non à ce code d'état."
#define M40078 "sans objet"   
#define M40079 "Si une observation a un code d'état général FAIL, les variables ayant un statut PASS ou MISS n'ont joué aucun rôle quant à l'attribution de ce statut pour cette observation. Par conséquent, les observations qui renferment ces variables sont enregistrées sous la colonne 'Observations sans objet'."
/* Related to status definition */
#define M40080 "DÉFINITION DES CODES D'ÉTAT DES OBSERVATIONS"
#define M40081 "Le code d'état d'une observation relative à une règle est"
#define M40082 "     i) PASS--si toutes les valeurs des variables ayant un coefficient non nul "
#define M40083 "              sont non manquantes et que la règle est satisfaite"
#define M40084 "    ii) MISS--si une ou plusieurs variables ayant un coefficient non nul "
#define M40085 "              ont une valeur manquante"
#define M40086 "   iii) FAIL--si toutes les valeurs des variables ayant un coefficient non nul "
#define M40087 "              sont non manquantes mais la règle n'est pas satisfaite"
#define M40088 "Le code d'état général d'une observation est"
#define M40089 "     i) PASS--si l'observation a un code d'état PASS pour chacune des règles"
#define M40090 "    ii) MISS--si l'observation a au moins un code d'état MISS pour l'une des "
#define M40091 "              règles et le code d'état PASS pour toutes les autres règles"
#define M40092 "   iii) FAIL--si l'observation a un code d'état FAIL pour l'une des règles"

/**************************************************************/
/* Special series - Algorithms predefined descriptions         */
/**************************************************************/
#define AUXTREND_D   "La valeur de la même unité lors du cycle d'enquête précédent, corrigée en fonction de la variation d'une variable auxiliaire, est imputée."
#define AUXTREND2_D  "Une moyenne de deux AUXTRENDs est imputée."
#define CURAUX_D     "La valeur courante d'une variable auxiliaire pour la même unité est imputée."
#define CURAUXMEAN_D "La moyenne courante d'une variable auxiliaire est imputée."
#define CURMEAN_D    "La valeur moyenne de la variable à imputer pour toutes les unités admissibles à l'occasion du cycle d'enquête courant est imputée."
#define CURRATIO_D   "Une estimation par quotient, établie à partir des moyennes calculées pour les unités admissibles à l'occasion du cycle d'enquête courant, est imputée."
#define CURRATIO2_D  "Une moyenne de deux CURRATIOs est imputée."
#define CURREG_D     "Régression linéaire simple basée sur une variable indépendante provenant du cycle d'enquête courant."
#define CURREG_E2_D  "Régression linéaire basée sur la valeur et le carré de la valeur d'une variable provenant du cycle d'enquête courant."
#define CURREG2_D    "Régression linéaire basée sur deux variables indépendantes provenant du cycle d'enquête courant."
#define CURREG3_D    "Régression linéaire basée sur trois variables indépendantes provenant du cycle d'enquête courant."
#define CURSUM2_D    "La somme de deux variables auxiliaires provenant du cycle d'enquête courant est imputée."
#define CURSUM3_D    "La somme de trois variables auxiliaires provenant du cycle d'enquête courant est imputée."
#define CURSUM4_D    "La somme de quatre variables auxiliaires provenant du cycle d'enquête courant est imputée."
#define DIFTREND_D   "La valeur de la même unité lors du cycle d'enquête précédent, corrigée en fonction de la variation de la moyenne de cette variable, est imputée."
#define HISTREG_D    "Régression linéaire basée sur les valeurs prises par la variable à imputer lors du cycle d'enquête précédent."
#define PREAUX_D     "La valeur historique d'une variable auxiliaire pour la même unité est imputée."
#define PREAUXMEAN_D "La moyenne historique d'une variable auxiliaire est imputée."
#define PREMEAN_D    "La valeur moyenne de la variable à imputer pour toutes les unités admissibles à l'occasion du cycle d'enquête précédent est imputée."
#define PREVALUE_D   "La valeur de la même unité lors du cycle d'enquête précédent est imputée."


/**************************************************************/
/* M00000 series - Miscellaneous                               */
/**************************************************************/
#define M00002 "Erreur près ou avant"
#define M00004 "Deux points"
#define M00005 "Fin"
#define M00006 "Erreur"
#define M00008 "Modificateur"
#define M00009 "Opérateur"
#define M00010 "Plus"
#define M00011 "Point virgule"
#define M00013 "Variable"
#define M00014 "Caractère non reconnu"
#define M00017 "Règle d'égalité: '%d' rend les règles incohérentes."
#define M00023 "Règle d'inégalité: '%d' est entièrement en dehors de la région d'acceptation. Elle est redondante."
#define M00026 "ANALYSE DIAGNOSTIQUE DES RÈGLES DE VÉRIFICATION"
#define M00027 "ENSEMBLE DES RÈGLES DE VÉRIFICATION INCOHÉRENTES"
#define M00028 "Incohérence entre les algorithmes du simplexe et ceux de Given: problème logiciel."
#define M00029 "ENSEMBLE MINIMAL DES RÈGLES DE VÉRIFICATION"
#define M00030 "L'ensemble minimal des règles de vérification est identique à l'ensemble des règles originales spécifiées."
#define M00031 "Règle d'inégalité: '%d' rend les règles incohérentes."
#define M00032 "courant"  /* for french grammar: 'male gender' form   */
#define M00033 "courante" /* for french grammar: 'female gender' form */
#define M00034 "historique"
#define M00035 "Parseur pour une fonction d'estimation"
#define M00036 "Parseur pour une régression linéaire"
#define M00037 "Parseur des règles pour le calcul proportionnel"
#define M00038 "Poids"
#define M00039 "Parseur pour les poids"
#define M00040 "Égal" 
#define M00041 "Nombre"
#define M00042 "Terme non reconnu"

/**************************************************************/
/* the following messages are added for the                   */
/* addition of RANDNUMVAR                                     */
/**************************************************************/

/* the following messages are added for DonorInternal.c       */
#define MsgRandnumvarDonorInternalTslistLookupFail "DonorInternal: Échec pendant la recherche de tSList dans DONOR_FindNextNearestDonorIndexAlt()"
#define MsgRandnumvarDonorInternalTadtlistIndexFail "DonorInternal: Échec pendant l'indexation de tADTList dans DONOR_FindNextNearestDonorIndexAlt()"
#define MsgRandnumvarDonorInternalNotFound "Aucun nombre aléatoire trouvé dans DONOR_FindNextNearestDonorIndexAlt()"
#define MsgRandnumvarDonorInternalLessThanZero "DonorInternal: Le nombre aléatoire du receveur %f est plus petit que 0. Veuillez corriger le nombre aléatoire et relancez la procédure."
#define MsgRandnumvarDonorInternalGreaterThanOne "DonorInternal: Le nombre aléatoire du receveur %f est plus grand que 1. Veuillez corriger le nombre aléatoire et relancez la procédure."

/* the following messages are added for EI_RandomDonor        */
#define MsgRandnumvarRandomDonorlTslistLookupFail "EI_RandomDonor: Échec pendant la recherche de tSList dans EI_RandomDonorWithPostEdits()"
#define MsgRandnumvarRandomDonorTadtlistIndexFail "EI_RandomDonor: Échec pendant l'indexation de tADTList dans EI_RandomDonorWithPostEdits()"
#define MsgRandnumvarRandomDonorNotFound "Aucun nombre aléatoire trouvé dans EI_RandomDonorWithPostEdits()"
#define MsgRandnumvarRandomDonorLessThanZero "EI_RandomDonor: Le nombre aléatoire du receveur %f est plus petit que 0. Veuillez corriger le nombre aléatoire et relancez la procédure."
#define MsgRandnumvarRandomDonorGreaterThanOne "EI_RandomDonor: Le nombre aléatoire du receveur %f est plus grand que 1. Veuillez corriger le nombre aléatoire et relancez la procédure."
#define MsgRandnumvarRandomDonorInvalidRange "DONOR: Au moins un nombre aléatoire du receveur est égal à 1 ou très près de 1, dans l'intervalle (1-1e-012,1].  En raison de contraintes de précision décimale, 1-1e-012 sera utilisé pour le choix aléatoire plutôt que le nombre aléatoire du receveur."

/* the following messages are added for EI_Chernikova.c       */
#define MsgRandnumvarErrorlocLessThanZero "ERRORLOC: Le nombre aléatoire du receveur %f est plus petit que 0. Veuillez corriger le nombre aléatoire et relancez la procédure."
#define MsgRandnumvarErrorlocGreaterThanOne "ERRORLOC: Le nombre aléatoire du receveur %f est plus grand que 1. Veuillez corriger le nombre aléatoire et relancez la procédure."
#define MsgRandnumvarErrorlocInvalid "ERRORLOC: Au moins un nombre aléatoire est égal à 1 ou très près de 1, dans l'intervalle (1-1e-012,1].  En raison de contraintes de précision décimale, 1-1e-012 sera utilisé pour le choix aléatoire plutôt que le nombre aléatoire de l'enregistrement."

#endif

