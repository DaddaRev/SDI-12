/* SDI-12 Protocol Integration in C
 * by Davide Reverberi
 *
 *
 * Required already configured:
 * -1 GPIO
 * -1 Low Power Timer (LPTIMER2)
 *
 * Notes:
 * The code is hard coded for using the LPTIMER2.
 * Read the doc in GitHub repository for a better understanding.
 *
 * Last Update: 22/09/2024
 */

#ifndef SDI_12_H
#define SDI_12_H

#include <stdint.h>
#include "stm32wlxx_hal.h"

// Function prototype
void Reconfigure_GPIO_Pins(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, int isOutput);

#endif

