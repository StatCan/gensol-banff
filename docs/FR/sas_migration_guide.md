# Guide de migration SAS du package Banff

## Dans ce guide

Le public cible de ce guide est constitué des utilisateurs de Banff 2.08.002 basé sur SAS ou d'une version antérieure qui migrent vers Banff 3.x basé sur Python.
Il résume les informations sur les modifications apportées dans la version 3, y compris les nouveaux noms des paramètres et des tables, ainsi que des exemples de la manière dont les programmes SAS qui appellent des procédures Banff peuvent être convertis en programmes Python équivalents.

Ce document n'est pas destiné aux nouveaux utilisateurs de Banff car il ne fournit pas un aperçu complet des procédures Banff. Veuillez vous référer au [Guide de l'utilisateur](/docs/FR/user_guide.md) pour obtenir des détails complets sur l'utilisation de chaque procédure, de ses paramètres et de ses tables.

## Table des matières

- [Procédures disponibles](#procédures-disponibles)
- [Paramètres de procédure](#paramètres-de-procédure)
  - [Tableau des paramètres et types de procédures](#tableau-des-paramètres-et-types-de-procédures)
  - [Nouvelles options de procédure](#nouvelles-options-de-procédure)
- [Tables de procédures](#tables-de-procédures)
  - [Tableau des identifiants du table de procédure](#tableau-des-identifiants-du-table-de-procédure)
  - [Modifications apportées aux tables](#modifications-apportées-aux-tables)
- [Specification des tables](#spécification-des-tables)
- [Modifications notables des procédures](#modifications-notables-des-procédures)
  - [Modifications apportées aux tables d'entrée](#modifications-apportées-aux-tables-dentrée)
  - [Modifications apportées aux tables de sortie](#modifications-apportées-aux-tables-de-sortie)
- [Autres options d'exécution Python](#autres-options-dexécution-python)
- [Considérations relatives aux performances](#considérations-relatives-aux-performances)
- [Erreurs et exceptions](#erreurs-et-exceptions)
- [Fonctions utilitaires](#fonctions-utilitaires)

## Procédures disponibles

Pour obtenir la liste des procédures disponibles, consultez le [Guide de l'utilisateur](/docs/FR/user_guide.md#tables-de-procédures).

Chaque procédure a été convertie en prenant le code source de la procédure Banff 2.08.002 dépendant de SAS et en le modifiant pour produire une procédure code source ouvert, qui est *"encapsulée"* dans un paquet Python. Les calculs mathématiques sous-jacents restent inchangés.

En raison des différences entre SAS et Python, les utilisateurs doivent adapter la *façon* dont ils spécifient les paramètres et les tables ; les ensembles de paramètres et de tables restent en grande partie inchangés (bien que la plupart des [*identifiants de paramètres*](#tableau-des-paramètres-et-types-de-procédures) et des [*identifiants de table*](#tableau-des-identifiants-du-table-de-procédure) aient changé).

## Paramètres de procédure

De nombreux noms de paramètres ont été modifiés pour mieux suivre les conventions de dénomination Python courantes.

Les identifiants utilisés dans les programmes SAS correspondent aux identifiants et types Python suivants:

### Tableau des paramètres et types de procédures

|Identifiant SAS|Identifiant Python|Type Python|Remarque|
|--|--|--|--|
|`ACCEPTNEGATIVE`|`accept_negative`|`bool`||
|`ACCEPTZERO`|`accept_zero`|`bool`||
|`BETAE`|`beta_e`|`int` or `float`||
|`BETAI`|`beta_i`|`int` or `float`||
|`BOUNDSTAT`|`outlier_stats`|`bool`|**MODIFIÉ** : concerne désormais le contenu de la table [`outstatus_detailed`](#modifications-apportées-à-outlier-outstatus)|
|`BY`|`by`|`str`||
|`CARDINALITY`|`cardinality`|`int` or `float`||
|`DATAEXCLVAR`|`data_excl_var`|`str`||
|`DECIMAL`|`decimal`|`int` or `float`||
|`DISPLAYLEVEL`|`display_level`|`int` or `float`||
|`EDITS`|`edits`|`str`||
|`ELIGDON`|`eligdon`|`str`||
|`EXPONENT`|`exponent`|`int` or `float`||
|`EXTREMAL`|`extremal`|`int` or `float`||
|`HISTEXCLVAR`|`hist_excl_var`|`str`||
|`ID`|`unit_id`|`str`||
|`IMPLY`|`imply`|`int` or `float`||
|`LOWERBOUND`|`lower_bound`|`int` or `float`||
|~~`MATCHFIELDSTAT`~~|~~`match_field_stat`~~|~~`bool`~~|**OBSOLÈTE** : utilisez plutôt la table de sortie [`outmatching_fields`](#modifications-apportées-à-imputation-par-donneur-outstatus)|
|`MAXCARDINALITY`|`extremal`|`int` or `float`||
|`MAXIMPLIEDEDITS`|`imply`|`int` or `float`||
|`MDM`|`mdm`|`int` or `float`||
|`MEI`|`mei`|`int` or `float`||
|`METHOD`|`method`|`str`||
|`MII`|`mii`|`int` or `float`||
|`MINDONORS`|`min_donors`|`int` or `float`||
|`MINOBS`|`min_obs`|`int` or `float`||
|`MODIFIER`|`modifier`|`str`||
|`MRL`|`mrl`|`int` or `float`||
|`MUSTIMPUTE`|`must_impute`|`str`||
|`MUSTMATCH`|`must_match`|`str`||
|`N`|`n`|`int` or `float`||
|`NLIMIT`|`n_limit`|`int` or `float`||
|`NOBYSTATS`|`no_by_stats`|`bool`||
|`OUTLIERSTAT`|`outlier_stats`|`bool`|**MODIFIÉ** : concerne désormais le contenu de la table [`outstatus_detailed`](#modifications-apportées-à-outlier-outstatus)|
|`PCENTDONORS`|`percent_donors`|`int` or `float`||
|`POSTEDITS`|`post_edits`|`str`||
|`RANDNUMVAR`|`rand_num_var`|`str`||
|`RANDOM`|`random`|`bool`||
|~~`REJECTNEGATIVE`~~|~~`reject_negative`~~|~~`bool`~~|**OBSOLÈTE** : spécifiez plutôt `accept_negative=False`|
|~~`REJECTZERO`~~|~~`reject_zero`~~|~~`bool`~~|**OBSOLÈTE** : spécifiez plutôt `accept_zero=False`|
|`SEED`|`seed`|`int` ou `float`||
|`SIDE`|`side`|`str`||
|`SIGMA`|`sigma`|`str`||
|`STARTCENTILE`|`start_centile`|`int` ou `float`||
|`TIMEPEROBS`|`time_per_obs`|`int` ou `float`||
|`UPPERBOUND`|`upper_bound`|`int` ou `float`||
|`VAR`|`var`|`str`||
|`VERIFYEDITS`|`verify_edits`|`bool`||
|`VERIFYSPECS`|`verify_specs`|`bool`||
|`WEIGHT`|`weight`|`str`||
|`WEIGHTS`|`weights`|`str`||
|`WITH`|`with_var`|`str`||

### Exemple de spécification de paramètres en Python

Le code suivant montre comment spécifier différents *types Python* associés à certains paramètres courants.

```python
foo = banff.donorimp(
    min_donors=2,
    percent_donors=0.1,
    accept_negative=True,
    edits="""x1>=-5; 
    x1<=15; 
    x2>=30; 
    x1+x2<=50;""",
    by="province city",
    unit_id='IDENT',
    trace=True,
    # ... etc. (tables)
)
```

> *Exemple de spécification de paramètres SAS*
>
> ```sas
> proc donorimputation
>     /* etc. (tables) ... */
>     mindonors=2
>     pcentdonors=0.1
>     acceptnegative
>     edits="x1>=-5; 
>     x1<=15; 
>     x2>=30; 
>     x1+x2<=50;"
>     ;
>     by province city;
>     id IDENT;
> run;
> ```

|paramètre|note|
|--|--|
|`unit_id`|Un seul nom de variable|
|`by`|une liste de 0 ou plusieurs noms de variables séparés par des espaces|
|`cardinality`|exclut un nombre, voir le guide d'utilisation pour obtenir des conseils|
|`time_per_obs`|s'attend à un nombre, voir le guide d'utilisation pour obtenir des conseils|
|`accept_negative`|boolean: `True`-> spécifié, \<*non spécifié*\> -> non spécifié|
|`edits`| envelopper les chaînes multilignes avec des guillemets triples (`"""edit>=string"""`)|

### Nouvelles options de procédure

Les nouvelles options incluent

- [`exclude_where_indata`](#option-exclude_where_indata)
- [`exclude_where_indata_hist`](#option-exclude_where_indata_hist)
- [`prefill_by_vars`](#option-prefill_by_vars)
- [`presort`](#option-presort)
- [`trace`](#option-trace)

#### Option `exclude_where_indata`

Cette option exclut les enregistrements en fonction d'une expression SQL spécifiée par l'utilisateur. Pour plus de détails, consultez la documentation spécifique à la procédure.

- [donorimp](/docs/FR/Procedure%20Guides/donorimp.md#parameters)
- [estimator](/docs/FR/Procedure%20Guides/estimator.md#parameters)
- [outlier](/docs/FR/Procedure%20Guides/outlier.md#parameters)

#### Option `exclude_where_indata_hist`

Cette option exclut les enregistrements basés sur une expression SQL spécifiée par l'utilisateur. Pour plus de détails, consultez la documentation spécifique à la procédure.

- [estimator](/docs/FR/Procedure%20Guides/estimator.md#parameters)

#### Option `prefill_by_vars`

Cette option est disponible et activée par défaut dans toutes les procédures qui acceptent une table `instatus`.
Voir [Guide de l'utilisateur](/docs/FR/user_guide.md#prefill_by_vars)

#### Option `presort`

Cette option est disponible et activée par défaut dans toutes les procédures qui acceptent des tables d'entrée.
Voir [Guide de l'utilisateur](/docs/FR/user_guide.md#presort)

#### Option `trace`

Cette option est disponible dans toutes les procédures et contrôle la verbosité du journal de la console.
Voir [Guide de l'utilisateur](/docs/FR/user_guide.md#python-log-verbosite-trace)

## Tables de procédures

De nombreux noms de paramètres de table ont été modifiés pour mieux suivre les conventions de dénomination Python courantes.

Les identifiants utilisés dans les programmes SAS correspondent aux identifiants suivants dans Python :

### Tableau des identifiants du table de procédure

|Identifiant SAS|Identifiant Python|Remarque|
|--|--|--|
|`ALGORITHM`|`inalgorithm`||
|`AUX`|`indata_hist`|pas d'option "AUX" en Python, utilisez plutôt `indata_hist`|
|`DATA`|`indata`||
|`DATASTATUS`|`instatus`||
|`DONORMAP`|`outdonormap`||
|`ESTIMATOR`|`inestimator`||
|`HIST`|`indata_hist`||
|`HISTSTATUS`|`instatus_hist`||
|`INSTATUS`|`instatus`|*nouveau*: désormais accepté par la procédure `errorloc`, voir [Ajout de instatus à Errorloc](#ajout-de-instatus-à-la-procédure-errorloc)|
|`OUT`|`outdata`||
|`OUTACCEPTABLE`|`outacceptable`||
|`OUTDATA`|`outdata`||
|`OUTEDITAPPLIC`|`outedit_applic`||
|`OUTEDITSTATUS`|`outedit_status`||
|`OUTGLOBALSTATUS`|`outglobal_status`||
|`OUTKEDITSSTATUS`|`outk_edits_status`||
|`OUTRANDOMERROR`|`outrand_err`||
|`OUTREDUCEDEDITS`|`outedits_reduced`||
|`OUTESTEF`|`outest_ef`||
|`OUTESTLR`|`outest_lr`||
|`OUTESTPARMS`|`outest_parm`||
|`OUTVARSROLE`|`outvars_role`||
|`OUTREJECT`|`outreject`||
|`OUTSTATUS`|`outstatus`|*nouveau*: désormais produit par la procédure `massimp`, voir [Ajout de outstatus à l'imputation de masse](#ajout-de-outstatus-à-la-procédure-dimputation-de-masse)|
|`OUTSUMMARY`|`outsummary`||
|`STATUS`|`instatus`||
|Added in Banff 3.1.1|`outstatus_detailed`|produit par `outlier`, voir [Modifications du statut Outlier Outstatus](#modifications-apportées-à-outlier-outstatus)|
|Added in Banff 3.1.1|`outmatching_fields`|produit par `donorimp`, voir [Modifications apportées au statut d'imputation par donneur](#modifications-apportées-à-imputation-par-donneur-outstatus)|

### Modifications apportées aux tables

Un aperçu des modifications est fourni dans les sous-sections suivantes, avec des liens vers des informations détaillées.
Pour une liste complète des informations détaillées, voir [Modifications notables des procédures](#modifications-notables-des-procédures).

#### Suppression des variables `by`

Les variables `by` ont été supprimées de nombreuses tables de sortie. Voir [*BY* Variables sur les tables de sortie](#variables-by-sur-les-tables-de-sortie) pour plus de détails.  

#### Normalisation de la table `outstatus`

> voir aussi [Tables de sortie > outstatus](/docs/FR/output_tables.md#outstatus)

La table « outstatus » est désormais [standardisée](#normalisation-des-tables-outstatus) dans toutes les procédures qui la produisent. De nouvelles tables de sortie ont été introduites dans les procédures [*Donor Imputation*](#modifications-apportées-à-imputation-par-donneur-outstatus), [*Mass Imputation*](#ajout-de-outstatus-à-la-procédure-dimputation-de-masse) et [*Outlier*](#modifications-apportées-à-outlier-outstatus) pour prendre en compte ce changement.

#### Ajout de la table `instatus`

La procédure *Errorloc* [accepte désormais une table `instatus`](#ajout-de-instatus-à-la-procédure-errorloc).

## Spécification des tables

Pour plus d'informations sur la spécification des tables d'entrée et de sortie, les formats pris en charge, etc., veuillez consulter le [Guide de l'utilisateur](/docs/FR/user_guide.md#spécification-des-données-dentrée-et-de-sortie)

## Modifications notables des procédures

### Modifications apportées aux tables d'entrée

#### Ajout de `instatus` à la procédure *Errorloc*

*Errorloc* accepte désormais une table `instatus`.

La procédure *Errorloc* traite sa table `instatus` de manière quelque peu différente des autres procédures. Pour favoriser la sélection des champs marqués pour imputation, pour chaque ligne dans `instatus` avec un indicateur d'état de `FTI`, la valeur correspondante dans `indata` sera traitée comme si elle était *manquante*.

### Modifications apportées aux tables de sortie

#### Variables *BY* sur les tables de sortie

Dans les procédures basées sur SAS, les variables « by » étaient incluses dans de nombreuses tables de sortie essentiellement par défaut à chaque fois qu'un traitement par groupe était effectué. Dans Banff 3.1.1 cependant, les variables « by » ne sont incluses que dans les tables suivantes:

- **Estimator**
  - `outacceptable`
  - `outest_ef`
  - `outest_lr`
  - `outest_parm`
  - `outrand_err`
- **Outlier**
  - `outsummary`

#### Normalisation des tables `outstatus`

Toutes les tables « outstatus » sont désormais normalisées et contiennent exactement les colonnes suivantes

|Nom de la colonne|Remarque|
|--:|:--|
|`<unit-id>`|colonne nommée d'après la colonne `unit_id` de la table `indata`|
|`FIELDID`|nom de la colonne à laquelle s'applique le statut|
|`STATUS`|le code d'état|
|`VALUE`|valeur de la variable à laquelle s'applique le statut\*|

> \***Colonne `VALUE`**
> Pour les procédures qui produisent une table `outdata`, la valeur provient de là.
> Sinon, la valeur provient de la table `indata`.

De plus, les informations non relatives au statut produites auparavant par les procédures *Imputation par donneur* et *Outlier* ont été supprimées. Voir ci-dessous pour plus d'informations.

##### Modifications apportées à *Imputation par donneur* `outstatus`

*Imputation par donneur* génère désormais une table `outstatus` standardisée. Les données qui ont été supprimées de `outstatus` sont disponibles dans la nouvelle table de sortie facultative `outmatching_fields`.
Cette nouvelle table est désactivée par défaut. Spécifiez `True` ou toute autre option de sortie valide pour l'activer. Cette nouvelle table remplace l'option `match_field_stat`, qui est désormais obsolète.
  
##### Ajout de `outstatus` à la procédure d'*Imputation de masse*

*Imputation de masse* produit une table `outstatus` avec l'indicateur `IMAS`.

##### Modifications apportées à *Outlier* `outstatus`

*Outlier* génère désormais une table `outstatus` standardisée. Les données qui ont été supprimées de `outstatus` sont disponibles dans la nouvelle table facultative `outstatus_detailed`. Cette nouvelle table contient les variables `<unit_id>`, `FIELDID`, `OUTLIER_STATUS` (anciennement `OUTSTATUS`, à ne pas confondre avec la variable *`status`* de la table `oustatus`) et toutes les variables activées en spécifiant `outlier_stats=True`. La table est activée par défaut. Spécifiez `False` pour la désactiver.

## Autres options d'exécution Python

### Prise en charge de la langue maternelle

Banff produit un [journal](/docs/FR/user_guide.md#journal-banff) qui peut afficher des messages en anglais ou en français. Pour plus de détails, consultez la section [*paramétrage de la langue du journal*](/docs/FR/user_guide.md#définir-la-langue-du-journal) du guide de l'utilisateur.

### Option `capture`

Lors de l'exécution dans *Jupyter Notebooks*, certains messages de journal peuvent être manquants. La spécification de `capture=True` dans un appel de procédure peut résoudre le problème. Pour plus de détails, consultez [suppression et dépannage des messages de journal](/docs/FR/user_guide.md#suppression-et-diagnostic-des-messages-du-journal-capture) dans le guide de l'utilisateur.

## Considérations relatives aux performances

Certaines options et formats de tableau peuvent offrir des performances optimales. Pour plus de détails, consultez [Considérations relatives aux performances](/docs/FR/user_guide.md#considérations-sur-la-performance).

## Erreurs et exceptions

Les erreurs sont traitées différemment dans SAS et dans Python, où elles sont appelées *exceptions*. Pour plus de détails, consultez [Erreurs et exceptions](/docs/FR/user_guide.md#erreurs-et-exceptions) dans le guide de l'utilisateur.

## Fonctions utilitaires

### Travailler avec des fichiers SAS en Python

Le paquet banff fournit quelques fonctions utiles pour lire des fichiers SAS en Python. Pour plus de détails, consultez [Travailler avec des fichiers SAS dans Python](/docs/FR/user_guide.md#travailler-avec-des-fichiers-sas-dans-python) dans le guide de l'utilisateur.
