<?php
require_once __DIR__ . '/vendor/autoload.php';
use InfluxDB\Client;

$metrics = array(
    'stm32_vbat' => 'STM32 Battery Voltage',
    'stm32_tmp' => 'STM32 Chip Temperature',
    'stm32_vref' => 'STM32 3.3V Rail Voltage',
    'gps_lat' => 'GPS Latitude',
    'gps_lon' => 'GPS Longitude',
    'bme680_co2' => 'BME680 CO2',
    'bme680_hum' => 'BME680 Humidity',
    'bme680_iaq' => 'BME680 Indoor Air Quality',
    'bme680_prs' => 'BME680 Pressure',
    'bme680_tmp' => 'BME680 Temperature',
    'bme680_tvoc' => 'BME680 TVOC',
    'mics_co' => 'MICS6814 CO',
    'mics_nh3' => 'MICS6814 NH3 (Unstable)',
    'mics_no2' => 'MICS6814 NO2',
    'pms5003_pm1' => 'PMS5003 PM1',
    'pms5003_pm2_5' => 'PMS5003 PM2.5',
    'pms5003_pm10' => 'PMS5003 PM10',
);

$influxdb = new InfluxDB\Client('***REMOVED***', 8086, '***REMOVED***', '***REMOVED***');
$db = $influxdb->selectDB('air_quality');

$metric = $_GET['metric'];
if(!array_key_exists($metric, $metrics)) {
    $metric = 'stm32_vbat';
}

$result = $db->query('select * from ' . $metric . ' where time > now() - 7d group by id order by time desc limit 3')
             ->getPoints();

uasort($result, function($a, $b) {
    if(intval($a['id']) != intval($b['id'])) return (intval($a['id']) < intval($b['id'])) ? -1 : 1;
    if($a['time'] != $b['time']) return ($a['time'] > $b['time']) ? -1 : 1;
    return 0;
});
?>
<!doctype html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
    <link rel="stylesheet" href="resources/bootstrap.min.css">
    <script src="resources/jquery-3.5.1.min.js"></script>
    <script src="resources/bootstrap.bundle.min.js"></script>
    <title>Realtime Status</title>
</head>
<body>
<nav class="navbar navbar-expand-lg navbar-light bg-light">
    <a class="navbar-brand" href="#">Realtime Status</a>
    <button class="navbar-toggler" type="button" data-toggle="collapse" data-target="#navbarSupportedContent" aria-controls="navbarSupportedContent" aria-expanded="false" aria-label="Toggle navigation">
        <span class="navbar-toggler-icon"></span>
    </button>
    <div class="collapse navbar-collapse" id="navbarSupportedContent">
        <ul class="navbar-nav mr-auto">
            <?php foreach($metrics as $key => $value) { ?>
                <a class="nav-link" href="<?php echo $_SERVER['PHP_SELF']; ?>?metric=<?php echo $key; ?>"><?php echo $value; ?></a>
            <?php } ?>
        </ul>
    </div>
</nav>
<div class="container">
<h1><?php echo $metrics[$metric]; ?></h1>
<table class="table table-sm">
<thead><tr><th>Time</th><th>Sensor ID</th><th>Value</th></tr></thead>
<tbody>
<?php
$occurrence = array();
foreach($result as $item) {
    echo '<tr' . (array_key_exists($item['id'], $occurrence) ? '' : ' class="table-primary"')
         . '><td>'
         . $item['time']
         . '</td><td>' 
         . $item['id'] 
         . '</td><td>' 
         . $item['value'] 
         . '</td></tr>';
    $occurrence[$item['id']] = 1;
}
?>
</tbody>
</table>
<!--<pre><code>
<?php var_dump($result); ?>
</code></pre>-->
</div>
</body>
</html>
