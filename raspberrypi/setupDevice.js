"use strict";

const child_process = require('child_process');
const inquirer = require('inquirer');
const fs = require('fs');
const chalk = require('chalk');
const Gauge = require('gauge');
const gauge = new Gauge();

const ValidIpAddressRegex = "^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$";
const ValidHostnameRegex = "^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\-]*[A-Za-z0-9])$";
const HostnameRegex = new RegExp('('+ValidIpAddressRegex+')|('+ValidHostnameRegex+')', 'i');

const questions = [{
    type: 'input',
    name: 'hostname',
    message: 'What is the IoT server hostname or IP ?',
    default: 'iotserver',
    validate: function(str){
        let pass = str.match(HostnameRegex);
        if (pass) return true;
        return 'Please enter a valid IP or DNS hostname';
    }
},{
    name: 'port',
    message: 'What is the HTTPS port of the IoT server?',
    default: '7102',
    validate: function(str){
        let pass = parseInt(str) && parseInt(str) <= 65535;
        if (pass) return true;
        return 'Please enter a valid port number';
    }
}];
const questions2 = [{
    name: 'deviceId',
    message: 'What is the ID of the device you registered?',
    default: '0-AQ'
},{
    name: 'sharedSecret',
    message: 'What the device shared secret?',
    default: 'raspberry'
}];
const finishSensors = 'Finish adding Sensors';
const questionsAddSensor = [{
    name: 'pin',
    message: 'What port on the Grove Pi is the sensor connected to?',
    type: 'list',
    choices: [finishSensors,'A0', 'A1', 'A2', 'D2', 'D3', 'D4', 'D5', 'D6', 'D7', 'D8'],
    default: finishSensors
},{
    name: 'type',
    message: 'What type of analog sensor is connected to that port?',
    type: 'list',
    choices: ['SoundAnalogSensor', 'RotaryAngleAnalogSensor', 'LightAnalogSensor'],
    when: answers => {
        console.log('answers = ', answers);
        return answers.pin.match(/^A./);
    }
},{
    name: 'type',
    message: 'What type of digital sensor is connected to that port?',
    type: 'list',
    choices: ['TemperatureAndHumiditySensor', 'UltrasonicRanger', 'LEDSocketKit', 'Button', 'Buzzer'],
    when: answers => {
        return answers.pin.match(/^D./);
    }
}];
const SENSOR_FILE_NAME = 'sensor-config.js';

const ui = new inquirer.ui.BottomBar();
const prompt = inquirer.createPromptModule();

var isSelfSigned;
var timer, progress;
var orighostname, hostname,port,deviceId,sharedSecret;
var sensorList;

prompt(questions)
    .then(answers => {
        ui.updateBottomBar('Checking server certificate...');
        orighostname = hostname = answers.hostname;
        port = answers.port;
        // now check if server is self signed
        return exec('openssl s_client -showcerts -connect '+hostname+':'+port+' -verify 0 -no_ign_eof', true);
    })
    .then(result => {
        isSelfSigned = result.stderr.match(/self signed certificate/);
        if (isSelfSigned) {
            ui.updateBottomBar('Server is using a self signed certificate so saving certificate...  Note: Can take 20seconds or so.\n');
            progress = 0;
            gauge.show('', progress);
            timer = setInterval(() => {
                progress += 0.01;
                gauge.show('', progress);
                gauge.pulse(0);
            }, 500);

            ui.updateBottomBar(chalk.bold.red('ALERT: You need to add the line "'+hostname+'    iotserver" to the bottom of the file "/etc/hosts"'));
            hostname = 'iotserver';
            return exec('openssl s_client -showcerts -connect '+orighostname+':'+port+' -no_ign_eof | (openssl x509 -out iotserver.pem; openssl x509 -out iotca.pem)');
        } else {
            return exec('wget --output-document=iotca.pem "http://www.symantec.com/content/en/us/enterprise/verisign/roots/Class-3-Public-Primary-Certification-Authority.pem"');
        }
    })
    .then(() => {
        clearInterval(timer);
        gauge.hide();
        return inquirer.prompt(questions2);
    })
    .then(answers => {
        ui.updateBottomBar('Creating device trust store...');
        deviceId = answers.deviceId;
        sharedSecret = answers.sharedSecret;
        // delete any old trust store first
        if (fs.existsSync(deviceId + '.json')) fs.unlinkSync(deviceId + '.json');
        return exec('node node_modules/iotcs-client-device/trusted_assets_provisioner.js  -serverHost '+hostname+' -serverPort '+port+' -truststore iotca.pem -taStorePassword changeit -deviceId '+deviceId+' -sharedSecret '+sharedSecret);
    })
    .then(() => {
        console.log(chalk.green('\nYou now need to tell us what sensors you have connected to the Grove Pi.'));
        return createSensorList();
    })
    .then((sensors) => {
        sensorList = sensors;
        if (fs.existsSync(SENSOR_FILE_NAME)) {
            return inquirer.prompt([{
                name: 'save',
                message: 'Sensor config ['+SENSOR_FILE_NAME+'] exists, overwrite?',
                type: 'confirm',
                default: true
            }]);
        } else {
            return {save:true};
        }
    })
    .then((answer) => {
        // write sensors config file
        if (answer.save) {
            fs.writeFileSync(SENSOR_FILE_NAME,
                'var config = ' + JSON.stringify(sensorList, null, 2) + ';\n' +
                'module.exports = config;\n',
                {encoding: 'utf8'});
            console.log('Written sensor config [' + SENSOR_FILE_NAME + ']');
        }
        // check if host file is correct
        if (isSelfSigned) {
            var hosts = fs.readFileSync('/etc/hosts', {encoding: 'utf8'});
            var hasIoTServer = hosts.match(/iotserver/) ? true : false;
            if (!hasIoTServer) {
                console.log(chalk.bold.red('\nALERT: You need to add the line "' + hostname + '    iotserver" to the bottom of the file "/etc/hosts"'));
            }
        }
        // give date time warning
        console.log(chalk.red('\nALERT: make sure you change the device model in the top of client.js before running it'));
        console.log(chalk.green('Done. You can now run the app with the command "node client.js '+deviceId+'.json"'));
    })
    .catch(err => {
        console.log('err = ', err);
    });

function createSensorList(sensors) {
    if (!sensors) sensors = [];
    return inquirer.prompt(questionsAddSensor)
        .then((answers) => {
            if (answers.pin === finishSensors) {
                return sensors;
            } else {
                sensors.push(answers);
                return createSensorList(sensors);
            }
        });
}

// helper function to make exec into a promise
function exec(command, ignoreErrResult) {
    return new Promise((resolve, reject) => {
        child_process.exec(command, (err,stdout, stderr) => {
            if (err && !ignoreErrResult) {
                reject(err);
            }
            resolve({stdout: stdout,stderr: stderr});
        });
    });
}
