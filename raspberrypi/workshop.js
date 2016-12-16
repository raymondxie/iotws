"use strict";
// Need to change this URN, according to your setup in IoTCS Admin Console
const DEVICE_MODEL_URN = 'urn:com:oracle:iot:device:iotws';

const GrovePi = require('node-grovepi').GrovePi;
const Board = GrovePi.board;
const UltrasonicDigitalSensor = GrovePi.sensors.UltrasonicDigital;
const DHTDigitalSensor = GrovePi.sensors.DHTDigital;
const LightAnalogSensor = GrovePi.sensors.LightAnalog;
const Digital = GrovePi.sensors.base.Digital;
const Analog = GrovePi.sensors.base.Analog;
const iot = require("iotcs-client-device");

// Setup IoT Device Client Lib
const iotClient = iot({debug: true});
iotClient.oracle.iot.tam.store = (process.argv[2]);
iotClient.oracle.iot.tam.storePassword = 'changeit';
const device = new iotClient.device.DirectlyConnectedDevice();


// Load GrovePI Sensor Config
const sensorConfig = require('./sensor-config.js');
console.log('sensorConfig = ', sensorConfig);

// Current Sensor Values
var currentData = {
    button: "Hello IoT world",
    angle: 0.0
};
// Virtual Device;
var virtualDev;

// RXIE: setup MQTT broker connection 
const mqtt = require("mqtt");
var options = {
    port: 11565,
    host: 'm12.cloudmqtt.com',
    clientId: 'raspberrypi3-oow',
    username: 'bvywboem',
    password: 'nAxTiX11geNt',
    keepalive: 43200,
    reconnectPeriod: 30000,
    protocolId: 'MQIsdp',
    protocolVersion: 3,
    clean: true,
    encoding: 'utf8'
};
var mqttClient = mqtt.connect(options);

mqttClient.on('connect', function() {
	console.log("mqtt client connected");

	// subscribe to 'iotcs-j1' topic	
	mqttClient.subscribe('iotcs-j1', function() {
		console.log("subscribed to iotcs topic");
		
		// listen for message
		mqttClient.on('message', function(topic, message, packet) {
			console.log("received '" + message + "'");
		    currentData['button'] = message + " - Hello IoT World";
            dataChange();
		});
	});
});

mqttClient.on('error', function(err){
	console.log("Connection to MQTT broker failure: ", err);
});
// RXIE: mqtt communication done


// Setup device and board and initialize them
activateDeviceIfNeeded(device)
    .then((device) => {
        return getModelGrovePiDeviceModel(device);
    })
    .then((deviceModel) => {
        virtualDev = device.createVirtualDevice(device.getEndpointId(), deviceModel);
        return createGrovePiBoard(virtualDev);
    })
    .then((board) => {
        return setupSensors();
    })
    .catch(err => {
        console.log('err = ', err);
    });

/**
 * Create Grove Pi Board and initialize it
 *
 * @returns {Promise} that completes when the board has been initialized
 */
function createGrovePiBoard() {
    return new Promise((resolve, reject) => {
	debugger;
        var board = new Board({
            debug: true,
            onError: function (err) {
                reject('Something went wrong with grove pi init. Err:'+err);
            },
            onInit: function (res) {
                if (res) {
                    resolve(board);
                } else {
                    reject('Init failed')
                }
            }
        });
        board.init();
    });
}


function setupSensors() {
    let writableSensors = {};

    virtualDev.onChange = tupples => {
        tupples.forEach(tupple => {
            var show = {
                name: tupple.attribute.id,
                lastUpdate: tupple.attribute.lastUpdate,
                oldValue: tupple.oldValue,
                newValue: tupple.newValue
            };
            console.log('------------------ON VIRTUAL DEVICE CHANGE ---------------------');
            console.log(JSON.stringify(show, null, 4));
            console.log('----------------------------------------------------------------');
            if (writableSensors[tupple.attribute.id]) writableSensors[tupple.attribute.id].write(tupple.newValue);
            // sensor[tupple.attribute.id] = tupple.newValue;
        });
    };

    sensorConfig.forEach(sensor => {
        var pinMatch = sensor.pin.match(/([AD])(\d+)/);
        var isAnalog = pinMatch[1] === 'A';
        var pin = parseInt(pinMatch[2]);
        switch (sensor.type) {
            case 'TemperatureAndHumiditySensor':
                let tempHumSensor = new DHTDigitalSensor(pin,DHTDigitalSensor.VERSION.DHT11,DHTDigitalSensor.CELSIUS);
                tempHumSensor.stream(200, function(res) {
                    if (res) {
                        let change = false;
                        if (res[0] !== currentData['temperature']) {
                            currentData['temperature'] = res[0];
                            change = true;
                        }
                        if (res[1] !== currentData['humidity']) {
                            currentData['humidity'] = res[1];
                            change = true;
                        }
                        if (res[2] !== currentData['heatIndex']) {
                            currentData['heatIndex'] = res[2];
                            change = true;
                        }
                        if (change) dataChange();
                    }
                    // console.log('TemperatureAndHumiditySensor temp = ', temp,'hum = ', hum,'heatIndex = ', heatIndex);
                });
                break;
            case 'LightAnalogSensor':
                new LightAnalogSensor(pin).stream(100, function(res) {
                    if (res !== currentData['lightLevel'] && res !== false) {
                        currentData['lightLevel'] = res;
                        dataChange();
                    }
                });
                break;
            case 'Button':
                new Digital(pin).stream(20, function(res) {
                    if ( currentData['button'].startsWith("RPi") !== true &&  res !== 0) {
                        currentData['button'] = "RPi - Hello IoT World";
                        dataChange();
                    }
                });
                break;
            case 'UltrasonicRanger':
                new UltrasonicDigitalSensor(pin).stream(100, function(res) {
                    if (res !== currentData['range'] && res !== false) {
                        currentData['range'] = res;
                        dataChange();
                    }
                });
                break;
            case 'SoundAnalogSensor':
                new Analog(pin).stream(100, function(res) {
                    if (res !== currentData['sound'] && res !== false) {
                        currentData['sound'] = res;
                        dataChange();
                    }
                });
                break;
            case 'RotaryAngleAnalogSensor':
                new Analog(pin).stream(100, function(res) {
                    if (res !== currentData['angle'] && res !== false) {
                      // RXIE: filter out little noise
                      if (Math.abs(res- currentData['angle']) > 5 ) {
                        currentData['angle'] = res;
                        dataChange();
                      }
                    }
                });
                break;
            case 'LEDSocketKit':
                writableSensors['led'] = new Digital(pin);
                break;
            case 'Buzzer':
                writableSensors['buzzer'] = new Digital(pin);
                break;
        }
    });
}


function dataChange() {
    console.log('updateChange() - currentData = ', currentData);
    virtualDev.update(currentData);
}


function getModelGrovePiDeviceModel(device){
    return new Promise((resolve, reject) => {
        device.getDeviceModel(DEVICE_MODEL_URN, function (response) {
            resolve(response);
            // humidityModel = response;
            // startVirtualHumidity(device, device.getEndpointId());
        });
    });
}

function activateDeviceIfNeeded(device) {
    return new Promise((resolve, reject) => {
	debugger;
        if (device.isActivated()) {
            resolve(device);
        } else {
            device.activate([DEVICE_MODEL_URN], () => {
                console.log('Activated device ',device.getEndpointId(),device.isActivated());
                if (device.isActivated()) {
                    resolve(device);
                } else {
                    reject('Failed to activate device')
                }
            });
        }
    });
}
