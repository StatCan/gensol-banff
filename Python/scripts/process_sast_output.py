import json

rules_to_ignore=[
    'CWE-134'  # globally ignore, we use format specifiers in macros
]

# open sast report
with open(r"gl-sast-report.json") as infile:
    sast_json=json.loads(infile.read())

# extract and reformat useful information
out_quality_report = []
out_stats = {
    'cwe_count': {},
    'severity_count': {},
    'ignore_count': 0
}
cc = out_stats['cwe_count']
sc = out_stats['severity_count']

for v in sast_json['vulnerabilities']:
    # extract CWE ID
    id = [x for x in v['identifiers'] if x['type']=='cwe'][0]

    # ignore certain rules
    if id['name'] in rules_to_ignore:
        # add to ignore count, skip to next
        out_stats['ignore_count'] += 1
        continue

    qr={}  # quality report
    qr['name'] = v['name']
    qr['fingerprint'] = v['id']
    qr['cwe_name'] = id['name']
    qr['cwe_url'] = id['url']
    qr['desc_long'] = v['description'].replace('\n', ' ')
    qr['location'] = f"{v['location']['file']}: {v['location']['start_line']}"
    qr['severity'] = v['severity']
    out_quality_report.append(qr)

    # add to statistics
    # rule count
    if qr['cwe_name'] not in cc.keys():
        cc[qr['cwe_name']] = 0
    cc[qr['cwe_name']] += 1
    # severity count
    if qr['severity'] not in sc.keys():
        sc[qr['severity']] = 0
    sc[qr['severity']] += 1

# write output
with open('sast-custom-quality-report.json', 'w', newline='\n') as outfile:
    outfile.write(json.dumps(out_quality_report, indent=2))

with open('sast-custom-quality-stats.json', 'w', newline='\n') as outfile:
    outfile.write(json.dumps(out_stats, indent=2))