# Algorithmes dans la prcédure Estimator

Les données d'entrée `inalgorithm` et `inestimator` définissent le modèle utilisé pour l'imputation (moyenne, valeur historique, régression linéaire, ..) et spécifient certains paramètres et options. Les données `inestimator` sont obligatoires, alors que `inalgorithm` sont seulement requises lorsque l'on utilise un algorithme personnalisé au lieu d'un algorithme prédéfini parmi les 20 algorithmes disponibles dans la procédure. 

Les algorithmes utilisés par la procédure `estimator` peuvent être classés en deux catégories :
* Les fonctions d'estimation (EF) sont des expressions mathématiques spécifiées par l'utilisateur et faisant intervenir des données auxiliaires et/ou historiques; ces expressions utilisent les opérations d'addition, soustraction, multiplication, division et élévation à une puissance. Pour chaque variable de la fonction, l'utilisateur peut spécifier ce qu'il veut utiliser entre la valeur individuelle de l'enregistrement cible, et la valeur moyenne des enregistrements au sein de la classe d'imputation, et s'il veut utiliser les données courantes ou celles historiques. Chaque paramètre de la fonction d'estimation doit être spécifié par l'utilisateur; aucune modélisation n'est effectuée par la procédure. 
* Les estimateurs par régression linéaire (LR) utilisent des modèles de régression linéaire pour imputer les valeurs manquantes. L'utilisateur spécifie les variables auxiliaires à inclure dans le modèle, ainsi que la période à utiliser et les exposants à appliquer. Les coefficients de régression sont alors estimés au sein de chaque classe d'imputation, avant que le modèle linéaire obtenu ne soit utilisé pour imputer la variable d'intérêt pour l'enregistrement en question. 

Ce document explique comment spécifier un algorithme personnalisé des deux types, et liste les algorithmes prédéfinis dans la procédure.

## Interaction entre inestimator et inalgorithm

Qu'il soit prédéfini ou personnalisé, un algorithme peut être évoqué dans les données `inestimator` dans le but d'être utilisé dans l'imputation d'une ou plusieurs variables. Avec au moins une ligne par variable à imputer, `inestimator` contient les colonnes qui définissent la méthode et les paramètres d'imputation. En particulier, trois colonnes sont utilisées pour spécifier les éléments reliés à l'algorithme:

- `fieldid`: variable à imputer.
- `algorithmname` : nom d'algorithme à utiliser, qu'il soit prédéfini ou personnalisé.
- `auxvariables` : variables auxiliaires utilisées dans l'algorithme. Plusieurs variables auxiliaires peuvent être utilisées dans chaque imputation (chaque variable auxiliaire doit être citée une seule fois même si ses valeurs historiques et actuelles sont utilisée); elles doivent être séparées de virgules. `auxvariables` doit contenir le même nombre de variables auxiliaires que celles utilisées dans l'algorithme; leur ordre d'apparition est important puisqu'il est utilisé pour les assigner aux variables citées dans la formule de l'algorithme. `auxvariables` doit être laissé vide lorsque l'algorithme qui lui est associée ne contient aucune variable auxiliaire (comme dans le cas de l'imputation par la valeur historique du champ à imputer). 

## Algorithme personnalisé

À l'instar des algorithmes prédéfinis, un algorithme personnalisé peut prendre la forme d'une fonction d'estimation (type EF) ou d'une imputation par régression linéaire (type LR). Les deux types utilisent des paramètres substituables pour définir la formule de l'algorithme.

### Paramètres substituables et leurs attributs

Le nom de l'algorithme, le type, le code du statut et le formule sont spécifiés à l'aide des colonnes `algorithmname`, `type`, `status` et `formula` respectivement. Aucune distinction n'est faite entre lettres minuscules et majuscules. Dans `formula`, il est seulement possible d'utiliser les paramètres substituables `fieldid` et `aux<n>` où 'n' est un entier positif; `aux<n-1>` doit être utilisé dans la formule lorsque `aux<n>` est utilisé.

- `fieldid` est un substituable du nom de la variable à imputer spécifiée dans `inestimator`.
- `aux<n>` est un substituable du nom de la variable auxiliaire spécifiée dans `inestimator`. La première variable dans `auxvariables` remplacera le substituable `aux1`, la seconde, `aux2` et ainsi de suite. Le nombre de variables dans `auxvariables` doit être égal au nombre de différents substituables `aux<n>`.

Selon le type d'algorithme utilisé (EF ou LR), les formats utilisés avec les substituables (`aux<n>` et/ou `fieldid`) sont les suivants: 

- **Algorithmes de type EF** :
    - `<placeholder>(period, aggregation)`;
    - `<placeholder>(aggregation, period)`.
- **Algorithmes de type LR** :
    - `<placeholder>(period)`.

Les options `period` et `aggregation` sont appelées les attributs des substituables. Les valeurs possibles pour `period` sont `c` pour les données courantes et `h` pour celles historiques. Quant à `aggregation`, elle prend `v` pour utiliser la valeur de la variable pour l'observation, et `a` pour la moyenne de la variable au niveau des observations acceptables. Noter que les algorithmes de type LR sont un cas particulier puisqu'ils utilisent seulement les valeurs des observations (et non les moyennes), ce qu'il fait que l'attribut `aggregation` ne s'applique pas dans leur cas.

### Algorithmes de type EF

Une fonction d'estimation (EF) est une expression mathématique qui peut inclure des constantes, des variables de données courantes et/ou historiques, et des moyennes de données courantes et/ou historiques. Ces moyennes sont calculées à partir d'un ensemble d'observations dites acceptables (grosso modo, une observation acceptable est une observation pour laquelle toutes les variables impliquées dans le calcul de moyennes sont disponibles, et où les valeurs imputés et/ou aberrantes sont exclues si demandé par l'utilisateur). Les opérations arithmétiques sont l'addition (+), la soustraction (-), la multiplication (*), la division (/), l'élévation à une puissance (^) et les parenthèses.

La valeur par défaut de `period` est `c` (données courantes) et celle relative à `aggregation` est `v` (valeur individuelle). Par conséquent :

- `aux1`, `aux1(c)`, `aux1(v)` et `aux1(c, v)` sont équivalents.
- `aux1(h)` et `aux1(v, h)` sont équivalents.
- `aux1(a)` et `aux1(c, a)` sont équivalents.

Noter que `fieldid`, `fieldid(c)`, `fieldid(v)` et `fieldid(c,v)` ne peuvent être utilisés dans `formula` puisqu'ils correspondent tous à la valeur à imputer.

**Les exposants** peuvent être appliqués aux constantes (différentes de 0), aux substituables et aux expressions :
- L'exposant pour une constante doit la suivre : `3^4`.
- L'exposant d'un substituable suit les fermeture de parenthèses des attributs : `aux1^3`, `aux1(c)^3` et `aux1(v)^3` sont équivalents.
- L'exposant d'une expression requiert que l'expression soit entre parenthèses : `(aux1(h) + aux2(h))^2`.

De plus, l'exposant doit être une constante (nombre non nul). Un substituable ou une expression (même si son résultat est constant) ne peuvent être utilisés comme exposant. Donc, `aux1^aux2` et `aux1^(2+1)` ne sont pas permis.

### Algorithmes de type LR

Imputation par régression qui consiste à imputer une variable $y$<sub>$i$</sub> par l'estimation par régression linéaire de la forme :

$$
\hat{y_i} = \hat{\beta_0} + \hat{\beta_1} x_{i_1 T_1}^{p_1} + \hat{\beta_2} x_{i_2 T_2}^{p_2} + ... + \hat{\beta_m} x_{i_m T_m}^{p_m} + \hat{\epsilon_i}  
$$

où : $T$<sub>$j$</sub> correspond aux périodes courante ou historique et $p$<sub>$j$</sub> sont les exposants. La variable $y$<sub>$j$</sub> à imputer est la variable dépendante du modèle, et les variables auxiliaires $x$<sub>$ij$</sub> sont les variables indépendantes ou les régresseurs. $\hat\beta$<sub>$j$</sub> sont les coefficients de régression, obtenues comme solution de la méthode des moindres carrés. $\hat\epsilon$<sub>$i$</sub> est un terme d'erreur aléatoire, qui peut être ajouté au modèle pour introduire une certaine variabilité dans les valeurs estimées de $y$<sub>$i$</sub>. 
Noter que $\beta$<sub>$0$</sub>, qui est l'ordonnée à l'origine de la droite de régression, peut être omise du modèle.

Puisque l'attribut `aggregation` n'est pas applicable aux algorithmes de type LR, seule l'attribut `period` est utilisé, avec une valeur par défaut de `c` (données courantes.). Par exemple, `aux1` et `aux1(c)` sont équivalents.

La formule prend les éléments suivants séparés par des virgules :
- Ordonnée à l'origine : ajouter `intercept` pour utiliser une régression linéaire avec ordonnée à l'origine.
- Régresseurs : Liste des variables à inclure dans la régression linéaire, séparées par des virgules. Ces régresseurs peuvent prendre la forme de variables auxiliaires (`aux1`, `aux2`, ..) ou de valeurs historiques de la variable à imputer (`fieldid(h)`). Les exposants peuvent être appliqués aux régresseurs en utilisant le symbole `^` après les parenthèses fermées de l'attribut du substituable (`aux1^3` et `aux1(c)^3` sont équivalents).

**L'exposant** doit être une constante (nombre non nul) pour être appliqué aux substituables. Un substituable ou une expression (même si son résultat est constant) ne peuvent être utilisés comme exposant. Donc, `aux1^aux2` et `aux1^(2+1)` ne sont pas permis.

Le tableau suivant montre des exemples de régressions linéaires, et la manière dont elles peuvent être traduites en formules :

| Régression linéaire| Formule d'algorithme |
| -----------| -------|
| $y = \beta$<sub>$1$</sub> $* x$<sub>$1c$</sub>| $aux1$ |
| $y = \beta$<sub>$0$</sub> $+ \beta$<sub>$1$</sub> $* x$<sub>$h$</sub>| $intercept, aux1(h)$  |
| $y = \beta$<sub>$0$</sub> $+ \beta$<sub>$1$</sub> $* y$<sub>$h$</sub>| $intercept, fieldid(h)$  |
| $y = \beta$<sub>$1$</sub> $* x$<sub>$1c$</sub> $+ \beta$<sub>$2$</sub> $* x$<sub>$2h$</sub> $+ \beta$<sub>$3$</sub> $* x$<sub>$3c$</sub> | $aux1, aux2(h), aux3$ |
| $y = \beta$<sub>$1$</sub> $* x$<sub>$1c$</sub> $+ \beta$<sub>$2$</sub> $* x$<sub>$1c$</sub><sup>$2$</sup> $+ \beta$<sub>$3$</sub> $* x$<sub>$1c$</sub><sup>$3$</sup>  | $aux1, aux1$^2 $, aux1$^3  |

Noter qu'il est possible d'utiliser `fieldid(h)` dans la formule de l'algorithme, contrairement au `fieldid` et `fieldid(c)` puisque les deux correspondent à la valeur à imputer.

## Algorithmes prédéfinis 

Un nom et un code de statut d'imputation sont associés à chaque algorithme prédéfini. Dans la suite, la structure de substituables et leurs attributs exposée plus haut est utilisée pour montrer la formule de chaque algorithme prédéfini, accompagnée d'une brève description. 

### Algorithmes de type EF
 
| Nom d'algorithme | Status d'imputation | Formule d'algorithme | Value imputée | 
| -----------| -------| --------| ------------  |
| AUXTREND   | IAT    | $fieldid(h,v)*(aux1(c,v)/aux1(h,v))$ | Ajustement par la tendance calculé à partir d'une variable auxiliaire et appliqué à la valeur historique du champ à imputer.|
| AUXTREND2  | IAT2   | $fieldid(h,v)*(aux1(c,v)/aux1(h,v) + aux2(c,v)/aux2(h,v))/2$ | Ajustement moyen par la tendance calculé à partir de deux variables auxiliaires et appliqué à la valeur historique du champ à imputer. |
| CURAUX     | ICA    | $aux1(c,v)$  | Valeur courante d'une variable auxiliaire pour l'enregistrement à imputer. |
| CURAUXMEAN | ICAM   | $aux1(c,a)$  | Moyenne courante d'une variable auxiliaire. |
| CURMEAN    | ICM    | $fieldid(c,a)$  | Moyenne courante de la variable à imputer. |
| CURRATIO   | ICR    | $fieldid(c,a) * (aux1(c,v) / aux1(c,a))$ | Estimation par quotient courant calculée à partir d'une variable auxiliaire et appliquée à la moyenne courante de la variable à imputer.|
| CURRATIO2  | ICR2   | $fieldid(c,a) * (aux1(c,v) / aux1(c,a) + aux2(c,v) / aux2(c,a))/2$ | Estimation par quotient moyen courant calculée à partir de deux variables auxiliaires et appliquée à la moyenne courante de la variable à imputer.| 
| CURSUM2    | ISM2   | $aux1 + aux2$  | Somme des valeurs actuelles de deux variables auxiliaires. |
| CURSUM3    | ISM3   | $aux1 + aux2 + aux3$ | Somme des valeurs actuelles de trois variables auxiliaires. |
| CURSUM4    | ISM4   | $aux1 + aux2 + aux3 + aux4$  | Somme des valeurs actuelles de quatre variables auxiliaires. |
| DIFTREND   | IDT    | $fieldid(h,v)* (fieldid(c,a) / fieldid(h,a))$  | Ajustement par la tendance de la moyenne calculé à partir de la variable à imputer et appliqué à sa valeur historique. |
| PREAUX     | IPA    | $aux1(h,v)$ | Valeur historique d'une variable auxiliaire pour l'enregistrement à imputer. |
| PREAUXMEAN | IPAM   | $aux1(h,a)$  | Moyenne historique d'une variable auxiliaire. |
| PREMEAN    | IPM    | $fieldid(h,a)$ | Moyenne historique de la variable à imputer.  |
| PREVALUE   | IPV    | $fieldid(h,v)$ | Valeur historique du champ à imputer. |

### Algorithmes de type LR

| Nom d'algorithme | Status d'imputation | Formule d'algorithme | Value imputée | 
| -----------| -------| --------| ------------  |
| CURREG   | ILR1    | $intercept, aux1(c)$ | Régression linéaire simple basée sur les valeurs courantes d'une variable auxiliaire.|
| CURREG_E2  | ILRE   | $intercept, aux1(c), aux1(c)$^2 | Régression linéaire basée sur les valeurs courantes d'une variable auxiliaire et leurs carrés. |
| CURREG2     | ILR2    | $intercept, aux1(c), aux2(c)$  | Régression linéaire basée sur les valeurs courantes de deux variables auxiliaires. |
| CURREG3 | ILR3   | $intercept, aux1(c), aux2(c), aux3(c)$  | Régression linéaire basée sur les valeurs courantes de trois variables auxiliaires. |
| HISTREG    | IHLR    | $intercept, fieldid(h)$  | Régression linéaire simple basée sur les valeurs historiques de la variable à imputer. |