# Plant-Monitor
Workshop- CASA0014 - 2 - Plant Monitor
## 1. 
![image1](https://user-images.githubusercontent.com/92024194/140019140-8c69edd4-4ce1-4a9b-900a-0bd337b38d2f.jpg)
Understand the connection of circuit.

Solder up a CASA Plant Monitor shield to make the packaging a little tidier.

2. 
![image2](https://user-images.githubusercontent.com/92024194/140019537-443905e2-342f-4d50-bc27-7a6b9ff734c9.jpg)
DHT22_MQTT with nails
![058c25ee84bd1ac9a884fb726640140](https://user-images.githubusercontent.com/92024194/140020120-82ba24e6-5e10-4b3d-817e-63b184e0acd6.jpg)
Send Soil data to MQTT

3.

<img width="691" alt="1c8e6895ff3390bfa00816cd84abab7" src="https://user-images.githubusercontent.com/92024194/140054131-f43ca00c-79fc-40e2-93b6-88e20b33289e.png">

Visualization of soil data on Grafana.

Set up a RPi as a gateway,then collect and save the data using Telegraf serveragent and send it to the Influx Database.
Set up a Grafana dashboard to visualise my sensed data.
