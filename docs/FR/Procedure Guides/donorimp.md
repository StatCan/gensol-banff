# Imputation par donneur

* Exécution: *banff.donorimp()*
* Type de fonction VSD: *Traitement*
* Statuts d'entrée: *FTI (requis), FTE(optionnel), I-- (optionnel)*
* Statuts de sortie: *IDN*

## Description

Effectue l'imputation par enregistrement donneur avec la méthode du voisin le plus proche tel que chaque enregistrement imputé satisfasse les règles de vérification post-imputation.

La procédure donorimp sépare les enregistrements en receveurs (enregistrements nécessitant d'être imputés) et donneurs (enregistrements n'ayant pas besoin d'être imputés et satisfaisant les règles de vérification). Pour chaque receveur, la procédure effectue les étapes suivantes:

1. À partir des champs dans les règles de vérification, un sous-ensemble est choisi comme champs d'appariement utilisés dans le calcul de la distance. Cette sélection peut varier selon le champ nécessitant l'imputation. L'utilisateur peut aussi spécifier des champs d'appariement obligatoire qui seront automatiquement inclus dans le calcul de distance aux côtés de ceux sélectionnés par le système.
2. Les champs d'appariement sont transformés en rangs normalisés pour enlever l'effet d'échelle et de regroupement dans les données. Sans cette transformation, les champs ayant de grandes amplitudes, à l'instar des valeurs en dollars, vont toujours dominer la distance calculée.
3. Les distances entre le receveur et les donneurs sont calculées avec les champs d'appariement transformés à l'aide de la norme L-$\infty$. Celle-ci est parfois appelée distance minimax parce qu'elle comme donneur le plus proche l'enregistrement qui a la différence absolue maximale la plus petite entre ses valeurs transformées des champs d'appariement et celles du receveur.
4. À partir des donneurs, un algorithme de recherche est utilisé pour trouver de manière efficace les plus proches donneurs dont les valeurs permettent au receveur de satisfaire les règles de vérification post-imputation (`post_edits`) spécifiées par l'utilisateur. Ces règles sont typiquement une version moins stricte des règles de vérification originales, ce qui permet d'augmenter les chances de trouver un donneur.   

*Note: La distance métrique de Banff sélectionnera souvent des donneurs différents de la distance métrique euclidienne. En utilisant cette dernière, les différences d'échelle et les distributions asymétriques qu'on retrouve typiquement dans les données économiques impactera la distance métrique qui pourrait être dominée par un seul champ comme le revenu. La distance métrique de Banff s'assure que tous les champs d'appariement ont le même poids dans le calcul de la distance.* 

Un receveur est un enregistrement avec au moins un champ dans les règles de vérification nécessitant l'imputation, et identifié comme tel par le statut FTI (champ à imputer) dans les données des statuts d'entrée (instatus). Un donneur est enregistrement qui satisfait l'ensemble des règles de vérification et qui n'est pas un receveur. La procédure donorimp requiert un ensemble de règles de vérification; pour une version d'imputation par donneur qui n'en prend pas un, se référer à la procédure massimp.

Il y a plusieurs manières d'exclure des enregistrements ou des valeurs du bassin des donneurs. Des enregistrements peuvent être exclus avec les paramètres `exclude_where_indata` ou `data_excl_var`. Ceci ne les exclut pas complètement de la procédure puisqu'ils peuvent toujours être imputés s'ils nécessitent l'imputation. Les enregistrements qui ont été précédemment imputés peuvent être exclus du bassin des donneurs à l'aide du paramètre `eligdon` (donneur éligible). La paramètre `n_limit` limite le nombre de fois un donneur est utilisé dans l'imputation. L'utilisateur peut identifier des valeurs qui ne nécessitent pas l'imputation, mais qui sont si inhabituelles au point de ne pas vouloir les utiliser pour imputer d'autres enregistrements; ces valeurs doivent avoir un statut FTE (valeur aberrante à exclure) dans les données des statuts d'entrée. 

La distance métrique de Banff ne convient pas aux variables catégoriques. Au lieu de cela, l'utilisateur peut créer des groupes de partition *by* en spécifiant des variables dans le paramètre `by`. Ces groupes *by* agissent comme des classes d'imputation. L'utilisation des paramètres `min_donors` et `percent_donors` permet de s'assurer qu'un nombre approprié ou ratio de receveurs et donneurs existe dans chaque classe d'imputation avant que l'imputation n'ait lieu. 

Pour une description mathématique complète des méthodes de la procédure accompagnée d'exemples, se référer à [la description des fonctions](/docs/FR/Description%20des%20fonctions%20Banff.pdf).

## Données d'entrée et de sortie

La description des données d'entrée et de sortie est donnée ci-dessous. Banff supporte plusieurs formats pour les données qu'elles soient d'entrée ou de sortie; se référer au guide d'utilisateur pour plus d'information. 

| Données d'entrée  | Description |
| ------------- | ----------- |
| indata          | Données d'entrée. Obligatoire. |
| instatus      | Données des statuts d'entrée qui contient les statuts FTI, FTE et I--. Obligatoire.<br><br> La procédure donorimp requiert données des statuts qui lui sert pour identifier comme receveurs les enregistrements avec un statut FTI et figurant dans au moins une règle de vérification. Les champs ayant un statut FTE ne sont ni imputés ni utilisés comme valeurs d'imputation. Si `eligdon = "ORIGINAL"`, alors la procédure cherchera aussi les statuts d'imputation de la forme I--.|

| Données de sortie | Description |
| -----------------  | ------------------------------------------------- |
| outdata         | Données de sortie contenant les données imputées.<br><br> Noter que outdata contiendra les enregistrements imputés avec succès et les champs affectés. Les utilisateurs devront mettre à jour indata avec les valeurs provenant de outdata avant de continuer le processus de vérification et d'imputation des données. |
| outstatus       | Données des statuts de sortie identifiant les champs imputés avec le statut IDN et contenant leurs valeurs après imputation.   |
| outdonormap        | Données de sortie contenant les paires receveur-donneur pour les enregistrements imputés avec succès.  |
| outmatching_fields | Données de sortie contenant les champs d'appariement pour chaque receveur.<br><br> Les champs d'appariement sont utilisés par la procédure donorimp pour calculer les distances entre les receveurs et les donneurs potentiels. Les champs d'appariement peuvent prendre l'une des trois formes: MFS (sélectionné par le système), MFU (spécifié par l'utilisateur) ou MFB (sélectionné par le système et spécifié par l'utilisateur).  |

Pour plus d'information sur le contenu des données de sortie, se référer au document des [données de sortie](/docs/FR/output_tables.md).

## Paramètres

| Paramètre       | Type en Python       | Description    | 
| ------------------- | -----------  | ----------- |
| unit_id             | chaîne de caractères | Identifier la variable clé (identifiant de l'unité) dans indata. Obligatoire.<br><br> unit_id devra être unique pour chaque enregistrement. Les enregistrements ayant une valeur manquante seront écartés avant le traitement. |
| edits               | chaîne de caractères | Liste des règles de vérification. Obligatoire.<br><br> Exemple: `"Revenu - Depenses = Profit; Revenu >= 0; Depenses >= 0;"` |
| post_edits          | chaîne de caractères          | Liste des règles de vérification qu'un receveur devra satisfaire après l'imputation. |
| min_donors          | entier          | Nombre minimal de donneurs requis dans un groupe de partition *by* pour que l'imputation ait lieu. |
| percent_donors      | flottant        | Pourcentage minimal de donneurs requis dans un groupe de partition *by* pour que l'imputation ait lieu. |
| n                   | entier          | Nombre maximal de donneurs à essayer pour chaque receveur. Obligatoire. |
| eligdon             | chaîne de caractères          | Identifie les donneurs éligibles avec 'ANY' (inclue tous les donneurs potentiels) ou 'ORIGINAL' (exclut les enregistrements avec des valeurs imputées). |
| n_limit             | entier          | Limite le nombre de fois qu'un donneur peut être utilisé. |
| random              | booléen         | Choisit les donneurs de façon aléatoire pour les receveurs sans champs d'appariement. |
| rand_num_var        | chaîne de caractères   | Identifie la variable du nombre aléatoire dans indata. |
| seed                | flottant | Spécifie la racine du générateur des nombres aléatoires. <br><br> Cette racine est utilisée pour s'assurer d'obtenir des résultats cohérents d'une exécution à une autre. Si elle n'est pas spécifiée comme une valeur non positive, un nombre aléatoire sera généré par la procédure. |
| mrl                 | flottant        | Multiplicateur de ratio limite permettant de limiter le nombre de fois qu'un donneur peut être utilisé. |
| must_match          | chaîne de caractères          | Liste de variables dans indata utilisées dans le calcul de la distance. |
| data_excl_var       | chaîne de caractères          | Identifie la variable dans indata permettant l'exclusion de certains donneurs <br><br> `data_excl_var` ne peut être combinée avec `exclude_where_indata`. |
| exclude_where_indata | chaîne de caractères         | Expression dans le langage SQL permettant l'exclusion de certains donneurs.<br><br> Une nouvelle colonne sera temporairement ajoutée, et les rangées satisfaisant l'expression SQL seront étiquetées comme étant à exclure du bassin des donneurs. Par la suite, cette colonne sera utilisée comme le paramètre `data_excl_var`. `exclude_where_indata` ne peut être combinée avec `data_excl_var`.<br><br> L'imputation sera tout de même effectuée sur les champs ayant un statut FTI même s'ils satisfont l'expression dans
`exclude_where_indata`. |
| accept_negative | booléen              | Traite les valeurs négatives comme des valeurs valides. Par défaut: False. <br><br> Par défaut, la règle de positivité est ajoutée pour chaque variable dans la liste des règles de vérification; ce paramètre permet à l'utilisateur d'enlever cette restriction. L'utilisateur peut aussi ajouter la règle de positivité de manière individuelle pour chaque variable qui la requiert. |
| by              | chaîne de caractères | Variable(s) utilisée(s) pour partitionner indata en des groupes *by* pour un traitement indépendant. <br><br> Dans donorimp, les groupes de partition *by* peuvent être vus comme des classes d'imputation. <br><br> Exemple: `by = "province industrie"` |
| prefill_by_vars | booléen              | Ajouter une ou plusieurs variables de partition aux données des statuts d'entrée pour améliorer la performance. Par défaut: True. |
| presort         | booléen              |  Trier les données d'entrée avant le traitement, et ce selon les exigences de la procédure. Par défaut: True. |
| no_by_stats     | booléen              | Réduire le journal de sortie en supprimant les messages spécifiques aux groupes de partition *by*. Par défaut: False. |
| display_level       | entier             | Valeur (0 ou 1) pour demander l'ajout dans le journal d'information en lien avec la variable du nombre aléatoire spécifiée dans `rand_num_var`.Par défaut: 0. |

## Notes

### Solutions multiples équivalentes

Dans certains cas, et pour un receveur donné, il peut y avoir plusieurs plusieurs donneurs équidistants (i.e. éloignés de la même distance du receveur) dont les valeurs permettent au receveur de satisfaire les règles de vérification. Dans ce cas de figure, la procédure choisit une solution de manière aléatoire.

Lors du développement ou des tests, l'utilisateur pourrait désirer de produire des résultats cohérents entre plusieurs exécutions de la procédure, ce qui peut être obtenu grâce aux paramètres `seed` ou `rand_num_var`. Ces deux paramètres permettent de s'assurer d'avoir les mêmes solutions choisies d'une exécution à une autre, toutes données d'entrée égales par ailleurs. Noter que si les deux paramètres `seed` et `rand_num_var` sont spécifiés, `seed` est alors ignoré. Si aucun d'eux n'est spécifié, le système générera une valeur de racine par défaut.

### Limiter l'utilisation répétée de donneurs

L'utilisateur peut limiter l'utilisation répétée de donneurs grâce aux paramètres interreliés `n_limit` et `mrl`. Dépendamment de la manière dont ces paramètres sont spécifiés, la limite d'utilisation de donneurs est comme suit:

| `n_limit` | `mrl`  | Limite d'utilisation de donneurs                   | 
| --------- | -------| ------------------------------------------------   |
| No        | No     | Le nombre de fois qu'un donneur peut être utilisé est illimité.  |
| No        | Yes    | arrondie $(mrl*(receveurs/donneurs))$.               |
| Yes       | No     | `n_limit`.                                         |
| Yes       | Yes    | arrondie $(max(n$ _ $limit,mrl*(receveurs/donneurs)))$.|

Lorsque l'utilisation répétée de donneurs est limitée par le paramètre `n_limit`, le nombre de donneurs restants peut se retrouver plus petit que `min_donors`. Dans pareil cas, donorimp continuera en ignorant `min_donors` qui a été vérifié au début. Ceci s'applique également pour `percent_donors`.