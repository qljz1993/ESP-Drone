/**
*
 * ESPlane Firmware
 *
 * Copyright 2019-2020  Espressif Systems (Shanghai)
 * Copyright (C) 2011-2012 Bitcraze AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, in version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * debug.c - Various debug functions
 */
#include <stdint.h>
#define DEBUG_MODULE "FreeRTOSConfig"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "config.h"
#include "debug_cf.h"
#include "nvicconf.h"
#include "led.h"


uint32_t traceTickCount;

void vApplicationMallocFailedHook(void)
{
    portDISABLE_INTERRUPTS();
    DEBUG_PRINTE("Malloc failed!\n");
    ledSet(ERR_LED1, 1);
    ledSet(ERR_LED2, 1);

    while (1);
}

#if (configCHECK_FOR_STACK_OVERFLOW == 1)
void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed portCHAR *pcTaskName)
{
    portDISABLE_INTERRUPTS();
    DEBUG_PRINTE("\nStack overflow!\n");
    ledSet(ERR_LED1, 1);
    ledSet(ERR_LED2, 1);

    while (1);
}
#endif

#ifdef UART_OUTPUT_TRACE_DATA
void debugSendTraceInfo(unsigned int taskNbr)
{
    uint32_t traceData;
    traceData = (taskNbr << 29) | (((traceTickCount << 16) + TIM1->CNT) & 0x1FFFFFF);
    uartSendDataDma(sizeof(traceData), (uint8_t *)&traceData);
}

void debugInitTrace(void)
{
//TODO:implement
}
#else
void debugSendTraceInfo(unsigned int taskNbr)
{
}
#endif
