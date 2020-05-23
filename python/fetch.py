from influxdb import InfluxDBClient

client = InfluxDBClient('***REMOVED***', 8086, '***REMOVED***', '***REMOVED***', 'air_quality')

# Obtain GPS coordinates
gps_lat = {}
gps_lon = {}
result = client.query('select last(value) from gps_lat, gps_lon where value != 0 and time > now() - 7d group by id')
for measurement, id in result.keys():
    if measurement == 'gps_lat':
        points = list(result.get_points(measurement, id))
        value = points[0]['last']
        # Conversion from NMEA to real coordinates
        value = (value // 100) + (value % 100) / 60
        gps_lat[id['id']] = value
    elif measurement == 'gps_lon':
        points = list(result.get_points(measurement, id))
        value = points[0]['last']
        # Conversion from NMEA to real coordinates
        value = (value // 100) + (value % 100) / 60
        gps_lon[id['id']] = value

print(gps_lat)
print(gps_lon)

result = client.query('show measurements')
measurements = [k['name'] for k in result['measurements']]

for measurement in measurements:
    if measurement.startswith('gps'):
        continue
    elif measurement.startswith('pms5003'):
        result = client.query('select last(value) from ' + measurement + ' where value != 65535 and time > now() - 7d group by id')
    else:
        result = client.query('select last(value) from ' + measurement + ' where value != -1 and time > now() - 7d group by id')
    for _, id in result.keys():
        points = list(result.get_points(measurement, id))
        value = points[0]['last']
        if id['id'] in gps_lat and id['id'] in gps_lon:
            lat, lon = gps_lat[id['id']], gps_lon[id['id']]
        else:
            lat, lon = 0, 0
        
        line = ', '.join([
            measurement, 
            str(id['id']), 
            str(lat),
            str(lon),
            str(value)
        ])
        print(line)
