# Banff (Procedures) Release Notes

## 2025-June-2 (Version `3.01.003`)

* Various documentation updates for public github release

## 2025-January-24 (Version `3.01.002`)

* Documentation:
  * Various minor updates
  * French version of metadata-tables.md has been added
* Dependencies:
  * Duckdb and Pyarrow versions are now bound below 2 and 19, respectively, as a result of compatibility issues with pyarrow version 19 and the available versions of pandas and duckdb
    * for details, see [[Python] to_pandas() fails when pandas option 'future.infer_string' is True #45296](https://github.com/apache/arrow/issues/45296)

## 2025-January-13 (Version `3.01.001`)

The initial production release of the Python version of Banff.

* For a detailed description of new features and differences between the SAS based version, please see our [migration guide](./sas_migration_guide.md)
