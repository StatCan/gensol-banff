# import Banff package
import banff
import pyarrow as pa

# create a schema for the indata dataset
indata_schema = pa.schema([
            ("IDENT", pa.string()),
            ("X1", pa.int64()),
            ("X2", pa.int64()),
            ("ZONE", pa.string()),
    ])

# create table using schema and lists of values for each column
indata = pa.table(
    schema=indata_schema,
    data=[
        ["R03", "R02", "R04", "R01", "R05", "R07", "R06"],
        [10, -4, 4, 16, 15, -4, 30],
        [40, 49, 49, 49, 51, 29, 70],
        ["B", "A", "A", "A", "B", "B", "B"],
    ],
)

# sort according to `by` variable and `unit_id`
indata = indata.sort_by([
    ("ZONE", "ascending"),
    ("IDENT", "ascending"),
])

# run Banff procedure
banff_call = banff.errorloc(
    indata=indata,
    edits="x1>=-5; x1<=15; x2>=30; x1+x2<=50;",
    weights="x1=1.5;",
    cardinality=2,
    time_per_obs=0.1,
    unit_id="IDENT",
    by="ZONE",
)

# TIP: this is how you access the ouput datasets
print("OUTSTATUS dataset\n", banff_call.outstatus, "\n")