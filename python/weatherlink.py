import requests, json, time
import datetime
from influxdb import InfluxDBClient

class Weatherlink:
    def __init__(self, id = '***REMOVED***'):
        self.id = id
        self.result = None
        self.parsed_result = None
    
    def update(self):
        try:
            timestamp = int(datetime.datetime.now().timestamp() * 1000)
            url = 'https://www.weatherlink.com/embeddablePage/summaryData/***REMOVED***?ts=' + str(timestamp)
            self.result = requests.get(url).json()

            self.parsed_result = {}
            for e in sorted(self.result['currConditionValues'], key = lambda e: e['sortOrder']):
                self.parsed_result[e['displayName']] = e['convertedValue']
        except:
            print('Update failed')
            pass

        return self
    
    def dump(self, file):
        with open(file, 'w') as f:
            f.write(json.dumps(self.result))
        return self

    def wind(self):
        return (
            float(self.parsed_result['1 Min Avg Wind Speed']), 
            float(self.parsed_result['1 Min Scalar Avg Wind Direction']) / 22.5
        )

if __name__ == '__main__':
    while True:
        try:
            wl = Weatherlink()
            ws, wd = wl.update().dump('weatherlink.json').wind()
            
            client = InfluxDBClient('***REMOVED***', 8086, '***REMOVED***', '***REMOVED***', 'air_quality')
            client.write_points([
                {
                    "measurement": "wind_speed",
                    "fields": {
                        "value": ws
                    }
                },
                {
                    "measurement": "wind_direction",
                    "fields": {
                        "value": wd
                    }
                }
            ])
            print('Write complete')
        except:
            pass

        print('Wait 900s...')
        time.sleep(900)
