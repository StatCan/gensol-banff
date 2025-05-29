# Imputation déterministe

* Exécution: *banff.deterministic()*
* Type de fonction VSD: *Traitement*
* Statuts d'entrée: *FTI (requis)*
* Statuts de sortie: *IDE*

## Description

Effectue l'imputation quand une seule combinaison de valeurs permet à l'enregistrement de passer l'ensemble des règles de vérification. 

La procédure d'imputation déterministe analyse chaque champ identifié comme nécessitant l'imputation, pour déterminer s'il existe une seule valeur possible permettant de satisfaire les règles de vérification originales. Si une telle valeur existe, alors elle est utilisée comme valeur d'imputation lors de l'exécution de la procédure. Cette méthode est aussi appelée imputation déductive puisqu'une valeur manquante ou incohérente peut être déduite avec certitude en se basant sur les autres champs du même enregistrement.  

## Données d'entrée et de sortie

La description des données d'entrée et de sortie est donnée ci-dessous. Banff supporte plusieurs formats pour les données qu'elles soient d'entrée ou de sortie; se référer au guide d'utilisateur pour plus d'information. 

| Données d'entrée  | Description |
| --------------- | ----------- |
| indata          | Données de données d'entrée. Obligatoire. |
| instatus        | Données de statuts d'entrée qui contient les statuts FTI.  <br><br> Cette procédure utilise les statut FTI pour identifier les champs nécessitant une imputation déterministe.  |

| Données de sortie | Description |
| --------------- | ----------------------------------------------------------------- |
| outdata         | Données de données de sortie contenant les données imputées.<br><br> Noter que outdata contiendra les enregistrements imputés avec succès et les champs affectés. Les utilisateurs devront mettre à jour indata avec les valeurs provenant de outdata avant de continuer le processus de vérification et d'imputation des données. |
| outstatus       | Données des statuts de sortie identifiant les champs imputés avec le statut IDE et contenant leurs valeurs après imputation.   |

Pour plus d'information sur le contenu des données de sortie, se référer au document des [données de sortie](../output_tables.md).

## Paramètres

| Paramètre       | Type en Python       | Description    | 
| ----------------| -------------------  | --------------------------- |
| unit_id         | chaîne de caractères | Identifier la variable clé (identifiant de l'unité) dans indata. Obligatoire.<br><br> unit_id devra être unique pour chaque enregistrement. Les enregistrements ayant une valeur manquante seront écartés avant le traitement. |
| edits           | chaîne de caractères | Liste des règles de vérification. Obligatoire.<br><br> Exemple: `"Revenu - Depenses = Profit; Revenu >= 0; Depenses >= 0;"` |
| accept_negative | booléen              | Traite les valeurs négatives comme des valeurs valides. Par défaut: False. <br><br> Par défaut, la règle de positivité est ajoutée pour chaque variable dans la liste des règles de vérification; ce paramètre permet à l'utilisateur d'enlever cette restriction. L'utilisateur peut aussi ajouter la règle de positivité de manière individuelle pour chaque variable qui la requiert. |
| by              | chaîne de caractères | Variable(s) utilisée(s) pour partitionner indata en des groupes *by* pour un traitement indépendant. <br><br> L'imputation déterministe aura les mêmes résultats avec ou sans l'utilisation de la partition par groupes. <br><br> Exemple: `by = "province industrie"` |
| prefill_by_vars | booléen              | Ajouter une ou plusieurs variables de partition aux données des statuts d'entrée pour améliorer la performance. Par défaut: True. |
| presort         | booléen              |  Trier les données d'entrée avant le traitement, et ce selon les exigences de la procédure. Par défaut: True. |
| no_by_stats     | booléen              | Réduire le journal de sortie en supprimant les messages spécifiques aux groupes de partition *by*. Par défaut: False. |