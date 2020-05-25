from fetch import AirQualityDB
from map import Map
from coord_transform import wgs84_to_gcj02
import numpy as np

db = AirQualityDB()
for measurement in db.get_enabled_measurements():
    print('Generating', measurement)

    data = db.get_results(measurement)
    # print(data)

    m = Map(100, 100, 0.1, 1000)
    x, y, value = [], [], []
    for measurement, id, time, lat, lon, val in data:
        lon_transform, lat_transform = wgs84_to_gcj02(lon, lat)
        x.append(lon_transform)
        y.append(lat_transform)
        value.append(val)
        # print(id, lat, lon, lat_transform, lon_transform)
    # print(x, y, value)
    preprocessed_values = m.preprocessSample(x, y, value,
        gps_mode = True,
        topleftPosition = (120.719, 30.525),
        bottomrightPosition = (120.735, 30.512)
    )
    # print(preprocessed_values)
    m.addSample(preprocessed_values)
    m.fillMap()
    m.generateMap(path = '../webpage/results/_{}_0.png'.format(measurement))

    max_value = np.max(m.map)
    min_value = np.min(m.map)

    nu = 0.02
    dt = 60
    step = 10
    windSpeed=5
    windDirection = 330
    for i in range(step):
        m.applyDiffusion(dt,nu)
        m.applyWind(windSpeed,windDirection,dt)
        m.generateMap(path='../webpage/results/_{}_{}.png'.format(measurement, str(i + 1)),
            max_value=max_value,min_value=min_value)

# x = Map(10,10,0.1,1000)
# # samples = [(0,0,1),(0,9,10),(9,0,10),(9,9,1),(5,5,14),(3,5,8),(7,8,18),(3,2,3),(7,4,8)]
# samples = x.loadSampleFile("./data.csv", gps_mode = True)
# x.addSample(samples)
# # print(x.map)
# x.fillMap()
# max_value = np.max(x.map)
# min_value = np.min(x.map)
# x.generateMap(path='./webpage/result/initialMap.png')
# # x.writeToCsv('./mydata.csv')


# ws = np.random.random(500) * 6
# wd = np.random.random(500) * 360
# x.generateWindroseMap(ws, wd, path='./webpage/result/windrose.png')


# nu = 0.02
# dt = 60
# step = 10
# # print(x.map[8,7])
# windSpeed=5
# windDirection = 330
# for i in range(step):
#     x.applyDiffusion(dt,nu)
#     # x.generateMap(path='./webpage/result/map'+str(i)+'.png')
#     x.applyWind(windSpeed,windDirection,dt)
#     x.generateMap(path='./webpage/result/map'+str(i)+'.png',max_value=max_value,min_value=min_value)
#     # x.generateMap(path='./webpage/result/map'+str(i)+'.png')
