#ifndef MessageBanffAPI_FR_H
#define MessageBanffAPI_FR_H

/**************************************************************/
/* M10000 series - Information messages                         */
/* - Severity is EIE_INFORMATION with EI_AddMessage()         */
/* - Other cases: word 'NOTE:' added at the beginning         */
/**************************************************************/
#define M10010 "Le temps minimum par observation est de %.2f. Ce param�tre a �t� r�initialis�."
#define M10011 "L'estimateur %d ne sera pas trait� parce qu'il n'y a pas d'observations acceptables permettant de calculer les moyennes."
#define M10012 "L'estimateur %d ne sera pas trait� parce que le d�nominateur d'au moins une moyenne vaut z�ro."
#define M10013 "L'estimateur %d ne sera pas trait� parce qu'il n'y a pas d'observations acceptables permettant de calculer les coefficients betas."
#define M10014 "Estimateur %d: Le nombre de donneurs disponibles pour d�duire l'erreur al�atoire est %d."
#define M10015 "L'estimateur %d ne satisfait pas COUNTCRITERIA:   exigence %-4d valeur %-4d"
#define M10016 "L'estimateur %d ne satisfait pas PERCENTCRITERIA:   exigence %-4.1f valeur %-4.1f"
#define M10017 "L'estimateur %d ne sera pas trait�."
#define M10018 "Nombre d'observations retenues pour l'identification des valeurs aberrantes: %ld."
#define M10019 "Valeur du 25e centile :"
#define M10020 "Valeur du 50e centile :"
#define M10021 "Valeur du 75e centile :"
#define M10022 "MDM sp�cifi� sera ignor� parce que la m�diane vaut 0."
#define M10023 "La m�diane des ratios : %.5f"
#define M10024 "MDM sp�cifi� sera ignor� parce que la m�diane des effets vaut 0."
#define M10025 "Intervalle d'acceptation : [ %.5f , %.5f ]"
#define M10026 "Intervalles d'exclusion  : [ %.5f , %.5f [ U ] %.5f , %.5f ]"
#define M10027 "Intervalles d'imputation : ] -infini , %.5f [ U ] %.5f , +infini ["
#define M10028 "Intervalles d'exclusion  : ] -infini , %.5f [ U ] %.5f , +infini ["
#define M10029 "L'�cart-sigma est 0."
#define M10030 "D�viation"
#define M10031 "Beta d'exclusion"
#define M10032 "Beta d'imputation"
#define M10033 "L'�cart-sigma d'exclusion"
#define M10034 "L'�cart-sigma d'imputation"
#define M10035 "Borne d'imputation de gauche"
#define M10036 "Borne d'exclusion de gauche"
#define M10037 "Borne d'exclusion de droite"
#define M10038 "Borne d'imputation de droite"
/**************************************************************/
/* M20000 series - Warning messages                            */
/* - Severity is EIE_WARNING with EI_AddMessage()             */
/* - Other cases: word 'AVERTISSEMENT:' added at the beginning*/
/**************************************************************/
#define M20024 "Estimateur %d: Le nombre de donneurs disponibles pour d�duire l'erreur al�atoire est inf�rieur � %d. La variance des valeurs imput�es ne sera pas fiable."
#define M20025 "Estimateur %d: %s, division par z�ro."
#define M20026 "Le nombre d'observations est inf�rieur � MINOBS %d. Aucune identification de valeurs aberrantes."
#define M20027 "Un poids a �t� assign� � la variable %s mais cette variable ne fait pas partie des r�gles de v�rification."
/**************************************************************/
/* M30000 series - Error messages                              */       
/* - Severity is EIE_ERROR with EI_AddMessage()               */
/* - Other cases: word 'ERREUR:' added at the beginning       */
/**************************************************************/
#define M30003 "Il y a trop d'erreurs. La compilation doit s'arr�ter."
#define M30007 "Cherchons '%s' mais avons trouv� '%s' � la place."
#define M30009 "Nom de variable trop long!"
#define M30010 "%s a �chou�."
#define M30032 "Matrices incompatibles (nombre diff�rent d'observations): X et Y"
#define M30033 "Matrices incompatibles (nombre diff�rent d'observations): D et X, Y"
#define M30034 "Incapable de calculer la matrice Xt, la transpos�e de la matrice X"
#define M30035 "Incapable de calculer la matrice U = X * Xt"
#define M30036 "Incapable d'allouer la m�moire pour la matrice U1"
#define M30037 "Incapable d'allouer la m�moire pour le vecteur w"
#define M30038 "Incapable d'allouer la m�moire pour la matrice V"
#define M30039 "L'estimateur %d ne sera pas trait� parce qu'aucune convergence ne peut �tre atteinte lors du calcul des coefficients betas."
#define M30040 "�chec � l'�tape SVDCMP"
#define M30041 "Incapable d'allouer la m�moire pour la matrice XtY"
#define M30042 "�chec � l'�tape SVBKSB."
#define M30043 "Estimateur %d: l'algorithme '%s' est un nom non d�fini dans la table ALGORITHM ou dans la liste des algorithmes pr�d�finis."
#define M30044 "Estimateur %d: les valeurs acceptables pour VARIANCEPERIOD sont 'C', 'H' ou une valeur manquante."
#define M30045 "Estimateur %d: les valeurs acceptables pour EXCLUDEIMPUTED sont 'Y', 'N' ou une valeur manquante."
#define M30046 "Estimateur %d: les valeurs acceptables pour EXCLUDEOUTLIERS sont 'Y', 'N' ou une valeur manquante."
#define M30047 "Estimateur %d: les valeurs acceptables pour RANDOMERROR sont 'Y' ou 'N'."
#define M30048 "Estimateur %d: COUNTCRITERIA doit �tre un entier plus grand ou �gal � 1."
#define M30049 "Estimateur %d: PERCENTCRITERIA doit �tre un nombre plus grand que 0 ou inf�rieur � 100."
#define M30050 "Estimateur %d: STATUS (%s) doit comprendre 1 � 3 caract�res."
#define M30051 "Estimateur %d: les valeurs acceptables pour TYPE sont '%s' ou '%s'."
#define M30052 "Estimateur %d: le nombre de noms diff�rents fournis comme variables auxiliaires est sup�rieur � celui exig� dans la formule."
#define M30053 "Estimateur %d: le nombre de noms diff�rents fournis comme variables auxiliaires est inf�rieur � celui exig� dans la formule."
#define M30054 "Estimateur %d: %s ne doit pas �tre sp�cifi�e pour un estimateur de type EF n'ayant aucune moyenne � calculer ou aucune erreur al�atoire � d�duire." /* %s: EXCLUDEIMPUTED or EXCLUDEOUTLIERS */
#define M30055 "Estimateur %d: %s doit �tre sp�cifi�e pour un estimateur de type EF ayant une erreur al�atoire � d�duire." /* %s: EXCLUDEIMPUTED or EXCLUDEOUTLIERS */
#define M30056 "Estimateur %d: %s ne doit pas �tre sp�cifi�e pour un estimateur de type EF n'ayant aucune moyenne � calculer." /* %s: COUNTCRITERIA, PERCENTCRITERIA or WEIGHTVARIABLE */
#define M30057 "Estimateur %d: %s doit �tre sp�cifi�e pour un estimateur de type LR ou un estimateur de type EF ayant au moins une moyenne � calculer." /* %s: EXCLUDEIMPUTED, EXCLUDEOUTLIERS,COUNTCRITERIA or PERCENTCRITERIA */
#define M30058 "Estimateur %d: %s doit �tre sp�cifi�e pour un estimateur de type LR lorsque VARIANCEVARIABLE est sp�cifi�e." /* %s: VARIANCEPERIOD or VARIANCEEXPONENT */
#define M30059 "Estimateur %d: %s ne doit pas �tre sp�cifi�e pour un estimateur de type LR lorsque VARIANCEVARIABLE n'est pas sp�cifi�e." /* %s: VARIANCEPERIOD or VARIANCEEXPONENT */
#define M30060 "Estimateur %d: %s ne doit pas �tre sp�cifi�e pour un estimateur de type EF." /* %s: VARIANCEVARIABLE, VARIANCEPERIOD or VARIANCEEXPONENT */
#define M30061 "Estimateur %d: la valeur courante de FIELDID %s est utilis�e comme variable auxiliaire."
#define M30062 "Estimateur %d: WEIGHTVARIABLE et FIELDID ne peuvent pas faire r�f�rence � la m�me variable %s pour un estimateur de type LR."
#define M30063 "Estimateur %d: WEIGHTVARIABLE et FIELDID ne peuvent pas faire r�f�rence � la m�me variable %s pour un estimateur de type EF ayant au moins une moyenne � calculer pour la p�riode courante."
#define M30064 "Estimateur %d: VARIANCEVARIABLE et FIELDID ne peuvent pas faire r�f�rence � la m�me variable %s lorsque VARIANCEPERIOD est la p�riode courante."
#define M30065 "Estimateur %d: WEIGHTVARIABLE et VARIANCEVARIABLE ne peuvent pas faire r�f�rence � la m�me variable %s lorsque VARIANCEPERIOD est la p�riode courante."
#define M30066 "Estimateur %d: WEIGHTVARIABLE %s ne peut pas �tre utilis�e comme variable auxiliaire courante pour un estimateur de type LR."
#define M30067 "Estimateur %d: WEIGHTVARIABLE %s ne peut pas �tre utilis�e comme variable auxiliaire (incluant FIELDID possiblement) pour un estimateur de type EF."
#define M30068 "Estimateur %d: VARIANCEVARIABLE %s ne peut pas �tre utilis�e comme variable auxiliaire pour une p�riode identique � celle de VARIANCEPERIOD."
#define M30069 "Estimateur %d: la cl� %s a une valeur %s de 0.0 pour %s."      /* ... une valeur 'VariablePeriod' de 0.0 pour 'VariableType' */
#define M30070 "Estimateur %d: la cl� %s a une valeur %s n�gative pour %s."    /* ... une valeur 'VariablePeriod' n�gative pour 'VariableType' */
#define M30071 "Estimateur %d: la cl� %s a une valeur %s manquante pour %s."   /* ... une valeur 'VariablePeriod' manquante pour 'VariableType' */
#define M30072 "Estimateur %d: la cl� %s a un statut pour la valeur %s de %s." /* ... un statut pour la valeur 'VariablePeriod' de 'VariableType' */
#define M30073 "�chec � l'�tape de la 'Recherche des champs d'appariement' pour le receveur %s."

#define M30077 "ERREUR: Incoh�rence de l'ensemble des r�gles de v�rification."
#define M30078 "ERREUR: Espace insuffisant."
#define M30079 "ERREUR: Espace insuffisant. Ancien num�ro des colonnes: max %d y_col %d"
#define M30080 "ERREUR: Espace insuffisant. En raison de contraintes d'espace, la cardinalit� a �t� r�duite de %d � %d."
#define M30081 "Emit(): �l�ment inconnu"
#define M30082 "Op�ration ill�gale."
#define M30083 "Exponent(): '^' doit �tre suivi d'un nombre"
#define M30084 "Recherche 'Aux' ou 'FieldId' mais trouve plut�t '%s'."
#define M30085 "�l�ment trop long."
#define M30086 "AUX non suivi d'un entier."
#define M30087 "Erreur syntaxique."
#define M30088 "La constante z�ro n'est pas permise."
#define M30089 "0 n'est pas un nombre valide pour AUX."
#define M30090 "AUX%d d�passe la limite permise."
#define M30091 "AUX%d est manquant."
#define M30092 "FIELDID ne peut pas �tre sp�cifi� pour la p�riode courante."
#define M30093 "Au moins deux r�gresseurs, incluant possiblement l'ordonn�e � l'origine, sont identiques. Ils font r�f�rence � la m�me combinaison de termes: nom, p�riode et exposant."
#define M30094 "DECIMAL doit �tre un entier compris entre %d et %d inclusivement."
#define M30095 "UPPERBOUND doit �tre plus grand que LOWERBOUND."
#define M30096 "LOWERBOUND et UPPERBOUND doivent �tre tous deux des nombres positifs avec la m�thode dite 'scaling' du calcul proportionnel."
#define M30098 "Les r�gles de v�rification font r�f�rence � plusieurs groupes de r�gles d'ajustement au moyen du calcul proportionnel. Il n'est permis de sp�cifier qu'un seul de ces groupes de r�gles."
#define M30099 "EditGroupFixHierarchy(): On ne peut pas avoir deux fois la m�me variable!"
#define M30100 "Nom de(s) variable(s) r�p�t�(s):"
#define M30101 "La variable '%s' d�signe � deux reprises un total!"
#define M30102 "Le poids doit �tre positif!"
#define M30103 "On ne peut pas avoir deux fois la m�me variable dans une r�gle de v�rification!"
#define M30104 "Une variable est sans valeur."
#define M30105 "Le nombre doit �tre plus grand que 0."
/**************************************************************/
/* M40000 series - Statistics (reports)                         */
/**************************************************************/
/* EI_CHECK(): Edits Analysis Statistics */
#define M40001 "ANALYSE STATISTIQUE DES R�GLES DE V�RIFICATION"
        /* Edits read */
#define M40002 "Nombre de r�gles de v�rification lues %.*s:  %d"
#define M40003 "%.*sNombre d'�galit�s lues %.*s:  %d"        /* one tab */
#define M40004 "%.*sNombre d'in�galit�s lues %.*s:  %d"      /* one tab */ 
        /* Minimal Set of Edits */
#define M40005 "Nombre de r�gles de v�rification dans l'ensemble minimal %.*s:  %d"
#define M40006 "%.*sNombre d'�galit�s retenues %.*s:  %d"    /* one tab */ 
#define M40007 "%.*sNombre d'�galit�s cach�es %.*s:  %d"     /* one tab */
#define M40008 "%.*sNombre d'in�galit�s retenues %.*s:  %d"  /* one tab */
        /* Redundant edits */ 
#define M40009 "Nombre de r�gles de v�rification redondantes %.*s:  %d"
#define M40010 "%.*sNombre d'�galit�s redondantes %.*s:  %d"    /* one tab */
#define M40011 "%.*sNombre d'in�galit�s redondantes %.*s:  %d"  /* one tab */
#define M40012 "%.*sNombre d'�galit�s cach�es redondantes %.*s:  %d"   /* one tab */
        /* M40013 and M40014 form one message. */
#define M40013 "%.*sNombre de r�gles strictes mais non restrictives"   /* one tab */
#define M40014 "%.*squant � la r�gion d'acceptation %.*s:  %d"         /* two tabs*/
        /* About variables in edits */
#define M40015 "Nombre de variables %.*s:  %d"
#define M40016 "Nombre de variables non born�es %.*s:  %d"
#define M40017 "Nombre de variables d�terministes %.*s:  %d"
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
#define M40020 "Inf�rieure"
#define M40021 "Sup�rieure"
#define M40022 "non born�e"
#define M40023 "d�terministe"

/* EI_IMPLY() */ 
#define M40024 "ANALYSE DIAGNOSTIQUE DES R�GLES DE V�RIFICATION IMPLICITES"
#define M40025 "PARTIE 1 - LISTE DES R�GLES DE V�RIFICATION IMPLICITES"
#define M40026 "Liste des r�gles de v�rification analys�es:"
#define M40027 "Liste des r�gles de v�rification implicites trouv�es:"
#define M40028 "Aucune r�gle de v�rification implicite n'a �t� trouv�e."
#define M40029 "PARTIE 2 - STATISTIQUES SUR LES R�GLES DE V�RIFICATION IMPLICITES"
#define M40030 "Nombre maximum d�sign� de r�gles implicites %.*s:  %s"  /* 2nd string : M40031 */ 
#define M40031 "entier max. (d�faut)" /* INT_MAX */
#define M40032 "Nombre maximum de r�gles implicites � g�n�rer %.*s:  %d"
#define M40033 "Nombre de r�gles implicites g�n�r�es %.*s:  %d"
#define M40034 "%.*sNombre d'�galit�s implicites %.*s:  %d"   /* one tab */
#define M40035 "%.*sNombre d'in�galit�s implicites %.*s:  %d" /* one tab */
       /* Paired Msg for Length of PlaceHolders: LPH */
#define M40030_LPH   6
#define M40032_LPH   6
#define M40033_LPH   6
#define M40034_LPH  10
#define M40035_LPH  10

/* EI_EXTREMAL() */
#define M40036 "ANALYSE DIAGNOSTIQUE DES POINTS EXTR�MAUX DES R�GLES DE V�RIFICATION"
#define M40037 "PARTIE 1 - AUCUN POINT EXTR�MAL ayant une cardinalit� inf�rieure ou �gale � %d n'a �t� trouv�."
#define M40038 "PARTIE 1 - LISTE DES POINTS EXTR�MAUX"
#define M40039 "Point extr�mal no. %d"
#define M40040 "  Toutes les variables ont une valeur de 0." /* Potential description of one extremal point */
#define M40041 "* Les variables non affich�es ont une valeur de 0." /* General note */
#define M40042 "PARTIE 2 - STATISTIQUES SUR LES POINTS EXTR�MAUX"
#define M40043 "Cardinalit� maximale sp�cififi�e %.*s:  %s"  /* 2nd string : M40044 */   
#define M40044 "nombre de variables (d�faut)"
#define M40045 "Cardinalit� maximale sp�cifi�e %.*s:  %d"
#define M40046 "Cardinalit� maximale r�elle %.*s:  %d"
#define M40047 "Nombre de points extr�maux %.*s:  %d"
#define M40048 "Nombre de points extr�maux ayant une cardinalit� de %3d %.*s:  %d"
        /* Paired Msg for Length of PlaceHolders: LPH */
#define M40043_LPH   6
#define M40045_LPH   6
#define M40046_LPH   6
#define M40047_LPH   6
#define M40048_LPH   6 /* do not count %3d */

/* EI_STATISTICS() */
#define M40049 "STATISTISTIQUES SOMMAIRES SUR LES R�GLES DE V�RIFICATION"
/* Table 1-1 */
#define M40050 "Tableau 1.1"
#define M40051 "R�partition des r�gles de v�rification selon le nombre d'observations ayant satisfait, comportant des valeurs manquantes ou ayant �chou� chacune des r�gles."
#define M40052 "No. r�gle"
#define M40053 "Observations"
#define M40054 "ayant r�ussi"
#define M40055 "manquantes"
#define M40056 "ayant �chou�"
/* Table 1.2 */
#define M40057 "Tableau 1.2"
#define M40058 "R�partition des observations selon qu'elles ont satisfait, comportaient des valeurs manquantes ou ont �chou� 'k' r�gles de v�rification."
#define M40059 "Nombre de r�gles"
#define M40060 "Total (observ.)"
/* Table 1.3 */
#define M40061 "Tableau 1.3"
#define M40062 "Nombre d'observations qui ont satisfait l'ensemble des r�gles de v�rification, ont �chou� ou comportaient des valeurs manquantes pour au moins l'une de ces r�gles."
/* Table 2.1 */
#define M40063 "Tableau 2.1"
#define M40064 "Nombre de fois o� chaque variable h�rite du code d'�tat PASS, MISS ou FAIL des observations, suivant que la r�gle soit concern�e* ou non par cette variable." /* see M40075 for the 

asterix note */
#define M40065 "Variable"
#define M40066 "Observ."   /* in english: M40066 <-> M40067 */
#define M40067 "et r�gles" /* in english: M40067 <-> M40066 */
#define M40068 "r�ussies"
#define M40069 "manquantes"
#define M40070 "�chou�es"
#define M40071 "non"
#define M40072 "concern�es"
#define M40073 "Nombre"
#define M40074 "de r�gles"
#define M40075 "Une r�gle est concern�e par la variable j si le j i�me coefficient est diff�rent de z�ro. Une r�gle non concern�e par la variable j se retrouve sous la colonne 'Observ. et r�gles non concern�es'."
/* Table 2.2 */
#define M40076 "Tableau 2.2"
#define M40077 "Nombre d'observations ayant PASS, MISS, ou FAIL comme code d'�tat g�n�ral selon que la variable j contribue* ou non � ce code d'�tat."
#define M40078 "sans objet"   
#define M40079 "Si une observation a un code d'�tat g�n�ral FAIL, les variables ayant un statut PASS ou MISS n'ont jou� aucun r�le quant � l'attribution de ce statut pour cette observation. Par cons�quent, les observations qui renferment ces variables sont enregistr�es sous la colonne 'Observations sans objet'."
/* Related to status definition */
#define M40080 "D�FINITION DES CODES D'�TAT DES OBSERVATIONS"
#define M40081 "Le code d'�tat d'une observation relative � une r�gle est"
#define M40082 "     i) PASS--si toutes les valeurs des variables ayant un coefficient non nul "
#define M40083 "              sont non manquantes et que la r�gle est satisfaite"
#define M40084 "    ii) MISS--si une ou plusieurs variables ayant un coefficient non nul "
#define M40085 "              ont une valeur manquante"
#define M40086 "   iii) FAIL--si toutes les valeurs des variables ayant un coefficient non nul "
#define M40087 "              sont non manquantes mais la r�gle n'est pas satisfaite"
#define M40088 "Le code d'�tat g�n�ral d'une observation est"
#define M40089 "     i) PASS--si l'observation a un code d'�tat PASS pour chacune des r�gles"
#define M40090 "    ii) MISS--si l'observation a au moins un code d'�tat MISS pour l'une des "
#define M40091 "              r�gles et le code d'�tat PASS pour toutes les autres r�gles"
#define M40092 "   iii) FAIL--si l'observation a un code d'�tat FAIL pour l'une des r�gles"

/**************************************************************/
/* Special series - Algorithms predefined descriptions         */
/**************************************************************/
#define AUXTREND_D   "La valeur de la m�me unit� lors du cycle d'enqu�te pr�c�dent, corrig�e en fonction de la variation d'une variable auxiliaire, est imput�e."
#define AUXTREND2_D  "Une moyenne de deux AUXTRENDs est imput�e."
#define CURAUX_D     "La valeur courante d'une variable auxiliaire pour la m�me unit� est imput�e."
#define CURAUXMEAN_D "La moyenne courante d'une variable auxiliaire est imput�e."
#define CURMEAN_D    "La valeur moyenne de la variable � imputer pour toutes les unit�s admissibles � l'occasion du cycle d'enqu�te courant est imput�e."
#define CURRATIO_D   "Une estimation par quotient, �tablie � partir des moyennes calcul�es pour les unit�s admissibles � l'occasion du cycle d'enqu�te courant, est imput�e."
#define CURRATIO2_D  "Une moyenne de deux CURRATIOs est imput�e."
#define CURREG_D     "R�gression lin�aire simple bas�e sur une variable ind�pendante provenant du cycle d'enqu�te courant."
#define CURREG_E2_D  "R�gression lin�aire bas�e sur la valeur et le carr� de la valeur d'une variable provenant du cycle d'enqu�te courant."
#define CURREG2_D    "R�gression lin�aire bas�e sur deux variables ind�pendantes provenant du cycle d'enqu�te courant."
#define CURREG3_D    "R�gression lin�aire bas�e sur trois variables ind�pendantes provenant du cycle d'enqu�te courant."
#define CURSUM2_D    "La somme de deux variables auxiliaires provenant du cycle d'enqu�te courant est imput�e."
#define CURSUM3_D    "La somme de trois variables auxiliaires provenant du cycle d'enqu�te courant est imput�e."
#define CURSUM4_D    "La somme de quatre variables auxiliaires provenant du cycle d'enqu�te courant est imput�e."
#define DIFTREND_D   "La valeur de la m�me unit� lors du cycle d'enqu�te pr�c�dent, corrig�e en fonction de la variation de la moyenne de cette variable, est imput�e."
#define HISTREG_D    "R�gression lin�aire bas�e sur les valeurs prises par la variable � imputer lors du cycle d'enqu�te pr�c�dent."
#define PREAUX_D     "La valeur historique d'une variable auxiliaire pour la m�me unit� est imput�e."
#define PREAUXMEAN_D "La moyenne historique d'une variable auxiliaire est imput�e."
#define PREMEAN_D    "La valeur moyenne de la variable � imputer pour toutes les unit�s admissibles � l'occasion du cycle d'enqu�te pr�c�dent est imput�e."
#define PREVALUE_D   "La valeur de la m�me unit� lors du cycle d'enqu�te pr�c�dent est imput�e."


/**************************************************************/
/* M00000 series - Miscellaneous                               */
/**************************************************************/
#define M00002 "Erreur pr�s ou avant"
#define M00004 "Deux points"
#define M00005 "Fin"
#define M00006 "Erreur"
#define M00008 "Modificateur"
#define M00009 "Op�rateur"
#define M00010 "Plus"
#define M00011 "Point virgule"
#define M00013 "Variable"
#define M00014 "Caract�re non reconnu"
#define M00017 "R�gle d'�galit�: '%d' rend les r�gles incoh�rentes."
#define M00023 "R�gle d'in�galit�: '%d' est enti�rement en dehors de la r�gion d'acceptation. Elle est redondante."
#define M00026 "ANALYSE DIAGNOSTIQUE DES R�GLES DE V�RIFICATION"
#define M00027 "ENSEMBLE DES R�GLES DE V�RIFICATION INCOH�RENTES"
#define M00028 "Incoh�rence entre les algorithmes du simplexe et ceux de Given: probl�me logiciel."
#define M00029 "ENSEMBLE MINIMAL DES R�GLES DE V�RIFICATION"
#define M00030 "L'ensemble minimal des r�gles de v�rification est identique � l'ensemble des r�gles originales sp�cifi�es."
#define M00031 "R�gle d'in�galit�: '%d' rend les r�gles incoh�rentes."
#define M00032 "courant"  /* for french grammar: 'male gender' form   */
#define M00033 "courante" /* for french grammar: 'female gender' form */
#define M00034 "historique"
#define M00035 "Parseur pour une fonction d'estimation"
#define M00036 "Parseur pour une r�gression lin�aire"
#define M00037 "Parseur des r�gles pour le calcul proportionnel"
#define M00038 "Poids"
#define M00039 "Parseur pour les poids"
#define M00040 "�gal" 
#define M00041 "Nombre"
#define M00042 "Terme non reconnu"

/**************************************************************/
/* the following messages are added for the                   */
/* addition of RANDNUMVAR                                     */
/**************************************************************/

/* the following messages are added for DonorInternal.c       */
#define MsgRandnumvarDonorInternalTslistLookupFail "DonorInternal: �chec pendant la recherche de tSList dans DONOR_FindNextNearestDonorIndexAlt()"
#define MsgRandnumvarDonorInternalTadtlistIndexFail "DonorInternal: �chec pendant l'indexation de tADTList dans DONOR_FindNextNearestDonorIndexAlt()"
#define MsgRandnumvarDonorInternalNotFound "Aucun nombre al�atoire trouv� dans DONOR_FindNextNearestDonorIndexAlt()"
#define MsgRandnumvarDonorInternalLessThanZero "DonorInternal: Le nombre al�atoire du receveur %f est plus petit que 0. Veuillez corriger le nombre al�atoire et relancez la proc�dure."
#define MsgRandnumvarDonorInternalGreaterThanOne "DonorInternal: Le nombre al�atoire du receveur %f est plus grand que 1. Veuillez corriger le nombre al�atoire et relancez la proc�dure."

/* the following messages are added for EI_RandomDonor        */
#define MsgRandnumvarRandomDonorlTslistLookupFail "EI_RandomDonor: �chec pendant la recherche de tSList dans EI_RandomDonorWithPostEdits()"
#define MsgRandnumvarRandomDonorTadtlistIndexFail "EI_RandomDonor: �chec pendant l'indexation de tADTList dans EI_RandomDonorWithPostEdits()"
#define MsgRandnumvarRandomDonorNotFound "Aucun nombre al�atoire trouv� dans EI_RandomDonorWithPostEdits()"
#define MsgRandnumvarRandomDonorLessThanZero "EI_RandomDonor: Le nombre al�atoire du receveur %f est plus petit que 0. Veuillez corriger le nombre al�atoire et relancez la proc�dure."
#define MsgRandnumvarRandomDonorGreaterThanOne "EI_RandomDonor: Le nombre al�atoire du receveur %f est plus grand que 1. Veuillez corriger le nombre al�atoire et relancez la proc�dure."
#define MsgRandnumvarRandomDonorInvalidRange "DONOR: Au moins un nombre al�atoire du receveur est �gal � 1 ou tr�s pr�s de 1, dans l'intervalle (1-1e-012,1].  En raison de contraintes de pr�cision d�cimale, 1-1e-012 sera utilis� pour le choix al�atoire plut�t que le nombre al�atoire du receveur."

/* the following messages are added for EI_Chernikova.c       */
#define MsgRandnumvarErrorlocLessThanZero "ERRORLOC: Le nombre al�atoire du receveur %f est plus petit que 0. Veuillez corriger le nombre al�atoire et relancez la proc�dure."
#define MsgRandnumvarErrorlocGreaterThanOne "ERRORLOC: Le nombre al�atoire du receveur %f est plus grand que 1. Veuillez corriger le nombre al�atoire et relancez la proc�dure."
#define MsgRandnumvarErrorlocInvalid "ERRORLOC: Au moins un nombre al�atoire est �gal � 1 ou tr�s pr�s de 1, dans l'intervalle (1-1e-012,1].  En raison de contraintes de pr�cision d�cimale, 1-1e-012 sera utilis� pour le choix al�atoire plut�t que le nombre al�atoire de l'enregistrement."

#endif

