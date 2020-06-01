Air Quality Sensing Network
===========================

Cloud Server Installation
-------------------------

1. I do not recommend using a cloud server located in mainland China, since then you need to register with the government (备案) to host a website.
   - You may choose servers located in Hong Kong, for example.
2. Install InfluxDB
   - [https://docs.influxdata.com/influxdb/v1.8/introduction/install/](https://docs.influxdata.com/influxdb/v1.8/introduction/install/)
3. Create a database named `air_quality`.
4. Create 3 users:
   - [https://docs.influxdata.com/influxdb/v1.8/administration/authentication_and_authorization/](https://docs.influxdata.com/influxdb/v1.8/administration/authentication_and_authorization/)
   - The first is administrator for InfluxDB server.
   - The second is for sensors to upload data.
     - Username: `***REMOVED***`
     - Password: `***REMOVED***`
   - The third is for website system to read data.
     - Username: `***REMOVED***`
     - Password: `***REMOVED***`
5. Make sure you can access InfluxDB via `[Server IP]:8086`.

STM32 Installation
------------------

1. Modify `stm32/Src/run.h`:
   - Change `HTTP_INFLUXDB_IP` etc to point to your cloud server
   - If you changed username or password for the upload user, also update them here
   - Change `ZJUWLAN_USERNAME` and `ZJUWLAN_PASSWORD` to your own, for auto login to ZJU Wi-Fi
2. Run `make` in `stm32` directory.
   - I'm assuming you use Mac or Linux. I never tried to compile the project on Windows.
   - You need to download GNU Arm Embedded Toolchain.
3. Put the jumper on the top of STM32 (closer to the 4 unsoldered holes) to right, to enter download mode
4. Connect a serial to USB converter:
   - PA9 (GPS module's TX) - Converter's RX
   - PA10 (GPS module's RX) - Converter's TX
   - Ground pin (choose any one) - Converter's GND
5. Run following command to flash program to STM32:
   - `stm32flash /dev/ttyUSB0 -w build/stm32.bin -v -g 0x0`
   - Now the program should start running
6. If you want to put STM32 to field-test, put the jumper on the top of STM32 to left, to enter normal working mode
   - Now STM32 will start running the flashed program once power is on
7. You can use STM32CubeMX to open the `stm32/stm32.ioc` project to change pin definition, regenerate library code, etc.

Web Server Installation
-----------------------

1. Install nginx and PHP on the cloud server:
   - [https://www.digitalocean.com/community/tutorials/how-to-install-linux-nginx-mysql-php-lemp-stack-ubuntu-18-04](https://www.digitalocean.com/community/tutorials/how-to-install-linux-nginx-mysql-php-lemp-stack-ubuntu-18-04)
   - Point website root folder to `webpage` folder.
2. Install Python 3 on the server, and libraries that are needed:
   - `apt install python3 python3-pip python3-numpy python3-matplotlib `
   - `pip3 install influxdb windrose`
3. Enter `python` directory, run `python3 run.py` to refresh website.
   - Remember to update InfluxDB address/user/password in `fetch.py`.
   - Use tools such as Cron or Systemd to run it periodically.
4. Run `python3 weatherlink.py` to crawl wind direction & speed from weather station and upload to InfluxDB.
   - Also remember to update InfluxDB address/user/password in `weatherlink.py`.
   - Use tools such as Supervisord or Systemd to run it continuously.
