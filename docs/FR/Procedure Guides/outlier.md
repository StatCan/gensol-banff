# Détection des valeurs aberrantes

* Exécution: *banff.outlier()*
* Type de fonction VDS: *Revue, Sélection*
* Statuts d'entrée: *Néant*
* Statuts de sortie: *FTI, FTE*

## Description

Identifie les valeurs aberrantes à l'aide de la méthode Hidiroglou-Berthelot ou la méthode de l'écart-sigma.

Cette procédure permet l'utilisation de deux méthodes de détection univariées des valeurs aberrantes. La méthode Hidiroglou-Berthelot (HB) sélectionne les valeurs aberrantes en se basant sur leur distance de la médiane, relativement à la distance de l'écart interquartile. La méthode de l'écart-sigma (SG) trie les données dans l'ordre ascendant puis recherche des écarts significatifs (relativement à l'écart-type) entre deux valeurs consécutives, avant de sélectionner toutes les  valeurs subséquentes comme valeurs aberrantes. Les deux méthodes peuvent détecter deux types de valeurs aberrantes, qui seront repertoriés dans les données des statuts en sortie outstatus:

- Les valeurs qui sont suffisamment extrêmes pour être considérées comme des erreurs. Ces valeurs sont marquées comme étant des *champs à imputer (FTI)* afin qu'ils soient imputés lors d'une étape ultérieure.

- Les valeurs qui ne sont pas suffisamment extrêmes pour être considérées comme erronées, mais sont assez exceptionnelles pour qu'elles soient traitées comme *champs à exclure (FTE)* lors d'une procédure subséquente d'imputation comme donorimp ou estimator. (Ce statut peut aussi être utile lors de la pondération ou de l'estimation robuste).

Pour les deux méthodes, l'utilisateur doit spécifier un seuil d'imputation ou d'exclusion; ce seuil n'a pas de valeur par défaut.

Caractéristiques additionnelles de la procédure:

- L'utilisateur peut rouler la détection des valeurs aberrantes pour plusieurs variables (`var`) dans une même exécution.
- L'utilisateur peut aussi rouler la détection des valeurs aberrantes sur des ratios de variables. Dans ce cas, seulement les numérateurs (`var`) sont marqués dans les données des statuts de sortie outstatus. L'utilisateur peut utiliser des variables auxiliaires (`with_var`) de la période courante (indata) ou des données historiques (indata_hist) comme dénominateur.
- La détection des valeurs aberrantes peut être effectuée à droite, à gauche, ou des deux côtés (`side`).
- La détection des valeurs aberrantes peut être effectuée à l'intérieur des groupes de partition *by*, avec un nombre minimal d'observations (`min_obs`) spécifié par l'utilisateur et requis pour que la détection des valeurs aberrantes ait lieu.

Pour une description mathématique complète des méthodes de la procédure accompagnée d'exemples, se référer à [la description des fonctions](/docs/FR/Description%20des%20fonctions%20Banff.pdf).

## Données d'entrée et de sortie

La description des données d'entrée et de sortie est donnée ci-dessous. Banff supporte plusieurs formats pour les données qu'elles soient d'entrée ou de sortie; se référer au guide d'utilisateur pour plus d'information.  

| Données d'entrée  | Description |
| ------------- | ----------- |
| indata          | Données d'entrée. Obligatoire. |
| indata_hist   | Données historiques d'entrée. <br><br> Les enregistrements dans indata sont liés à ceux dans indata_hist par la variable `unit_id`. Les enregistrements qui apparaissent dans indata_hist sans qu'ils se retrouvent dans indata seront écartés avant la détection, de même que les enregistrements dont la valeur de `unit_id` est manquante. |

| Données de sortie | Description |
| ----------------- | ------------------------------------------------- |
| outstatus       | Données des statuts de sortie identifiant les champs des valeurs aberrantes avec les statuts FTI ET FTE et contenant leurs valeurs.   |
| outstatus_detailed  | Statut détaillé des valeurs aberrantes (ODER/ODEL/ODIR/ODIL). <br><br> Statut détaillé indiquant si la valeur aberrante se trouve à l'extérieur de l'intervalle d'exclusion à droite (ODER) ou à gauche (ODEL). Lorsque la valeur aberrante concerne un champ FTI, le statut détaillé identifie si la valeur aberrante se trouve à l'extérieur de l'intervalle d'imputation à droite (ODIR) ou à gauche (ODIL). <br><br> Ces données de sortie contiendront plus d'information (bornes d'exclusion et d'imputation, valeurs courantes et auxiliaires, ..) lorsque l'on spécifie `outlier_stats=True`.  |
| outsummary          | Informations sommaires sur les valeurs aberrantes détectées, telles que le compte des observations et les bornes de l'intervalle d'acceptation.|

Pour plus d'information sur le contenu des données de sortie, se référer au ().

## Paramètres

| Parameter       | Python type  | Description                 | 
| ----------------| -------------| --------------------------- |
| unit_id         | chaîne de caractères | Identifier la variable clé (identifiant de l'unité) dans indata. Obligatoire.<br><br> unit_id devra être unique pour chaque enregistrement. Les enregistrements ayant une valeur manquante seront écartés avant le traitement. |
| method          | chaîne de caractères | Méthode à utiliser dans la détection des valeurs aberrantes ('CURRENT', 'RATIO', 'HISTORIC' or 'SIGMAGAP'). Obligatoire.<br><br> La méthode de l'écart-sigma (SG) est appliquée lorsque  `method='SIGMAGAP'`, sinon c'est la méthode Hidiroglou-Berthelot (HB) qui est utilisée; veuillez vous référer aux notes pour plus d'information.|
| var             | chaîne de caractères | Variables qui font l'objet de la détection des valeurs aberrantes. <br><br> `var` devient obligatoire lorsque les variables historiques ou auxiliaires sont utilisées. Si elles ne le sont pas, `var` peut être omise, auquel cas toutes les variables numériques dans indata (à l'exception de celles utilisées pour définir les groupes de partition *by*) seront considérées dans la détection des valeurs aberrantes. <br><br> Exemple: `var = "Revenu Depenses"` |
| with_var        | chaîne de caractères | Variables historiques ou auxiliaires.<br><br> Le nombre de variables listées dans `with_var` doit être le même que celui des variables listées dans `var`. Les variables `with_var` sont lues à partir de indata_hist si ces données sont utilisées, sinon elles sont lues à partir de indata. Lorsque `var` et `with_var` sont identiques (i.e. chaque variable dans `var` a son équivalente historique avec le même nom dans indata_hist), alors `with_var` peut être omis. |
| weight          | chaîne de caractères          | Variable à utiliser dans la pondération.<br><br> le poids est au niveau de l'enregistrement et doit avoir une valeur pour chacun. `weight` sera multiplié par les valeurs des variables qui font l'objet de la détection des valeurs aberrantes. Lorsqu'un enregistrement dans indata a un poids est manquant, il sera écarté de la détection des valeurs aberrantes.|
| exclude_where_indata | chaîne de caractères         | Expression dans le langage SQL permettant l'exclusion de certains observations de la détection des valeurs aberrantes. |
| mii             | flottant        | Multiplicateur pour l'intervalle d'imputation de HB (positif).<br><br> `mii` contrôle la largeur de l'intervalle d'imputation. Une plus grande valeur de ce multiplicateur se traduira par un plus petit nombre de valeurs aberrantes détectées à imputer. `mii` devient obligatoire pour HB lorsque `mei` n'est pas spécifié. |
| mei             | flottant        | Multiplicateur pour l'intervalle d'exclusion de HB (positif).<br><br> `mei` contrôle la largeur de l'intervalle d'exclusion. Une plus grande valeur de ce multiplicateur se traduira par un plus petit nombre de valeurs aberrantes détectées à exclure. `mei` devient obligatoire pour HB lorsque `mii` n'est pas spécifié. |
| mdm             | flottant        | Multiplicateur de distance minimale de HB (positif). Par défaut: 0,05.<br><br> `mdm` correspond à la distance interquartile minimale requise pour calculer les intervalles. |
| exponent        | flottant        | Exposant des ratios ou des tendances historiques de HB (entre 0 et 1). Par défaut: 0. |
| min_obs         | entier      | Nombre minimal d'observations requises dans les données d'entrée ou dans les groupes de partition *by* le cas échéant (positif). Par défaut: 10.<br><br> `min_obs` >= 3 pour HB; `min_obs` >= 5 pour SG.<br><br> Aucune valeur aberrante ne sera détectée lorsque le nombre d'enregistrements est égal à 3 **(Contradiction!)**. Un nombre minimal de 10 observations par groupe de partition *by* est recommandé; avec moins de 10 observations, les résultats de la détection des valeurs aberrantes doivent être utilisés avec précaution.   |
| side            | chaîne de caractères  | Côté ('LEFT', 'RIGHT', ou 'BOTH') des données ordonnées à utiliser dans la détection des valeurs aberrantes. Par défaut: 'BOTH'. |
| start_centile   | flottant        | Centile à utiliser pour déterminer le point de départ (entre 0 et 100). Par défaut: 75 pour 'side="BOTH"', 0 sinon. <br><br> Le centile doit être supérieur ou égal à 0 et inférieur à 100 quand `side='LEFT'` ou `side='RIGHT'`. Le centile doit être supérieur ou égal à 50 et inférieur à 100 quand `side='BOTH'`. |
| beta_i          | flottant        | Multiplicateur de l'intervalle d'imputation de SG (non-négatif).<br><br> 0<`beta_e`<`beta_i`. `beta_i` devient obligatoire lorsque `beta_e` n'est pas spécifié.|
| beta_e          | flottant        | Multiplicateur de l'intervalle d'exclusion de SG (non-négatif).<br><br> 0<`beta_e`<`beta_i`. `beta_e` devient obligatoire lorsque `beta_i` n'est pas spécifié. |
| sigma           | chaîne de caractères | Écart-type ('MAD' or 'STD') à calculer. Par défaut: 'MAD'.<br><br> MAD: écart médian absolu; STD: écart-type classique. |
| outlier_stats   | booléen         | Ajoute de l'information additionnelle aux données de sortie outstatus_detailed, incluant les bornes des intervalles d'exclusion et d'imputation. Par défaut: False.|
| accept_zero     | booléen         | Traite les valeurs nulles comme des valeurs valides. Par défaut: False lorsque des variables historiques ou auxiliaires sont utilisées, True sinon. |
| accept_negative | booléen              | Traite les valeurs négatives comme des valeurs valides. Par défaut: False. <br><br> Par défaut, la règle de positivité est ajoutée pour chaque variable dans la liste des règles de vérification; ce paramètre permet à l'utilisateur d'enlever cette restriction. L'utilisateur peut aussi ajouter la règle de positivité de manière individuelle pour chaque variable qui la requiert. |
| by              | chaîne de caractères | Variable(s) utilisée(s) pour partitionner indata en des groupes *by* pour un traitement indépendant. <br><br> La détection des valeurs aberrantes est effectuée séparément sur chaque groupe *by*. <br><br> Exemple: `by = "province industrie"` |
| prefill_by_vars | booléen              | Ajouter une ou plusieurs variables de partition aux données des statuts d'entrée pour améliorer la performance. Par défaut: True. |
| presort         | booléen              |  Trier les données d'entrée avant le traitement, et ce selon les exigences de la procédure. Par défaut: True. |
| no_by_stats     | booléen              | Réduire le journal de sortie en supprimant les messages spécifiques aux groupes de partition *by*. Par défaut: False. |

## Notes

### Seuils d'imputation et d'exclusion

L'identification des valeurs aberrantes aux fins d'imputation ou d'exclusion requiert des seuils spécifiés par l'utilisateur. Il n'y a pas de valeur par défaut ou de suggestion pour ces valeurs; elles dépendent entièrement des critères que l'utilisateur utilise pour considérer une valeur comme extrême. Au moins un seuil doit être spécifié pour chaque méthode: `mii` ou `mei` pour la méthode HB, et `beta_i` ou `beta_e` pour la méthode SG.

### Spécifier la méthode de détection des valeurs aberrantes

La paramètre `method` permet de spécifier la détection des valeurs aberrantes de plusieurs manières, que ce soit pour des variables individuelles ou des ratios, avec ou sans données historiques. Ceci dépend de la combinaison des paramètres `method`, `var`, et `with_var`, et si `indata_hist` est fourni ou non:

| Pour faire ceci: | Paramètre `method` | Paramètre `with_var` | `indata_hist` fourni |
| ---------------- | ------------------ | -------------------- | ---------------------|
| Appliquer la méthode HB aux données courantes          | `"CURRENT"`  | Non | Non   |
| Appliquer la méthode HB au ratio des données courantes | `"RATIO"`    | Oui | Non   |
| Appliquer la méthode HB aux tendances historiques      | `"CURRENT"`  | Oui | Oui   |
| Appliquer la méthode SG aux données courantes          | `"SIGMAGAP"` | Non | Non   |
| Appliquer la méthode SG au ratio des données courantes | `"SIGMAGAP"` | Oui | Non   |
| Appliquer la méthode SG aux tendances historiques      | `"SIGMAGAP"` | Oui | Oui   |

La méthode de détection des valeurs aberrantes est toujours appliquée aux variables listées dans `var`. (Si `var` n'est pas spécifié, la détection des valeurs aberrantes sera effectuée pour les variables numériques dans indata, à l'exception de celles listées dans le paramètre `by`.) Pour appliquer la détection des valeurs aberrantes sur un ratio de variables, spécifier la liste des numérateurs dans `var` et la liste des dénominateurs dans `with_var`; la procédure passera à travers les pairs dans l'ordre spécifié. (Noter qu'il est possible de lister les même variables plusieurs fois dans `var` comme dans `with_var`.)

Lorsque indata_hist est fourni, la procédure utilisera les données courantes (provenant de indata) comme numérateurs et les données historiques (provenant de `indata_hist`) comme dénominateurs. Lorsque `with_var` n'est pas spécifié, la procédure utilisera les mêmes variables au numérateur et au dénominateur.

