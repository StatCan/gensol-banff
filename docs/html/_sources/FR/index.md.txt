# Banff

Banff est un progiciel statistique développé par Statistique Canada, et consistant en neuf procédures modulaires qui permettent d'implémenter plusieurs fonctions de **vérification statistique des données** (VSD), incluant l'imputation. Quelques considérations générales à propos de Banff :

* La plupart des méthodes de VSD incluses dans Banff sont conçues pour les enquêtes économiques, et en particulier pour des variables numériques comme le revenu ou le nombre d'employés. À l'heure actuelle, Banff n'offre pas de méthode recommandée pour l'imputation de données catégoriques ou ordinales.
* Banff contient un nombre de méthodes conçues pour les données dont les variables ont des contraintes sous la forme de relations linéaires. Ces relations sont aussi appelées *règles de vérification linéaires* ou simplement *règles de vérification*. Ceci concerne les procédures qui vérifient les données par rapport aux règles de vérification, choisissent les variables à imputer lorsque la vérification échoue, et imputent des enregistrements pour s'assurer que toutes les règles de vérification soient satisfaites.
* Bien que chaque procédure Banff s'exécute des manière indépendante, elles suivent toutes un gabarit modulaire, et peuvent être roulées séquentiellement dans le cadre d'un processus plus général de VSD. 
* Banff utilise les *codes de statut* comme les statuts de sélection et d'imputation pour suivre les métadonnées. Ces codes de statut permettent aux procédures Banff de relayer l'information d'une procédure à l'autre, et sont également consignés dans le journal du processus général de VSD.

Lorsque les procédures Banff sont exécutées séquentiellement dans le cadre d'un processus de VSD, l'utilisateur est responsable des éléments d'entrée et de sortie entre les étapes. Un paquet cousin, le **processeur Banff**, est une application basée sur les métadonnées, et conçue spécifiquement pour une production VSD à grande échelle, intégrant les procédures Banff et se chargeant de la gestion des données intermédiaires.

Le guide d'utilisation de Banff utilise souvent la terminologie du [Modèle générique de vérification statistique des données (MGVSD)](https://statswiki.unece.org/spaces/sde/pages/117771706/GSDEM). Les utilisateurs sont encouragés à se référer au MGVSD pour la terminologie commune relative aux concepts de VSD.


## User Documentation

- [Guide de l'utilisateur](./user_guide.md) - *Informations détaillées sur l'utilisation du Banff basé sur Python*
- [Guide de migration](./sas_migration_guide.md) - *Informations détaillées sur l'utilisation des différences entre Banff basés sur Python et SAS*
- [Tutoriel de migration](./sas_migration_tutorial.md) - *Un tutoriel démontrant les équivalents python des étapes SAS impliquées dans l'exécution d'un programme Banff*
- [Guide des procédures](./Procedure%20Guides/index.md) - *Descriptions détaillées pour chacune des procédures individuelles de Banff*
- [Données de sortie](./output_tables.md) - *Description des tableaux de sortie générés par chaque procédure*
- [Notes de version](./release-notes.md) - *Résumé des changements*