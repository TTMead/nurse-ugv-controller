/******** nurse controller **********
 *
 * @file irreader.cpp
 * @desc Reads the IR sensors and publishes to the sensor topic.
 *
 * IMPORTANT NOTE: MX_DMA_Init() must be called before MX_ADC1_Init() or this won't work.
 * There is a current bug in STM32Cube that does this the wrong way.
 *
 * ADC details:
 *  - Scan Conversion Mode: Enabled
 *  - Continuous Conversion Mode: Disabled
 *  - Discontinuous Conversion Mode: Disabled
 *  - DMA Continuous Requests: Disabled
 *  - End of Conversion: EOC Flag at the end of a single channel conversion
 *
 * DMA details:
 *  - DMA2 Stream 0
 *  - Priority: Very High
 *  - Data Width: Half Word
 *  - Mode: Normal
 *
 * @author Timothy Mead
 *
 ************************************/

#include "irreader.hpp"

static bool print;

// Sensor ADC variables
volatile uint16_t ADC_VAL[8];
const int ADC_CHANNELS = 8;
volatile int ADC_READY = 0; // set by callback


static void run() {
	HAL_ADC_Start_DMA(sensor_adc_handle, (uint32_t*)ADC_VAL, 8);

	// While data is being sent by the dma
	while (ADC_READY == 0) {
		// Just wait a few milliseconds
		HAL_Delay(3);
	}
	ADC_READY = 0;


	// Data has been filled


	// Print the real-time sensor values to serial
	if (print) {
		ROVER_PRINTLN("[irreader] S1: %d, S2: %d, S3 %d, S4: %d, S5: %d, S6 %d, S7 %d, S8 %d", ADC_VAL[0], ADC_VAL[1], ADC_VAL[2], ADC_VAL[3], ADC_VAL[4], ADC_VAL[5], ADC_VAL[6], ADC_VAL[7]);
	}

	// Run time cyclic, record data 10 Hz
	HAL_Delay(100);
}


// Override hal callback for adc dma
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	ADC_READY = 1;
}




void StartIRReader(void *argument) {
	print = false;

	for (;;)
	{
		run();
	}

	osThreadTerminate(NULL);
}


int irreader_main(int argc, const char *argv[]) {
	if (argc < 2) {
		ROVER_PRINTLN("[irreader] Please provide a command: print");
		return 1;
	}

	if (!strcmp(argv[1], "print")) {
		if (argc < 3) {
			ROVER_PRINTLN("[irreader] Please state \"on\" or \"off\".");
			return 1;
		}

		if (!strcmp(argv[2],"off")) {
			print = false;
			ROVER_PRINTLN("[irreader] Printing off.");
			return 0;
		}

		if (!strcmp(argv[2],"on")) {
			print = true;
			ROVER_PRINTLN("[irreader] Printing on.");
			return 0;
		}

		ROVER_PRINTLN("[irreader] Please state \"on\" or \"off\".");
		return 1;
	}

	ROVER_PRINTLN("[irreader] Unknown command");
	return 1;
}





