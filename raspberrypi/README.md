# Project for using Grove Pi Sensors with Oracle IoT CS

This project sets out to make it easy to connect a Raspberry Pi with GrovePi sensors to Oracle IoT Cloud Service.

## Hardware Setup

Make sure you have connected the Grove Pi board to the Raspberry Pi while it is 
*switched off* and then connect the sensors you want to use. Pay attention to if 
the sensors are **DIGITAL** or **ANALOG** and connect them to the **D2-D8** or 
**A0-A2** ports respectivly. You sould then have something that looks like:
![GrovePi](http://www.dexterindustries.com/wp-content/uploads/2014/05/Grove_Pi_02-1024x644.jpg)

## Software Setup Instructions
*It is assumed you have already installed Raspian OS on a memory card and configured network access for the Pi.*

1. **Install Node**
```
wget http://node-arm.herokuapp.com/node_latest_armhf.deb 
sudo dpkg --remove nodered nodejs-legacy 
sudo dpkg -i node_latest_armhf.deb
```
2. **Download this project**
```
git clone https://orahub.oraclecorp.com/iot/grovepi-client.git
cd grovepi-client
```
3. **Install node library dependencies**
```
 npm install
```
4. **Enable i2c**  
if you have not yet enabled i2c support then run
```
 raspi-config
```
Under advanced enable i2c and reboot

5. **Added IoT Server to /etc/hosts**  
Edit the `/etc/hosts` file and add a line at the end with the ip of the IoT CS Server mapping to 'iotserver'. eg.
```
....
00.00.00.00    iotserver
```

6. **Check your data and time is correct**  
Run the 'date' command and see if the data and time are correct. If not then you can use 'raspi-config' to help 
setup your time zone. To enable date/time syncing see <https://victorhurdugaci.com/raspberry-pi-sync-date-and-time>

7. **Create a Device Model on IoT CS**  
if you have not created a device model yet or are not on a server with one already created for you then go to 
<http://IOTSERVER/ui/?root=modal&modal=deviceModelModal> and create a device model. It is important that you create 
attributes for any sensors you are going to use like the example below. The attribute names have to be the same as in 
the example.  
![Example Device Model](example-device-model.png)

8. **Register a device on IoT CS ui**  
Go to <http://IOTSERVER/ui/?root=registrationModal&registrationModal=registrationSingle> then enter details for your device
on the next screen it will give you the device ID and shared secret. You will need those for the next stage.

9. **Run Setup Script**  
This will guide you though creating a trust store for secure communication to the IoT CS Server and creating a 
configuration file for what sensors you have plugged into the Grove Pi.
```
node setupDevice.js
``` 

10. **As root, run the client**  
If you are not using the *grovepi:jp* device model you will need to edit the first line if client.js and change the device 
model URN. You will need to pass the trust file for the device that was created by step 8.
```
node workshop.js 0-AB.json
```

## What can I do now

### Data from Pi to Enterprise
There are 3 ways you can get the data that is streaming from the Pi to IoT CS.
1.  View messages in the UI under Devices -> Mangement double click on your device. <http://IOTSERVER/ui/?root=modal&modal=deviceModal&deviceModal=0-BA>
2.  Query the server REST api and get the corrent state of the device attributes
```
curl http://IOTSERVER/iot/api/v2/apps/0-AE/devices/0-BA/deviceModels/urn:com:oracle:iot:grovepi:jp/attributes
```
Replace the IOTSERVER, application ID , device ID and device model URN with the ones you are using.
3.  Setup an enterprise application intergration, under Applications -> My Application -> Integrations click "Create Intergration". 
Pick "Enterprise Application" then enter the URL for your web server and pick the device model stream. The IoT CS Server will then
start doing HTTP POST the URL you provided when it gets updates from the device.

### Setting device attributes from the Enterprise
The IoT CS Server has a REST api for setting properties on the device, this could be turning a LED on/off or Buzzer on/off.
```
curl -v -k --user iot:welcome1 -X PUT -H Content-Type:application/json  -d '{ "value" : true }' 'http://IOTSERVER/iot/api/v2/apps/0-AE/devices/0-BA/deviceModels/urn:com:oracle:iot:grovepi:jp/attributes/led'
```
Replace the IOTSERVER, application ID , device ID and device model URN with the ones you are using.

You can also use the enterprise client libaray in multiple languages <http://www.oracle.com/technetwork/topics/cloud/downloads/iot-client-libraries-2705514.html>
