# Firmware for the CUBesat x-ray Explorer using Scintillators (CUBES)

- [Introduction](#introduction)
- [Getting Started](#getting-started)
- [Folder Structure](#folder-structure)
  - [Regenerating the `firmware` folder](#regenerating-the-firmware-folder)
  - [Regenerating the `msp` folder](#regenerating-the-msp-folder)
- [Programming](#programming)
  - [Programming Port and Board ID](#programming-port-and-board-id)
  - [Debug: Program to CUBES RAM](#debug-program-to-cubes-ram)
  - [Production: Program to CUBES NVM](#production-program-to-cubes-nvm)

## Introduction

The code on this repository runs on the Cortex-M3 core embedded on the SmartFusion2 FPGA.

## Getting Started

- Clone this repository/unzip the archive downloaded via GitHub
- Download SoftConsole (any latest version should work)
- Import this project into SoftConsole
- Most of the code is under `main.c`, in the `while (1)` loop:
  - `REQ_HK` data is prepared once a second; the second counting is handled
    by `Timer1` (see also `Timer1_IRQHandler()`);
  - `REQ_PAYLOAD` data is prepared once the DAQ has finished; note that this
    is a lenghty process (several hundreds of microseconds), especially if
    histogram binning is performed;
  - MSP commands are then handled in the next `if`/`else if` statements.
- MSP callbacks functions and some ISRs are also present at the bottom of
  `main.c`.
- Other [folders](#folder-structure) under `cubes-fw` contain various APIs for handling devices
  on the CUBES board.

## Folder Structure

- `cubes-fw`
  - `firmware    // Code generated from Libero`
  - `hk_adc      // API to handle the ADS1015 ADC`
  - `hvps        // API to handle the C11204-02 HVPS module`
  - `mem         // API to handle CUBES memory accesses`
  - `msp         // MSP API functions, generated using the Python script supplied with MSP`
  - `utils       // Various utilitary APIs, e.g., to handle the on-board LED`

### Regenerating the `firmware` folder

- See [firmware/README.md](firmware/README.md). 

### Regenerating the `msp` folder

- Externally from the `cubes-fw` repository, obtain the MSP repository from GitLab:
  - `git clone https://gitlab.com/kth-mist/msp msp-repo`
- Copy/move the file `cubes-fw/msp/msp_configuration.h` to somewhere you know; this
  file will need to replace whatever gets generated by the MSP `conf.py` script!
- Delete all contents inside the `cubes-fw/msp/` folder;
- Use `msp-repo/conf.py` to generate a new set of MSP files;
- Copy these files from `msp-repo/target/` to `cubes-fw/msp/`.
- Copy back the `msp_configuration.h` file you saved earlier over that generated by
  the `conf.py` script.

## Programming

### Programming Port and Board ID

CUBES should be programmed using a FlashPro5 programmer from Microchip. The standard
10-pin JTAG connector supplied with the FlashPro5 should be adapted to the 1.27-mm pitch
programming port on the CUBES board using a 2.54-to-1.27-mm adapter. One such adapter can
be found in AlbaNova, in the "PoGO lab" on the bottom floor of the building. The adaptor
should be located in the plastic box labeled "CUBES".

Each CUBES board has its own ID, labeled on the top side of CUBES boards, corresponding to
the -Z face of the satellite. The number on this label corresponds to the second character
in the `BOARD_ID` macro, defined via a `-D` compiler flag:

- `BOARD_ID = C<number-on-label>` (`C` stands for CUBES)
- Example: `BOARD_ID = C3`:

  <img alt="cubes-prog.png" src="/fig/cubes-prog.png" width=800px />

### Debug: Program to CUBES RAM

1. Open the **Properties** window for the `cubes-fw` project by either right-clicking
   the project in SoftConsole and pressing **Properties** in the pop-up menu, or
   selecting the project in SoftConsole and hitting **Alt+Enter** on your keyboard.
2. Navigate to **C/C++ Build > Settings > GNU ARM Cross C Linker > General** and
   double-click the script file:
   
   <img alt="linker-script-file-nav.png" src="/fig/linker-script-file-nav.png" width=800px />

3. Ensure the `esram` linker script is used for programming:

   <img alt="esram-linker-script.png" src="/fig/esram-linker-script.png" width=640px />
   
4. Press the **Apply and Close** button in the **Properties** dialog.
5. Connect the FlashPro5 programmer to the CUBES PCB of interest.
6. Press the **Debug** (bug) button on the SoftConsole interface.
7. The code should now be programmed to CUBES volatile memory; HK can be read out
   from CUBES and checked if it "makes sense".

### Production: Program to CUBES NVM

1. Open the **Properties** window for the `cubes-fw` project by either right-clicking
   the project in SoftConsole and pressing **Properties** in the pop-up menu, or
   selecting the project in SoftConsole and hitting **Alt+Enter** on your keyboard.
2. Navigate to **C/C++ Build > Settings > GNU ARM Cross C Linker > General** and
   double-click the linker script file:
   
   <img alt="linker-script-file-nav.png" src="/fig/linker-script-file-nav.png" width=800px />
   
4. To ensure the code is programmed to non-volatile memory, change `esram` to `envm`
   at the very end of the text line in the dialog that appears:
   
   <img alt="envm-linker-script.png" src="/fig/envm-linker-script.png" width=640px />

4. Still in the **Properties** window, **C/C++ Build > Settings**, navigate to
   **GNU ARM Cross C Compiler > Preprocessor**.
5. Change the `BOARD_ID` macro for the board you are programming as outlined in
   the [section above](#programming-port-and-board-id).
6. Change the `MSP_EXP_ADDR` macro for the board according to the `i2c.h` config file
   under the `obcsw` GitLab repository.
   
   ![board-id-i2c-addr-change.png](fig/board-id-i2c-addr-change.png)
   
7. Click the Debug (yes -- Debug!) button in the SoftConsole main window to program the
   board. The **Console** window should display information about it programming to NVM.
   Programming should also take longer than in the case of RAM.
8. End the debug session from SoftConsole's _Debug_ perspective.
9. Move the FlashPro5 programmer from the board you just programmed to the next one you
   want to program.
10. Repeat steps 4-7.
11. Reset the CUBES stackup by power-cycling the `5V_CUBES` power rail.
12. Perform a `REQ_HK` MSP command from each of the two CUBES boards and ensure the
    readouts "make sense".

**IMPORTANT:**
- Revert the project file back to its original contents, to ensure you are not
  unnecessarily programming to NVM:

  `git restore .project`
   
