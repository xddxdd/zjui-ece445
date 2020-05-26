from influxdb import InfluxDBClient
import json
from coord_transform import wgs84_to_gcj02

class AirQualityDB:
    def __init__(self):
        self._records = {}
        self.client = InfluxDBClient('***REMOVED***', 8086, '***REMOVED***', '***REMOVED***', 'air_quality')
        self._get_gps_coordinates()

    def _get_gps_coordinates(self):
        self.gps_lat = {}
        self.gps_lon = {}
        result = self.client.query('select last(value) from gps_lat, gps_lon where value != 0 and time > now() - 7d group by id')
        for measurement, id in result.keys():
            points = list(result.get_points(measurement, id))
            value = points[0]['last']
            time = points[0]['time']
            # Conversion from NMEA to real coordinates
            value = (value // 100) + (value % 100) / 60

            if measurement == 'gps_lat':
                self.gps_lat[id['id']] = value
            elif measurement == 'gps_lon':
                self.gps_lon[id['id']] = value
            self._add_record(id['id'], time, measurement, value)

        for id in self.gps_lat:
            self.gps_lon[id], self.gps_lat[id] = wgs84_to_gcj02(self.gps_lon[id], self.gps_lat[id])

        print(self.gps_lat)
        print(self.gps_lon)
        return self
    
    def get_enabled_measurements(self):
        return [
            'bme680_co2',
            'bme680_hum',
            'bme680_iaq',
            'bme680_prs',
            'bme680_tmp',
            'bme680_tvoc',
            'mics_co',
            # Disabled MICS_NH3 for inaccurate result
            # 'mics_nh3',
            'mics_no2',
            'pms5003_pm1',
            'pms5003_pm2_5',
            'pms5003_pm10',
            # Disabled since STM32's metrics are useless to users
            # 'stm32_tmp',
            # 'stm32_vbat',
            # 'stm32_vref',
        ]

    def get_results(self, measurement):
        if measurement.startswith('pms5003'):
            result = self.client.query('select last(value) from ' + measurement + ' where value != 65535 and time > now() - 7d group by id')
        else:
            result = self.client.query('select last(value) from ' + measurement + ' where value != -1 and time > now() - 7d group by id')

        results = []        
        for _, id in result.keys():
            points = list(result.get_points(measurement, id))
            value = points[0]['last']
            time = points[0]['time']
            if id['id'] in self.gps_lat and id['id'] in self.gps_lon:
                lat, lon = self.gps_lat[id['id']], self.gps_lon[id['id']]
            else:
                lat, lon = 0, 0

            results.append([measurement, id['id'], time, lat, lon, value])
            self._add_record(id['id'], time, measurement, value)
        return results

    def _add_record(self, id, time, measurement, value):
        # Only allow replacing with newer records
        if id not in self._records:
            self._records[id] = {}
        elif self._records[id]['time'] < time:
            self._records[id] = {}
        elif self._records[id]['time'] > time:
            return self

        self._records[id]['id'] = id
        self._records[id]['time'] = time
        self._records[id][measurement] = value
        
        return self

    def get_records(self):
        return self._records

    def get_geojson(self):
        geojson = {
            'type': 'FeatureCollection',
            'features': []
        }
        for id in self._records:
            if id not in self.gps_lat or id not in self.gps_lon:
                continue
            geojson['features'].append({
                "type": "Feature",
                "properties": self._records[id],
                "geometry": {
                    "type": "Point",
                    "coordinates": [
                        self.gps_lon[id], 
                        self.gps_lat[id]
                    ]
                }
            })
        return json.dumps(geojson)