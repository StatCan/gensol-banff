# Localisation des erreurs

* Exécution: *banff.errorloc()*
* Type de fonction VSD: *Revue, Sélection*
* Statuts d'entrée: *FTI (optionnel)*
* Statuts de sortie: *FTI*

## Description

Sélectionne, pour chaque enregistrement, le nombre minimal de variables à imputer tel que chaque observation puisse passer toutes les règles de vérification.

Les règles de vérification spécifie les relations entre variables qu'un enregistrement doit satisfaire. Lorsqu'un enregistrement ne satisfait pas l'une de ces relations, l'utilisateur doit déterminer les variables à changer; la localisation des erreurs est le terme donné à ce processus. La localisation des erreurs de Banff suit le principe de changement minimal de Felligi-Holt, et utilise un algorithme pour sélectionner les variable à changer. Ce processus est effectué de façon indépendante sur chaque enregistrement. Les valeurs sélectionnées sont sauvegardées dans les données des statuts de sortie, marqué par un statut FTI (champ à imputer).

Cette procédure requiert un ensemble de règles de vérification qui consistent en des égalités et des inégalités linéaires, lesquelles doivent être cohérentes entre elles. La procédure effectuera la localisation des erreurs uniquement sur les variables incluses dans les règles de vérification. Toute valeur manquante dans ces variables sera automatiquement sélectionnée pour l'imputation.  

Par défaut, la procédure minimisera le nombre de variables à changer. L'utilisateur peut aussi spécifier des poids des variables, auquel cas la procédure minimisera le compte pondéré des variables à changer. Pour certains enregistrements, la localisation des erreurs peut avoir des  solutions multiples (i.e. choix des variables) qui satisfont le principe de changement minimal; dans ce cas, l'une des solutions est sélectionnée de façon aléatoire.

Pour une description mathématique complète des méthodes de la procédure accompagnée d'exemples, se référer à [la description des fonctions](/docs/FR/Description%20des%20fonctions%20Banff.pdf).

## Données d'entrée et de sortie

La description des données d'entrée et de sortie est donnée ci-dessous. Banff supporte plusieurs formats pour les données qu'elles soient d'entrée ou de sortie; se référer au guide d'utilisateur pour plus d'information. 

| Données d'entrée  | Description |
| --------------- | ----------- |
| indata          | Données d'entrée. Obligatoire. |
| instatus        | Données des statuts d'entrée qui contient les statuts FTI. <br><br> Les valeurs déjà identifiées comme FTI seront sélectionnées en priorité. De ce fait, la procédure ne sélectionnera pas un champ pour être imputé lorsque un autre champ déjà identifié permet de résoudre le problème de localisation, minimisant ainsi le nombre de variables qui requièrent l'imputation. |

| Données de sortie | Description |
| --------------  | ----------- |
| outstatus       | Données de sortie identifiant les champs sélectionnés avec le statut FTI et contenant leurs valeurs.   |
| outreject       | Données de sortie contenant les enregistrements pour lesquels la localisation des erreurs a échoué. <br><br> outreject contient les enregistrements pour lesquels la localisation des erreurs n'a pas pu s'effectuer, à cause du dépassement de la cardinalité maximale autorisée (error = "CARDINALITY EXCEEDED") ou du temps alloué par observation (error = "TIME EXCEEDED"). |

Pour plus d'information sur le contenu des données de sortie, se référer au document des [données de sortie](/docs/FR/output_tables.md).

## Paramètres

| Paramètre           | Type en Python       | Description                 | 
| ------------------- | -------------------  | ----------- |
| unit_id             | chaîne de caractères | Identifier la variable clé (identifiant de l'unité) dans indata. Obligatoire.<br><br> unit_id devra être unique pour chaque enregistrement. Les enregistrements ayant une valeur manquante seront écartés avant le traitement. |
| edits               | chaîne de caractères | Liste des règles de vérification. Obligatoire.<br><br> Exemple: `"Revenu - Depenses = Profit; Revenu >= 0; Depenses >= 0;"` |
| weights             | chaîne de caractères | Spécifie les poids de la localisation des erreurs. <br><br> Les poids peuvent être utilisées pour influencer le choix des variables qui seront sélectionnées pour une étape subséquente du traitement. Avec l'utilisation des poids, errorloc minimise les comptes pondérés des variables à changer, tel que l'enregistrement en question puisse être altéré pour satisfaire les règles de vérification. Par défaut, errorloc assigne la valeur un comme poids à chacune des variables. Pour assigner un poids différent, spécifier `variable = value`, où *variable* est l'une des variables spécifiées dans `edits`, et *value* est un nombre plus grand que zéro. Plusieurs poids peuvent êtes assignés séparés par un point-virgule de la manière suivante. <br><br> Exemple: `weights = "revenu = 1.5; depenses = 0.8"` |
| accept_negative | booléen         | Traite les valeurs négatives comme des valeurs valides. Par défaut: False. <br><br> Par défaut, la règle de positivité est ajoutée pour chaque variable dans la liste des règles de vérification; ce paramètre permet à l'utilisateur d'enlever cette restriction. L'utilisateur peut aussi ajouter la règle de positivité de manière individuelle pour chaque variable qui la requiert. |
| cardinalité         | flottant   | Spécifie la cardinalité maximale. <br><br> La cardinalité correspond au nombre pondéré des variables nécessitant l'imputation, et qui peut varier par enregistrement. Le problème de localisation des erreurs pour les enregistrements ayant une cardinalité élevée peut nécessiter un délai plus long pour trouver une solution. Spécifier une cardinalité maximale peut améliorer le délai de traitement mais peut faire en sorte que la procédure échoue à résoudre le problème de localisation des erreurs pour tous les enregistrements; ceux-ci seront répertoriés dans outreject. |
| time_per_obs        | flottant           | Spécifie le temps de traitement maximal alloué par observation. |
| seed                | flottant | Spécifie la racine du générateur des nombres aléatoires. <br><br> Cette racine est utilisée pour s'assurer d'obtenir des résultats cohérents d'une exécution à une autre. Si elle n'est pas spécifiée comme une valeur non positive, un nombre aléatoire sera généré par la procédure. |
| rand_num_var        | chaîne de caractères             | Spécifie la variable du nombre aléatoire utilisée lorsqu’un choix doit être fait pendant la localisation des erreurs. <br><br> Cette variable doit exister dans indata; elle doit être numérique et ses valeurs doivent être des nombres réels non négatifs et plus petit ou égaux à 1. Les valeurs doivent être uniques pour chaque enregistrement. |
| by                  | chaîne de caractères             | Variable(s) utilisée(s) pour partitionner indata en des groupes *by* pour un traitement indépendant. <br><br> Puisque la localisation des erreurs est effectuée sur chaque enregistrement de manière indépendante, ce paramètre n'a pas d'incidence sur les résultats. Les versions futures pourraient utiliser ce paramètre pour améliorer l'efficacité de la procédure. <br><br> Exemple: `by = "province industrie"` |
| prefill_by_vars     | booléen            | Ajouter une ou plusieurs variables de partition aux données des statuts d'entrée pour améliorer la performance. Par défaut: True. |
| presort             | booléen            |  Trier les données d'entrée avant le traitement, et ce selon les exigences de la procédure. Par défaut: True. |
| no_by_stats         | booléen            | Réduire le journal de sortie en supprimant les messages spécifiques aux groupes de partition *by*. Par défaut: False. |
| display_level       | entier             | Valeur (0 ou 1) pour demander l'ajout dans le journal d'information en lien avec la variable du nombre aléatoire spécifiée dans `rand_num_var`.Par défaut: 0. |

## Notes

### Solutions multiples équivalentes

Dans certains cas, plusieurs solutions peuvent permettre de résoudre le problème de localisation des erreurs. Par exemple, pour un enregistrement qui ne passe pas la règle de vérification `"Profit = Revenu - Depenses;"`, changer une variable, peu importe laquelle, est une solution valide. Dans pareil cas, la procédure choisit une solution au hasard.  

Lors du développement ou des tests, l'utilisateur pourrait désirer de produire des résultats cohérents entre plusieurs exécutions de la procédure, ce qui peut être obtenu grâce aux paramètres `seed` ou `rand_num_var`. Ces deux paramètres permettent de s'assurer d'avoir les mêmes solutions choisies d'une exécution à une autre, toutes données d'entrée égales par ailleurs. Noter que si les deux paramètres `seed` et `rand_num_var` sont spécifiés, `seed` est alors ignoré. Si aucun d'eux n'est spécifié, le système générera une valeur de racine par défaut.