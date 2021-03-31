#include <usbtransfer.h>
#include "measurement_logger.h"
#include "gui_comm.h"

/**
 * Number of entries in all buffers
 */
#define BUF_SIZE 4096//TODO

/**
 * Example buffer
 */
int32_t ringBuffer[4][BUF_SIZE]; //TODO

/**
 * DO NOT CHANGE: Number of entries in all buffers
 */
const uint32_t outgoingBufferSize = BUF_SIZE;

/**
 * Offset into all ringBuffers. Points to the next-to-write
 */
uint32_t outgoingBufferOffset = 30;

const bufferDef_t buffers[] = { //TODO
		{ ringBuffer[0], (ringBuffer[0] + BUF_SIZE) }, { ringBuffer[1],
				(ringBuffer[1] + BUF_SIZE) }, { ringBuffer[2], (ringBuffer[2]
				+ BUF_SIZE) }, { ringBuffer[3], (ringBuffer[3] + BUF_SIZE) } };

/**
 * DO NOT CHANGE: Number of buffers declared in [buffers]
 */
const uint8_t outgoingBufferCount = sizeof(buffers) / sizeof(bufferDef_t);

void logMeasurements(int count, const int32_t measurements[]) {
	//TODO
	int ii;

	if (outgoingBufferWriteProtection == 0) {
		for (ii = 0; ii < count; ii++) {
			ringBuffer[ii][outgoingBufferOffset] = measurements[ii]; //verschieden Messdaten synchron aufgezeichnet werden
		}

		outgoingBufferOffset = (outgoingBufferOffset + 1) % BUF_SIZE; //nextIndex=(index+increment) mod size
	}
}
