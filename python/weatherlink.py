import requests, json, time
import datetime
from influxdb import InfluxDBClient

while True:
    try:
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

        client = InfluxDBClient('***REMOVED***', 8086, '***REMOVED***', '***REMOVED***', 'air_quality')
        client.write_points([
            {
                "measurement": "wind_speed",
                "fields": {
                    "value": float(d['1 Min Avg Wind Speed'])
                }
            },
            {
                "measurement": "wind_direction",
                "fields": {
                    "value": float(d['1 Min Scalar Avg Wind Direction']) / 22.5
                }
            }
        ])
        print('Write complete')
    except:
        pass

    print('Wait 900s...')
    time.sleep(900)