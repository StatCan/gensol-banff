# Imputation par estimation

* Exécution: *banff.estimator()*
* Type de fonction VSD: *Traitement*
* Statuts d'entrée: *FTI (requis), FTE (optionnel), I--(optionnel)*
* Statuts de sortie: *I--* (le code exact dépend de l'algorithme spécifié)

## Description

Effectue l'imputation en utilisant des fonctions d'estimation et/ou des estimateurs par régression linéaire.

La procédure estimator permet l'imputation selon plusieurs méthodes comme la moyenne, le ratio ou la régression en utilisant les données courantes (`indata`) et/ou historiques (`indata_hist`) de la variable à imputer et éventuellement des variables auxiliaires. L'utilisateur peut choisir parmi les vingt (20) algorithmes d'imputation par estimation prédéfinis dans la procédure, ou définir ses propres algorithmes personnalisés.

L'imputation est seulement effectuée sur les champs marqués par un statut FTI (champs à imputer) dans les données des statuts d'entrée. Les champs ayant un statut FTE (champs à exclure) ou I-- (champs imputés) peuvent être exclus du modèle d'imputation. (Noter que ceci n'inclut pas le statut IDE associé à l'imputation déterministe).

Les paramètres d'estimation ou de régression linéaire (comme les moyennes ou les coefficients de régression) peuvent être calculés en utilisant tous les enregistrements ou seulement un sous-ensemble particulier des enregistrements acceptables. La restriction des enregistrements acceptables peuvent être appliquée à l'aide d'un paramètre d'exclusion ou en imputant par groupes de partition *by*.

Pour une description mathématique complète des méthodes de la procédure accompagnée d'exemples, se référer à [la description des fonctions](../Description%20des%20fonctions%20Banff.pdf).

## Données d'entrée et de sortie

La description des données d'entrée et de sortie est donnée ci-dessous. Banff supporte plusieurs formats pour les données qu'elles soient d'entrée ou de sortie; se référer au guide d'utilisateur pour plus d'information. 

| Données d'entrée  | Description |
| ------------- | ----------- |
| indata          | Données d'entrée. Obligatoire.<br><br> Les enregistrements avec des valeurs manquantes pour une ou plusieurs variables spécifiés dans estimator et n'ayant pas le statut FTI dans instatus ne seront pas traités.  |
| instatus      | Données des statuts d'entrée qui contient les statuts FTI, FTE et I--. Obligatoire.<br><br> Les valeurs ayant un statut FTI sont identifiées comme nécessitant d'être imputées. Les valeurs aberrantes (FTE) et celles précédemment imputées (I--) peuvent être exclues de l'ensemble des enregistrements acceptables. Les valeurs ayant un statut IDE sont considérées comme des valeurs originales (et non précédemment imputées).|
| inestimator     | Table de spécification des estimateurs. Obligatoire.<br><br> Au moins une méthode d'imputation est définie pour chaque champs nécessitant d'être imputé. |
| inalgorithm     | Table des algorithmes spécifiés par l'utilisateur.<br><br> Un algorithme défini par l'utilisateur ne peut pas avoir le même nom qu'un algorithme prédéfini.|
| indata_hist   | Données historiques d'entrée. <br><br> Les enregistrements dans indata sont liés à ceux dans indata_hist par la variable `unit_id`. Les enregistrements qui apparaissent dans indata_hist sans qu'ils se retrouvent dans indata seront écartés avant l'imputation, de même que les enregistrements dont la valeur de `unit_id` est manquante. |
| instatus_hist | Données des statuts historiques d'entrée qui contient les statuts FTI, FTE and I--.<br><br> Si instatus_hist n'est pas spécifié mais que indata_hist l'est, alors toutes les valeurs historiques seront considérées comme des valeurs originales. Les valeurs ayant un statut FTI seront exclues de l'ensemble des enregistrements acceptables. |

| Données de sortie | Description |
| --------------- | ----------- |
| outdata         | Données de sortie contenant les données imputées.<br><br> Noter que outdata contiendra les enregistrements imputés avec succès et les champs affectés. Les utilisateurs devront mettre à jour indata avec les valeurs provenant de outdata avant de continuer le processus de vérification et d'imputation des données. |
| outstatus       | Données des statuts de sortie identifiant les champs imputés avec un statut de la forme I-- et contenant leurs valeurs après imputation.<br><br> Le statut des champs imputés aura comme préfixe "I", suivi du statut associé à l'algorithme utilisé dans l'imputation.  |
| outacceptable  | Données de sortie contenant les observations acceptables retenues pour calculer les paramètres de chaque estimateur. |
| outest_ef      | Données de sortie contenant les moyennes des fonctions d'estimation.  |
| outest_lr      | Données de sortie contenant les coefficients « beta » des estimateurs par régression linéaire. |
| outest_parm    | Données de sortie contenant les statistiques d'imputation par estimateur.  |
| outrand_err    | Données de sortie contenant l'information sur les erreurs aléatoires ajoutées aux valeurs imputées.  |

Pour plus d'information sur le contenu des données de sortie, se référer au document des [données de sortie](../output_tables.md).

## Paramètres

| Paramètre       | Type en Python          | Description                 | 
| ----------------| ----------------------  | --------------------------- |
| unit_id         | chaîne de caractères    | Identifier la variable clé (identifiant de l'unité) dans indata. Obligatoire.<br><br> unit_id devra être unique pour chaque enregistrement. Les enregistrements ayant une valeur manquante seront écartés avant le traitement. |
| data_excl_var       | chaîne de caractères          | Identifie la variable dans indata permettant l'exclusion de certains observations de l'ensemble des observations acceptables.<br><br> Quand la valeur de la variable associée à `data_excl_var` est `E` pour une observation, alors cette dernière ne sera pas utilisée dans le calcul de paramètres d'imputation. `data_excl_var` ne peut être combinée avec `exclude_where_indata`. |
| data_excl_var       | chaîne de caractères          | Identifie la variable dans indata permettant l'exclusion de certains observations historiques de l'ensemble des observations acceptables.<br><br> Quand la valeur de la variable associée à `hist_excl_var` est `E` pour une observation, alors cette dernière ne sera pas utilisée dans le calcul de paramètres d'imputation. `hist_excl_var` ne peut être combinée avec `exclude_where_indata_hist`. |
| exclude_where_indata | chaîne de caractères         | Expression dans le langage SQL permettant l'exclusion de certains observations de l'ensemble des observations acceptables.<br><br> Une nouvelle colonne sera temporairement ajoutée, et les rangées satisfaisant l'expression SQL seront étiquetées comme étant à exclure du bassin des donneurs. Par la suite, cette colonne sera utilisée comme le paramètre `data_excl_var`.`exclude_where_indata` ne peut être combinée avec `data_excl_var`.<br><br> L'imputation sera tout de même effectuée sur les champs ayant un statut FTI même s'ils satisfont l'expression dans `exclude_where_indata`. |
| exclude_where_indata_hist | chaîne de caractères         | Expression dans le langage SQL permettant l'exclusion de certains observations historiques de l'ensemble des observations acceptables.<br><br> Une nouvelle colonne sera temporairement ajoutée, et les rangées satisfaisant l'expression SQL seront étiquetées comme étant à exclure du bassin des donneurs. Par la suite, cette colonne sera utilisée comme le paramètre `hist_excl_var`.`exclude_where_indata_hist` ne peut être combinée avec `hist_excl_var`. |
| seed                | flottant | Spécifie la racine du générateur des nombres aléatoires. <br><br> Cette racine est utilisée pour s'assurer d'obtenir des résultats cohérents d'une exécution à une autre. Si elle n'est pas spécifiée comme une valeur non positive, un nombre aléatoire sera généré par la procédure. |
| verify_specs              | booléen         | Permet de vérifier les spécifications de la procédure sans rouler l'imputation.<br><br> Vérifie les spécifications, calcule les paramètres et s'arrête après avoir les imprimés au journal.|
| accept_negative | booléen              | Traite les valeurs négatives comme des valeurs valides. Par défaut: False. <br><br> Par défaut, la règle de positivité est ajoutée pour chaque variable dans la liste des règles de vérification; ce paramètre permet à l'utilisateur d'enlever cette restriction. L'utilisateur peut aussi ajouter la règle de positivité de manière individuelle pour chaque variable qui la requiert. |
| by              | chaîne de caractères | Variable(s) utilisée(s) pour partitionner indata en des groupes *by* pour un traitement indépendant. <br><br> Dans estimator, les groupes de partition *by* peuvent être vus comme des classes d'imputation. <br><br> Exemple: `by = "province industrie"` |
| prefill_by_vars | booléen              | Ajouter une ou plusieurs variables de partition aux données des statuts d'entrée pour améliorer la performance. Par défaut: True. |
| presort         | booléen              |  Trier les données d'entrée avant le traitement, et ce selon les exigences de la procédure. Par défaut: True. |
| no_by_stats     | booléen              | Réduire le journal de sortie en supprimant les messages spécifiques aux groupes de partition *by*. Par défaut: False. |

## Notes

Les données de spécification `inalgorithm` et `inestimator` sont employées pour définir les modèles (imputation par la moyenne, par régression linéaire, etc.) utilisés dans l'imputation et pour spécifier certains paramètres et options. Les données `inestimator` sont obligatoires, alors que celles associées à `inlagorithm` sont uniquement requises lorsque l'on a recourt à un algorithme personnalisé, et non au 20 algorithmes prédéfinis dans la procédure.

### Inestimator

Les données de spécification `inestimator` doivent être préparées avant de rouler la procédure. Elles sont employées pour indiquer l'algorithme (c'est-à-dire le modèle) à utiliser dans l'imputation, les variables à imputer, ainsi que d'autres paramètres. L'algorithme utilisé peut être prédéfini dans la procédure ou personnalisé et défini dans inalgorithm. Plusieurs algorithmes peuvent être spécifiés et seront traités dans selon leur ordre d'apparition. Noter que la même variable à imputer peut être spécifiée pour plusieurs algorithmes; dans pareil cas, le premier algorithme lui sera appliqué, mais s'il échoue à imputer une valeur, l'algorithme suivant lui sera appliqué, et ainsi de suite.

Le tableau suivant montre les variables qui doivent apparaître dans inestimator. Toutes les colonnes sont obligatoires.



|  Colonne         | Description                                                                 |
| ---------------- | --------------------------------------------------------------------------- |
| fieldid          | Nom de la variable à imputer.  |
| algorithmname    | Nom de l'algorithme à utiliser pour imputer la variable. <br><br> Il peut s'agir d'un algorithme prédéfini ou d'un algorithme qui se trouve dans inalgorithm.               |
| auxvariables     | Liste des noms des variables auxiliaires (séparés par virgule) dans indata ou indata_hist.<br><br> Ces noms remplaceront les paramètres de substitution dans la colonne `formula` dans inalgorithm ou dans la formule d'un algorithme prédéfini. La premier nom de variable remplacera aux1 dans la formule, le second remplacera aux2, et ainsi de suite.     |
| weightvariable   | Nom de la variable de pondération.<br><br> Il est optionnel si l'algorithme calcule un paramètre, sinon il doit être laissé vide. Pour les algorithmes "LR", weightvariable doit exister dans indata, alors que pour les algorithmes "EF" elle peut se trouver dans indata ou dans indata_hist dépendamment de la période de la moyenne demandée.           |
| countcriteria    | Nombre entier positif indiquant le nombre minimal d'observations acceptables nécessaires dans le groupe de partition *by* pour le calcul des paramètres l'estimateur.<br><br> Il est obligatoire pour les algorithmes qui calculent des paramètres, sinon il doit être laissé vide.        |
| percentcriteria  | Nombre réel (entre 0 et 100) indiquant le pourcentage minimal d'observations acceptables nécessaires dans le groupe de partition *by* pour le calcul des paramètres de l'estimateur.<br><br> Il est obligatoire pour les algorithmes qui calculent des paramètres, sinon il doit être laissé vide.   |
| variancevariable | Nom de la variable de variance.<br><br> Il est optionnel lorsque l'algorithme est de type "LR", sinon il doit être vide. La variable doit exister dans indata ou indata_hist dépendamment de la valeur spécifiée dans `varianceperiod`.         |
| varianceperiod   | Période de la variance ('C' pour courante ou 'H' pour historique).          |
| varianceexponent | Nombre servant d'exposant à la variance.          |
| excludeimputed   | 'Y' (oui) ou 'N' (non) pour indiquer si les observations ayant une variable déjà imputée seront exclues ou non du groupe d'observations acceptables pour le calcul de paramètres.<br><br> Il est obligatoire pour les algorithmes qui calculent des paramètres. Il doit être laissé vide pour les algorithmes ne calculant pas de paramètres.         |
| excludeoutliers  | 'Y' (oui) ou 'N' (non) pour indiquer si les observations ayant un statut FTE seront exclues ou non du groupe d'observations acceptables pour le calcul de paramètres.<br><br> Il est obligatoire pour les algorithmes qui calculent des paramètres. Il doit être laissé vide pour les algorithmes ne calculant pas de paramètres.           |
| randomerror      | 'Y' (oui) ou 'N' (non) pour indiquer si une erreur aléatoire est ajoutée à la valeur imputée.<br><br> Un avertissement est imprimé lorsque moins de 5 observations sont disponibles pour le choix aléatoire de l'erreur.        |

### Inalgorithm

En plus des 20 algorithmes prédéfinis dans la procédure, des algorithmes personnalisés peuvent être définis par l'utilisateur dans inalgorithm. Ces algorithmes personnalisés peuvent être de deux types:

- Fonctions d'estimation (EF): Expression mathématique avec des constantes, des valeurs courantes et/ou historiques de certaines variables de l'enregistrement, et des moyennes courantes et historiques de certaines variables, ces moyennes étant calculées à partir des enregistrements acceptables. Les expressions mathématiques peuvent inclure des parenthèses et les opérations arithmétiques de l'addition (+), la soustraction (-), la multiplication (*), la division (/) et l'exponentiation. 
- Régression linéaire (LR): Imputation par régression qui consiste à imputer une variable $y$<sub>$i$</sub> par l'estimation par régression linéaire de la forme

$$
\hat{y_i} = \hat{\beta_0} + \hat{\beta_1} x_{i_1 T_1}^{p_1} + \hat{\beta_2} x_{i_2 T_2}^{p_2} + ... + \hat{\beta_m} x_{i_m T_m}^{p_m} + \hat{\epsilon_i}  
$$

où : $T$<sub>$j$</sub> correspond aux périodes courante ou historique et $p$<sub>$j$</sub> sont les exposants. La variable $y$<sub>$j$</sub> à imputer est la variable dépendante du modèle, et les variables auxiliaires $x$<sub>$ij$</sub> sont les variables indépendantes ou les régresseurs. $\hat\beta$<sub>$j$</sub> sont les coefficients de régression, obtenues comme solution de la méthode des moindres carrés. $\hat\epsilon$<sub>$i$</sub> est un terme d'erreur aléatoire, qui peut être ajouté au modèle pour introduire une certaine variabilité dans les valeurs estimées de $y$<sub>$i$</sub>. 
Noter que $\beta$<sub>$0$</sub>, qui est l'ordonnée à l'origine de la droite de régression, peut être omise du modèle.

Le tableau suivant présente les variables qui doivent apparaître dans inalgorithm. Toutes les colonnes sont obligatoires sauf la colonne `description`.  

| Colonne           | Description                                                  |
| ------------------| ------------------------------------------------------------ |
| algorithmname     | Nom de l'algorithme.                                         |
| type              | Type de l'algorithme: 'EF' pour fonction d'estimation et 'LR' pour régression linéaire. |
| status            | Chaîne de 1 à 3 caractère qui sera inscrite comme statut (après lui avoir ajouté le préfixe "I") dans outstatus lorsqu'un champ est imputé à l'aide de cet algorithme.      |
| formula           | Formule de l'algorithme. <br><br> seuls les paramètres de substitution comme `aux1` et `fieldid` peuvent être utilisés dans les formules. Ils doivent être de la forme `aux1(period, aggregation)` où:<br><br> - `period`: `c` pour courante et `h` pour historique.<br><br> - `aggregation`: `v` pour utiliser la valeur de l'observation et `a` pour utiliser la moyenne des observations acceptables.                                   |
| description       | Texte pour décrire l'algorithme.                                         |   