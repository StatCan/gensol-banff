# Spécification et analyse des règles de vérification

* Exécution: *banff.verifyedits()*
* Type de fonction VSD: *Néant*
* Statuts d'entrée: *Néant*
* Statuts de sortie: *Néant*

## Description

Vérifie la cohérence et la redondance des règles de vérification.

La procédure verifyedits n'analyse de données et n'effectue aucune fonction de VSD (revue, sélection, traitement). À la place, elle est utilisée pour vérifier qu'un ensemble de règles de vérification spécifié par l'utilisateur est cohérent, ainsi que pour identifier les règles redondantes, les variables déterministes et les égalités cachées. Une fois ce travail est effectué, un ensemble minimal de règles de vérification est déterminé. Avant d'utiliser les procédures basées sur les règles de vérification, à savoir errorloc, deterministic, donorimp et prorate, l'utilisateur est encouragé à analyser ses règles de vérification à l'aide de verifyedits. L'analyse effectuée est la suivante:

* Cohérence: la cohérence est vérifiée pour l'ensemble des règles de vérification, c'est-à-dire que les contraintes définissent une région d'acceptation non vide.
* Redondance: la liste des règles de vérification redondantes, c'est-à-dire les règles qui peuvent être enlevées sans que le région d'acceptation ne soir affectée.
* Bornes: les bornes implicites inférieures et supérieures pour chaque variable. Ceci montre également les variables déterministes, c'est-à-dire les variables qui peuvent prendre une seule valeur.
* Points extrémaux: l'ensembles des points extrémaux ou sommets de la région d'acceptation. 
* Égalités cachées: liste des égalités cachées et non spécifiées dans l'ensemble original des règles de vérification.
* Règles de vérification implicites: l'ensemble des règles de vérification implicites et non spécifiées dans l'ensemble original des règles.
* Règles de vérification minimales: l'ensemble des règles de vérification minimal requis pour définir la région d'acceptation généré par les règles originales.

Les sorties de verifyedits peuvent donne à l'utilisateur une meilleure idée de la région d'acceptation définies par les règles de vérification originales, et ce avant de pouvoir les utiliser dans les autres procédures. Même si les règles de vérification sont cohérentes, les sorties peuvent révéler des contraintes inattendues ou involontaires qui peuvent être corrigées en ajoutant, enlevant ou modifiant les règles de vérification. Utiliser l'ensemble minimal des règles de vérification peut aussi accroître la performance dans les autres procédures.

Pour une description mathématique complète des méthodes de la procédure accompagnée d'exemples, se référer à [la description des fonctions](/docs/FR/Description%20des%20fonctions%20Banff.pdf).

## Données d'entrée et de sortie

La procédure verifyedits ne prend pas de données d'entrée et n'en produit pas en sortie. L'information en sortie est directement imprimé au journal de l'exécution.

## Paramètres

| Parameter       | Python type          | Description                  | 
| ----------------| -------------------- | ---------------------------  |
| edits           | chaîne de caractères | Liste des règles de vérification. Obligatoire.<br><br> Exemple: `"Revenu - Depenses = Profit; Revenu >= 0; Depenses >= 0;"` |
| extremal        | entier               | Cardinalité maximale des sommets montrés. Par défaut: 0.<br><br> La cardinalité représente le nombre de coordonnées non nulles d'un sommet. Aucun sommet ne sera généré lorsque `accept_negative=True`.                                |
| imply           | entier               | Nombre de règles implicites à générer. Par défaut: 0. |
| accept_negative | booléen              | Traite les valeurs négatives comme des valeurs valides. Par défaut: False. <br><br> Par défaut, la règle de positivité est ajoutée pour chaque variable dans la liste des règles de vérification; ce paramètre permet à l'utilisateur d'enlever cette restriction. L'utilisateur peut aussi ajouter la règle de positivité de manière individuelle pour chaque variable qui la requiert. |