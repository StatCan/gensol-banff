# Aperçu

Banff est un paquet statistique développé par Statistique Canada, et consistant en neuf procédures modulaires qui permettent d'implémenter plusieurs fonctions de vérification statistique des données (VSD), incluant l'imputation. Quelques considérations à propos de Banff:

* La plupart des méthodes de VSD incluses dans Banff sont conçues pour les enquêtes économiques, et en particulier les variables numériques comme le revenu ou le nombre d'employés. À l'heure actuelle, Banff n'offre pas de méthode recommandée pour l'imputation de données catégoriques ou ordinales.
* Banff contient un nombre de méthodes conçues pour les données dont les variables ont des contraintes sous la forme de relations linéaires. Ces relations sont aussi appelées règles de vérification linéaires ou simplement règles de vérification. Ceci concerne les procédures qui vérifient les données par rapport aux règles de vérification, choisissent les variables à imputer lorsque la vérification échoue, et imputent des enregistrements pour s'assurer que toutes les règles de vérification soient satisfaites.
* Bien que chaque procédure Banff s'exécute des manière indépendante, elles suivent toutes un gabarit modulaire, et peuvent être roulées séquentiellement dans le cadre d'un processus plus général de VDS.    
* Banff utilise les codes de statut comme les statuts de sélection et d'imputation pour suivre les métadonnées. Ces codes de statut permettent aux procédures Banff de relayer l'information d'une procédure à l'autre, et sont également consignés dans le journal du processus général de VDS.   

Ce guide de l'utilisateur est destiné à fournir aux utilisateurs de Banff l'information générale à propos du paquet, couvrant les concepts méthodologiques importants et communs à plusieurs procédures, ainsi que les détails techniques. Pour l'information spécifique à une procédure particulière, incluant les descriptions des paramètres, se référer aux [guides des procédure](./Procedure%20Guides/). La liste de toutes les données de sortie générées par les procédures peut être trouvée dans le document des [données de sortie](output_tables.md). Une description complète des méthodes sous-jacentes, avec des exemples et des conseils sur leur application peut être trouvée dans la [description des fonctions](./Description%20des%20fonctions%20Banff.pdf).    

Lorsque les procédures Banff sont exécutées séquentiellement dans le cadre d'un processus de VDS, l'utilisateur est responsable des éléments d'entrée et de sortie entre les étapes. Un paquet supplémentaire, le *Processeur Banff*, est une application basée sur les métadonnées, et conçue spécifiquement pour une production VDS à grande échelle, intégrant les procédures Banff et se chargeant de la gestion des données intermédiaires. 

Le guide de l'utilisateur de Banff utilise souvent la terminologie qui provient du [**Modèle générique de vérification statistique des données** (MGVSD)](https://statswiki.unece.org/spaces/sde/pages/117771706/GSDEM). L'utilisateur est encouragé à se référer au MGVDS pour la terminologie commune relative aux concepts de VDS.

# Table des matières

- [Méthodologie](#méthodologie)
    - [Liste des procédures](#liste-des-procédures)
    - [Interaction entre procédures](#interaction-entre-procédures)
    - [Données statistiques `indata` et `outdata`](#données-statistiques-indata-et-outdata)
    - [Codes de statut et leur données](#codes-de-statut-et-leur-données)
        - [Tableau des codes de statut produits par Banff](#tableau-des-codes-de-statut-produits-par-banff)
        - [Données de statuts `instatus` et `outstatus`](#données-de-statuts-instatus-et-outstatus)
        - [Données de statuts par procédure](#données-de-statuts-par-procédure)
    - [Spécifier les règles de vérification linéaires](#spécifier-les-règles-de-vérification-linéaires)
        - [Format](#format)
        - [Règles de vérification autorisées](#règles-de-vérification-autorisées)
    - [Exécution par groupes de partition](#exécution-par-groupes-de-partition)
- [Guide technique](#guide-technique)
    - [Exécution des procédures Banff](#exécution-des-procédures-banff)
    - [Noms de variables dans les données d'entrée](#noms-de-variables-dans-les-données-dentrée)
    - [Journal Banff](#journal-banff)
        - [Définir la langue du journal](#définir-la-langue-du-journal)
        - [Messages Python du journal](#messages-python-du-journal)
            - [Verbosité du journal Python (`trace=`)](#verbosité-du-journal-python-trace)
        - [Messages du journal issus des procédures C](#messages-du-journal-issus-des-procédures-c)
            - [Verbosité du journal issu de procédure C](#verbosité-du-journal-issu-de-procédure-c)
        - [Suppression et diagnostic des messages du journal (`capture=`)](#suppression-et-diagnostic-des-messages-du-journal-capture)
        - [Utiliser votre propre logger (`logger=`)](#utiliser-votre-propre-logger-logger)
        - [presort](#presort)
        - [prefill_by_vars](#prefill_by_vars)
        - [no_by_stats](#no_by_stats)
    - [Expressions](#expressions)
    - [Spécification des données d'entrée et de sortie](#spécification-des-données-dentrée-et-de-sortie)
        - [Formats pris en charge](#formats-pris-en-charge)
        - [Spécifier les données en entrée](#spécifier-les-données-en-entrée)
        - [Spécifier les données de sortie](#spécifier-les-données-de-sortie)
        - [Personnaliser la spécification par défaut des données de sortie](#personnaliser-la-spécification-par-défaut-des-données-de-sortie)
        - [Accès aux données de sortie](#accès-aux-données-de-sortie)
    - [Autre](#autre)
        - [Caractères d'échappement et chemins d'accès au fichier](#caractères-déchappement-et-chemins-daccès-au-fichier)
        - [Erreurs et exceptions](#erreurs-et-exceptions)
        - [Travailler avec des fichiers SAS dans Python](#travailler-avec-des-fichiers-sas-dans-python)
        - [Considérations sur la performance](#considérations-sur-la-performance)


# Méthodologie

La plupart des fonctions de VDS peuvent être catégorisées en l'un des **types de fonction** définis dans le *MGVDS*:

* **Revue:** Fonctions qui examinent les données pour identifier des problèmes potentiels. 
* **Sélection:** Fonctions qui sélectionnent les unités ou les champs au sein des unités pour un traitement spécifique ultérieur.
* **Traitement:** Fonctions qui changent les données de manière jugée appropriée pour améliorer la qualité des données. La modification des champs spécifiques au sein de l'unité (c'est-à-dire remplir les valeurs manquantes ou modifier les valeurs erronées) est désignée par le terme *imputation*.

Les neuf procédures sont listées dans le tableau ci-dessous, chacune accompagnée d'une brève description et des types de fonction qu'elle effectue. Chaque procédure a un *pseudonyme* dans Banff, utilisé pour exécuter la procédure dans Python; il est aussi utilisé pour faire référence à la procédure au sein du guide de l'utilisateur.

## Liste des procédures

| Nom de la procédure      | Pseudonyme           | Types de fonction            | Description |
| ------------------------ | --------------- | ---------------------------- |--------------- |
| [Spécification et analyse des règles de vérification](/docs/FR/Procedure%20Guides/verifyedits.md) | `verifyedits`   | *Aucune*          | Vérifie la cohérence et la redondance des règles de vérification. |
| [Statistiques sur les règles de vérification](/docs/FR/Procedure%20Guides/editstats.md)    | `editstats`     | Revue             | Produit des statistiques sommaires sur les enregistrements qui satisfont ou ne satisfont pas (à cause d'une valeur manquante ou non manquante) chaque règle de vérification. |
| [Détection des valeurs aberrantes](/docs/FR/Procedure%20Guides/outlier.md)   | `outlier` | Revue, sélection        | Identifie les valeurs aberrantes à l'aide de la méthode Hidiroglou-Berthelot ou la méthode de l'écart-sigma.|
| [Localisation des erreurs](/docs/FR/Procedure%20Guides/errorloc.md)      | `errorloc`      | Revue, sélection            | Sélectionne, pour chaque enregistrement, le nombre minimal de variables à imputer tel que chaque observation puisse passer toutes les règles de vérification. |
| [Imputation déterministe](/docs/FR/Procedure%20Guides/deterministic.md) | `deterministic` | Traitement                    | Effectue l'imputation quand une seule combinaison de valeurs permet à l'enregistrement de passer l'ensemble des règles de vérification. |
| [Imputation par donneur](/docs/FR/Procedure%20Guides/donorimp.md)        | `donorimp`      | Traitement                    | Effectue l'imputation par enregistrement donneur avec la méthode du voisin le plus proche tel que chaque enregistrement imputé satisfasse les règles de vérification post-imputation. |
| [Imputation par estimation](/docs/FR/Procedure%20Guides/estimator.md)     | `estimator`     | Traitement                    | Effectue l'imputation en utilisant des fonctions d'estimation et/ou des estimateurs par régression linéaire. |
| [Ajustement au prorata](/docs/FR/Procedure%20Guides/prorate.md)         | `prorate`       | Revue, sélection, traitement | Ajuste au prorata et arrondie les enregistrements pour satisfaire les règles de vérification spécifiées par l'utilisateur. |
| [Imputation massive](/docs/FR/Procedure%20Guides/massimp.md)    | `massimp`       | Revue, sélection, traitement | Effectue l'imputation par donneur pour un bloc de variables avec l'approche du plus proche voisin ou une sélection aléatoire.  |

Ce guide de l'utilisateur ne contient pas d'information spécifique aux procédures; cette information peut être retrouvée dans le [guide des procédures](./Procedure%20Guides/). Le guide des procédures possède toute l'information nécessaire pour rouler les procédures, incluant la description détaillée de chaque paramètre, et une brève description des méthodes. Pour une description mathématique détaillée des méthodes des procédures, veuillez consulter la [description des fonctions Banff](./Description%20des%20fonctions%20Banff.pdf).

## Interaction entre procédures

Les procédures de Banff sont conçues pour être exécutées de manière séquentielle dans le cadre d'un processus de VDS. Les éléments de sortie d'une procédure servent souvent d'éléments d'entrée pour la procédure suivante, et les données statistiques qui sont la cible du processus de VDS sont mises à jour en permanence et tout au long du processus. Une approche standard utilisant les procédures Banff peut ressembler à ce qui suit :

1. La validité et la cohérence des règles de vérification sont revues et peaufinées à l'aide de
`verifyedits`.
2. Une revue initiale des données statistiques brutes est effectuée par le biais de `editstats`.
3. Les fonctions de revue et de sélection sont réalisées à l'aide de `outlier` et `errorloc` afin d'identifier les problèmes potentiels, et de sélectionner les champs au sein des unités pour un traitement ultérieur.
4. L'imputation est effectuée en utilisant des fonctions de traitement disponibles dans `deterministic`, `donorimp`, et `estimator` pour imputer les valeurs manquantes, les valeurs aberrantes, et corriger les incohérences.
5. L'ajustement au prorata est mené à l'aide de `prorate` pour s'assurer que les valeurs obtenues satisfont les contraintes de totalisation, et pour arrondir les valeurs décimales.
6. Éventuellement,  `massimp` est utilisée pour imputer de grands blocs de non réponse.
7. Une revue finale des données imputées est réalisée avec `editstats` pour s'assurer que les données obtenues à la suite du processus répondent aux normes de qualité souhaitées. 

Pour que ce processus fonctionne, l'information doit être relayée d'une étape à l'autre. les codes de statut générés par `outlier` et `errorloc` sont conservés dans les données de statut et sont lus par les procédures de traitement subséquentes. Lorsque l'une des procédures de traitement réussit à imputer un champ nécessitant l'imputation, le code de sélection dans les données de statut est remplacé par le code d'imputation, indiquant ainsi aux procédures subséquentes que le traitement a déjà été effectué.

La gestion des modifications faites aux données statistiques et aux codes de statut est possible grâce à la modularité des procédures Banff, dont les éléments d'entrée et de sortie sont du même format pour les procédures effectuant les mêmes types de fonction de VDS. Sur les neuf procédures, seulement deux (`prorate` et `massimp`) effectuent les trois types de fonction; ces procédures peuvent être roulées de manière isolée pour revoir les données, sélectionner les enregistrements et/ou les champs pour le traitement et effectuer l'imputation. Les procédures qui font uniquement du traitement (`deterministic`, `donorimp`, et `estimator`) exigent d'abord des codes de sélection générés par l'une des procédures Banff effectuant la sélection (`outlier` ou `errorloc`). La procédure `editstats` examine les données pour détecter d'éventuelles erreurs mais ne produit pas de codes de sélection. Finalement, `verifyedits` ne réalise aucun type de fonction de vérification statistique des données, mais devrait être utilisée avant que le processus de VDS ne commence afin d'examiner et d'améliorer les règles de vérification utilisées par les autres procédures.

L'utilisateur doit se familiariser avec les deux types de données qui constituent un élément central dans la compréhension du fonctionnement de Banff: 
1. Les données statistiques `indata` et `outdata`: Ces éléments d'entrée et de sortie représentent les données statistiques, ou les microdonnées, et sont la cible du processus de VDS. À l'exception de `verifyedits`, toutes les procédures agissent sur les données d'entrée requises `indata`. Les données résultantes de l'imputation sont contenues dans `outdata`, et sont uniquement produites par les procédures qui effectuent les fonctions de traitement (`deterministic`, `donorimp`, `estimator`, `prorate`, et `massimp`).
2. Les données de statuts `instatus` et `outstatus`: Ces éléments d'entrée et de sortie contiennent les codes de statut, qui sont des métadonnées importantes permettant d'identifier les champs qui nécessitent le traitement (statuts de sélection) et ceux qui ont déjà fait l'objet de traitement (statuts d'imputation).

Les données statistiques et celles de statuts sont plus amplement abordées dans les sections suivantes.

## Données statistiques `indata` et `outdata`

À l'exception de `verifyedits`, toutes les procédures Banff s'opèrent sur des données statistiques (aussi appelées microdonnées) arrangées en format tabulaire. L'élément d'entrée principal pour la plupart des procédures est `indata`, qui est la cible du processus de vérification de données. Certaines procédures ont aussi recours à `indata_hist`, qui correspond aux données historiques du même ensemble d'enregistrements. Les procédures qui effectuent des fonctions de traitement produisent les données de sortie `outdata`.

Les données `indata` doivent consister en des données tabulaires arrangées en lignes et en colonnes. La documentation de Banff désigne les lignes comme *enregistrements*, tandis que les colonnes sont appelées *variables*. Dans la plupart des procédures, au moins une variable en caractères doit servir comme identifiant unique spécifié dans le paramètre `unit_id`. Banff utilise cet identifiant pour suivre les métadonnées tout au long du processus de vérification de données. Les noms des variables dans les éléments d'entrée de Banff font l'objet de quelques restrictions; veuillez consulter [cette section](#noms-de-variables-dans-les-données-dentrée) pour plus d'informations.

Les procédures effectuant des fonctions de traitement (`deterministic`, `donorimp`, `estimator`, `prorate` and `massimp`) produisent les données de sortie `outdata`, qui contiennent les résultats de la fonction de traitement. Ces données incluent les valeurs imputées (imputées par exemple par  `donorimp`) et les valeurs modifiées (par exemple ajustées au prorata par `prorate`). Voici quelques remarques importantes à propos de `outdata` :

* Les données `outdata` **ne sont typiquement pas une copie intégrale de `indata`**, mais contiennent uniquement les lignes et les colonnes affectées par la procédure. Par exemple, si `indata` est constitué de 2000 lignes et de 25 colonnes, mais que seulement 500 lignes et 10 colonnes sont affectées par la procédure, alors `outdata` comportera uniquement les 500 lignes et les 10 colonnes. Pour continuer le processus de VDS, l'utilisateur doit manuellement mettre à jour les données `indata` à l'aide de l'information provenant de `outdata`. (*Note: L'équipe de Banff étudie l'ajout de la mise à jour automatique de `indata` à partir des résultats de `outdata`, et ce dans une future version.*)
* Les données `outdata` contiennent toujours les variables spécifiées dans les paramètres `unit_id` et `by`.
* Si aucun enregistrement n'est imputé avec succès ou modifié par la procédure, alors `outdata` sera vide. Aucune erreur ne se produira dans ce cas.

## Codes de statut et leur données

Banff emploie les codes de statut pour conserver l'information importante des métadonnées qui concerne le processus de VDS. Ces codes de statut capturent l'information contenue dans les statuts de sélection, les statuts de l'exclusion et ceux de l'imputation. Cette information est utilisée de deux manières :

* Comme élément d'entrée des étapes subséquentes du processus de vérification de données. Par exemple, la procédure de localisation des erreurs `errorloc` produit des statuts de sélection pour identifier les variables qui nécessitent l'imputation. Ces statuts de sélection sont lus par les procédures d'imputation comme `donorimp` et `estimator` afin d'effectuer l'imputation.
* Comme une liste complète des enregistrements affectés à la suite d'un processus de vérification de données. Par exemple, l'historique des codes de statut pour une observation particulière peut expliquer *pourquoi*, *comment*, et *quand* elle a fait l'objet de modifications.

### Tableau des codes de statut produits par Banff

| Code de statut | Description |
| -------------- | ----------- |
| `FTI`          | *Champ à imputer :* Statut de sélection indiquant qu'une observation nécessite un traitement supplémentaire comme l'imputation. Généré par les procédures `outlier` et `errorloc`, ou par une procédure définie par l'utilisateur. |
| `FTE`          | *Champ à exclure :* Statut de sélection indiquant qu'une observation devrait être exclue de certaines méthodes. Généré par la procédure `outlier` ou par une procédure définie par l'utilisateur. |
| `IDE`          | *Imputé par l'imputation déterministe :* Champ imputé par la procédure `deterministic` et qui devrait être considéré comme une valeur rapportée (non imputée) dans le cadre des méthodes subséquentes. (Aussi appelée imputation déductive.) |
| `IDN`          | *Imputé par l'imputation par donneur :* Champ imputé par la procédure `donorimp`. |
| `I--`          | *Imputé par l'imputation par estimation :* Champ imputé par la procédure `estimator`. Le code exact dépend de l'algorithme spécifié; par exemple un statut ICR indique que l'algorithme `CURRATIO` a été utilisé, tandis qu'un statut IDT correspond à l'algorithme `DIFTREND`. | 
| `IPR`          | *Imputé par l'ajustement au prorata :* Champ imputé par la procédure `prorate`. |
| `IMAS`         | *Imputé par l'imputation massive :* Champ imputé par la procédure `massimp`. |

### Données de statuts `instatus` et `outstatus`

Les statuts de sélection et d'imputation sont toujours associés aux valeurs individuelles dans `indata`. Puisque `indata` a un format tabulaire, chaque observation est associée à un enregistrement (ligne) et à un champ (colonne) spécifiques. Les enregistrements sont identifiés par l'identifiant unique de l'enregistrement `unit_id` spécifié par l'utilisateur, tandis que les champs sont désignés par leur nom de variable. Les codes de statut sont conservés dans les données de statuts dont les colonnes sont comme suit : 

| Colonne                   | Description |
| ------------------------- | ----------- |
| <nobr>\<unit-id\></nobr>  | Identifiant de l'enregistrement (c'est-à-dire la ligne) pour lequel le code de statut s'applique. (Le véritable nom de colonne est le nom de la variable spécifié dans le paramètre `unit_id`.) |
| `FIELDID`                   | Identifiant du champ (c'est-à-dire la colonne) pour lequel le code de statut s'applique.   |
| `STATUS`                    | Codes de statut tels que "FTI", "FTE", ou "IDN".        |
| `VALUE`                     | Valeur de la variable quand le code de statut est généré. Pour les procédures effectuant la sélection (`errorloc` et `outlier`), cette colonne correspond à la valeur de l'observation dans `indata` lorsque la sélection a eu lieu. Pour les procédures effectuant le traitement (`deterministic`, `donorimp`, `estimator`, `prorate` et `massimp`), cette colonne représente la valeur de l'observation dans `outdata`, après que le traitement a eu lieu. |

Toutes les procédures effectuant des fonctions de sélection ou de traitement (c'est-à-dire toutes les procédures Banff à l'exception de `verifyedits` et `editstats`) produisent automatiquement des données des statuts de sortie intitulées `outstatus` et contenant les statuts de sélection ou d'imputation. Certaines procédures utilisent les données de statuts (`instatus`) comme éléments d'entrée; selon la procédure en question, ces données peuvent être requises. Un bref résumé du comportement de chaque procédure par rapport aux données de statuts est présenté dans le tableau ci-dessous.

### Données de statuts par procédure

Le tableau suivant résume les codes de statut lus à partir de `instatus` ou produits dans `outstatus`, et ce pour chaque procédure. Lorsqu'un code de statut est requis par une procédure, alors `instatus` l'est également. 

| Procédure  | Code de statut lu à partir de `instatus` | Code de statut produit dans `outstatus` |
| --------------- | ---------------------------------------------- | ---------------------------- |
| `verifyedits`   | N/A                                            | N/A      |
| `editstats`     | N/A                                            | N/A      |
| `outlier`       | N/A                                            | FTI, FTE |
| `errorloc`      | FTI (optionnel)                                | FTI      |
| `deterministic` | FTI (requis)                                   | IDE      |
| `donorimp`      | FTI (requis), FTE (optionnel), I-- (optionnel) | IDN      |
| `estimator`     | FTI (requis), FTE (optionnel), I-- (optionnel) | I-- (le code exact dépend de l'algorithme spécifié) |
| `prorate`       | I-- (optionnel)                                | IPR      |
| `massimp`       | N/A                                            | IMAS     |

Certains codes de statut sont optionnels, mais peuvent changer le comportement de la procédure. Dans le cadre de `prorate` par exemple, l'utilisateur peut choisir d'ajuster au prorata toutes les valeurs, seulement les valeurs originales ou seulement les valeurs déjà imputées. Si son choix se porte sur l'imputation des valeurs originales ou celles déjà imputées, alors des données de statuts d'entrée `instatus` avec des statuts I-- sont requises. 

## Spécifier les règles de vérification linéaires

Dans le processus de vérification statistique des données, le terme *règle de vérification* désigne généralement les contraintes que les enregistrements doivent satisfaire pour être considérés comme valables. Les règles linéaires correspondent aux contraintes qui peuvent être exprimées comme des égalités ou des inégalités linéaires de la forme :

$$
a_1 x_1 + a_2 x_2 + ... + a_n x_n = b  \\ ou \\
a_1 x_1 + a_2 x_2 + ... + a_n x_n \le b
$$



où les termes $x$<sub>$1$</sub> à $x$<sub>$n$</sub> sont des variables numériques des données cibles, les termes $a$<sub>$1$</sub> à $a$<sub>$n$</sub> et $b$ sont des constantes spécifiées par l'utilisateur. Sur les neuf procédures Banff, six d'entre elles requièrent les règles de vérification comme paramètre d'entrée :

* `verifyedits`;
* `editstats`;
* `errorloc`;
* `deterministic`;
* `donorimp`;
* `prorate`. (Note: les règles de vérification de cette procédure se caractérisent par une syntaxe unique et des restrictions supplémentaires.)

### Format

Le paramètre `edits` est utilisé pour spécifier la liste des règles de vérification selon la syntaxe suivante:

* Comme pour tous les paramètres en chaîne de caractères, la liste des règles de vérification doit être entourée de guillemets `"` ou de triple guillemets `"""` pour des règles en plusieurs lignes.
* Chaque règle individuelle doit être suivie d'un point-virgule `;`.
* Les règles individuelles doivent inclure l'un des *opérateurs* suivants : 
    * `<` (inférieur à);
    * `<=` (inférieur ou égal à);
    * `=` (égal à);
    * `!=` (n'est pas égal à);
    * `>=` (supérieur ou égal à);
    * `>` (supérieur à).
* À l'intérieur de chaque règle individuelle, une ou plusieurs *composantes* doit apparaître de chaque côté d'un opérateur, séparée par `+` ou `-`. Une composante peut être une `constante`, une `variable` (qu'on retrouve dans `indata`) ou le produit des deux:  `constante * variable` ou `variable * constante`. Lorsque l'on multiplie une constante par une variable, elles doivent être séparées par un astérisque `*`.
* Optionnellement, l'utilisateur peut ajouter un modificateur (`modifier`) au début de chaque règle individuelle, suivi d'un deux-point `:`. Les modificateurs acceptables sont `pass` et `fail` et ne sont pas sensibles à l'utilisation de lettres minuscules ou majuscules.

Un exemple simple de trois règles de vérification spécifiées dans une ligne est le suivant :


```python
    appel_errorloc  = banff.errorloc(
        edits= "Profit = Revenu - Depenses; Profit >= 0; Depenses >= 0;"
        ... # etc. (paramètres, données de sortie)
        )
```

Dans l'exemple ci-dessous, les règles de vérification s'étendent sur trois lignes et sont entourées de triple guillemets `"""`. Ces règles comportent également des constantes, ainsi que les modificateurs `pass` et `fail`.

```python
    appel_errorloc  = banff.errorloc(
        edits= """
            Profit = Revenu - Depenses; Profit >= 0; Depenses >= 0;
            0.9 * Total <= Var1 + Var2 + Var3;
            Var1 + Var2 + Var3 <= 1.1 * Total;
            Pass: Var4 >= Var5;
            Fail: Var4 > Var5;
            Fail: Nb_Employes != BR_Employes
            """
        ... # etc. (paramètres, données de sortie)
        )
```

### Règles de vérification autorisées

Bien que l'utilisateur puisse exprimer les règles de vérification de plusieurs manière, les procédures Banff commencent par les convertir en une *forme canonique* avant de procéder à la suite; sous cette forme, chaque règle de vérification est exprimée comme une règle avec un modificateur *pass* et un opérateur `=` ou `<=`. Les inégalités strictes ne sont pas permises, ce qui se traduit par les règles ou les modifications suivantes :

* Règle de vérification (modificateur *pass*) avec `<` ou `>` est remplacé par `<=` et `>=` respectivement. 
* Règle de vérification (modificateur *fail*) avec `<=` ou `>=` est remplacé par `<` et `>` respectivement.
* Règle de vérification (modificateur *pass*) avec `!=` ne peut pas être convertie en une forme canonique et produit une erreur.
* Règle de vérification (modificateur *fail*) avec `=` ne peut pas être convertie en une forme canonique et produit une erreur.

(*Note: L'utilisateur qui souhaite spécifier une inégalité stricte devrait plutôt inclure une petite constante dans leur règle de vérification, c'est-à-dire que `A < B` peut être remplacée par `A <= B - 0.005` pour les valeurs qui sont rapportées avec une précision de deux décimales.*)

Le tableau suivant donne des exemples de règles de vérification originales valables et non valables, accompagnées de leur forme canonique lorsque possible:

| Règle de vérification originale    | Forme canonique |
| ---------------------------------- | --------------- |
| `Pass: A > B + 3` | `-A + B <= -3` |
| `Pass: C = D`     | `C - D = 0`    |
| `Pass: Z < A`     | `-A + Z <= 0`  |
| `Pass: M != N`    | `INVALIDE`     |
| `Fail: A > B + 3` | `A - B <= 3`   |
| `Fail: C = D`     | `INVALIDE`     |
| `Fail: Z <= A`    | `A - Z <= 0`   |
| `Fail: N != M`    | `-M + N = 0`   |

De plus, l'ensemble des règles de vérification spécifié par l'utilisateur doit être cohérent; autrement dit, les règles de vérification linéaires doivent former un région d'acceptation non vide. Spécifier un ensemble incohérent de règles de vérification produira une erreur. Il est recommandé d'avoir recours à `verifyedits` pour examiner la cohérence, la redondance et les variables déterministes ainsi que les égalités cachées des règles de vérification, et ce avant de les utiliser dans une autre procédure.

## Exécution par groupes de partition

Le paramètre `by` qu'on retrouve dans toutes les procédures Banff à l'exception de `verifyedits` permet à l'utilisateur de partitionner les données statistiques d'entrée en plusieurs groupes qui sont traités de manière indépendante. Dans la documentation de Banff, on désigne ces groupes par la dénomination *groupes de partition*. L'utilisateur peut spécifier une ou plusieurs variables dans `indata` qui seront utilisées pour former les groupes de partition. Par exemple, `by = "Province"` créera un groupe de partition pour chaque valeur unique de la variable `Province` dans `indata`, tandis que `by = "Province Industrie"` créera un groupe de partition pour chaque combinaison des variables `Province` et `Industrie`.

Notes sur les groupes de partition:
* Les procédures `errorloc`, `deterministic`, et `prorate` s'opèrent sur chaque enregistrement de `indata` de manière indépendante; utiliser les groupes de partition pour ces procédures n'a aucune incidence sur les données de sortie. Néanmoins, le paramètre `by` est encore possible d'être spécifié dans ces procédures, et pourrait être utilisé dans le futur pour améliorer la performance.
* Lorsque `by` et `indata_hist` sont tous deux spécifiés dans une procédure, alors les données `indata_hist` doivent aussi contenir les variables `by`.
* Les données d'entrée telles que `indata` et `indata_hist` doivent être triées selon les variables `by`, et ce avant l'exécution de la procédure. Par défaut, les procédure Banff effectuent ce tri automatiquement; ce tri peut être désactivé (par exemple, si les données d'entrée sont déjà triées) en spécifiant `presort=False`.
* Lorsque `instatus` est spécifié, certaines procédures sont plus efficaces si les variables `by` sont incluses dans `instatus`. Par défaut, Banff y ajoutera les variables `by` automatiquement; cet ajout peut être désactivé (par exemple, si les variables sont déjà présentes) en spécifiant `prefill_by_vars=False`.
* Par défaut, plusieurs procédures ajouteront les diagnostics de chaque groupe de partition au journal; ceci peut être désactivé en spécifiant `no_by_stats=True`.
* Les données de sotie `outdata` contiendront toujours les variables `by`.

# Guide technique

## Exécution des procédures Banff

Pour exécuter une procédure Banff dans un script Python, le paquet Banff doit d'abord être importé, accompagné de tout autre paquet qu'on prévoit utiliser :

```python
import banff
import pandas as pandas
```

On crée ensuite un nouvel objet, auquel on associe un nom de notre choix (dans ce cas, "mon_errorloc"), à partir de l'une des procédures Banff et à l'aide de son [pseudonyme](#liste-des-procédures) :

```python
mon_errorloc = banff.errorloc(
    indata=exemple_indata,
    outstatus=True,
    outreject=True,
    edits="x1>=-5; x1<=15; x2>=30; x1+x2<=50;",
    weights="x1=1.5;",
    cardinality=2,
    time_per_obs=0.1,
    unit_id="IDENT",
    by="ZONE"
)
```

* Les procédures doivent être appelées en utilisant le nom du paquet Banff; c'est-à-dire, `banff.errorloc()`.  
* Les paramètres (par exemple, `edits`) et les données (par exemple, `indata`) sont spécifiés comme des paires clé-valeur séparés par des virgules, leur ordre d'apparition n'étant pas important (`edits` avant `indata` ou l'inverse).

Les éléments de sortie peuvent être conservés en mémoire ou sauvegardés sur disque (consulter [cette section](#spécifier-les-données-de-sortie) pour plus de détails). Lorsque conservés en mémoire, ils seront conservés dans l'objet spécifié par l'utilisateur (par exemple, `mon_errorloc`) et créé lors de l'exécution de la procédure :

```python
print(mon_errorloc.outstatus) # Imprime les statuts de sortie outstatus
errorloc_outstatus = mon_errorloc.outstatus # Sauvegarde outstatus comme de nouvelles données PyArrow appelées "errorloc_outstatus"
```

## Noms de variables dans les données d'entrée

Dans les procédures Banff, plusieurs paramètres font référence aux variables (c'est-à-dire les colonnes) contenues dans les données d'entrée comme `indata`. Toute variable mentionnée dans l'appel de la procédure Banff doit consister en une seule chaîne de caractères sans espacements ou caractères spéciaux à l'exception du sous-tiret (`_`). Par exemple, `"nom_de_famille"` est un nom valable de variable, tandis que `"nom de famille"`, `"nom-de-famille$"` ou `"prénom"` ne le sont pas. Pour s'assurer que les données d'entrée sont compatibles avec Banff, l'utilisateur pourrait avoir besoin de modifier les noms de variables. Quelques contraintes supplémentaires:

* Les noms de variables ne peuvent pas excéder 64 caractères en longueur.
* Les noms de variables doivent être uniques à l'intérieur de chaque élément de données d'entrée. (La même variable peut apparaître dans de différents données d'entrée sans problème.)

Des exemples de paramètres Banff qui font référence aux variables sont `unit_id`, `var`, `weights`, et `by`. Le nom de variable est simplement utilisée, et lorsqu'il s'agit d'une liste de deux variables ou plus, il suffit de les séparer d'un seul espacement. Par exemple:

* Une seule variable: `unit_id = "numero_entreprise"`
* Une liste de variables: `by = "province industrie"`

Noter que les noms de variables ne sont pas sensibles à la casse; lorsque des données d'entrée possèdent deux colonnes ou plus qui diffèrent seulement en matière de casse, il n'est pas clair laquelle des colonnes sera utilisée durant le traitement. (Les utilisateurs sont fortement encouragés à utiliser des noms distincts et non sensibles à la casse pour toute colonne faisant partie des données d'entrée.)

## Journal Banff

Les messages du journal sont inscris dans le terminal lors de l'exécution. Ces messages proviennent de deux sources: le code Python et le code C de la procédure. Les messages peuvent être affichés en français ou en anglais.

### Définir la langue du journal

Le paquet `banff` produit des messages du journal en français ou en anglais. Le paquet tente de détecter la langue de l'environnement hôte lors de l'importation. Utiliser la fonction `banff.set_language()` pour définir la langue au moment de l'exécution, en spécifiant l'une des deux possibilité de `banff.SupportedLanguage` (c'est-à-dire `.fr` ou `.en`).

> **Exemple**: définir le français comme langue
>
> ```python
> banff.set_language(banff.SupportedLanguage.fr)
> ```

### Messages Python du journal

Python gère le journal à l'aide du paquet standard [`logging`](https://docs.python.org/3/library/logging.html#). Tous les messages Python du journal sont associés à un [*niveau du journal*](https://docs.python.org/3/library/logging.html#logging-levels), tel que *ERROR*, *WARNING*, *INFO*, et *DEBUG*. Les messages issus de Python consistent généralement en une ligne par message, et sont horodatés et préfixés par leur niveau. 

Par défaut, seuls les messages d'avertissement et d'erreur sont affichés. Utiliser le paramètre `trace` pour changer le niveau du journal affiché. 

> **Exemple de messages Python**  
> Dans ce qui suit, 3 messages non liés sont issus des niveaux *INFO*, *DEBUG*, et *ERROR*. Par défaut, seul le troisième message est imprimé au terminal.  
>
> ```plaintext
> 2024-11-29 10:47:51,853 [INFO]:  Time zone for log entries: Eastern Standard Time (UTC-5.0)
> 2024-11-29 10:48:47,654 [DEBUG   , banff.donorimp._execute._preprocess_inputs]:  Adding BY variables to 'instatus' dataset
> 2024-11-29 10:49:59,867 [ERROR]:  Procedure 'Donor Imputation' encountered an error and terminated early: missing mandatory dataset (return code 4)
> ```

#### Verbosité du journal Python (`trace=`)

Le paramètre `trace` permet de contrôler le niveau du journal à imprimer, grâce à la spécification de l'un des niveaux suivants :

- `banff.log_level.ERROR`
- `banff.log_level.WARNING`
- `banff.log_level.INFO`
- `banff.log_level.DEBUG`

Les messages relatifs au niveau spécifié et aux niveaux supérieurs seront imprimés, ce qui ne sera pas le cas des niveaux inférieurs.

Par souci de commodité, spécifier `trace=True` permet de garder toutes les sorties du journal.

### Messages du journal issus des procédures C

Les messages issus de la procédure C n'ont pas de niveau de journal associé et ne sont pas horodatés. Certains messages sont préfixés par `ERROR:`, `WARNING:` et `NOTE:`, alors que d'autres messages n'ont pas de préfixe et peuvent s'étendre sur plusieurs lignes. La plupart des messages sont informatifs. Dès qu'un message avec `ERROR:` est imprimé, il sera accompagné d'un niveau d'erreur lié issu de Python, et une exception sera soulevée.

> **Exemple des messages issus de procédure C**  
>
> ```plaintext
> NOTE: --- Banff System 3.01.001b19.dev6 developed by Statistics Canada ---
> NOTE: PROCEDURE DONORIMPUTATION Version 3.01.001b19.dev6
> . . .
> NOTE: The minimal set of edits is equivalent to the original set of edits specified.
> . . .
> Number of valid observations ......................:      12     100.00%
> Number of donors ..................................:       5      41.67%
> Number of donors to reach DONORLIMIT ..............:       3      60.00%
> Number of recipients ..............................:       7      58.33%
> 
> NOTE: The above message was for the following by group:
>       prov=30
> ```

#### Verbosité du journal issu de procédure C

Voir [no_by_stats](#no_by_stats).  

### Suppression et diagnostic des messages du journal (`capture=`)

Le paramètre `capture` peut être utilisé pour supprimer toutes les sorties du journal (en spécifiant `capture=None`). Cette option est désactivée par défaut (`capture=False`).

Spécifier `capture=True` fera en sorte que les messages du journal soient imprimés en une seule fois à la fin de la procédure, au lieu de les imprimer au fur et à mesure de l'exécution. La différence peut uniquement être constatée lors des appels de procédure qui ont une longue durée d'exécution. Utiliser cette option peut améliorer la performance dans certains cas, notamment lorsque l'on traite un très grand nombre de groupes de partition sans spécifier `no_by_stats=True`. 

> *Jupyter Notebooks et les messages du journal manquants*  
> Lorsque *Jupyter Notebooks* est utilisé pour rouler une procédure, les messages du journal issus de procédure C peuvent être manquants, notamment quand l'exécution se fait à partir de Visual Studio Code dans Windows.  
> Utiliser l'option `capture=True` permet de remédier à cela.  
>
> * Autrement [utiliser votre propre *logger*](#utiliser-votre-propre-logger-logger)

À cause de la manière dont *Jupyter Notebooks* gère les sorties Python du terminal, les messages issus de procédure C peuvent ne pas s'afficher. Pour remédier à cela, spécifier `capture=True` dans l'appel de la procédure lorsque Jupyter notebook est utilisé pour exécuter la procédure.

### Utiliser votre propre *logger* (`logger=`)

Utiliser le paramètre `logger` pour spécifier un [*logger*](https://docs.python.org/3/library/logging.html#logger-objects) qui a été créé. Tous les messages issus de Python et de C seront envoyés vers ce *logger*. Ceci permettra la personnalisation des préfixes des messages, la prise en charge de l'écriture dans un fichier, etc.

Noter que les messages de procédure C sont envoyés au *logger* dans une seul niveau de message Python du journal.  

> ***Exemple***: écriture du journal dans un fichier
>
> ```python
> import banff
> import logging
> mon_logger = logging.getLogger(__name__)
> logging.basicConfig(filename='example.log', encoding='utf-8', level=logging.DEBUG)
> 
> # rouler la procédure Banff 
> appel_banff = banff.donorimp(
>     logger=mon_logger,
>     indata=indata,
>     instatus=donorstat,
>     outmatching_fields=True,
> ...
> ```

### presort

Lorsque des variables `by` sont spécifiées, les procédures Banff requièrent des données d'entrée triées selon ces variables, et ce avant l'exécution. Certaines procédures requièrent aussi le tri selon `unit_id`. Spécifier `presort = True` permettra de trier automatiquement toutes les données d'entrée (par exemple `indata`, `indata_hist`, et `instatus`) selon les exigences de la procédure. Toutes les procédures Banff ont l'option `presort = True` par défaut. L'utilisateur peut désactiver cette option en spécifiant `presort = False`.

### prefill_by_vars

Spécifier `prefill_by_vars = True` permettra d'ajouter automatiquement les variables `by` aux données d'entrée au besoin, et ce avant de rouler la procédure Banff. Dans certains cas, la présence de variables `by` dans les données d'entrée peut améliorer significativement la performance de la procédure. Par défaut, `prefill_by_vars = True` est activée pour toutes les procédure Banff. L'utilisateur peut désactiver cette option en spécifiant `prefill_by_vars = False`.

### no_by_stats

La plupart des procédures Banff envoient de l'information au journal de Banff. Lorsque les groupes de partition sont spécifiés à l'aide du paramètre `by`, cette information est typiquement produite pour chaque groupe de partition. Spécifier `no_by_stats = True` permet de réduire l'information envoyée au journal en supprimant les messages relatifs aux groupes de partition. Ce paramètre est disponible pour la plupart des procédures qui permettent l'utilisation du paramètre `by`, bien que certaines procédures n'aient que peu de messages spécifiques aux groupes de partition.

## Expressions

Les paramètres `exclude_where_indata` et `exclude_where_indata_hist` appliquent une logique booléenne sur les données d'entrée en utilisant des expressions SQL. La prise en charge des expressions SQL est mise en oeuvre à l'aide de DuckDB. Veuillez vous référer à la [documentation sur les expressions](https://duckdb.org/docs/sql/expressions/overview) pour un guide complet de la syntaxe supportée.

## Spécification des données d'entrée et de sortie

Pour les données d'entrée et de sortie, l'utilisateur peut spécifier des objets en mémoire ou des fichiers sur disque. Pour les deux types de données, plusieurs formats sont pris en charge. Les objets sont associés à des identifiants (par exemple `"pandas dataframe"`), tandis que les fichiers sont associés à des extensions (par exemple `"nom_fichier.parquet"`); Veuillez vous référer au tableau ci-dessous pour plus de détails. Noter que certains formats sont recommandés à des fins de test uniquement, et que certains formats ne sont pas pris en charge pour les données de sortie.

### Formats pris en charge

| Format                | Type    | Identifiant ou extemsion pris en charge    | Notes           |
| --------------------- | ------- | ------------------------------------------ | ---------- |
| Table PyArrow         | Objet   | `"pyarrow"`, `"table"`, `"pyarrow table"`  | Format recommandé pour les objets en mémoire.                 | 
| Structure de données Pandas      | Objet   | `"pandas"`, `"dataframe"`, `"pandas dataframe"` |                                                           |
| Apache Parquet        | Fichier | `.parquet`, `.parq`                             | Utilisation minimale de la mémoire vive, bonne performance pour les données de grande taille.    |
| Apache Feather       | Fichier | `.feather`                                      | Utilisation minimale de la mémoire vive, bonne performance pour les données de grande taille.      |
| Ensemble de données SAS | Fichier | `.sas7bdat`                                     | Données d'entrée seulement, et à fins de test uniquement; n'est pas recommandé en production. |
| Valeur séparée par virgule | Fichier | `.csv`                                          | Pour des fins de test uniquement; n'est pas recommandé en production.  |

Pour des conseils sur les chemins d'accès aux fichiers en Python, consulter [caractère d'échappement et chemins d'accès au fichier](#caractères-déchappement-et-chemins-daccès-au-fichier)

### Spécifier les données en entrée

Pour utiliser un objet en mémoire comme élément d'entrée, simplement spécifier le nom de l'objet directement dans l'appel de la procédure. La procédure détectera automatiquement le type de l'objet parmi les types pris en charge.

```python
    appel_donorimp = banff.donorimp(
        indata=df, # où df est une structure de données Pandas précédemment généré
        instatus=table, # où table est une Table PyArrow précédemment générée
        ... # etc. (paramètres, données de sortie)
        )
```
Pour spécidier des données d'entrée à partir d'un fichier, utiliser un chemin d'accès relatif ou complet :

```python
    appel_donorimp  = banff.donorimp(
        indata="./input_data.parquet", # Fichier Parquet avec référence locale
        instatus=r"C:\temp\input_status.feather", # Fichier Feather avec référence Windows
        ... # etc. (paramètres, données de sortie)
        )
```

L'utilisateur peut aussi mélanger les deux types de données d'entrée :

```python
    appel_donorimp  = banff.donorimp(
        indata="./input_data.parquet", # Fichier Parquet avec référence locale
        instatus=table, # où table est une Table PyArrow précédemment générée
        ... # etc. (paramètres, données de sortie)
        )
```

### Spécifier les données de sortie

Les procédures Banff créent automatiquement un certain nombre de données de sortie. Certaines de ces données sont optionnelles, dans le sens où elles peuvent être désactivées en spécifiant `False`. (Spécifier `False` pour des données de sortie optionnelles les empêchera d'être produites, ce qui pourrait améliorer l'utilisation de la mémoire. Spécifier `False` pour des données de sortie obligatoires se traduira par une erreur.) Le format par défaut des données de sortie est la Table PyArrow en mémoire. Pour produire des données de sortie en mémoire dans un autre format, spécifier leur _identifiant_ associé comme chaîne de caractère. Pour écrire des données de sortie dans un fichier, spécifier le chemin d'accès au fichier ainsi que son _extension_ prise en charge.

Consulter le [Tableau des formats pris en charge](#formats-pris-en-charge) pour une liste des identifiants et des extensions. Veuillez vous référer au [document des données sortie](output_tables.md) pour une liste complète des données de sortie par procédure.

Les exemples suivants contiennent les données de sortie optionnelles et obligatoires, sauvegardées dans un mélange d'objets en mémoire et de fichiers.

```python
    appel_estimator  = banff.estimator(
        outdata=True, # Sauvegardées comme une Table PyArrow 
        outstatus=True, # Sauvegardées comme une Table PyArrow
        outacceptable=True, # Optionnelles sauvegardées comme une Table PyArrow
        outest_ef="pyarrow", # Optionnelles sauvegardées comme une Table PyArrow
        outest_lr="dataframe", # Optionnelles sauvegardées comme une structure de données Pandas 
        outest_parm=False, # Optionnelles désactivées
        outrand_err="./output_data.parquet", # Optionnelles sauvegardées comme un fichier parquet        
        ... # etc. (paramètres, données de sortie)
        )
```

*Note : puisque toutes les données sont activées par défaut, et que le format de sortie par défaut est la Table PyArrow, l'appel ci-dessous produira des résultats identiques à l'appel précédent :*

```python
    appel_estimator  = banff.estimator(
        outest_lr="dataframe", # Optionnelles sauvegardées comme une structure de données Pandas
        outest_parm=False, # Optionnelles désactivées
        outrand_err="./output_data.parquet", # Optionnelles sauvegardées comme un fichier parquet       
        ... # etc. (paramètres, données de sortie)
        )
```

**NOTE: Les données de sortie vont automatiquement écraser les objets et fichiers existants et portant le même nom.** 

### Personnaliser la spécification par défaut des données de sortie

Pour déterminer le format actuel par défaut des données de sortie

```python
>>> banff.get_default_output_spec()
'pyarrow'
```

- ceci correspond à `pyarrow.Table`

Le format par défaut peut être choisi comme un *identifiant* parmi ceux dans le [tableau des formats pris en charge](#formats-pris-en-charge)

> **Exemple** : Changer le format par défaut des données de sortie au `pandas.DataFrame`
>
> ```python
> banff.set_default_output_spec('pandas')
> ```

### Accès aux données de sortie

L'utilisateur accède aux objets sauvegardés en mémoire en utilisant le nom des données de sortie d'intérêt :  

```python
    appel_estimator  = banff.estimator(
        outdata=True, # Sauvegardées comme une Table PyArrow
        outstatus=True, # Sauvegardées comme une Table PyArrow
        ... # etc.
        )
    print(appel_estimator.outdata) # Imprime outdata dans le terminal
    ma_table = estimator_call.outstatus # Sauvegarde outstatus comme un nouvel objet intitulé ma_table
```

*Note : puisque `outdata` et `outstatus` sont des données de sortie obligatoires, elles sont encore disponibles par le biais de `estimator_call.outdata` et `estimator_call.outstatus`, même si l'on ne les spécifie pas à l'aide de la déclaration `True`.*  

## Autre

### Caractères d'échappement et chemins d'accès au fichier

Dans Windows, la barre oblique inversée (`\`) est typiquement utilisée pour séparer les dossiers et les fichiers dans un chemin d'accès au fichier.

- Exemple `"C:\users\stc_user\documents\donnees.csv"`

Cependant, le caractère `\` en Python est un **caractère d'échappement** et est traité particulièrement. Fournir un chemin d'accès au fichier à l'aide de l'exemple ci-dessus peut provoquer une erreur d'exécution. Pour désactiver ce traitement spécial, utiliser une "*chaîne de caractère brute*" en ajoutant `r` comme préfixe: 

- `r"C:\users\stc_user\documents\donnees.csv"`

Autres possibilités,

- double barres obliques inversées: `C:\\users\\stc_user\\documents\\donnees.csv`
- barre oblique: `C:/users/stc_user/documents/donnees.csv`

### Erreurs et exceptions

De manière générale, Python gère les erreurs d'exécution en "soulevant une exception". Ceci a été adopté par le paquet `banff`. Chaque fois qu'une erreur se produit, une exception est soulevée. Ceci peut se produire pendant le chargement du paquet, la prétraitement des données d'entrée, l'exécution d'une procédure ou l'écriture des données de sortie.

Généralement, les exceptions contiendront un message d'erreur informatif. Les exceptions sont souvent "enchaînées" pour fournir un contexte supplémentaire à l'exception.

> ***Exemple** : Exception lors de l'écriture des données de sortie*
>
> Ce qui suit montre la sortie de console générée lorsque le paquet ne parvient pas à écrire les données de sortie à cause d'un dossier de destination introuvable.  
>
> ```plaintext
> [ERROR   , banff.donorimp._execute._write_outputs]:  Error occurred while writing 'outmatching_fields' output dataset
> [ERROR   , banff.donorimp._execute._write_outputs]:  Directory of output file does not exist: 'C:\temp\definitely\a\fake'
> [ERROR   , banff.donorimp._execute._write_outputs]:  [WinError 3] The system cannot find the path specified: 'C:\\temp\\definitely\\a\\fake'
> Traceback (most recent call last):
>   File "C:\git\banff_redesign\Python\src\banff\_common\src\io_util\io_util.py", line 578, in write_output_dataset
>     dst.parent.resolve(strict=True)  # strict: exception if not exists
>   File "C:\Program Files\Python310\lib\pathlib.py", line 1077, in resolve
>     s = self._accessor.realpath(self, strict=strict)
>   File "C:\Program Files\Python310\lib\ntpath.py", line 689, in realpath
>     path = _getfinalpathname(path)
> FileNotFoundError: [WinError 3] The system cannot find the path specified: 'C:\\temp\\definitely\\a\\fake'
> 
> The above exception was the direct cause of the following exception:
> 
> Traceback (most recent call last):
>   File "C:\git\banff_redesign\Python\src\banff\_common\src\proc\stc_proc.py", line 649, in _write_outputs
>     ds.user_output = io.write_output_dataset(ds.ds_intermediate, ds.user_spec, log_lcl)
>   File "C:\git\banff_redesign\Python\src\banff\_common\src\io_util\io_util.py", line 581, in write_output_dataset
>     raise FileNotFoundError(mesg) from e
> FileNotFoundError: Directory of output file does not exist: 'C:\temp\definitely\a\fake'
> 
> The above exception was the direct cause of the following exception:
> 
> Traceback (most recent call last):
>   File "C:\git\banff_redesign\Python\sample_programs\conversion_examples\DonorImp01.py", line 61, in <module>
>     banff_call = banff.donorimp(
>   File "C:\git\banff_redesign\Python\src\banff\proc\proc_donorimp.py", line 119, in __init__
>     super().__init__(
>   File "C:\git\banff_redesign\Python\src\banff\proc\banff_proc.py", line 66, in __init__
>     self._execute()
>   File "C:\git\banff_redesign\Python\src\banff\_common\src\proc\stc_proc.py", line 367, in _execute
>     self._write_outputs(log=log_lcl)
>   File "C:\git\banff_redesign\Python\src\banff\_common\src\proc\stc_proc.py", line 654, in _write_outputs
>     raise ProcedureOutputError(mesg) from e
> banff._common.src.exceptions.ProcedureOutputError: Error occurred while writing 'outmatching_fields' output dataset
> ```
>
> Les trois premières lignes sont les messages du journal générés par le paquet `banff`. Le reste des lignes sont une exception standard générée par Python lui-même. De haut en bas, une chaîne de trois exceptions apparaît.  
> La première est un niveau inférieur d'erreur indiquant que le chemin d'accès au fichier est introuvable, "*`[WinError 3] The system cannot find the path specified: 'C:\\temp\\definitely\\a\\fake'`*".  
> La deuxième indique spécifiquement que "*`Directory of output file does not exist: 'C:\temp\definitely\a\fake'`*".  
> La troisième fournit du contexte par rapport à ce qui se passait lorsque l'erreur s'est produite, "*`Error occurred while writing 'outmatching_fields' output dataset`*".  

### Travailler avec des fichiers SAS dans Python

Le paquet `banff` fournit quelques fonctions utiles pour la lecture de fichiers SAS en mémoire ou leur conversion à un autre format.

Pour pouvoir utiliser ces fonctions, le paquet `banff` doit être importé à l'aide de `import banff`.

|Fonction|Description|
|--|--|
|`banff.io_util.SAS_file_to_feather_file(file_path, destination)`|Lire l'ensemble de données SAS depuis `file_path` et le convertir en un fichier *feather* dans `destination`|
|`banff.io_util.SAS_file_to_parquet_file(file_path, destination)`|Lire l'ensemble de données SAS depuis `file_path` et le convertir en un fichier *parquet* dans `destination`|
|`banff.io_util.DF_from_sas_file(file_path)`|Lire l'ensemble de données SAS depuis `file_path` et le retourner comme un *`pandas.DataFrame`*|
|`banff.io_util.PAT_from_sas_file(file_path)`|Lire l'ensemble de données SAS depuis `file_path` et le retourner comme une *`pyarrow.Table`*|

### Considérations sur la performance 

Les formats utilisés dans les données d'entrée et de sortie affecteront la performance. 

Lorsque la mémoire vive disponible n'est pas suffisante (en raison d'une petite taille de la mémoire vive ou des données de grande taille), les données doivent être stockées sur disque. Le format du fichier sélectionné affectera la performance. Les formats de fichier Apache Parquet (`.parquet`) et Apache Feather (`.feather`) offrent actuellement les meilleures performances lors de l'utilisation de fichiers pour les données d'entrée ou de sortie.

Feather devrait utiliser la plus petite quantité de mémoire vive, ce qui le rend idéal pour les données de grande taille ou les environnements d'exécution avec peu de mémoire vive; c'est le format recommandé pour les fichiers temporaires. Parquet est généralement le format de fichier le plus petit en taille, ce qui ne l'empêche pas d'offrir des performances de lecture et d'écriture impressionnantes dans les environnements multiprocesseurs et une utilisation raisonnablement minimale de la mémoire vive; il est recommandé pour le stockage de données à moyen et long terme.  

L'utilisation du format SAS pour les données d'entrée de grande taille peut entraîner une dégradation de la performance, notamment dans les environnements disposant de peu de mémoire vive. Ce format n'est recommandé que pour les petits ensembles de données (quelques centaines de Mo et moins). L'utilisation du format SAS est généralement déconseillée, au profit des formats Apache Arrow (*parquet* et *feather*) qui sont recommandés.  