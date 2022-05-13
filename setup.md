# How to start using this project

## Hardware requirements
This software is written specifically for the MSP432E411Y-BGAEVM development kit and VF's MDG-04 sensor.
The sensor is expected to be connected to the UART6 peripheral using an RS-485 to UART converter, with the DE/nRE wire connected to the GPIO port H pin 4.
The display is expected to be the KDK350ADPTR-EVM plug-in board connected to the J9 header.
The microcontroller can be connected to a PC via USB using the Spectrum Digital XDS200 JTAG Emulator or compatible devices.

## Software requirements
To easily compile and run this project on the target device, a user needs to have the Texas Instrument's Code Composer Studio software with the Simplelink MSP432 component installed and the Simplelink MSP432E4 SDK.
Both of these can be downloaded at the Texas Instruments website.
For the development and testing of this project, the Code Composer Studio version 11.2.0 and Simplelink MSP432E4 SDK version 4.20.00.12 were used.

## Importing the project
Once the hardware and software requirements are satisfied, take the following steps:
1. Open Code Composer Studio
2. From the top menu, open "Project" -> "Import CCS Projects..."
3. In the import dialog, select the "Select archive file:" radio button at the top
4. Click "Browse..." and select the source_code.zip file
5. Click "Finish"

## Possible issues
If the project fails to build with the "Product com.ti.SIMPLELINK_MSP432E4_SDK..." is not currently installed even though it is, follow these steps:
1. From the top menu, open "Window" -> "Preferences"
2. In the menu on the left select "Code Composer Studio" -> "Products"
3. Click "Refresh"
4. The Simplelink MSP432E4 SDK should show in the "Discovered Products" field.
5. Click "Apply and close"

Now the project should be ready to build.
