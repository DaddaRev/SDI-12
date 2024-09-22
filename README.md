# SDI-12
Implementation of the SDI-12 protocol in the C language for integration into STM32 microcontrollers code.
To integrate the code for the operation of the protocol, it is necessary to have a microcontroller from the STM32 family. Code development via STM32CubeIDE is recommended for configuring GPIOs and Timers through the graphical interface made available. <br>
For a better understanding of the SDI-12, check the official documentation [here](https://www.sdi-12.org/current_specification/SDI-12%20Specification%201.4%20February%2020%202023.pdf).
<br>
Required:
- 1 GPIO already configured. Pin used for the digital communication.
- LPTIMER2. Low Power Timer used for synchronization functions.
  
The code shown includes two C files (with their respective headers) :
1. **reconfigFun.c**. In which the reconfigFun() is used for the reconfiguration as an input or output of the pin used for digital communication on the SDI-12 bus.
2. **sdi12.c**. Inside there are the functions for digital communication of the SDI-12 protocol.

### Example of application:
Use of the digital protocol for the communication of agricultural sensors such as TEROS 12. The reported code provides the functions necessary for communication operations.




