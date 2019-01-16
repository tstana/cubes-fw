# MSP Simulator config file

The software running on the OBC simulator is set up through the config file in
this folder.

To generate the OBC simulator code, replace the "config.txt" file in
  > https://gitlab.com/kth-mist/msp-simulator/tree/b1-addsim
with the one found in this folder. Then run "setup.py" with Python3 and the
code to be run on the MSP simulator will be generated in the "target/" folder.

In the current version, the simulator is configured to do some voltage changes
and a power cycle of the power supply.

The power cycle is set up to run through the default "ACTIVE", "SLEEP"
and "POWER_OFF" commands. *NB: This should be changed with custom commands!*
