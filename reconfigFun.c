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

#include "reconfigFun.h"

/**
 * @brief Reconfigure a GPIO pin as input or output.
 *
 * @param GPIOx    Pointer to the GPIO port where the pin is located.
 * @param GPIO_Pin The specific GPIO pin to be reconfigured.
 * @param isOutput If true, configure as output; otherwise, configure as input.
 *
 * @note Configures the pin as either input or output, with high speed for output mode.
 */
void Reconfigure_GPIO_Pins(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, int isOutput)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = GPIO_Pin;

    if (isOutput)
    {
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    }
    else
    {
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
    }

    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

