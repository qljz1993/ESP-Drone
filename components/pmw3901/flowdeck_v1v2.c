/*
 *
 *
 * LPS node firmware.
 *
 * Copyright 2017, Bitcraze AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Foobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */
/* flowdeck.c: Flow deck driver */
#define DEBUG_MODULE "FLOW"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "debug_cf.h"

//#include "deck.h"
#include "debug_cf.h"
#include "system.h"
#include "log.h"
#include "param.h"
#include "pmw3901.h"
#include "sleepus.h"
#include "config.h"

#include "stabilizer_types.h"
#include "estimator.h"

#include "cf_math.h"

#include <stdlib.h>

#define AVERAGE_HISTORY_LENGTH 4
#define OULIER_LIMIT 100
#define LP_CONSTANT 0.8f
//#define USE_LP_FILTER
#define USE_MA_SMOOTHING

#if defined(USE_MA_SMOOTHING)
static struct {
    float32_t averageX[AVERAGE_HISTORY_LENGTH];
    float32_t averageY[AVERAGE_HISTORY_LENGTH];
    size_t ptr;
} pixelAverages;
#endif

float dpixelx_previous = 0;
float dpixely_previous = 0;

static uint8_t outlierCount = 0;

static bool isInit1 = false;
static bool isInit2 = false;

motionBurst_t currentMotion;

// Disables pushing the flow measurement in the EKF
static bool useFlowDisabled = false;

#define NCS_PIN 15

static void flowdeckTask(void *param)
{
    systemWaitStart();

    while (1) {
        vTaskDelay(10);

        pmw3901ReadMotion(NCS_PIN, &currentMotion);

        // Flip motion information to comply with sensor mounting
        // (might need to be changed if mounted differently)
        int16_t accpx = currentMotion.deltaY;
        int16_t accpy = currentMotion.deltaX;

        // Outlier removal
        if (abs(accpx) < OULIER_LIMIT && abs(accpy) < OULIER_LIMIT) {

            // Form flow measurement struct and push into the EKF
            flowMeasurement_t flowData;
            flowData.stdDevX = 0.25;    // [pixels] should perhaps be made larger?
            flowData.stdDevY = 0.25;    // [pixels] should perhaps be made larger?
            flowData.dt = 0.01;

#if defined(USE_MA_SMOOTHING)
            // Use MA Smoothing
            pixelAverages.averageX[pixelAverages.ptr] = (float32_t)accpx;
            pixelAverages.averageY[pixelAverages.ptr] = (float32_t)accpy;

            float32_t meanX;
            float32_t meanY;

            xtensa_mean_f32(pixelAverages.averageX, AVERAGE_HISTORY_LENGTH, &meanX);
            xtensa_mean_f32(pixelAverages.averageY, AVERAGE_HISTORY_LENGTH, &meanY);

            pixelAverages.ptr = (pixelAverages.ptr + 1) % AVERAGE_HISTORY_LENGTH;

            flowData.dpixelx = (float)meanX;   // [pixels]
            flowData.dpixely = (float)meanY;   // [pixels]
#elif defined(USE_LP_FILTER)
            // Use LP filter measurements
            flowData.dpixelx = LP_CONSTANT * dpixelx_previous + (1.0f - LP_CONSTANT) * (float)accpx;
            flowData.dpixely = LP_CONSTANT * dpixely_previous + (1.0f - LP_CONSTANT) * (float)accpy;
            dpixelx_previous = flowData.dpixelx;
            dpixely_previous = flowData.dpixely;
#else
            // Use raw measurements
            flowData.dpixelx = (float)accpx;
            flowData.dpixely = (float)accpy;
#endif

            // Push measurements into the estimator
            if (!useFlowDisabled) {
                estimatorEnqueueFlow(&flowData);
            }
        } else {
            outlierCount++;
        }
    }
}

void flowdeck2Init()
{
    if (isInit1 || isInit2) {
        return;
    }

    // Initialize the VL53L1 sensor using the zRanger deck driver
    // const DeckDriver *zRanger = deckFindDriverByName("bcZRanger2");
    // zRanger->init(NULL);

    if (pmw3901Init(NCS_PIN)) {
        xTaskCreate(flowdeckTask, FLOW_TASK_NAME, FLOW_TASK_STACKSIZE, NULL, FLOW_TASK_PRI, NULL);

        isInit2 = true;
    }
}

bool flowdeck2Test()
{
    if (!isInit2) {
        DEBUG_PRINTD("Error while initializing the PMW3901 sensor\n");
    }

    // Test the VL53L1 driver
    //const DeckDriver *zRanger = deckFindDriverByName("bcZRanger2");

    return isInit2;//zRanger->test();
}

LOG_GROUP_START(motion)
LOG_ADD(LOG_UINT8, motion, &currentMotion.motion)
LOG_ADD(LOG_INT16, deltaX, &currentMotion.deltaX)
LOG_ADD(LOG_INT16, deltaY, &currentMotion.deltaY)
LOG_ADD(LOG_UINT16, shutter, &currentMotion.shutter)
LOG_ADD(LOG_UINT8, maxRaw, &currentMotion.maxRawData)
LOG_ADD(LOG_UINT8, minRaw, &currentMotion.minRawData)
LOG_ADD(LOG_UINT8, Rawsum, &currentMotion.rawDataSum)
LOG_ADD(LOG_UINT8, outlierCount, &outlierCount)
LOG_GROUP_STOP(motion)

PARAM_GROUP_START(motion)
PARAM_ADD(PARAM_UINT8, disable, &useFlowDisabled)
PARAM_GROUP_STOP(motion)

PARAM_GROUP_START(deck)
PARAM_ADD(PARAM_UINT8 | PARAM_RONLY, bcFlow, &isInit1)
PARAM_ADD(PARAM_UINT8 | PARAM_RONLY, bcFlow2, &isInit2)
PARAM_GROUP_STOP(deck)
