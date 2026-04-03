# Setup for Mega + Yaseu GS 5500 rotator with HackRF

## Docker setup

`- 'c 166:* rmw'` is for serial devices

`- 'c 189:* rmw` is for HackRF 

HackRF needs usb 
Mega needs serial - use `/dev/serial/by-id/`

`rigctld` is need - but isn't used - HackRF uses Soapy

rotator control is mapped directly into SatNOGS - not using `rotctld` 

### HackRF 
make sure `hackrf_info` works on the host before trying it in the container - you'll need to install the HackRF tools on the host 
also install this rules file: https://github.com/greatscottgadgets/hackrf/blob/main/host/libhackrf/53-hackrf.rules 

## Mega setup 
Needed to wipe EEPROM between flashes - used Arduino Example 

Needed to change `rotator_settings.h`
```c++
#define AZIMUTH_STARTING_POINT_EEPROM_INITIALIZE 0      // the starting point in degrees of the azimuthal rotator - only used for initializing EEPROM the first time the code is run                                               
```
With this flow: 

set rotator to AZ: 0 EL: 0 --> wipe Mega's EEPROM --> program with updated K3NG rotator controller code --> Done. 

Don't use debug macro use `\D` in the terminal to toggle it - debug cannot be on or it will not work with anything but manual commands