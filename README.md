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
### Roto misc
`/toRoto/global/misc/[1-6] [value]` 
### Acid params
`/toAcid/global/param[1-10] [value]` 

#### Object alive, with firmware version:
`/fromRoto/ping [obj_id] [fw_version]`, `/fromAcid/ping [obj_id] [fw_version]`

# Network config

Network parameters are defined in `NetworkConfig.h`

SSID: `TrychtyrLOM`
Password: `LomLomLom`

Port for sending to mesh objects: `54345`
Port for listening to messages from mesh: `54355`

There is no infrastructure access point. Every object runs painlessMesh in
AP_STA and advertises `TrychtyrLOM` as its own softAP, so **join any object's
AP** and send to its gateway address (`10.x.y.1`). That object relays every
`/to...` message it receives across the mesh, and re-emits telemetry it hears
from the mesh to its own AP, so it does not matter which one you attach to.
Roto and Acid share one mesh; Roto object 0 is the mesh root anchor.

# Firmware

Object ids live in NVS, not in the binary, so one image serves every object.

1. **Provision** each board once over USB with its own env: `xiao_dev_0` ...
   `xiao_dev_4` (in `ESPRoto/` or `ESPAcid/`). This writes the id and nothing
   else depends on it afterwards.
2. **Update** over the mesh: join any object's AP and run
   `python PlatformIO/tools/ota_update.py --project roto --watch`
   (`--project acid` for the Acid objects; the two roles cannot flash each
   other, so run it once per project). It builds `xiao_deploy`, uploads to the
   object's `/fw` endpoint, and that object distributes to the mesh and
   self-flashes last. `http://10.x.y.1/` serves the same thing as a form.

A crash-looping image can only be recovered over USB, so bench one board with
`xiao_ota` before distributing.
