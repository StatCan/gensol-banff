# Banff (Procedures) Release Notes

## 2025-janvier-24 (Version `3.01.002`)

* Documentation :
  * Diverses mises à jour mineures
  * La version française de metadata-tables.md a été ajoutée.
* Dépendances :
  * Les versions de Duckdb et de Pyarrow sont maintenant limitées à moins à 2 et 19, respectivement, suite à des problèmes de compatibilité avec la version 19 de pyarrow et les versions disponibles de pandas et de duckdb.
    * pour plus de détails, voir [[Python] to_pandas() fails when pandas option 'future.infer_string' is True #45296](https://github.com/apache/arrow/issues/45296)

## 2025-janvier-13 (Version `3.01.001`)

Version de production initiale de la version Python du Banff.

* Pour une description détaillée des nouvelles fonctionnalités et des différences entre les versions basées sur SAS, veuillez consulter notre [guide de migration](/docs/FR/sas_migration_guide.md)
