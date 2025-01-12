# Statistiques sur les règles de vérification

* Exécution: *banff.editstats()*
* Type de fonction VDS: *Revue*
* Statuts d'entrée: *Néant*
* Statuts de sortie: *Néant*

## Description

Produit des statistiques sommaires sur les enregistrements qui satisfont ou ne satisfont pas (à cause d'une valeur manquante ou non manquante) chaque règle de vérification.  

Cette procédure applique un groupe de règles de vérification sur des données afin de déterminer pour chaque enregistrement s'il satisfait une règle de vérification ou ne la satisfait pas à cause d'une valeur manquante ou non manquante. Ce qu'il en résulte est sauvegardé dans cinq tableaux de sortie, et peut être utilisé pour peaufiner les règles de vérification, estimer les ressources requises pour des procédures subséquentes et évaluer les effets de l'imputation. Noter que cette procédure effectue une revue des données et produit des statistiques sommaires; utiliser errorloc (avec le même ensemble des règles de vérification) pour sélectionner les enregistrements et les champs à imputer.

## Données d'entrée et de sortie

La description des données d'entrée et de sortie est donnée ci-dessous. Banff supporte plusieurs formats pour les données qu'elles soient d'entrée ou de sortie; se référer au guide d'utilisateur pour plus d'information. 

| Données d'entrée  | Description |
| --------------- | ----------- |
| indata          | Données d'entrée. Obligatoire. |

| Données de sortie | Description |
| ------------------| ----------------------------------------------------- |
| outedits_reduced  | Ensemble minimal des règles de vérification.          |
| outedit_status    | Nombre d'enregistrements qui satisfont chaque règle de vérification ou ne la satisfont pas à cause d'une valeur manquante ou non manquante.           |
| outk_edits_status | Distribution des enregistrements qui satisfont un nombre donné de règles de vérification ou ne les satisfont pas à cause d'une valeur manquante ou non manquante.    |
| outglobal_status  | Nombre d'enregistrements qui satisfont toutes les règles de vérification de l'ensemble minimal ou ne les satisfont pas à cause d'une valeur manquante ou non manquante.    |
| outedit_applic    | Nombre de fois chaque variable est impliquée dans une règle de vérification qui est satisfaite ou non satisfaite à cause d'une valeur manquante ou non manquante. |
| outvars_role      | Nombre de fois chacune des variables contribue à l'état général de l'enregistrement. |

Pour plus d'information sur le contenu des données de sortie, se référer au ().

## Paramètres

| Paramètre       | Type en Python       | Description                 | 
| ----------------| -------------------  | --------------------------- |
| edits               | chaîne de caractères | Liste des règles de vérification. Obligatoire.<br><br> Exemple: `"Revenu - Depenses = Profit; Revenu >= 0; Depenses >= 0;"` |
| accept_negative | booléen              | Traite les valeurs négatives comme des valeurs valides. Par défaut: False. <br><br> Par défaut, la règle de positivité est ajoutée pour chaque variable dans la liste des règles de vérification; ce paramètre permet à l'utilisateur d'enlever cette restriction. L'utilisateur peut aussi ajouter la règle de positivité de manière individuelle pour chaque variable qui la requiert. |
| by              | chaîne de caractères | Variable(s) utilisée(s) pour partitionner indata en des groupes *by* pour un traitement indépendant. <br><br> Les tableaux de sortie seront indépendamment générés pour chaque groupe de partition *by*. <br><br> Exemple: `by = "province industrie"` |
| presort         | booléen              |  Trier les données d'entrée avant le traitement, et ce selon les exigences de la procédure. Par défaut: True. |