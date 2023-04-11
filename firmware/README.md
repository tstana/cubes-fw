## Libero SoC Firmware Export Files

Drivers corresponding to peripherals implemented on the FPGA.

The latest programming files and gateware can be found at the links below.

*NB: Check the dates in the file name for downloading the appropriate file.*

* Libero SoC project archives: [archives folder on CUBES Dropbox](https://www.dropbox.com/sh/cmfis964frhkbvq/AAD4gc_wlUnyZN-tQpHhtea9a?dl=0)
* STAPL programming files: [stapl folder on CUBES Dropbox](https://www.dropbox.com/sh/9nj39ly537fefnt/AAAtknOk6K9m5W_FcHFw_FaSa?dl=0)

In the off-case you need to regenerate the firmware drivers:

1. Leave this folder as it stands.
2. In Libero, run through the design compilation process.
3. After Libero design compilation has finished, double-click the **Export Firmware** button
   in the _Design Flow_ pane.
4. Copy the contents of the `firmware/` folder from the Libero project over to this folder.
5. Compile the `cubes-fw` project.
