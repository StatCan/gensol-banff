# Tutoriel

Ce tutoriel montrera l'[équivalent Python](/Python/sample_programs/conversion_examples/Errorloc01.py) du [programme d'exemple 1 en langage SAS Proc Errorloc](/Python/sample_programs/conversion_examples/Errorloc01.sas).

L'exemple de programme illustre comment
- créer un tableau synthétique
- trier le tableau
- spécifier
  - paramètres
  - tableau d'entrée
  - options du tableau de sortie
- accéder aux résultats

Il abordera également certaines différences pertinentes entre SAS et Python.

## Exemple de langage SAS

```sas
/* créer une table d'entrée synthétique */
data example_indata;
input IDENT $ X1 X2 ZONE $1.;
cards;
R03 10 40 B
R02 -4 49 A
R04 4 49 A
R01 16 49 A
R05 15 51 B
R07 -4 29 B
R06 30 70 B
;
run;

/* trier par variables BY et KEY */
proc sort data=example_indata; by ZONE IDENT;run;

/* exécuter la procédure Banff */
proc errorloc
data=example_indata
outstatus=outstatus
outreject=outreject
edits="x1>=-5; x1<=15; x2>=30; x1+x2<=50;"
weights="x1=1.5;"
cardinality=2
timeperobs=.1
;
id IDENT;
by ZONE;

/* exécuter l'appel Banff */
run; 
```

## Équivalent du langage Python

```python
# importer le paquet Banff
import banff
import pyarrow as pa

# créer un schéma pour la table indata
indata_schema = pa.schema([
            ("IDENT", pa.string()),
            ("X1", pa.int64()),
            ("X2", pa.int64()),
            ("ZONE", pa.string()),
    ])

# créer une table en utilisant un schéma et des listes de valeurs pour chaque colonne
indata = pa.table(
    schema=indata_schema,
    data=[
        ["R03", "R02", "R04", "R01", "R05", "R07", "R06"],
        [10, -4, 4, 16, 15, -4, 30],
        [40, 49, 49, 49, 51, 29, 70],
        ["B", "A", "A", "A", "B", "B", "B"],
    ],
)

# trier selon la variable `by` et `unit_id`
indata = indata.sort_by([
    ("ZONE", "ascending"),
    ("IDENT", "ascending"),
])

# exécuter la procédure Banff
banff_call = banff.errorloc(
    indata=indata,
    edits="x1>=-5; x1<=15; x2>=30; x1+x2<=50;",
    weights="x1=1.5;",
    cardinality=2,
    time_per_obs=0.1,
    unit_id="IDENT",
    by="ZONE",
)
```

## Explication ligne par ligne

### Paquets d'importation

En Python, les *paquets* doivent être *importés* dans une session avant de pouvoir être utilisés.

```python
import banff
import pyarrow as pa
```

Le paquet [`pyarrow`](https://pypi.org/project/pyarrow/) est utilisé pour créer et manipuler des tableaux. Notez que le pseudonyme `pa` est utilisé pour le paquet `pyarrow`.

### Créer des données synthétiques

Nous créons la même table synthétique que celle créée dans SAS.

Un objet *Pyarrow Schema* est créé et attribué à la variable `indata_schema`.  

```python
# créer un schéma pour la table indata
indata_schema = pa.schema([
            ("IDENT", pa.string()),
            ("X1", pa.int64()),
            ("X2", pa.int64()),
            ("ZONE", pa.string()),
    ])
```

- ceci est utilisé pour définir le nom et le type de données de chaque colonne d'une table
- Pyarrow propose un [ensemble complet de types de données](https://arrow.apache.org/docs/python/api/datatypes.html#factory-functions)
- pour plus de détails, voir la [documentation `pyarrow.schema`](https://arrow.apache.org/docs/python/generated/pyarrow.schema.html)

Un objet *Pyarrow Table* est créé et stocké dans la variable `indata`.

```python
# créer une table en utilisant un schéma et des listes de valeurs pour chaque colonne
indata = pa.table(
    schema=indata_schema,
    data=[
        ["R03", "R02", "R04", "R01", "R05", "R07", "R06"],
        [10, -4, 4, 16, 15, -4, 30],
        [40, 49, 49, 49, 51, 29, 70],
        ["B", "A", "A", "A", "B", "B", "B"],
    ],
)
```

- l'ordre des colonnes dans le schéma correspond à l'ordre des listes de données
- pour plus de détails, voir la [documentation `pyarrow.table`](https://arrow.apache.org/docs/python/generated/pyarrow.table.html)
- contrairement à SAS, où les tables sont généralement stockées sous forme de fichiers, les tables Pyarrow sont généralement stockées en mémoire sous forme d'*objets*, d'où la nécessité d'assigner à la variable `indata`

La table `indata` est triée à l'aide de sa *méthode* `sort_by()` et réassignée à elle-même.

```python
# trier selon la variable `by` et `unit_id`
indata = indata.sort_by([
    ("ZONE", "ascending"),
    ("IDENT", "ascending"),
])
```

- dans SAS, cela serait analogue à l'utilisation de `proc sort data=indata` sans explicitement l'option `out`
- la plupart des opérations dans pyarrow renvoient une sorte de copie de la table, ce qui signifie que la table source n'est pas modifiée
- pour conserver la table non triée, affectez simplement la table triée à une variable différente
- `indata_sorted = indata.sort_by(...` vous donnerez une nouvelle copie qui est triée et laissera la copie `indata` d'origine telle quelle

#### Concepts Python

> - en Python, les variables sont souvent des *objets*, contenant non seulement des données mais aussi des *méthodes*
> - les méthodes sont souvent utiles pour obtenir des informations sur les données ou effectuer des opérations sur celles-ci
> - la variable `indata` est un objet *Pyarrow Table* (`pyarrow.Table`) et possède donc sa propre [méthode `sort_by()`](https://arrow.apache.org/docs/python/generated/pyarrow.Table.html#pyarrow.Table.sort_by)
> - la documentation sur les autres méthodes disponibles pour Pyarrow Tables est disponible [ici](https://arrow.apache.org/docs/python/generated/pyarrow.Table.html#pyarrow.Table).

### Exécution de la procédure Banff

```python
banff_call = banff.errorloc(
    indata=indata,
    edits="x1>=-5; x1<=15; x2>=30; x1+x2<=50;",
    weights="x1=1.5;",
    cardinality=2,
    time_per_obs=0.1,
    unit_id="IDENT",
    by="ZONE",
)
```

L'appel de `banff.errorloc()` entraîne l'exécution de la procédure errorloc et l'affectation d'un objet à la variable `banff_call`. L'objet peut être utilisé pour accéder aux tables de sortie.

- notez que tous les paramètres et tables sont spécifiés sous forme de paires clé-valeur séparées par des virgules
- ils peuvent apparaître dans n'importe quel ordre
- `indata=indata` spécifie que la table `indata` (récemment triée) doit être fournie comme table « indata »

### Exécution de la procédure

Pendant l'exécution de la procédure, le texte écrit sur la console par la procédure doit être quasiment identique au journal SAS de l'exemple équivalent.

### Accès aux tables de sortie

Une fois l'exécution terminée, les options de la table de sortie sont traitées. L'option par défaut a été spécifiée ci-dessus, de sorte que les tables de sortie sont disponibles sous forme de tables pyarrow.

Elles sont stockées dans l'objet `banff_call`, accédez-y en utilisant `banff_call.outstatus` et `banff_call.outreject`. À partir de là, elles peuvent être traitées par les utilisateurs comme n'importe quelle autre table pyarrow, par exemple:

- écrit dans un fichier
- manipulé (trié, fusionné, etc.)
- utilisé comme entrée pour une autre procédure (`instatus=banff_call.outstatus`)

### Autres options d'entrée et de sortie

Pour fournir un moyen cohérent de lecture/écriture de fichiers et de conversion entre les formats de tableau, tout en conservant la plus haute précision possible en virgule flottante, la prise en charge de divers formats de tableau d'entrée et de sortie a été implémentée. Pour plus de détails, consultez le [Guide de l'utilisateur](/docs/FR/user_guide.md#supported-formats).

Le code suivant illustre l'utilisation de fichiers pour les tableaux d'entrée et de sortie en modifiant l'exemple ci-dessus.

```python
banff.errorloc( 
    indata=r"C:\temp\in_data.feather",
    outstatus=r"C:\temp\out_status.parquet",
    outreject=r"C:\temp\out_reject.parquet",
    edits="x1>=-5; x1<=15; x2>=30; x1+x2<=50;",
    weights="x1=1.5;",
    cardinality=2,
    time_per_obs=0.1,
    unit_id="IDENT",
    by="ZONE"
)
```

- notez qu'il n'est pas nécessaire d'affecter l'objet renvoyé par `banff.errorloc()` à une variable car les données de sortie sont écrites sur le disque.
