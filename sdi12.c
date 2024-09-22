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

#include <stdint.h>
#include <stdio.h>

#include "stm32wlxx_hal.h"
#include "stm32wlxx_hal_lptim.h"
#include "sdi12.h"

#include <stdbool.h>

// Define constants for buffer size and timeout
#define BUFFER_SIZE 64
#define TIMEOUT_MS 1000
uint32_t autoreload_value = 19999;  //20000 -1 + div1 prescaler = 1200Hz

// Function prototypes to manage timer
void StartTimer(void);
void NextTimer(void);
void ResetTimer(void);

//Function to start the Timer
void StartTimer(){
    HAL_LPTIM_Counter_Start(&hlptim2, autoreload_value);
}

//Function to wait till the end of the execution of the timer
void NextTimer(){
    while (__HAL_LPTIM_GET_FLAG(&hlptim2, LPTIM_FLAG_ARRM) == RESET)
    {
    	//Wait till the timer reset flag is changed (timer finished)
    }

    //Timer period is terminated
    __HAL_LPTIM_CLEAR_FLAG(&hlptim2, LPTIM_FLAG_ARRM);
}

//Function to reset the timer
void ResetTimer(void)
{
    // Stop the timer
    HAL_LPTIM_Counter_Stop(&hlptim2);

    // Clear the ARRM flag
    __HAL_LPTIM_CLEAR_FLAG(&hlptim2, LPTIM_FLAG_ARRM);

    // Restart the timer with the same autoreload value
    HAL_LPTIM_Counter_Start(&hlptim2, autoreload_value);
}

/// Wake the sensor on the bus with break and marking signals.
///
/// The break signal is at least 12 milliseconds (12,000 탎) long.
/// The marking signal is at least 8.333 milliseconds (8,333 탎) long.
///
/// # Arguments
///
/// * `GPIO_Pin` - The output pin used for the communication.
/// * 'GPIOx'  - Register port pointer that identifies the GPIO_Pin
///
/// # Returns
///
/// * `HAL_OK` - The signals were sent successfully.
///
/// # Type parameters
///
/// * `uint16_t` - The type of the pin used for the communication.
HAL_StatusTypeDef wake_up_sensors(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {

    // Break of at least 12 milliseconds (12,000 탎).
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
    HAL_Delay(13);

    // Marking of at least 8.33 milliseconds (8,333 탎).
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
    HAL_Delay(9);

    return HAL_OK;
}

/// Send a command to the SDI-12 device.
///
/// # Arguments
///
/// * `GPIO_Pin` - The output pin used for the communication.
/// * 'GPIOx'  - Register port pointer that identifies the GPIO_Pin.
/// * `command` - The command to send.
/// * 'length' - length of the command to send.
///
/// # Returns
///
/// * `HAL_OK` - The command is written successfully.
HAL_StatusTypeDef write_command(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, const uint8_t* command, size_t length) {
    HAL_StatusTypeDef status;

    ResetTimer();

    // Send each character of the command.
    for (size_t i = 0; i < length; i++) {
        status = write_char(GPIOx, GPIO_Pin, command[i]);
        if (status != HAL_OK) {
            return status;  // Error handling if write_char fails
        }
    }
    return HAL_OK;
}

/// Send a character to the SDI-12 device.
///
/// # Arguments
///
/// * `GPIO_Pin` - The output pin used for the communication.
/// * 'GPIOx'  - Register port pointer that identifies the GPIO_Pin.
/// * `character` - The character to send.
///
/// # Returns
///
/// * `HAL_OK` - The command is written successfully.
HAL_StatusTypeDef write_char(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t character) {

    // Start bit
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
    ResetTimer();
    StartTimer();
    ResetTimer();

    // Calculate the parity bit and add it as MSB
    character |= (even_parity_bit(character) << 7);

    //  Hold the line for the rest of the start bit duration.
    NextTimer();
    StartTimer();
    NextTimer();
    StartTimer();

	// Transmit each of the 7 bit using negative logic
	// starting from the least significant bit
    for (int i = 0; i < 8; i++) {
        if (character & 1) {
            HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
        } else {
            HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
        }
        character >>= 1;

		// Hold the line for this bit duration
        NextTimer();
        StartTimer();
        NextTimer();
        StartTimer();
    }

    // Stop Bit
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);

    //  Hold the line low for the rest of the stop bit
    NextTimer();
    StartTimer();
    NextTimer();
    StartTimer();

    return HAL_OK;
}

/// Function to read response form the SDI-12 bus.
///
/// # Arguments
///
/// * `GPIO_Pin` - The output pin used for the communication.
/// * 'GPIOx'  - Register port pointer that identifies the GPIO_Pin.
/// * `character` - The character to send.
/// *  buffer   - Pointer to the buffer for storing the read characters.
/// *  buffer_size - Size of the buffer.
/// *  timeout_ms  - Timeout in milliseconds.
/// *  error  -  Pointer to store the error status.
///
/// # Returns
///
/// * uint8_t*   Pointer to the buffer with the response or NULL on error.
uint8_t* read_response(uint8_t* buffer, size_t buffer_size, uint32_t timeout_ms, ReadError* error, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
    uint8_t byte = 0;
    uint8_t char_value;
    size_t char_idx = 0;
    bool pin_low;

    uint32_t start_tick = HAL_GetTick(); // Getting the initial tick for the timeout
    ResetTimer();

    while (1) {

        // Wait for the start bit.
        while (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == GPIO_PIN_RESET) {
            if ((HAL_GetTick() - start_tick) > timeout_ms) {
                *error = ReadError_Timeout; 
                return NULL; 
            }
        }
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6);

        // Offset the sampling by half the bit period:
        ResetTimer();
        for (int i = 0; i < 3; i++) {
        	NextTimer();
        	StartTimer();
        }

        //Sample 7 data bits + parity bit.
        for (int i = 0; i < 8; i++) {

            byte >>= 1;  // Left shift the byte by one bit.

            // Read the bit value using negative logic (example logic, replace with actual pin read logic)
            pin_low = HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == GPIO_PIN_RESET;
            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6);

            if (pin_low) {
                byte |= 0b10000000; // Set the MSB
            } else {
                byte &= 0b01111111; // Reset the MSB
            }

			//  Hold the line for the rest of the bit
            NextTimer();
            StartTimer();
            NextTimer();
            StartTimer();
        }

        // Check the stop bit (replace with actual pin read logic)
        if (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == GPIO_PIN_SET) {   //Stop bit must be LOW
            *error = ReadError_WrongStopBit;
            return NULL;
        }

        // Check the validity of the even parity bit.
        char_value = byte & 0b01111111;
        //printf("%x\n", char_value);
        if (even_parity_bit(char_value) != (byte & 0b10000000)) {
            *error = ReadError_ParityBitMismatch;
            return NULL;
        }

        // Save the character into the buffer, if enough space is available.
        if (char_idx >= buffer_size) {
            *error = ReadError_BufferOverflow;
            return NULL;
        }

        // Break the loop if a new line is detected.
        if (char_value == '\r') {
            break;
        }

        // Save the character into the buffer.
        buffer[char_idx++] = char_value;
    }

    // Null-terminate the buffer (optional based on application requirement)
    buffer[char_idx] = '\0';

    *error = ReadError_None;
    return buffer;
}


// Function to count the number of bits set to 1 in a byte
uint8_t count_ones(uint8_t word) {
    uint8_t count = 0;
    while (word) {
        count += word & 1; // Increment count if the least significant bit is 1
        word >>= 1;        // Shift the word to the right by 1 bit
    }
    return count;
}

// Function to calculate the even parity bit and shift it to the most significant bit (MSB)
uint8_t even_parity_bit(uint8_t word) {
    // Calculate parity: 0 if the number of 1s is even, 1 if odd
    uint8_t parity = (count_ones(word) % 2) << 7; // Shift the parity bit to the MSB position
    return parity;
}




