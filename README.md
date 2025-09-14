# Geofonie
A networked sound art installation born out of [Trychtyr](https://svitava.org/project/trychtyr/) project of [SVITAVA transmedia art lab](https://svitava.org/).

This repository contains all of the software and also the hardware documentation.

The ESP32 firmware is in the `PlatformIO` directory, the `Daisy` directory contains Gen patches for [Oopsy](github.com/electro-smith/oopsy/), current: `Granular/geofonie_gen_simpler2.maxpat`.

# Dependencies
- [OSC Control](https://git.iem.at/uC/OSC_control)
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
`/fromRoto/reading [obj_id] [value]`  

### Misc parameters:
Sent to Daisy Seed as MIDI CC 5-10 to be used as sound synthesis parameters  
`/toRoto/misc/[1-6] [obj_id] [value]`   

## Acid objects

Generic parameters sent to Daisy Seed as MIDI CC 1-10  
`/toAcid/param[1-10] [obj_id] [value]`  

## Global
### Rotation speed
`/toRoto/global/rotation/speed [value]`
### Rotation direction
`/toRoto/global/rotation/direction [value]`
### Autonomous mode
Automatic randomization of `/toAcid` parameters
#### Auto mode on/off
# TODO: auto messages new version
`/global/auto [value]`
#### Auto mode density
`/global/autoDensity [value]`

# Network config

Network parameters are defined in `NetworkConfig.h`

SSID: `TrychtyrLOM`  
Password: `LomLomLom`  

Port for sending to mesh objects: `54350`  
Port for listening to messages from mesh: `54355`  

#### (Deprecated)
Access point IP: `192.168.0.201`  
Port for sending OSC messages to AP (objects): `54345`  
