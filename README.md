[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![GitHub Scan](https://github.com/StatCan/gensol-banff/actions/workflows/scan.yml/badge.svg?event=release&status=success)](https://github.com/StatCan/gensol-banff/actions/workflows/scan.yml?query=event%3Arelease+is%3Asuccess)
[![GitHub Build Linux](https://github.com/StatCan/gensol-banff/actions/workflows/build_linux.yml/badge.svg?event=release&status=success)](https://github.com/StatCan/gensol-banff/actions/workflows/build_linux.yml?query=event%3Arelease+is%3Asuccess)
[![GitHub Build Windows](https://github.com/StatCan/gensol-banff/actions/workflows/build_windows.yml/badge.svg?event=release&status=success)](https://github.com/StatCan/gensol-banff/actions/workflows/build_windows.yml?query=event%3Arelease+is%3Asuccess)

[(Le message en français suit)](#aperçu)

# Banff Procedures

Welcome to the project repository for the Banff procedures. Please consider watching this repo to be notified of updates.

## Overview

Banff is a statistical package developed by Statistics Canada, consisting of nine modular procedures performing various statistical data editing (SDE) functions, including imputation. Some general notes about Banff:

* Most of the SDE methods included in Banff are designed for economic surveys, and in particular numerical variables such as revenue and employee counts. Banff does not currently include methods recommended for the imputation of categorical or ordinal data.
* Banff includes a number of methods designed for data whose variables are constrained by linear relationships, also commonly referred to as linear edit rules or simply edits. This includes procedures that review data with respect to the edits, choose which variables to impute when the edits fail, and impute records to ensure that all edits are satisfied.
* While each Banff procedure can be run independently, they follow a modular template and can be run in sequence as part of a larger SDE process flow. Outputs from one procedure act as natural inputs for subsequent procedures.
* Banff uses status flags to track metadata such as selection and imputation flags. These status flags allow the Banff procedures to pass information from one procedure to another, and also serve as a log of the overall SDE process.

When running Banff procedures in sequence as part of an SDE process flow, users are responsible for input and output between steps. An additional package, the **Banff Processor**, is a metadata-driven utility designed specifically for large-scale SDE production, incorporating the Banff procedures, and handling all intermediate data management.

The Banff user guide often uses terminology from the [Generic Statistical Data Editing Model (GSDEM)](https://statswiki.unece.org/spaces/sde/pages/117771706/GSDEM). Users are encouraged to reference the GSDEM for common terminology regarding SDE concepts.

## Installation

The Banff package is installed using the `pip` package installer for Python. For general information about `pip`, please see https://pip.pypa.io/en/stable/cli/pip_install/.

To install the latest release directly from PyPi:

```shell
pip install banff
```

Alternatively, you can find the Banff packages on Statistics Canada’s GitHub release page : https://github.com/StatCan/gensol-banff/releases.

Download the Windows or Linux binary for your environment from the release page. Then, follow the installation instructions below :

```shell
pip install --no-cache-dir package_name.whl 
```

## Handling of Sensitive Statistical Information

Please respect your organization's policies regarding the handling for Sensitive Statistical Information. Output files may require encryption and need to stored in a secure location. The consumer of the package is responsible for ensuring sensitive data is processed in a secure environment.

## Documentation

Documentation in both English and French can be found in [this folder](./docs)  and are detailed below:

| Document(s)            | Description                                                                                   |
| ---------------------- | --------------------------------------------------------------------------------------------- |
| [User guide](./docs/EN/user_guide.md) | An overview of the Banff package, detailing concepts and parameters common to all procedures. |
| [Procedure guides](./docs/EN/Procedure%20Guides/) | Guides for each individual Banff procedure. |
| [Output tables](./docs/EN/output_tables.md) | Description of output tables produced by each procedure. |
| [Functional description](./docs/EN/Banff%20Functional%20Description.pdf) | A detailed guide of the statistical methods used in Banff, with examples. |
| [SAS migration guide](./docs/EN/sas_migration_guide.md) | A guide for existing users switching from a SAS-based version of Banff. |
| [SAS migration tutorial](./docs/EN/sas_migration_tutorial.md) | A step-by-step tutorial demonstrating how to replicate a typical SAS-based call to Banff using the Python version. |
| [Release notes](./docs/EN/release-notes.md) | Detailed list of changes in each new version of Banff, starting with version 3.0. |

## List of procedures

The following procedures are included in the Banff package. Please follow the links below to access the individual procedure guides.

* [Verify Edits](./docs/EN/Procedure%20Guides/verifyedits.md): Checks the edits for consistency and redundancy.
* [Edit Statistics](./docs/EN/Procedure%20Guides/editstats.md): Produces edit summary statistics tables on records that pass, miss or fail each consistency edit
* [Outlier Detection](./docs/EN/Procedure%20Guides/outlier.md): Identifies outlying observations using Hidiroglou-Berthelot or Sigma-Gap methods.
* [Error Localization](./docs/EN/Procedure%20Guides/errorloc.md): For each record, selects the minimum number of variables to impute such that each observation can be made to pass all edits.
* [Deterministic Imputation](./docs/EN/Procedure%20Guides/deterministic.md): Performs imputation when only one combination of values permits the record to pass the set of edits.
* [Donor Imputation](./docs/EN/Procedure%20Guides/donorimp.md): Performs nearest neighbour donor imputation such that each imputed record satisfies the specified post-imputation edits.
* [Estimator Imputation](./docs/EN/Procedure%20Guides/estimator.md): Performs imputation using estimation functions or linear regression estimators.
* [Prorating](./docs/EN/Procedure%20Guides/prorate.md): Prorates and rounds records to satisfy user-specified edits.
* [Mass Imputation](./docs/EN/Procedure%20Guides/massimp.md): Performs donor imputation for a block of variables using a nearest neighbour approach or random selection.

## Examples

Sample programs for each procedure are available in [this folder](./Python/sample_programs).

---

# Procédures de Banff

Bienvenue dans le référentiel du projet pour les procédures Banff. Surveiller ce référentiel permettra de se tenir informé des mises à jour.

## Aperçu

Banff est un progiciel statistique développé par Statistique Canada, et consistant en neuf procédures modulaires qui permettent d'implémenter plusieurs fonctions de **vérification statistique des données** (VSD), incluant l'imputation. Quelques considérations générales à propos de Banff :

* La plupart des méthodes de VSD incluses dans Banff sont conçues pour les enquêtes économiques, et en particulier pour des variables numériques comme le revenu ou le nombre d'employés. À l'heure actuelle, Banff n'offre pas de méthode recommandée pour l'imputation de données catégoriques ou ordinales.
* Banff contient un nombre de méthodes conçues pour les données dont les variables ont des contraintes sous la forme de relations linéaires. Ces relations sont aussi appelées *règles de vérification linéaires* ou simplement *règles de vérification*. Ceci concerne les procédures qui vérifient les données par rapport aux règles de vérification, choisissent les variables à imputer lorsque la vérification échoue, et imputent des enregistrements pour s'assurer que toutes les règles de vérification soient satisfaites.
* Bien que chaque procédure Banff s'exécute des manière indépendante, elles suivent toutes un gabarit modulaire, et peuvent être roulées séquentiellement dans le cadre d'un processus plus général de VSD. 
* Banff utilise les *codes de statut* comme les statuts de sélection et d'imputation pour suivre les métadonnées. Ces codes de statut permettent aux procédures Banff de relayer l'information d'une procédure à l'autre, et sont également consignés dans le journal du processus général de VSD.

Lorsque les procédures Banff sont exécutées séquentiellement dans le cadre d'un processus de VSD, l'utilisateur est responsable des éléments d'entrée et de sortie entre les étapes. Un paquet cousin, le **processeur Banff**, est une application basée sur les métadonnées, et conçue spécifiquement pour une production VSD à grande échelle, intégrant les procédures Banff et se chargeant de la gestion des données intermédiaires.

Le guide d'utilisation de Banff utilise souvent la terminologie du [Modèle générique de vérification statistique des données (MGVSD)](https://statswiki.unece.org/spaces/sde/pages/117771706/GSDEM). Les utilisateurs sont encouragés à se référer au MGVSD pour la terminologie commune relative aux concepts de VSD.

## Installation

Le paquet Banff est installé à l'aide du programme d'installation du paquet `pip` de Python. Pour de l'information générale sur `pip`, veuillez consulter https://pip.pypa.io/en/stable/cli/pip_install/.

Pour installer la dernière version directement depuis PyPi:

```shell
pip install banff
```

Alternativement, vous pouvez trouver les packages Banff sur la page de publication GitHub de Statistique Canada : https://github.com/StatCan/gensol-banff/releases.

Téléchargez le binaire Windows ou Linux pour votre environnement depuis la page de publication. Ensuite, suivez les instructions d’installation ci-dessous :

```shell
pip install --no-cache-dir package_name.whl 
```

## Gestion des informations statistiques sensibles

Veuillez respecter les politiques de votre organisation concernant la gestion des informations statistiques sensibles. Les fichiers de sortie peuvent nécessiter un cryptage et doivent être stockés dans un emplacement sécurisé. Le consommateur du package est responsable de s'assurer que les données sensibles sont traitées dans un environnement sécurisé.

## Documentation

La documentation en français et en anglais se trouve dans [ce dossier](./docs) et est détaillée ci-dessous :

| Document(s) | Description |
| ------------------------- | --------------------------------------------------------------------------------------------- |
| [Guide de l'utilisateur](./docs/FR/user_guide.md)| Aperçu du paquet Banff, détaillant les concepts et les paramètres communs à toutes les procédures. |
| [Guides de procédure](./docs/FR/Procedure%20Guides/)| Guide pour chaque procédure Banff. |
| [Description des fonctions](./docs/EN/Banff%20Functional%20Description.pdf)| Guide détaillé des méthodes statistiques utilisées dans Banff, avec des exemples.|
| [Guide de migration](./docs/FR/sas_migration_tutorial.md)| Guide pour les utilisateurs existants passant de la version de Banff basée sur SAS. |
| [Notes de version](./docs/FR/release-notes.md)| Liste détaillée des modifications apportées à chaque nouvelle version de Banff, à partir de la version 3.0. |

L'information sur les versions SAS de Banff précédemment publiées sont disponibles dans confluence: https://confluenceb.statcan.ca/display/GENSYS/Banff

## Liste des procédures

Les procédures suivantes sont incluses dans le paquet Banff. Veuillez suivre les liens ci-dessous pour accéder au guide de la procédure d'intérêt :

* [Spécification et analyse des règles de vérification](./docs/FR/Procedure%20Guides/verifyedits.md) : vérifie la cohérence et la redondance des règles de vérification.
* [Statistiques sur les règles de vérification](./docs/FR/Procedure%20Guides/editstats.md) : produit des statistiques sommaires sur les enregistrements qui satisfont ou ne satisfont pas (à cause d'une valeur manquante ou non manquante) chaque règle de vérification.
* [Détection des valeurs aberrantes](./docs/FR/Procedure%20Guides/outlier.md) : identifie les valeurs aberrantes à l'aide de la méthode Hidiroglou-Berthelot ou la méthode de l'écart-sigma.
* [Localisation des erreurs](./docs/FR/Procedure%20Guides/errorloc.md) : sélectionne, pour chaque enregistrement, le nombre minimal de variables à imputer tel que chaque observation puisse passer toutes les règles de vérification.
* [Imputation déterministe](./docs/FR/Procedure%20Guides/deterministic.md) : effectue l'imputation lorsqu'une seule combinaison de valeurs permet à l'enregistrement de passer l'ensemble des vérifications.
* [Imputation par donneur](./docs/FR/Procedure%20Guides/donorimp.md) : effectue l'imputation par enregistrement donneur avec la méthode du voisin le plus proche tel que chaque enregistrement imputé satisfasse les règles de vérification post-imputation.
* [Imputation par estimation](./docs/FR/Procedure%20Guides/estimator.md) : effectue l'imputation en utilisant des fonctions d'estimation et/ou des estimateurs par régression linéaire.
* [Ajustement au prorata](./docs/FR/Procedure%20Guides/prorate.md) : ajuste au prorata et arrondie les enregistrements pour satisfaire les règles de vérification spécifiées par l'utilisateur.
* [Imputation massive](./docs/FR/Procedure%20Guides/massimp.md) : Effectue l'imputation par donneur pour un bloc de variables avec l'approche du plus proche voisin ou une sélection aléatoire.

## Exemples

Un exemple d'appel de chaque procédure est disponible dans [ce dossier](./Python/sample_programs).
