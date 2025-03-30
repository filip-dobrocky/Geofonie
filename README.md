# Dependencies
- [ArduinoOSC](https://docs.arduino.cc/libraries/arduinoosc/)
- [MIDI Library](https://docs.arduino.cc/libraries/midi-library/)
- [ESP-FlexyStepper](https://docs.arduino.cc/libraries/esp-flexystepper/)
- [ESP32Servo](https://docs.arduino.cc/libraries/esp32servo/)
- [Pololu VL53L4CD](https://github.com/pololu/vl53l4cd-arduino)

# Roto MIDI messages:
    CC1: sensor value
    CC2: servo min angle / (sample pos) loop start pos
    CC3: servo max angle / (sample pos) loop end pos
    CC4: rotation speed  / (sample pos) loop speed
    CC5..10: misc1..6


# OSC messages:
    [obj_id]: 0, 1, 2, 3, 4
    [value] : 0..1

## Roto objects

### Rotation
#### Speed:
`/toRoto/rotation/speed [obj_id] [value]`  
#### Direction:
`/toRoto/rotation/direction [obj_id] [value]`  

### Servo
#### Center position:  
`/toRoto/servo/center [obj_id] [value]`  
#### Angle range (0 = no movement):  
`/toRoto/servo/angle [obj_id] [value]`  
#### Movement mode:
    0 = STATIC
    1 = ALTERNATING
    2 = LINEAR
`/toRoto/servo/mode [obj_id] [value]`  
#### Movement direction (0 / 1):  
`/toRoto/servo/direction [obj_id] [value]`  
#### Servo calibration (0 / 1):
Stop automatic movement and set angle based on `center` message  
`/toRoto/servo/calibrate [obj_id] [value]`  

### Sensor calibration
#### Lower limit:
`/toRoto/calibration/minDist [obj_id] [value]`  
#### Upper limit:
`/toRoto/calibration/maxDist [obj_id] [value]`  

#### Value read from sensor (0..1):
Sent from object to all devices  
`/info/reading [obj_id] [value]`  

### Misc parameters:
Sent to Daisy Seed as MIDI CC 5-10 to be used as sound synthesis parameters  
`/toRoto/misc/[1-6] [obj_id] [value]`   

## Acid objects

Generic parameters sent to Daisy Seed as MIDI CC 1-6  
`/toAcid/param[1-6] [obj_id] [value]`  

## Global
### Rotation speed
`/global/rotSpeed [value]`
### Autonomous mode
Automatic randomization of `/toAcid` parameters
#### Auto mode on/off
`/global/auto [value]`
#### Auto mode density
`/global/autoDensity [value]`

# Network config

Network parameters are defined in `NetworkConfig.h`

SSID: `TrychtyrLOM`  
Password: `LomLomLom`  

Access point IP: `192.168.0.201`  
Port for sending OSC messages to AP (objects): `54345`  
Port for listening to `/info` messages: `54355`  
Port for sending to individual objects (IPs are defined in the header): `54350`  
