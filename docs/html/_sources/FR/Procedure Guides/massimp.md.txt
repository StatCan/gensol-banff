# Imputation massive

* Exécution: *banff.massimp()*
* Type de fonction VSD: *Revue, Sélection, Traitement*
* Statuts d'entrée: *Néant*
* Statuts de sortie: *IMAS*

## Description

Effectue l'imputation par donneur pour un bloc de variables avec l'approche du plus proche voisin ou une sélection aléatoire.  

La procédure massimp est destinée à l'usage lorsqu'un grand bloc de variables est manquant pour un ensemble de répondants. Ce cas de figure survient typiquement lorsque l'information détaillée est uniquement collectée pour un sous-ensemble (ou échantillon de seconde phase) des unités. Alors que la procédure donorimp utilise à la fois des champs d'appariement sélectionnés par le système et spécifiés par l'utilisateur, l'imputation massive considère uniquement les champs d'appariement spécifiés par l'utilisateur pour trouver un enregistrement valide (donneur) le plus similaire è celui nécessitant l'imputation (receveur).

L'imputation massive considère comme receveur tout enregistrement pour lequel toutes les variables à imputer (listées dans `must_impute`) sont manquantes dans indata. Elle considère comme donneur tout enregistrement pour lequel aucune des variables listées n'est manquante. Si les champs d'appariement (`must_match`) sont fournis par l'utilisateur, la procédure massimp les utilise pour trouver le donneur le plus proche en ayant recours à la même fonction de distance que donorimp. Si les champs d'appariement ne sont pas fournis, la sélection des donneurs se fait de manière aléatoire.

Contrairement à donorimp, la procédure massimp n'utilise pas les règles de vérification. Avant de rouler la procédure, l'utilisateur devrait s'assurer que le bassin des donneurs ne contient pas d'erreurs, incluant les valeurs aberrantes et les erreurs de cohérence.

L'utilisateur peut créer des groupes de partition *by* en spécifiant des variables dans le paramètre `by`. Ces groupes *by* agissent comme des classes d'imputation. L'utilisation des paramètres `min_donors` et `percent_donors` permet de s'assurer qu'un nombre approprié ou ratio de receveurs et donneurs existe dans chaque classe d'imputation avant que l'imputation n'ait lieu.  

Pour une description mathématique complète des méthodes de la procédure accompagnée d'exemples, se référer à [la description des fonctions](../Description%20des%20fonctions%20Banff.pdf).

## Données d'entrée et de sortie

La description des données d'entrée et de sortie est donnée ci-dessous. Banff supporte plusieurs formats pour les données qu'elles soient d'entrée ou de sortie; se référer au guide d'utilisateur pour plus d'information.  

| Données d'entrée  | Description |
| ------------- | ----------- |
| indata          | Données d'entrée. Obligatoire. |

| Données de sortie | Description |
| --------------- | ----------------------------------------------------------------- |
| outdata         | Données de sortie contenant les données imputées.<br><br> Noter que outdata contiendra les enregistrements imputés avec succès et les champs affectés. Les utilisateurs devront mettre à jour indata avec les valeurs provenant de outdata avant de continuer le processus de vérification et d'imputation des données. |
| outstatus       | Données des statuts de sortie identifiant les champs imputés avec le statut IMAS et contenant leurs valeurs après imputation.   |
| outdonormap        | Données de sortie contenant les paires receveur-donneur pour les enregistrements imputés avec succès.  |

Pour plus d'information sur le contenu des données de sortie, se référer au document des [données de sortie](../output_tables.md).

## Paramètres

| Paramètre       | Type en Python | Description | 
| ----------------| -------------- | ----------- |
| unit_id         | chaîne de caractères | Identifier la variable clé (identifiant de l'unité) dans indata. Obligatoire.<br><br> unit_id devra être unique pour chaque enregistrement. Les enregistrements ayant une valeur manquante seront écartés avant le traitement. |
| must_impute      | chaîne de caractères | Variable(s) à imputer. Obligatoire. <br><br> Pour être un receveur, toutes les variables listées dans `must_impute` doivent être manquantes pour une observation. Si toutes ces variables sont non manquantes, alors cette observation est un donneur potentiel.<br><br> Exemple: `must_impute="revenu_q1 revenu_q2 revenu_q3 revenu_q4"`|
| must_match       | chaîne de caractères | Champ(s) d'appariement spécifié(s) par l'utilisateur.<br><br> `must_match` est optionnel; lorsqu'il n'est pas spécifié, le paramètre `random` doit être spécifié, et un donneur sera sélectionné aléatoirement pour chaque receveur.<br><br> Exemple: `must_match="revenu taille"`|
| random           | booléen         | Sélection aléatoire des donneurs.<br><br> Lorsque `random` est utilisé au côté de `must_match`, la sélection aléatoire est alors appliquée aux receveurs ayant des valeurs manquantes pour tous les champs listés dans `must_match`. |
| min_donors       | entier          | Nombre minimal de donneurs requis dans un groupe de partition *by* pour que l'imputation ait lieu. Par défaut: 30 |
| percent_donors   | flottant        | Pourcentage minimal de donneurs requis dans un groupe de partition *by* pour que l'imputation ait lieu. Par défaut: 30 |
| n_limit          | entier          | Limite le nombre de fois qu'un donneur peut être utilisé. |
| mrl                 | flottant        | Multiplicateur de ratio limite permettant de limiter le nombre de fois qu'un donneur peut être utilisé. <br><br> Ce paramètre est multiplié par le ratio du nombre de receveurs sur le nombre de donneurs, puis le résultat sera la limite maximale de fois qu'un donneur peut être utilisé.|
| seed                | flottant | Spécifie la racine du générateur des nombres aléatoires. <br><br> Cette racine est utilisée pour s'assurer d'obtenir des résultats cohérents d'une exécution à une autre. Si elle n'est pas spécifiée comme une valeur non positive, un nombre aléatoire sera généré par la procédure. |
| accept_negative | booléen              | Traite les valeurs négatives comme des valeurs valides. Par défaut: False. <br><br> Par défaut, la règle de positivité est ajoutée pour chaque variable dans la liste des règles de vérification; ce paramètre permet à l'utilisateur d'enlever cette restriction. L'utilisateur peut aussi ajouter la règle de positivité de manière individuelle pour chaque variable qui la requiert. |
| by              | chaîne de caractères | Variable(s) utilisée(s) pour partitionner indata en des groupes *by* pour un traitement indépendant. <br><br> Dans massimp, les groupes de partition *by* peuvent être vus comme des classes d'imputation. <br><br> Exemple: `by = "province industrie"` |
| presort         | booléen              |  Trier les données d'entrée avant le traitement, et ce selon les exigences de la procédure. Par défaut: True. |
| no_by_stats     | booléen              | Réduire le journal de sortie en supprimant les messages spécifiques aux groupes de partition *by*. Par défaut: False. |

## Notes

### Plus proche voisin ou donneur aléatoire

Les paramètres `must_match` et `random` détermine si l'algorithme du plus proche voisin ou la sélection aléatoire est utilisé dans la sélection de donneurs. Le tableau suivant montre comment la spécification de ces paramètres affecte l'imputation massive.

| `must_match` spécifié      | `random` spécifié   | Syntaxe    | Imputation                | 
| -------------| ----------- | -----------| -----------------------------------------------  |
| Non          | Non         | Incorrecte | Produit une erreur, aucune imputation n'est effectuée. |
| Non          | Oui         | Correcte   | Sélection aléatoire de donneurs.                       |
| Oui          | Non         | Correcte   | Sélection par plus proche voisin en utilisant les variables de `must_match`. |
| Oui          | Oui         | Correcte   | Sélection par plus proche voisin en utilisant les variables de `must_match`, ou sélection aléatoire pour les donneurs ayant des valeurs manquantes pour les variables dans `must_match`. |

Lorsqu'un receveur a des valeurs manquantes pour une partie (et non toutes) des variables `must_match`, la distance au donneur le plus proche sera calculée en se basant uniquement sur les valeurs valides. Si le receveur a des valeurs manquantes pour les variables `must_match`, alors on lui sélectionnera un donneur de manière aléatoire si le paramètre `random` est spécifié, et on ne lui sélectionnera aucun si le paramètre `random` n'est pas utilisé. 

### Solutions multiples équivalentes

Dans certains cas, et pour un receveur donné, il peut y avoir plusieurs plusieurs donneurs équidistants (i.e. éloignés de la même distance du receveur) dont les valeurs permettent au receveur de satisfaire les règles de vérification. Dans ce cas de figure, la procédure choisit une solution de manière aléatoire.

Lors du développement ou des tests, l'utilisateur pourrait désirer de produire des résultats cohérents entre plusieurs exécutions de la procédure, ce qui peut être obtenu grâce au paramètre `seed`. Il permet de s'assurer d'avoir les mêmes solutions choisies d'une exécution à une autre, toutes données d'entrée égales par ailleurs. Noter que si `seed` n'est pas spécifié, le système générera une valeur de racine par défaut.

Ce paramètre peut également être utilisé pour répliquer les résultats lorsqu'une sélection aléatoire de donneurs est effectuée.

### Limiter l'utilisation répétée de donneurs

L'utilisateur peut limiter l'utilisation répétée de donneurs grâce aux paramètres interreliés `n_limit` et `mrl`. Dépendamment de la manière dont ces paramètres sont spécifiés, la limite d'utilisation de donneurs est comme suit:

| `n_limit` | `mrl`  | Limite d'utilisation de donneurs                   | 
| --------- | -------| ------------------------------------------------   |
| No        | No     | Le nombre de fois qu'un donneur peut être utilisé est illimité.  |
| No        | Yes    | arrondie $(mrl*(receveurs/donneurs))$.               |
| Yes       | No     | `n_limit`.                                         |
| Yes       | Yes    | arrondie $(max(n$ _ $limit,mrl*(receveurs/donneurs)))$.|

Lorsque l'utilisation répétée de donneurs est limitée par le paramètre `n_limit`, le nombre de donneurs restants peut se retrouver plus petit que `min_donors`. Dans pareil cas, massimp continuera en ignorant `min_donors` qui a été vérifié au début. Ceci s'applique également pour `percent_donors`.

