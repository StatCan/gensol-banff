[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![GitHub Scan](https://github.com/StatCan/gensol-banff/actions/workflows/scan.yml/badge.svg?event=release&status=success)](https://github.com/StatCan/gensol-banff/actions/workflows/scan.yml?query=event%3Arelease+is%3Asuccess)
[![GitHub Build Linux](https://github.com/StatCan/gensol-banff/actions/workflows/build_linux.yml/badge.svg?event=release&status=success)](https://github.com/StatCan/gensol-banff/actions/workflows/build_linux.yml?query=event%3Arelease+is%3Asuccess)
[![GitHub Build Windows](https://github.com/StatCan/gensol-banff/actions/workflows/build_windows.yml/badge.svg?event=release&status=success)](https://github.com/StatCan/gensol-banff/actions/workflows/build_windows.yml?query=event%3Arelease+is%3Asuccess)

[(Le message en français suit)](#aperçu)

# Banff Procedures

Welcome to the project repository for the Banff procedures, please consider watching this repo to be notified of updates.

## Overview

Banff is a package developed by Statistics Canada, consisting of nine modular procedures performing various statistical data editing (SDE) functions:

* Verify Edits: Checks the edits for consistency and redundancy.
* Edit Statistics: Produces edit summary statistics tables on records that pass, miss or fail each consistency edit
* Outlier Detection: Identifies outlying observations using Hidiroglou-Berthelot or Sigma-Gap methods.
* Error Localization: For each record, selects the minimum number of variables to impute such that each observation can be made to pass all edits.
* Deterministic Imputation: Performs imputation when only one combination of values permits the record to pass the set of edits.
* Donor Imputation: Performs nearest neighbour donor imputation such that each imputed record satisfies the specified post-imputation edits.
* Estimator Imputation: Performs imputation using estimation functions or linear regression estimators.
* Prorating: Prorates and rounds records to satisfy user-specified edits.
* Mass Imputation: Performs donor imputation for a block of variables using a nearest neighbour approach or random selection.

Some general notes about Banff:

* Most of the SDE methods included in Banff are designed for economic surveys, and in particular numerical variables such as revenue and employee counts. Banff does not currently include methods recommended for the imputation of categorical or ordinal data.
* Banff includes a number of methods designed for data whose variables are constrained by linear relationships, also commonly referred to as linear edit rules or simply edits. This includes procedures that review data with respect to the edits, choose which variables to impute when the edits fail, and impute records to ensure that all edits are satisfied.
* While each Banff procedure can be run independently, they follow a modular template and can be run in sequence as part of a larger SDE process flow. Outputs from one procedure act as natural inputs for subsequent procedures.
* Banff uses status flags to track metadata such as selection and imputation flags. These status flags allow the Banff procedures to pass information from one procedure to another, and also serve as a log of the overall SDE process.

When running Banff procedures in sequence as part of an SDE process flow, users are responsible for input and output between steps. An additional package, the **Banff Processor**, is a metadata-driven utility designed specifically for large-scale SDE production, incorporating the Banff procedures, and handling all intermediate data management.

The Banff user guide often uses terminology from the [**Generic Statistical Data Editing Model** (GSDEM)](https://statswiki.unece.org/spaces/sde/pages/117771706/GSDEM). Users are encouraged to reference the GSDEM for common terminology regarding SDE concepts.


## Installation

Banff is installed using the `pip` package installer for Python. For general information about `pip`, please see https://pip.pypa.io/en/stable/cli/pip_install/.

You can find the Banff packages on Statistics Canada’s GitHub release page : https://github.com/StatCan/gensol-banff/releases. A PyPI release will be available soon.

Download the Windows or Linux binary for your environment from the release page. Then, follow the installation instructions below :

```shell
pip install --no-cache-dir package_name.whl 
```

## Handling of Sensitive Statistical Information

Please respect your organization's policies regarding the handling for Sensitive Statistical Information. Output files may require encryption and need to stored in a secure location. The consumer of the package is responsible for ensuring sensitive data is processed in a secure environment.

## Documentation

Documentation in both English and French can be found in [this folder](./docs)  and are detailed below:

<mark>Add links in the first column to relevant document or folder.</mark>

| Document(s)            | Description                                                                                   |
| ---------------------- | --------------------------------------------------------------------------------------------- |
| User guide             | An overview of the Banff package, detailing concepts and parameters common to all procedures. |
| Procedure guides       | Guides for each individual Banff procedure.                                                   |
| Functional description | A detailed guide of the statistical methods used in Banff, with examples.                     |
| Migration guide        | A guide for existing users switching from a SAS-based version of Banff.                       |
| Release notes          | Detailed list of changes in each new version of Banff, stating with version 3.0.              |


## Support

Please do not hesitate submit an Issue for any support requests, including the following:

* Help with installation
* Errors when running the procedures
* Guidance on how to choose appropriate data editing or imputation methods
* Setting up Banff for production 

Support requests will typically receive a reply within one or two business days.

---

# Procédures de Banff

Bienvenue dans le référentiel du projet pour les procédures de Banff, veuillez envisager de surveiller ce référentiel pour être informé des mises à jour.

## Aperçu

Banff est un progiciel développé par Statistique Canada, composé de neuf procédures modulaires exécutant diverses fonctions d'édition de données statistiques (EDS) :

* Vérifier les modifications: vérifie la cohérence et la redondance des modifications.
* Modifier les statistiques: produit des tableaux de statistiques récapitulatives de modification sur les enregistrements qui réussissent, échouent ou échouent à chaque vérification de cohérence.
* Détection des valeurs aberrantes: identifie les observations aberrantes à l'aide des méthodes Hidiroglou-Berthelot ou Sigma-Gap.
* Localisation des erreurs: pour chaque enregistrement, sélectionne le nombre minimum de variables à imputer de sorte que chaque observation puisse être amenée à passer toutes les vérifications.
* Imputation déterministe: effectue l'imputation lorsqu'une seule combinaison de valeurs permet à l'enregistrement de passer l'ensemble des vérifications.
* Imputation par donneur: effectue l'imputation par donneur du voisin le plus proche de sorte que chaque enregistrement imputé satisfasse aux vérifications post-imputation spécifiées.
* Imputation par estimateur: effectue l'imputation à l'aide de fonctions d'estimation ou d'estimateurs de régression linéaire.
* Prorata: Prorata et arrondit les enregistrements pour satisfaire aux modifications spécifiées par l'utilisateur.
* Imputation massive: Effectue une imputation par donneur pour un bloc de variables en utilisant une approche du plus proche voisin ou une sélection aléatoire.

Quelques notes générales sur Banff :

* La plupart des méthodes SDE incluses dans Banff sont conçues pour les enquêtes économiques, et en particulier les variables numériques telles que les revenus et le nombre d'employés. Banff n'inclut pas actuellement les méthodes recommandées pour l'imputation de données catégorielles ou ordinales.
* Banff comprend un certain nombre de méthodes conçues pour les données dont les variables sont contraintes par des relations linéaires, également appelées règles d'édition linéaires ou simplement éditions. Cela comprend des procédures qui examinent les données par rapport aux modifications, choisissent les variables à imputer lorsque les modifications échouent et imputent les enregistrements pour s'assurer que toutes les modifications sont satisfaites.
* Bien que chaque procédure Banff puisse être exécutée indépendamment, elles suivent un modèle modulaire et peuvent être exécutées en séquence dans le cadre d'un flux de processus EDS plus large. Les sorties d'une procédure agissent comme des entrées naturelles pour les procédures suivantes.
* Banff utilise des indicateurs d'état pour suivre les métadonnées telles que les indicateurs de sélection et d'imputation. Ces indicateurs d'état permettent aux procédures Banff de transmettre des informations d'une procédure à une autre et servent également de journal du processus EDS global.

Lors de l'exécution de procédures Banff en séquence dans le cadre d'un flux de processus SDE, les utilisateurs sont responsables des entrées et des sorties entre les étapes. Un package supplémentaire, le **Processeur Banff**, est un utilitaire basé sur les métadonnées conçu spécifiquement pour la production de SDE à grande échelle, intégrant les procédures Banff et gérant toute la gestion des données intermédiaires.

Le guide d'utilisation de Banff utilise souvent la terminologie du [**Modèle générique d'édition de données statistiques** (GSDEM)](https://statswiki.unece.org/spaces/sde/pages/117771706/GSDEM). Les utilisateurs sont encouragés à se référer au GSDEM pour la terminologie courante concernant les concepts SDE.

<mark>Additional translation required once english text is finalized.</mark>

## Installation

Banff est installé à l'aide du programme d'installation du package `pip` pour Python. Pour des informations générales sur `pip`, veuillez consulter https://pip.pypa.io/en/stable/cli/pip_install/.

Vous pouvez trouver les packages Banff sur la page de publication GitHub de Statistique Canada : https://github.com/StatCan/gensol-banff/releases. Une version PyPI sera bientôt disponible.

Téléchargez le binaire Windows ou Linux pour votre environnement depuis la page de publication. Ensuite, suivez les instructions d’installation ci-dessous :

```shell
pip install --no-cache-dir package_name.whl 
```