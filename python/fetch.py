from influxdb import InfluxDBClient

class AirQualityDB:
    def __init__(self):
        self.client = InfluxDBClient('***REMOVED***', 8086, '***REMOVED***', '***REMOVED***', 'air_quality')
        self._get_gps_coordinates()

    def _get_gps_coordinates(self):
        self.gps_lat = {}
        self.gps_lon = {}
        result = self.client.query('select last(value) from gps_lat, gps_lon where value != 0 and time > now() - 7d group by id')
        for measurement, id in result.keys():
            if measurement == 'gps_lat':
                points = list(result.get_points(measurement, id))
                value = points[0]['last']
                # Conversion from NMEA to real coordinates
                value = (value // 100) + (value % 100) / 60
                self.gps_lat[id['id']] = value
            elif measurement == 'gps_lon':
                points = list(result.get_points(measurement, id))
                value = points[0]['last']
                # Conversion from NMEA to real coordinates
                value = (value // 100) + (value % 100) / 60
                self.gps_lon[id['id']] = value
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
        return results
