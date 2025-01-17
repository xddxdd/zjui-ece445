from fetch import AirQualityDB
from map import Map
from weatherlink import Weatherlink
import numpy as np
import json
import matplotlib.pyplot as plt
from windrose import WindroseAxes

wind_speed, wind_direction = Weatherlink().update().dump('weatherlink.json').wind()
print('Wind', wind_direction, 'at', wind_speed)

db = AirQualityDB()
f_all = open('../webpage/results/all.csv', 'w')
f_all.write('# Measurement\tSensor ID\tUpload Time\tLatitude\tLongitude\tValue\n')
for measurement in db.get_enabled_measurements():
    print('Generating', measurement)

    data = db.get_results(measurement)
    csv_data = '\n'.join([
        '{}\t{}\t{}\t{}\t{}\t{}'.format(
            measurement,
            str(id),
            str(time),
            str(lat),
            str(lon),
            str(val)
        )
        for measurement, id, time, lat, lon, val in data
    ] + [''])
    with open('../webpage/results/{}.csv'.format(measurement), 'w') as f:
        f.write('# Measurement\tSensor ID\tUpload Time\tLatitude\tLongitude\tValue\n' + csv_data)
    f_all.write(csv_data)
    # print(data)

    m = Map(10, 10, 0.1, 1000)
    x, y, value = [], [], []
    for measurement, id, time, lat, lon, val in data:
        x.append(lon)
        y.append(lat)
        value.append(val)
        # print(id, lat, lon, lat_transform, lon_transform)
    
    preprocessed_values = m.preprocessSample(x, y, value,
        gps_mode = True,
        topleftPosition = (120.719, 30.525),
        bottomrightPosition = (120.735, 30.512)
    )
    # print(preprocessed_values)
    m.addSample(preprocessed_values)
    m.fillMap()
    m.generateMap(path = '../webpage/results/{}_0.png'.format(measurement))

    with open('../webpage/results/{}.json'.format(measurement), 'w') as f:
        thresh = list(m.getColorThreshold())
        thresh.reverse()
        json.dump(thresh, f)

    max_value = np.max(m.map)
    min_value = np.min(m.map)

    nu = 0.02
    dt = 60
    step = 10
    for i in range(step):
        m.applyDiffusion(dt,nu)
        m.applyWind(wind_speed,wind_direction,dt)
        m.generateMap(path='../webpage/results/{}_{}.png'.format(measurement, str(i + 1)),
            max_value=max_value,min_value=min_value)

print('Fetch remaining fields')
db.get_results('stm32_vref')
db.get_results('stm32_vbat')
db.get_results('stm32_tmp')

with open('../webpage/results/all.json', 'w') as f:
    f.write(db.get_geojson())

# Windrose
wd, ws = db.get_wind()
ax = WindroseAxes.from_ax()
ax.bar(wd, ws, normed=True, opening=0.8, edgecolor='none')
ax.set_legend()
plt.savefig('../webpage/results/windrose.png', bbox_inches='tight', pad_inches=0, transparent=True)
