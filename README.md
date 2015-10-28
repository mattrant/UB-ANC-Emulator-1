# UB-ANC Emulator
The software emulator for UB-ANC, an open test-bed platform for airborne networking and communications

===================================

There are three main steps to use UB-ANC Emulator:

1) build and configure ArduCopter firmware

2) build APM Planner 

3) build and configure UB-ANC Emulator

===================================

The steps for making ArduCopter firmware:

1) git clone https://github.com/diydrones/ardupilot

2) cd ardupilot/ArduCopter

3) make sitl -j4

4) ./ArduCopter.elf --home 43.000496,-78.777788,0,0 --model quad --wipe --instance 0

-----------------------------------

For building APM Planner, follow the steps here:

https://github.com/diydrones/apm_planner

-----------------------------------

In order to configure firmware, run the APM Planner, and make a TCP connection to firmware, and change these parameters:

1) SYSID_THISMAV

2) ARMING_CHECK_ALL

-----------------------------------

For building and configuring UB-ANC Emulator, follow the steps below:

1) git clone https://github.com/jmodares/UB-ANC-Emul

2) cd UB-ANC-Emul

3) qmake UB-ANC-Emul.pro

4) make -j4

5) mkdir objects

6) cp path_to_ArduCopter.elf_and_eeprom.bin objects/your_drone_name/firmware_and_eeprom.bin

7) cp path_to_agent objects/your_drone_name/agent

8) ./emulator --waypoints mission.txt

9) connect to TCP ports 5763, 5773, ..., each port represents a drone.
