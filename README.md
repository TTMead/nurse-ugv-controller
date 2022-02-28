# Nurse UGV  
Hello all,  
This is a repository to store the embedded source files for the rover controller.  

This codebase was generated based on the **64-NUCLEO** board for the **STM32F411RE** microcontroller.  

## Codebase Architecture  
This codebase runs [CMSIS-RTOS V2](https://www.keil.com/pack/doc/CMSIS/RTOS2/html/group__CMSIS__RTOS.html) which is an abstraction layer of [FreeRTOS](https://www.freertos.org/a00106.html) (Free, Real Time Operating System). This is probably a different architecture to the Arduino Library that you may be used to.

When developing C code using an Arduino Libary a *Super Loop* architecture is used. This is where you have a *setup* function which runs at the start of the program and the titular *loop* function which runs constantly in a loop. This is different to developing in an RTOS where multiple threads are created and run concurrently by a scheduler that switches between tasks. For more information on this, I'd recommend the [DigiKey tutorial](https://www.digikey.com.au/en/videos/d/digi-key-electronics/introduction-to-rtos-part-1-what-is-a-real-time-operating-system-rtos) series.  

## Where do our files go?  
Most of the code in this repo is auto-generated by the STM32Cube SDK and should not be touched. The parts of the codebase that should be modified are the **nucleo-f411re-rover.ioc** file or the files in **Core/Src/Modules** and **Core/Src/Libraries**. The rest of the code should be auto-generated via the .ioc and project settings files.  

## Build/Development Environment  
To build this repository you will require the **STM32CubeIDE** to run the build target for the microcontroller.  
To simply develop the code without running build targets, any IDE should be fine.  

I would recommend you develop your software on your personal microcontrollers before transferring them to this codebase.  
When you want to test the code on the STM, you can add your code as a thread to the **Core/Src/Modules** directory.  

- Timothy
