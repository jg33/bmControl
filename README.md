# bmControl
A complete interface for programatic control of Black Magic Design cameras via the BMD Arduino SDI shield.

## Goals
* Create complete interface to control Black Magic cameras using OSC messages
* Develop Max patch to create GUI and presettable control

## Dependencies
* OSCuino: https://github.com/CNMAT/OSC
* CNMAT odot Max Objects: https://github.com/CNMAT/CNMAT-odot/releases
* Blackmagic SDI Library (provided with hardware)

## Usage
OSC Commands
* "/ping" returns "/pong"
* "/getStatus '<camera number>'" int, 0-n, returns multiple "/status/<camera number>/<variable> <value>"
* "/bmc/\<camera number>/aperture \<value>" float, 0.-22.
* "/bmc/\<camera number>/focus \<value>" float, 0.-22.
* "/bmc/\<camera number>/whiteBalance \<value>" int, 3000-7500
* "/bmc/\<camera number>/sensorGain \<value>" int, 1,2,4,8 or 16
* "/bmc/\<camera number>/lift <red> <green> <blue> <luma>" four floats, -4.-4.
* "/bmc/\<camera number>/gamma <red> <green> <blue> <luma>" four floats, -4.-4.
* "/bmc/\<camera number>/gain <red> <green> <blue> <luma>" four floats, -4.-4.


## License
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
