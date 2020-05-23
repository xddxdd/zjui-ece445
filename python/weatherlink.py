import requests, json
import datetime

timestamp = int(datetime.datetime.now().timestamp() * 1000)
url = 'https://www.weatherlink.com/embeddablePage/summaryData/***REMOVED***?ts=' + str(timestamp)
result = requests.get(url).json()
with open('weatherlink.json', 'w') as f:
    f.write(json.dumps(result))

d = {}
for e in sorted(result['currConditionValues'], key = lambda e: e['sortOrder']):
    d[e['displayName']] = e['convertedValue']
    # print(e['displayName'], e['convertedValue'], e['unitLabel'])
# print(d)

print('Wind', float(d['1 Min Scalar Avg Wind Direction']) / 22.5, 'at', float(d['1 Min Avg Wind Speed']))