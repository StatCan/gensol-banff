# Ajustement au prorata

* Exécution: *banff.prorate()*
* Type de fonction VDS: *Revue, Sélection, Traitement*
* Statuts d'entrée: *I-- (optionnel)*
* Statuts de sortie: *IPR*

## Description

Ajuste au prorata et arrondie les enregistrements pour satisfaire les règles de vérification spécifiées par l'utilisateur.

Contrairement aux autres procédures de Banff, les règles de vérification de prorate suivent des critères spécifiques: seules les égalités sont autorisées, et l'ensemble des règles doit avoir une structure hiérarchique menant à un total global. Par exemple:

$$
soustotal1 + soustotal2 = totalglobal \\
a + b + c = soustotal1 \\
d + e + f = soustotal2
$$

Chacune des règles de vérification doit consister en un ensemble de composantes *x<sub>i</sub>* qui somme à un total *y*, c'est-à-dire de la forme *x<sub>1</sub> + ... x<sub>n</sub> = y*. Les inégalités et les constantes ne sont pas permises. Pour chaque égalité non satisfaite, l'un des deux algorithmes d'ajustement au prorata (`basic` ou `scaling`) est appliqué dans le but d'établir, par transformation proportionnelle, l'égalité entre les composantes et le total. La procédure suit une approche descendante, commençant par le total global (qui n'est jamais modifié) et ajustant les composantes au besoin, jusqu'à ce que l'ensemble complet des règles de vérification soit satisfait. Les valeurs manquantes ne sont pas ajustées au prorata; elles sont mises à zéro durant la procédure puis remises comme manquantes par la suite. Les valeurs nulles ne sont jamais modifiées.

Caractéristiques additionnelles:

* Arrondissement automatique au nombre désiré de décimales.
* Limites optionnelles comme contraintes au changement relatif des valeurs durant l'ajustement au prorata.
* Contrôle sur les variables éligibles à l'ajustement au prorata.
* Option de limiter l'ajustement au prorata aux valeurs originales ou à celles imputées précédemment, que ce soit de manière globale ou par variable.
* Poids pour ajuster le changement relatif des variables de manière individuelle.  

Pour une description mathématique complète des méthodes de la procédure accompagnée d'exemples, se référer à [la description des fonctions](/docs/FR/Description%20des%20fonctions%20Banff.pdf).

## Données d'entrée et de sortie

La description des données d'entrée et de sortie est donnée ci-dessous. Banff supporte plusieurs formats pour les données qu'elles soient d'entrée ou de sortie; se référer au guide d'utilisateur pour plus d'information. 

| Données d'entrée  | Description |
| --------------- | ----------- |
| indata          | Données d'entrée. Obligatoire. |
| instatus        | Données des statuts d'entrée qui contient les statuts d'imputation I--.<br><br> Le fichier instatus est requis lorsque l'un des modificateurs est "ORIGINAL" ou "IMPUTED". Dans ce cas, la procédure identifiera les champs déjà imputés à l'aide des indicateurs d'imputation de la forme I--. Lors de l'ajustement au prorata, les valeurs avec un statut d'imputation FTI ou IDE sont considérées comme des valeurs originales.|

| Données de sortie | Description |
| --------------- | ----------- |
| outdata         | Données de sortie contenant les données imputées.<br><br> Noter que outdata contiendra les enregistrements imputés avec succès et les champs affectés. Les utilisateurs devront mettre à jour indata avec les valeurs provenant de outdata avant de continuer le processus de vérification et d'imputation des données. |
| outstatus       | Données des statuts de sortie identifiant les champs imputés avec le statut IPR et contenant leurs valeurs après imputation.   |
| outreject       | Données de sortie contenant les enregistrements pour lesquels l'ajustement au prorata a échoué.<br><br> Pour un enregistrement individuel, il y a plusieurs raisons pour lesquels l'ajustement au prorata pourrait ne pas aboutir; se référer aux notes plus bas pour plus d'information. |

Pour plus d'information sur le contenu des données de sortie, se référer au ().

## Paramètres

| Paramètre       | Type en Python          | Description                 | 
| ----------------| ----------------------  | --------------------------- |
| unit_id         | chaîne de caractères    | Identifier la variable clé (identifiant de l'unité) dans indata. Obligatoire.<br><br> unit_id devra être unique pour chaque enregistrement. Les enregistrements ayant une valeur manquante seront écartés avant le traitement. |
| edits           | chaîne de caractères    | Liste des règles de vérification que l'ajustement au prorata devra satisfaire. Obligatoire. <br><br> Se référer au guide d'utilisateur pour de l'information sur la manière de spécifier les règles de vérification. Contrairement aux autres procédures de Banff basées sur les règles de vérification, la procédure prorate pose certaines restrictions sur les règles de vérification qu'elle accepte; se référer aux notes plus bas pour plus d'information.|
| decimal         | entier          | Nombre de décimales utilisées dans l'algorithme d'arrondissement (entre 0 et 9). Par défaut: 0. <br><br> Le nombre de décimales spécifié devra être égal ou plus grand que le nombre de décimales trouvé le total.|
| method          | chaîne de caractères    | La méthode d'ajustement au prorata ("SCALING" ou "BASIC"). Par défaut: "BASIC". <br><br> Quand `method="BASIC"`, le signe de certaines variables peut changer durant l'ajustement au prorata. Quand `method="SCALING"`, les signes des variables ne changent pas. Si toutes les variables ont le même signe, (i.e., toutes positives ou toutes négatives), les deux méthodes produisent des résultats identiques. |
| modifier        | chaîne de caractères          | Modificateur global ("ALWAYS", "IMPUTED", "ORIGINAL") pour déterminer les valeurs à ajuster au prorata. Par défaut: "ALWAYS". <br><br> Spécifier `modifier="IMPUTED"` pour uniquement ajuster au prorata les données déjà imputées, et `modifier="ORIGINAL"` pour uniquement imputer les données originales. Si l'un d'eux est spécifié, alors instatus est requis. Les valeurs imputées sont identifiées grâce à leur statut d'entrée de la forme I--, à l'exception du statut IDE. Les valeurs avec un autre statut (incluant FTI et FTE) ou sans statut sont traitées comme des valeurs originales. Spécifier `modifier="ALWAYS"` pour imputer toutes les valeurs quel que soit leur statut. |
| lower_bound     | flottant        | Borne inférieure du changement relatif des variables. Par défaut: 0. <br><br> Le changement relatif d'une variable est égal à la valeur ajustée au prorata (après arrondissement) divisée par la valeur originale. Le paramètre `lower_bound` met une limite à ce ratio; l'ajustement au prorata ne sera pas effectué si le changement relatif est plus petit. Pour `method = "SCALING"`, la borne inférieure devra être plus grande ou égal à zéro; ceci fait en sorte que les valeurs ne peuvent pas changer de signe durant l'ajustement au prorata. Pour `method = "BASIC"`, l'utilisateur peut spécifier une borne inférieure négative si `accept_negative=True`. |
| upper_bound     | flottant        | Borne supérieure du changement relatif des variables. <br><br> Le changement relatif d'une variable est égal à la valeur ajustée au prorata (après arrondissement) divisée par la valeur originale. Le paramètre `upper_bound` met une limite à ce ratio; l'ajustement au prorata n'aura pas lieu si le changement relatif dépasse ce seuil. |
| verify_edits    | booléen         | Vérifie la cohérence des règles de vérification sans effectuer l'ajustement au prorata. Par défaut: False. |
| accept_negative | booléen         | Traite les valeurs négatives comme des valeurs valides. Par défaut: False. <br><br> Par défaut, la règle de positivité est ajoutée pour chaque variable dans la liste des règles de vérification; ce paramètre permet à l'utilisateur d'enlever cette restriction. L'utilisateur peut aussi ajouter la règle de positivité de manière individuelle pour chaque variable qui la requiert. |
| by              | booléen          | Variable(s) utilisée(s) pour partitionner indata en des groupes *by* pour un traitement indépendant. <br><br> L'ajustement au prorata est effectué sur une observation à la fois. Il en résulte que spécifier des variables de partition en groupes n'aura pas d'incidence sur les résultats. <br><br> Exemple: `by = "province industrie"` |
| prefill_by_vars | booléen         | Ajouter une ou plusieurs variables de partition aux données des statuts d'entrée pour améliorer la performance. Par défaut: True.  |
| presort         | booléen         | Trier les données d'entrée avant le traitement, et ce selon les exigences de la procédure. Par défaut: True. |
| no_by_stats     | booléen         | Réduire le journal de sortie en supprimant les messages spécifiques aux groupes de partition *by*. Par défaut: False. |

## Notes

### Enregistrements écartés

Les enregistrements suivants sont écartés avant le traitement:

* Enregistrements avec un identifiant unique manquant. (spécifié par `unit_id`.)
* Enregistrements avec une valeur manquante pour une variable des règles dans `edits`.
* Enregistrements avec une valeur négative pour une variable des règles dans `edits`, à moins que `accept_negative = True` soit spécifié.

### Syntaxe et restrictions des règles de vérification

Contrairement aux autre procédures de Banff, la procédure prorate se caractérise par sa propre syntaxe et restriction sur `edits`. Chaque règle de vérification doit avoir la forme suivante:

$$ w_1 x_1 : m_1 + ... + w_n x_n : m_n = y $$

où *x<sub>i</sub>* et *y* sont des variables dans indata, *w<sub>i</sub>* sont des poids et *m<sub>i</sub>* sont les modificateurs. Les variables *x<sub>i</sub>* correspondent les composantes et doivent se trouver du côté gauche de l'équation. Les variables *y* correspondent au total et doivent apparaître du côté droit de l'équation. Chaque règle de vérification doit se terminer par un point-virgule.

L'ensemble des règles de vérification doit avoir une structure hiérarchique. Ceci veut dire qu'il doit y avoir une variable, le *total global*, qui apparaît comme un total *y* dans une équation, mais n'apparaît comme composante *x<sub>i</sub>* dans aucune autre équation. Les composantes qui contribuent au *total global* peuvent agir comme *sous-totaux* pour d'autres équations, mais seulement une fois; les décompositions doubles ne sont pas permises. Il n'y a pas de limite au nombre de niveaux hiérarchiques spécifiés par l'utilisateur. Ces restrictions peuvent être résumées dans les points suivants:

* *Total global*: Une seule variable doit agir comme total mais jamais comme composante. Cette variable n'est jamais modifiée par la procédure.
* *Sous-total*: Chaque variable qui apparaît comme total dans une équation doit apparaître comme composante dans une autre équation.
* Dans l'ensemble des règles de vérification, chaque variable doit apparaître au plus deux fois: une fois comme composante *x<sub>i</sub>* et une fois comme total *y*.

Les poids *w<sub>i</sub>* sont optionnels. Ils doivent avoir des valeurs positives; si une variable n'a de poids assigné, un poids de un lui est appliqué. Les poids peuvent être utilisés pour contrôler l'ampleur du changement relatif dans chaque composante dû à l'ajustement au prorate, cette ampleur étant inversement proportionnelle au poids assigné. Les poids sont uniquement assignés aux composantes et non aux totaux.

Les codes des modificateurs sont optionnels. Comme pour les poids, ils peuvent être assignés aux variables de manière individuelle, spécifiés après le nom de la variable et précédés d'un deux-points. Les modificateurs déterminent les variables éligibles à l'ajustement au prorate. Ils se comportent de la même façon qu'un paramètre global `modifier`. Les codes permis sont les suivants:

* "A" (Always): La variable peut toujours être ajustée au prorata. (approche par défaut.)
* "N" (Never): La variable n'est jamais ajustée au prorata.
* "I" (Imputed): Seulement les valeurs précédemment imputées sont ajustées au prorata. Ces valeurs sont identifiées par le statut d'imputation I-- dans les données des statuts d'entrée, à l'exception de IDE dont les valeurs sont traitées comme des valeurs originales.
* "O" (Original): Seulement les valeurs originales sont ajustées au prorata. Ceci inclut les valeurs avec un statut IDE, FTI et FTE, aussi bien que toute valeur sans statuts dans instatus.

Si les codes des modificateurs "A" ou "I" sont utilisés, alors instatus doit être spécifié. Noter que les modificateurs spécifiés dans `edits` priment sur les modificateurs globaux spécifiés dans le paramètre `modifier`.

Exemple:

```python
prorate_call = banff.prorate(
    edits="""
    soustot1 + soustot2 + soustot3:N = totalglobal;
    2a + b = soustot1;
    c:I + d:I + e:I + f:I = soustot2;
    2g:O + 3h:A = soustot3;
    """,
    ... # autres paramètres
    )
```

Dans l'exemple ci-dessus:
* La variable `soustot3` n'est jamais ajustée au prorata.
* La variable `a` a un poids de 2.
* Les variables `c,d,e,f` sont ajustées au prorata uniquement pour les valeurs imputées précédemment.
* La variable `g` a un poids de 2 et seulement ses valeurs originales sont ajustées au prorata.
* La variable `h` a un poids de 3 et toutes ses valeurs sont ajustées au prorata, qu'elles soient originales ou imputées.

### Ajustement descendant au prorata

La procédure prorate commence avec le total global. Si les composantes de l'équation du total global ne somment pas à ce grand total, alors l'ajustement au prorata est effectué, ajustant les valeurs des composantes au besoin. (Le total global n'est jamais modifié). Ce processus est ensuite répété pour toute composante qui agît aussi comme sous-total, c'est-à-dire les composantes qui apparaissent comme total dans une autre équation. Noter que dans ce processus, chaque valeur est ajustée au prorata une seule fois, et ce lorsqu'elle apparaît comme composante.

### Arrondissement

La procédure prorate contient un algorithme d'arrondissement pour ajuster tous les champs au nombre correct de décimales tout en satisfaisant les règles de vérifications. L'utilisateur peut spécifier le nombre désiré de décimales grâce au paramètre `decimal`; la valeur par défaut est zéro (c'est-à-dire des nombres entiers). La valeurs de `decimal` doit être un entier dans [0,9], et doit être supérieure ou égale au nombre actuel de décimales trouvé dans le total.

Noter que l'ordre des variables composantes dans les équations de vérification peut affecter les résultats de l'algorithme d'arrondissement lorsque l'ampleur de l'ajustement requis au prorata ne peut pas être répartie de manière égale entre les composantes.

Lorsqu'il y a des composantes avec de très grandes sommes (c'est-à-dire plus grandes que 9 chiffres avec ou sans décimales), la procédure n'est pas capable de les ajuster précisément au prorata au total avec une précision de 9 décimales. Dans un tel cas, l'observation sera rejetée et listée avec une erreur de décimale dans les données de sortie outreject. Pour éviter cela, il est possible de réduire la valeur de `decimal`, par exemple en choisissant `decimal=8` ou moins.

### Enregistrements rejetés

Parfois, l'justement au prorata ne peut pas être effectué avec succès sur un individu. Dans pareil cas, la procédure prorate listera cet enregistrement dans les données de sortie outreject, accompagné de la raison de l'échec de l'ajustement au prorata. Cette raison peut être l'une des suivantes:

* Les valeurs ajustées au prorata sont en dehors des bornes spécifiées par l'utilisateur.
* La valeur d'échelle *k* est en dehors de l'intervalle [-1,1]. (Seulement pour `method="scaling"`)
* Le facteur *k* ne peut être calculé à cause d'une somme pondérée des colonnes ajustées égale à zéro.
* L'enregistrement contient des valeurs négatives et `accept_negative=True` n'est pas spécifié.
* Il n'y a pas de valeur éligible à l'ajustement au prorata à cause des modificateurs "ORIGINAL" ou "IMPUTED".
* L'utilisateur a spécifié un nombre de décimales inférieur à celui qui existe dans le total ajusté.