# Table des matières

- [Introduction](#introduction)
    - [Liste des données de sortie](#liste-des-données-de-sortie)
    - [Données de sortie communes à plusieurs procédures](#données-de-sortie-communes-à-plusieurs-procédures)
- [Données de sortie standards](#données-de-sortie-standards)
    - [outdata](#outdata)
    - [outstatus](#outstatus)
- [editstats](#editstats)
    - [outedits_reduced](#outedits_reduced)
    - [outedit_status](#outedit_status)
    - [outk_edits_status](#outk_edits_status)
    - [outglobal_status](#outglobal_status)
    - [outedit_applic](#outedit_applic)
    - [outvars_role](#outvars_role)
- [outlier](#outlier)
    - [outstatus_detailed](#outstatus_detailed)
    - [outsummary](#outsummary)
- [errorloc](#errorloc)
    - [outreject](#outreject)
- [donorimp](#donorimp)
    - [outmatching_fields](#outmatching_fields)
    - [outdonormap](#outdonormap)
- [estimator](#estimator)
    - [outacceptable](#outacceptable)
    - [outest_ef](#outest_ef)
    - [outest_lr](#outest_lr)
    - [outest_parm](#outest_parm)
    - [outrand_err](#outrand_err)
- [prorate](#prorate)
    - [outreject](#outreject-1)
- [massimp](#massimp)
    - [outdonormap](#outdonormap-1)

# Introduction

Les procédures de Banff génèrent un certain nombre de données de sortie, qui peuvent être catégorisées comme suit:
- Données de sortie standards:
    - `outdata`, contenant les résultats de l'imputation (pour les procédures effectuant un traitement)
    - `outstatus`, contenant les statuts (pour les procédures effectuant une sélection ou un traitement)
- Données de diagnostique, contenant l'information utile spécifique à la procédure.

Noter que pour plusieurs données de sortie, **\<unit_id\>** apparaît comme en-tête de colonne. Il s'agit d'un élément de substitution; l'en-tête réel de la colonne est le nom de la variable spécifié dans le paramètre `unit_id`.

### Liste des données de sortie

| Procédure     | Données de sortie                              | 
| ------------- | --------------------------------------------   |
| verifyedits   | *néant - sortie imprimée au journal*           |
| editstats     | `outedits_reduced` `outedit_status` `outk_edits_status` `outglobal_status` `outedit_applic` `outvars_role`   |
| outlier       | `outstatus` `outstatus_detailed` `outsummary`  |
| errorloc      | `outstatus` `outreject`                        |
| deterministic | `outdata` `outstatus`                          |  
| donorimp      | `outdata` `outstatus` `outdonormap` `outmatching_fields`  |  
| estimator     | `outdata` `outstatus` `outacceptable` `outest_ef` `outest_lr` `outest_parm` `outrand_err` |  
| prorate       | `outdata` `outstatus` `outreject`              |  
| massimp       | `outdata` `outstatus` `outdonormap`            |  

### Données de sortie communes à plusieurs procédures

| Procédure     | outdata | outstatus | outdonormap | outreject | 
| ------------- | :-----: | :-------: | :---------: | :-------: |
| outlier       |         | X         |             |           |
| errorloc      |         | X         |             | X         |
| deterministic | X       | X         |             |           |  
| donorimp      | X       | X         | X           |           |  
| estimator     | X       | X         |             |           |  
| prorate       | X       | X         |             | X         |  
| massimp       | X       | X         | X           |           |  

# Données de sortie standards

Les procédures de Banff sont conçues pour être exécutées de manière séquentielle dans le cadre d'un processus de VDS. Les éléments de sortie d'une procédure servent souvent d'éléments d'entrée pour la procédure suivante, et les données statistiques qui sont la cible du processus de VDS sont mises à jour en permanence et tout au long du processus. Parmi les données de sortie, `outdata` et `outstatus` sont celles standards et communes à plusieurs procédures. Veuillez vous référer à la section intitulée [interaction entre procédures](/docs/FR/user_guide.md#interaction-entre-procédures) du guide de l'utilisateur pour plus de détails.

### outdata

**Du guide de l'utilisateur**: 

* Si aucun enregistrement n'a été imputé avec succès ou modifié par la procédure, alors `outdata` sera vide. Aucune erreur ne se produira.
* Les données `outdata` vont toujours contenir la variable identifiée par le paramètre `unit_id`. 
* Typiquement, les données `outdata` **ne sont pas une copie à l'image de  `indata`** mais contiennent uniquement les lignes et les colonnes affectées par la procédure. Par exemple, si `indata` contient 2000 lignes et 25 colonnes, mais seulement 500 lignes et 10 colonnes sont affectées par la procédure, alors `outdata` n'inclura que les 500 lignes et 10 colonnes. L'utilisateur doit manuellement mettre à jour les données `indata` à l'aide de l'information contenue dans `outdata`. (*Note: l'équipe de Banff étudie la possibilité de mettre à jour de manière automatique les données `indata` à partir de l'information dans `outdata`, et ce dans une future version.*)   

### outstatus

**Du guide de l'utilisateur**: 

Puisque Banff fonctionne avec des données tabulaires, chaque observation peut être associée à une ligne et une colonne spécifiques dans `outstatus`. Les lignes sont identifiées par l'identifiant de l'enregistrement spécifié par l'utilisateur `unit_id`, alors que l'on fait référence aux colonnes par leur nom. Les statuts de sélection et/ou de traitement associés aux observations affectées par la procédure sont conservés dans les données des statuts de sortie, dont les colonnes sont les suivantes:

| Colonne      | Description |
| ------------ | ----------- |
| \<unit_id\>  | Identifiant de l'enregistrement (c'est-à-dire la ligne) auquel le statut s'applique.    |
| FIELDID      | Identifiant du champ(c'est-à-dire la colonne ) auquel le statut s'applique.   |
| STATUS       | Code du statut tel que "FTI", "FTE", ou "IDN".                         |
| VALUE        | Valeur de la variable lorsque le code du statut a été généré. Pour les procédure effectuant la sélection (`errorloc` et `outlier`), cette colonnes correspond à la valeur de l'observation dans `indata` lorsque la sélection a eu lieu. Pour les procédures effectuant le traitement (`deterministic`, `donorimp`, `estimator`, `prorate`, `massimp`), cette colonne représente la valeur de l'observation dans `outdata` après que le traitement ait eu lieu.   |

# editstats

### outedits_reduced

Contient l'ensemble minimal des règles de vérification. La valeur `EDITID` est créée par la procédure pour être utilisée comme référence dans d'autres données de sortie.

| Colonne        | Description |
| -------------- | ----------- |
| EDITID         | Numéro d'identification de la règle de vérification de l'ensemble minimal.    |
| EDIT_EQUATION  | La formulation de la règle de vérification.                                   |

### outedit_status

Nombre d'enregistrements qui satisfont chaque règle de vérification ou ne la satisfont pas à cause d'une valeur manquante ou non manquante. 

| Colonne     | Description |
| ------------| ----------- |
| EDITID      | Numéro d'identification de la règle de vérification de l'ensemble minimal.     |
| OBS_PASSED  | Nombre d'observations qui satisfont la règle de vérification.                  |
| OBS_MISSED  | Nombre d'observations qui ont une ou plusieurs valeurs manquantes pour les variables visées par la règle.   |
| OBS_FAILED  | Nombre d'observations qui ne satisfont pas la règle à cause d'une ou plusieurs valeurs non manquantes.  |

### outk_edits_status

Distribution des enregistrements qui satisfont un nombre donné de règles de vérification ou ne les satisfont pas à cause d'une valeur manquante ou non manquante.

| Colonne     | Description |
| ------------| ----------- |
| K_EDITS     | Nombre cumulé « k » de règles de vérification relevant de l’ensemble minimal. |
| OBS_PASSED  | Nombre d’observations qui satisfont « k » règles de vérification.           |
| OBS_MISSED  | Nombre d’observations comportant une ou plusieurs valeurs manquantes pour les variables visées par « k » règles de vérification.   |
| OBS_FAILED  | Nombre d’observations qui ne satisfont pas « k » règles de vérification à cause d’une ou plusieurs valeurs non manquantes. |

### outglobal_status

Nombre d'enregistrements qui satisfont toutes les règles de vérification de l'ensemble minimal ou ne les satisfont pas à cause d'une valeur manquante ou non manquante. 

| Colonne     | Description |
| ------------| ----------- |
| OBS_PASSED  |  Nombre d’observations dont l’état général est « PASS » (c'est-à-dire ayant satisfait à toutes les règles de vérification de l’ensemble minimal).                         |
| OBS_MISSED  | Nombre d’observations dont l’état général est « MISS » (c'est-à-dire ayant une ou plusieurs valeurs manquantes pour une ou plusieurs règles de l’ensemble minimal, mais n’ayant aucun code d’état « FAIL » pour l’une de ces règles).                                 |
| OBS_FAILED  | Nombre d’observations dont l’état général est « FAIL » (c'est-à-dire ayant au moins un code d’état FAIL pour l’une des règles de l’ensemble minimal).                       |
| OBS_TOTAL   | Nombre total d’observations.           |

### outedit_applic

Nombre de fois chaque variable est impliquée dans une règle de vérification qui est satisfaite ou non satisfaite à cause d'une valeur manquante ou non manquante. 

| Colonne             | Description |
| ------------------- | ----------- |
| FIELDID             | Nom de la variable.                        |
| EDIT_APPLIC_PASSED  | Nombre de fois où la variable a hérité du code d’état « PASS » attribué aux observations pour les règles concernées par cette variable.               |
| EDIT_APPLIC_MISSED  | Nombre de fois où la variable a hérité du code d’état « MISS » attribué aux observations pour les règles concernées par cette variable.               |
| EDIT_APPLIC_FAILED  | Nombre de fois où la variable a hérité du code d’état « FAIL » attribué aux observations pour les règles concernées par cette variable.               |
| EDIT_APPLIC_NOTINVOLVED  | Nombre de règles non concernées par la variable multiplié par le nombre d'observations.                                                        |
| EDITS_INVOLVED  | Nombre de règles de vérification concernées par la variable.       |

### outvars_role

Nombre de fois chacune des variables contribue à l'état général de l'enregistrement.

| Colonne              | Description |
| -------------------  | ----------- |
| FIELDID              | Nom de la variable.                        |
| OBS_PASSED           | Nombre de fois où la variable a eu une incidence sur le code d’état général « PASS » attribué aux observations.                         |
| OBS_MISSED           | Nombre de fois où la variable a eu une incidence sur le code d’état général « MISS » attribué aux observations.                            |
| OBS_FAILED           | Nombre de fois où la variable a eu une incidence sur le code d’état général « FAIL » attribué aux observations.                            |
| OBS_NOT_APPLICABLE   | Nombre de fois où la variable n’a aucune incidence sur le code d’état général « MISS » ou « FAIL » attribué aux observations.              |

# outlier

### outstatus_detailed

Contient le statut détaillé de chaque valeur aberrante détectée. De l'information additionnelle est ajoutée lorsque `outlier_stats=True` est spécifié dans l'appel de la procédure.

| Colonne        | Description |
| -------------- | ----------- |
| \<unit_id\>    | Identifiant de l'enregistrement.     |
| FIELDID        | Identifiant du champ.   |
| OUTLIER_STATUS | Statut détaillé des valeurs aberrantes ayant quatre valeurs possibles:<br><br> - `ODER`: Valeur aberrante à l'extérieur de l'intervalle d'exclusion à droite.<br> - `ODEL`: valeur aberrante à l'extérieur de l'intervalle d'exclusion à gauche.<br> - `ODIR`: Valeur aberrante à l'extérieur de l'intervalle d'imputation à droite.<br> - `ODIL`: Valeur aberrante à l'extérieur de l'intervalle d'imputation à gauche.      |
| METHOD | Méthode de détection des valeurs aberrantes.                       |
| CURRENT_VALUE | Valeur aberrante issue de `indata`.                         |
| WEIGHT | Poids appliqué à `CURRENT_VALUE`, au ratio ou à la tendance historique lorsque le paramètre `with_var` est spécifié. |
| HIST_AUX | Nom de la variable jumelée avec la variable `var` correspondante.     |
| HIST_AUX_VALUE | Valeur de la variable jumelée avec la variable `var` correspondante (lorsque le paramètre `with_var` est spécifié).                        |
| EFFECT | Valeur de l'effet calculée pour l'enregistrement et utilisée pour comparer avec les bornes de l'intervalle dans le cadre des étapes de la détection des valeurs aberrantes.    |
| GAP | Écart entre cet enregistrement et le précédent ayant une valeur différente (méthode SG).|
| IMP_SIGMAGAP | Écart-sigma de l'imputation calculé en multipliant `beta_i` par la déviation (méthode SG). |
| EXCL_SIGMAGAP | Écart-sigma de l'exclusion calculé en multipliant `beta_e` par la déviation (méthode SG). |
| IMP_BND_L | **Méthode HB**: Borne supérieure de l'intervalle d'imputation à gauche.<br> **Méthode SG**: Premier écart identifiant les enregistrements à imputer à gauche. |
| EXCL_BND_L | **Méthode HB**: Borne supérieure de l'intervalle d'exclusion à gauche.<br> **Méthode SG**: Premier écart identifiant les enregistrements à exclure à gauche. |
| EXCL_BND_R | **Méthode HB**: Borne supérieure de l'intervalle d'exclusion à droite.<br> **Méthode SG**: Premier écart identifiant les enregistrements à exclure à droite.         |
| IMP_BND_R | **Méthode HB**: Borne supérieure de l'intervalle d'imputation à droite.<br> **Méthode SG**: Premier écart identifiant les enregistrements à imputer à droite.    |

### outsummary

Informations sommaires sur les valeurs aberrantes détectées, telles que le compte des observations et des états, ainsi que les sorties statistiques (comme les bornes de l'intervalle d'acceptation) calculées par la procédure.

| Colonne        | Description |
| -------------- | ----------- |
| NObs        | Nombre d'observations dans `indata`.     |
| NValid  | Nombre d'observations valides.     |
| NRejected_NoMatch | Nombre d'observations rejetées en raison de l'impossibilité de trouver une observation dans indata_hist lors de l'appariement à l'aide de `unit_id`.  |
| FIELDID        | Identifiant du champ. Si la détection des valeurs aberrantes est appliquée sur un ratio ou une tendance historique, alors `FIELDID` sera le numérateur.    |
| NUsed         | Nombre d'observations utilisées dans la détection des valeurs aberrantes.     |
| NRejected     | Nombre total d'observations rejetées pour le groupe de partition *by* en cours d'exécution et el champ `FIELDID`.  |
| NRejected_VarMissing | Nombre d'observations rejetées en raison d'une variable d'intérêt manquante. |
| NRejected_VarZero    | Nombre d'observations rejetées parce que la valeur de la variable d'intérêt est nulle (s'applique seulement lorsque `accept_zero=False`).            |
| NRejected_VarNegative     | Nombre d'observations rejetées parce que la valeur de la variable d'intérêt est négative (s'applique seulement lorsque `accept_negative=False`).     |
| NFTI     | Nombre d'observations identifiées pour une imputation (c'est-à-dire ayant un statut FTI).           |
| NFTE     | Nombre d'observations identifiées pour une exclusion (c'est-à-dire ayant un statut FTE).            |
| IMP_BND_L     | **Méthode HB**: Borne supérieure de l'intervalle d'imputation à gauche.<br> **Méthode SG**: Premier écart identifiant les enregistrements à imputer à gauche.           |
| EXCL_BND_L     | **Méthode HB**: Borne supérieure de l'intervalle d'exclusion à gauche.<br> **Méthode SG**: Premier écart identifiant les enregistrements à exclure à gauche.            |
| EXCL_BND_R     | **Méthode HB**: Borne supérieure de l'intervalle d'exclusion à droite.<br> **Méthode SG**: Premier écart identifiant les enregistrements à exclude à droite.            |
| IMP_BND_R     | **Méthode HB**: Borne supérieure de l'intervalle d'imputation à droite.<br> **Méthode SG**: Premier écart identifiant les enregistrements à imputer à droite.            |
| AuxVarID       | Variable auxiliaire servant de dénominateur lorsque la détection des valeurs aberrantes est utilisée sur un ratio ou une tendance historique. |
| NRejected_AuxVarMissing  | Nombre d'observations rejetées en raison d'une variable auxiliaire manquante. |
| NRejected_AuxVarZero     | Nombre d'observations rejetées en raison d'une variable auxiliaire nulle.   |
| NRejected_AuxVarNegative | Nombre d'observations rejetées en raison d'une variable auxiliaire négative. |
| Q1     | Premier quartile (Méthode HB).          |
| M      | Médiane (Méthode HB).                    |        
| Q3     | Troisième quartile (Méthode HB).        |
| DEVIATION     | Écart médian absolu si `sigma='MAD'` ou écart-type si `sigma='STD'` (Méthode SG).            |
| IMP_SIGMAGAP  | Écart-sigma d'imputation obtenu en multipliant `beta_i` par `DEVIATION` (Méthode SG). |
| EXCL_SIGMAGAP | Écart-sigma d'exclusion obtenu en multipliant `beta_e` par `DEVIATION` (Méthode SG).  |

# errorloc

### outreject

Enregistrements pour lesquels le problème de localisation des erreurs n'a pas pu être résolu.

| Colonne      | Description |
| ------------ | ----------- |
| \<unit_id\>  | Identifiant de l'enregistrement.     |
| NAME_ERROR   | Prend une parmi deux valeurs possibles:<br><br> - `CARDINALITY EXCEEDED`: La cardinalité de la solution pour cette observation dépasse la cardinalité maximale spécifiée dans le paramètre `cardinality`.<br> - `TIME EXCEEDED`: Le temps requis pour traiter cette observation dépasse le temps maximal par observation spécifié dans le paramètre `time_per_obs`. |

# donorimp

### outmatching_fields

Statuts des champs d'appariement utilisés dans le calcul de la distance. Ces statuts peuvent être différents pour chaque receveur.

| Colonne      | Description |
| ------------ | ----------- |
| \<unit_id\>  | Identifiant du receveur.     |
| FIELDID      | Identifiant du champ.   |
| STATUS       | Prend une parmi quatre valeurs possibles:<br><br> - `IDN`: Champ imputé en utilisant l'imputation par donneur.<br> - `MFS`: Champ d'appariement sélectionné par le système. <br> - `MFU`: Champ d'appariement spécifié par l'utilisateur. <br> - `MFB`: Champ sélectionné par le système et spécifié par l'utilisateur.  |

### outdonormap

Paires receveur-donneur pour les enregistrements imputés avec succès.

| Colonne           | Description |
| ----------------- | ----------- |
| RECIPIENT         | Identifiant du receveur.  |
| DONOR             | Identifiant du donneur.   |
| NUMBER_OF_ATTEMPTS | Nombre de donneurs essayés avant que le receveur ait pu satisfaire les règles de vérification après imputation. |
| DONORLIMIT        | Reproduit la valeur du paramètre `n_limit` (même valeur pour tous les enregistrements).   |

# estimator

### outacceptable

Liste des observations acceptables utilisées dans le calcul des paramètres du modèle d'imputation.

| Colonne        | Description |
| -------------- | ----------- |
| ESTIMID        | Numéro d'identification de l'estimateur défini à partir de l'ordre des modèles dans inestimator (commence par un zéro).  |
| ALGORITHMNAME  | Nom de l'algorithme (prédéfini dans la procédure ou personnalisé). |
| \<unit_id\>    | Identifiant de l'enregistrement.      |

### outest_ef

Rapport du calcul des moyennes (functions d'estimation).

| Colonne        | Description |
| -------------- | ----------- |
| ESTIMID        | Numéro d'identification de l'estimateur défini à partir de l'ordre des modèles dans inestimator (commence par un zéro).   |
| ALGORITHMNAME  | Nom de l'algorithme (prédéfini dans la procédure ou personnalisé). |
| FIELDID        | Identifiant du champ pour lequel une moyenne est calculée.    |
| PERIOD         | Période courante (C) or historique (H) associée à la variable `FIELDID`. |
| AVERAGE_VALUE  | Valeur moyenne pour le variable `FIELDID`.     |
| COUNT          | Nombre d'observations acceptables utilisées dans le calcul de la distance. Le nombre d'observations acceptables est le même dans le calcul de toutes les moyennes présentes dans la formule d'un estimateur.    |

### outest_lr

Rapport du calcul des coefficients « beta »  (estimateurs par régression linéaire).

| Colonne        | Description |
| -------------- | ----------- |
| ESTIMID        | Numéro d'identification de l'estimateur par régression linéaire défini à partir de l'ordre des modèles dans inestimator (commence par un zéro).   |
| ALGORITHMNAME  | Nom de l'algorithme (prédéfini dans la procédure ou personnalisé). |
| FIELDID        | Nom de la variable (ou régresseur) pour laquelle un coefficient beta est calculé.  |
| EXPONENT       | Exposant du régresseur. |
| PERIOD         | Période courante (C) ou historique (H) associée au régresseur.    |
| BETA_VALUE     | Valeur du coefficient « beta » associée au régresseur.            |
| COUNT          | Nombre d'observations acceptables utilisées dans le calcul des coefficients « beta ». Le nombre d'observations acceptables est le même dans le calcul de tous les coefficient « beta » présents dans la formule d'un estimateur.     |

### outest_parm

Statistiques de l'imputation.

| Colonne        | Description |
| -------------- | ----------- |
| ESTIMID        | Numéro d'identification de l'estimateur par régression linéaire défini à partir de l'ordre des modèles dans inestimator (commence par un zéro).   |
| ALGORITHMNAME  | Nom de l'algorithme (prédéfini dans la procédure ou personnalisé). |
| FIELDID        | Identifiant du champ.      |
| FTI            | Nombre de statuts FTI dans `instatus`, c'est-à-dire le nombre d'enregistrements nécessitant l'imputation.                                           |
| IMP            | Nombre de statuts I-- dans `outstatus`, c'est-à-dire le nombre d'enregistrements imputés avec succès.                                |
| DIVISIONBYZERO | Nombre d’imputations non réussies car le calcul implique une division 
par 0.        |
| NEGATIVE       | Nombre d’imputations non réussies car la valeur imputée est négative. (Not applicable if `accept_negative=True`.)    |

### outrand_err

Rapport sur l'erreur aléatoire.

| Colonne        | Description |
| -------------- | ----------- |
| ESTIMID        | Numéro d'identification de l'estimateur par régression linéaire défini à partir de l'ordre des modèles dans inestimator (commence par un zéro).   |
| ALGORITHMNAME  | Nom de l'algorithme (prédéfini dans la procédure ou personnalisé). |
| RECIPIENT      | Identifiant du receveur.   |
| DONOR          | Identifiant du donneur.    |
| FIELDID        | Identifiant du champ.      |
| RESIDUAL       | Calculé comme la différence entre la valeur originale du donneur et la valeur estimée, toutes deux associées au donneur.   |
| RANDOMERROR    | Si le type de l'algorithme est LR et une variance est utilisée: RESIDUAL* sqrt ((variance du receveur ^ exposant) / (variance du donneur ^ exposant)); sinon c'est la même valeur que `RESIDUAL`.    |
| ORIGINALVALUE  | Valeur de la variable avant imputation.  |
| IMPUTEDVALUE   | Valeur de la variable après imputation.  |

# prorate

### outreject

Enregistrements pour lesquels l'ajustement au prorata n'a pas pu être effectué.

| Colonne      | Description |
| ------------ | ----------- |
| \<unit_id\>  | Identifiant de l'enregistrement.     |
| NAME_ERROR   | Prend une parmi sept valeurs possibles:<br><br> - `DECIMAL ERROR`: L'utilisateur a spécifié moins de décimales qu'il n'en existe dans le total ajusté.<br> - `SCALING VALUE K GREATER THAN +1`: L'intervalle acceptable pour le facteur k est: -1 <= k <= 1 .<br> - `SCALING VALUE K LESS THAN -1`: L'intervalle acceptable pour le facteur k est: -1 <= k <= 1 .<br> - `NOTHING TO PRORATE`: Il ne reste aucune variable à ajuster au prorata; elles ont toutes été éliminées parce que le modificateur n'indique pas que la variable peut être ajustée au prorata (en prenant en considération leur état dans `instatus` si le modificateur est O ou I) ou leur valeur est 0.<br> - `OUT OF BOUNDS`: La valeur arrondie divisée par la valeur originale n'est pas à l'intérieur de l'intervalle défini par les bornes.<br> - `SUM OF PRORATABLE COLUMNS IS 0`: Le facteur k ne peut être calculé car la somme pondérée des variables qui peuvent être ajustées au prorata est 0.<br> - `NEGATIVE VALUE IN DATA`: Une variable a une valeur négative alors que `accept_negative=False`. |

# massimp

### outdonormap

Paires receveur-donneur pour les enregistrements imputés avec succès.

| Colonne           | Description |
| ----------------- | ----------- |
| RECIPIENT         | Identifiant du receveur.  |
| DONOR             | Identifiant du donneur.   |
| NUMBER_OF_ATTEMPTS | Peut prendre une de deux valeurs possibles: (1) si le donneur a été sélectionné par la méthode du voisin le plus proche ou (0) s'il a été choisi de manière aléatoire. |
| DONORLIMIT        | Reproduit la valeur du paramètre `n_limit` (même valeur pour tous les enregistrements).   |
