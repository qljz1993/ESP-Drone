/**
 *
 * ESPlane Firmware
 * 
 * Copyright 2019-2020  Espressif Systems (Shanghai) 
 * Copyright (C) 2012 BitCraze AB
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
 * log.h - Dynamic log system
 */

#ifndef __QUEUE_MONITOR_H__
#define __QUEUE_MONITOR_H__


#include "freertos/FreeRTOS.h"

//#define DEBUG_QUEUE_MONITOR

#ifdef DEBUG_QUEUE_MONITOR
  #include "freertos/queue.h"

  void queueMonitorInit();
  #define DEBUG_QUEUE_MONITOR_REGISTER(queue) qmRegisterQueue(queue, __FILE__, #queue)

  void qm_traceQUEUE_SEND(void* xQueue);
  void qm_traceQUEUE_SEND_FAILED(void* xQueue);
  void qmRegisterQueue(xQueueHandle* xQueue, char* fileName, char* queueName);
#else
  #define DEBUG_QUEUE_MONITOR_REGISTER(queue)
#endif // DEBUG_QUEUE_MONITOR

#endif // __QUEUE_MONITOR_H__