/**
 *
 * ESPlane Firmware
 *
 * Copyright 2019-2020  Espressif Systems (Shanghai)
 * Copyright (C) 2011-2017 Bitcraze AB
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
 * 注册用于飞行控制的协议端口，在commanderInit时，调用crtpCommanderInit
 *
 */
#include <stdbool.h>
#define DEBUG_MODULE "CRTP_CMD"
#include "crtp_commander.h"

#include "commander.h"
#include "crtp.h"
#include "debug_cf.h"

static bool isInit;

static void commanderCrtpCB(CRTPPacket *pk);

void crtpCommanderInit(void) //在commanderInit时，调用crtpCommanderInit
{
    if (isInit) {
        return;
    }

    crtpInit();
    crtpRegisterPortCB(CRTP_PORT_SETPOINT, commanderCrtpCB);
    crtpRegisterPortCB(CRTP_PORT_SETPOINT_GENERIC, commanderCrtpCB);
    isInit = true;
}

/* command step - receive  09 callback function of port CRTP_PORT_SETPOINT and CRTP_PORT_SETPOINT_GENERIC */
static void commanderCrtpCB(CRTPPacket *pk)
{
    static setpoint_t setpoint;

    if (pk->port == CRTP_PORT_SETPOINT && pk->channel == 0) {
        crtpCommanderRpytDecodeSetpoint(&setpoint, pk);
        commanderSetSetpoint(&setpoint, COMMANDER_PRIORITY_CRTP);
        DEBUG_PRINTD("CRTP_PORT_SETPOINT");
    } else if (pk->port == CRTP_PORT_SETPOINT_GENERIC && pk->channel == 0) {
        crtpCommanderGenericDecodeSetpoint(&setpoint, pk);
        commanderSetSetpoint(&setpoint, COMMANDER_PRIORITY_CRTP);
        DEBUG_PRINTD("CRTP_PORT_SETPOINT_GENERIC");
    }
}
