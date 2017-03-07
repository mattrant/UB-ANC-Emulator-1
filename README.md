Copyright © 2014 - 2017 Jalil Modares

This program is part of my Ph.D. Dissertation research in the Department of Electrical Engineering at the University at Buffalo. I work in UB's Multimedia Communications and Systems Laboratory with my Ph.D. adviser, Prof. Nicholas Mastronarde <http://www.eng.buffalo.edu/~nmastron/>.

If you use this program for your work/research, please cite:
J. Modares, N. Mastronarde, K. Dantu, "UB-ANC Emulator: An Emulation Framework for Multi-agent Drone Networks" <https://doi.org/10.1109/SIMPAR.2016.7862404>.

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.

# UB-ANC Emulator
An Emulation Framework for Multi-Agent Drone Networks

Build
--------------

There are three main steps to build UB-ANC Emulator:

1) build ArduCopter Software in the Loop (SITL):
  - http://ardupilot.org/dev/docs/setting-up-sitl-on-linux.html

2) build APM Planner 
  - https://github.com/ArduPilot/apm_planner

3) build UB-ANC Emulator

```
git clone https://github.com/jmodares/UB-ANC-Emulator
mkdir build-emulator
cd build-emulator
qmake ../UB-ANC-Emulator
make -j4
```

Configuration
-------------

In order to configure firmware, run the APM Planner, and make a TCP connection to firmware, and change these parameters:
  - SYSID_THISMAV
  - ARMING_CHECK

For configuring UB-ANC Emulator, follow the steps below:

```
mkdir objects
cp path_to_ArduCopter.elf_and_eeprom.bin objects/your_drone_name/firmware_and_eeprom.bin
cp path_to_agent objects/your_drone_name/agent
```

Each port represents an agent, starting from 5763, 5773, etc.
