/*******************************************************************************
**
**      GSC-18128-1, "Core Flight Executive Version 6.7"
**
**      Copyright (c) 2006-2019 United States Government as represented by
**      the Administrator of the National Aeronautics and Space Administration.
**      All Rights Reserved.
**
**      Licensed under the Apache License, Version 2.0 (the "License");
**      you may not use this file except in compliance with the License.
**      You may obtain a copy of the License at
**
**        http://www.apache.org/licenses/LICENSE-2.0
**
**      Unless required by applicable law or agreed to in writing, software
**      distributed under the License is distributed on an "AS IS" BASIS,
**      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**      See the License for the specific language governing permissions and
**      limitations under the License.
**
** File: gps_app.c
**
** Purpose:
**   This file contains the source code for the GPS App.
**
*******************************************************************************/

/*
** Include Files:
*/
#include "osconfig.h"

#include "gps_app_events.h"
#include "gps_app_version.h"
#include "gps_app.h"
#include "gps_app_table.h"

#include "mpu9dof_lib.h"
#include "bcm2835_lib.h"

/*
** global data
*/
GPS_APP_Data_t GPS_APP_Data;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/* GPS_APP_Main() -- Application entry point and main process loop         */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void GPS_APP_Main(void)
{
    int32            status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(GPS_APP_PERF_ID);

    /*
    ** Perform application specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    status = GPS_APP_Init();
    if (status != CFE_SUCCESS)
    {
        GPS_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** GPS Runloop
    */
    while (CFE_ES_RunLoop(&GPS_APP_Data.RunStatus) == true)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(GPS_APP_PERF_ID);

        /* Pend on receipt of command packet */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, GPS_APP_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(GPS_APP_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            GPS_APP_ProcessCommandPacket(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(GPS_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "GPS APP: SB Pipe Read Error, App Will Exit");

            GPS_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(GPS_APP_PERF_ID);

    CFE_ES_ExitApp(GPS_APP_Data.RunStatus);

} /* End of GPS_APP_Main() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* GPS_APP_Init() --  initialization                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 GPS_APP_Init(void)
{
    int32 status;

    GPS_APP_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Initialize app command execution counters
    */
    GPS_APP_Data.CmdCounter = 0;
    GPS_APP_Data.ErrCounter = 0;

    /*
    ** Initialize app configuration data
    */
    GPS_APP_Data.PipeDepth = GPS_APP_PIPE_DEPTH;

    strncpy(GPS_APP_Data.PipeName, "GPS_APP_CMD_PIPE", sizeof(GPS_APP_Data.PipeName));
    GPS_APP_Data.PipeName[sizeof(GPS_APP_Data.PipeName) - 1] = 0;

    /*
    ** Initialize event filter table...
    */
    GPS_APP_Data.EventFilters[0].EventID = GPS_APP_STARTUP_INF_EID;
    GPS_APP_Data.EventFilters[0].Mask    = 0x0000;
    GPS_APP_Data.EventFilters[1].EventID = GPS_APP_COMMAND_ERR_EID;
    GPS_APP_Data.EventFilters[1].Mask    = 0x0000;
    GPS_APP_Data.EventFilters[2].EventID = GPS_APP_COMMANDNOP_INF_EID;
    GPS_APP_Data.EventFilters[2].Mask    = 0x0000;
    GPS_APP_Data.EventFilters[3].EventID = GPS_APP_COMMANDRST_INF_EID;
    GPS_APP_Data.EventFilters[3].Mask    = 0x0000;
    GPS_APP_Data.EventFilters[4].EventID = GPS_APP_INVALID_MSGID_ERR_EID;
    GPS_APP_Data.EventFilters[4].Mask    = 0x0000;
    GPS_APP_Data.EventFilters[5].EventID = GPS_APP_LEN_ERR_EID;
    GPS_APP_Data.EventFilters[5].Mask    = 0x0000;
    GPS_APP_Data.EventFilters[6].EventID = GPS_APP_PIPE_ERR_EID;
    GPS_APP_Data.EventFilters[6].Mask    = 0x0000;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(GPS_APP_Data.EventFilters, GPS_APP_EVENT_COUNTS, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("GPS App: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
        return (status);
    }

    /*
    ** Initialize housekeeping packet (clear user data area).
    */
    CFE_MSG_Init(&GPS_APP_Data.HkTlm.TlmHeader.Msg, GPS_APP_HK_TLM_MID, sizeof(GPS_APP_Data.HkTlm));

    /*
    ** Create Software Bus message pipe.
    */
    status = CFE_SB_CreatePipe(&GPS_APP_Data.CommandPipe, GPS_APP_Data.PipeDepth, GPS_APP_Data.PipeName);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("GPS App: Error creating pipe, RC = 0x%08lX\n", (unsigned long)status);
        return (status);
    }

    /*
    ** Subscribe to Housekeeping request commands
    */
    status = CFE_SB_Subscribe(GPS_APP_SEND_HK_MID, GPS_APP_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("GPS App: Error Subscribing to HK request, RC = 0x%08lX\n", (unsigned long)status);
        return (status);
    }

    /*
    ** Subscribe to ground command packets
    */
    status = CFE_SB_Subscribe(GPS_APP_CMD_MID, GPS_APP_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("GPS App: Error Subscribing to Command, RC = 0x%08lX\n", (unsigned long)status);

        return (status);
    }

    /*
    ** Register Table(s)
    */
    status = CFE_TBL_Register(&GPS_APP_Data.TblHandles[0], "GpsAppTable", sizeof(GPS_APP_Table_t),
                              CFE_TBL_OPT_DEFAULT, GPS_APP_TblValidationFunc);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("GPS App: Error Registering Table, RC = 0x%08lX\n", (unsigned long)status);

        return (status);
    }
    else
    {
        status = CFE_TBL_Load(GPS_APP_Data.TblHandles[0], CFE_TBL_SRC_FILE, GPS_APP_TABLE_FILE);
    }

    CFE_EVS_SendEvent(GPS_APP_STARTUP_INF_EID, CFE_EVS_EventType_INFORMATION, "GPS App Initialized.%s",
                      GPS_APP_VERSION_STRING);
                      
    /* Initialize the mpu9dof class to acquire from the GPS */
    GPS_APP_Data.mpu9dof.slave_address =         MPU9DOF_XLG_I2C_ADDR_0;
    GPS_APP_Data.mpu9dof.magnetometer_address =  MPU9DOF_M_I2C_ADDR_0;

    return (CFE_SUCCESS);

} /* End of GPS_APP_Init() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  GPS_APP_ProcessCommandPacket                                       */
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the GPS       */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void GPS_APP_ProcessCommandPacket(CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (MsgId)
    {
        case GPS_APP_CMD_MID:
            GPS_APP_ProcessGroundCommand(SBBufPtr);
            break;

        case GPS_APP_SEND_HK_MID:
            GPS_APP_ReportHousekeeping((CFE_MSG_CommandHeader_t *)SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(GPS_APP_INVALID_MSGID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "GPS: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }

    return;

} /* End GPS_APP_ProcessCommandPacket */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* GPS_APP_ProcessGroundCommand() -- GPS ground commands                      */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void GPS_APP_ProcessGroundCommand(CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    /*
    ** Process "known" GPS app ground commands
    */
    switch (CommandCode)
    {
        case GPS_APP_NOOP_CC:
            if (GPS_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_APP_NoopCmd_t)))
            {
                GPS_APP_Noop((GPS_APP_NoopCmd_t *)SBBufPtr);
            }

            break;

        case GPS_APP_RESET_COUNTERS_CC:
            if (GPS_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_APP_ResetCountersCmd_t)))
            {
                GPS_APP_ResetCounters((GPS_APP_ResetCountersCmd_t *)SBBufPtr);
            }

            break;

        case GPS_APP_PROCESS_CC:
            if (GPS_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(GPS_APP_ProcessCmd_t)))
            {
                GPS_APP_Process((GPS_APP_ProcessCmd_t *)SBBufPtr);
            }

            break;

        /* default case already found during FC vs length test */
        default:
            CFE_EVS_SendEvent(GPS_APP_COMMAND_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Invalid ground command code: CC = %d", CommandCode);
            break;
    }

    return;

} /* End of GPS_APP_ProcessGroundCommand() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  GPS_APP_ReportHousekeeping                                         */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
int32 GPS_APP_ReportHousekeeping(const CFE_MSG_CommandHeader_t *Msg)
{
    int i;
    
    if(OS_MutSemTake(i2c_mutexvar) != OS_SUCCESS){
        OS_printf("GPS APP: I2C Busy. \n");
        return CFE_SUCCESS;
    }
    
    /* Get the acceleration values */
    mpu9dof_read_accel ( &GPS_APP_Data.mpu9dof, &GPS_APP_Data.Accel_x, &GPS_APP_Data.Accel_y, &GPS_APP_Data.Accel_z );
    

    /*
    ** Get command execution counters...
    */
    GPS_APP_Data.HkTlm.Payload.CommandErrorCounter = GPS_APP_Data.ErrCounter;
    GPS_APP_Data.HkTlm.Payload.CommandCounter      = GPS_APP_Data.CmdCounter;
    GPS_APP_Data.HkTlm.Payload.Accel_x             = GPS_APP_Data.Accel_x;
    GPS_APP_Data.HkTlm.Payload.Accel_y             = GPS_APP_Data.Accel_y;
    GPS_APP_Data.HkTlm.Payload.Accel_z             = GPS_APP_Data.Accel_z;

    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(&GPS_APP_Data.HkTlm.TlmHeader.Msg);
    CFE_SB_TransmitMsg(&GPS_APP_Data.HkTlm.TlmHeader.Msg, true);

    /*
    ** Manage any pending table loads, validations, etc.
    */
    for (i = 0; i < GPS_APP_NUMBER_OF_TABLES; i++)
    {
        CFE_TBL_Manage(GPS_APP_Data.TblHandles[i]);
    }
    
    CFE_EVS_SendEvent(GPS_APP_STARTUP_INF_EID, CFE_EVS_EventType_INFORMATION, "GPS App: Report HK Done. Accel X: %d. Accel Y: %d. Accel Z: %d. %s",
                      GPS_APP_Data.Accel_x, GPS_APP_Data.Accel_y, GPS_APP_Data.Accel_z, GPS_APP_VERSION_STRING);
                      
    if(OS_MutSemGive(i2c_mutexvar) != OS_SUCCESS){
        OS_printf("GPS APP: Cannot give mutex. \n");
        return CFE_SUCCESS;
    }

    return CFE_SUCCESS;

} /* End of GPS_APP_ReportHousekeeping() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* GPS_APP_Noop -- GPS NOOP commands                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 GPS_APP_Noop(const GPS_APP_NoopCmd_t *Msg)
{

    GPS_APP_Data.CmdCounter++;

    CFE_EVS_SendEvent(GPS_APP_COMMANDNOP_INF_EID, CFE_EVS_EventType_INFORMATION, "GPS: NOOP command %s",
                      GPS_APP_VERSION);

    return CFE_SUCCESS;

} /* End of GPS_APP_Noop */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  GPS_APP_ResetCounters                                               */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
int32 GPS_APP_ResetCounters(const GPS_APP_ResetCountersCmd_t *Msg)
{

    GPS_APP_Data.CmdCounter = 0;
    GPS_APP_Data.ErrCounter = 0;

    CFE_EVS_SendEvent(GPS_APP_COMMANDRST_INF_EID, CFE_EVS_EventType_INFORMATION, "GPS: RESET command");

    return CFE_SUCCESS;

} /* End of GPS_APP_ResetCounters() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  GPS_APP_Process                                                     */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function Process Ground Station Command                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
int32 GPS_APP_Process(const GPS_APP_ProcessCmd_t *Msg)
{
    int32               status;
    GPS_APP_Table_t *TblPtr;
    const char *        TableName = "GPS_APP.GpsAppTable";

    /* GPS Use of Table */

    status = CFE_TBL_GetAddress((void *)&TblPtr, GPS_APP_Data.TblHandles[0]);

    if (status < CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("GPS App: Fail to get table address: 0x%08lx", (unsigned long)status);
        return status;
    }

    CFE_ES_WriteToSysLog("GPS App: Table Value 1: %d  Value 2: %d", TblPtr->Int1, TblPtr->Int2);

    GPS_APP_GetCrc(TableName);

    status = CFE_TBL_ReleaseAddress(GPS_APP_Data.TblHandles[0]);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("GPS App: Fail to release table address: 0x%08lx", (unsigned long)status);
        return status;
    }

    return CFE_SUCCESS;

} /* End of GPS_APP_ProcessCC */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* GPS_APP_VerifyCmdLength() -- Verify command packet length                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool GPS_APP_VerifyCmdLength(CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    bool              result       = true;
    size_t            ActualLength = 0;
    CFE_SB_MsgId_t    MsgId        = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode      = 0;

    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    /*
    ** Verify the command packet length.
    */
    if (ExpectedLength != ActualLength)
    {
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(GPS_APP_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;

        GPS_APP_Data.ErrCounter++;
    }

    return (result);

} /* End of GPS_APP_VerifyCmdLength() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* GPS_APP_TblValidationFunc -- Verify contents of First Table      */
/* buffer contents                                                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 GPS_APP_TblValidationFunc(void *TblData)
{
    int32               ReturnCode = CFE_SUCCESS;
    GPS_APP_Table_t *TblDataPtr = (GPS_APP_Table_t *)TblData;

    /*
    ** GPS Table Validation
    */
    if (TblDataPtr->Int1 > GPS_APP_TBL_ELEMENT_1_MAX)
    {
        /* First element is out of range, return an appropriate error code */
        ReturnCode = GPS_APP_TABLE_OUT_OF_RANGE_ERR_CODE;
    }

    return ReturnCode;

} /* End of GPS_APP_TBLValidationFunc() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* GPS_APP_GetCrc -- Output CRC                                     */
/*                                                                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void GPS_APP_GetCrc(const char *TableName)
{
    int32          status;
    uint32         Crc;
    CFE_TBL_Info_t TblInfoPtr;

    status = CFE_TBL_GetInfo(&TblInfoPtr, TableName);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("GPS App: Error Getting Table Info");
    }
    else
    {
        Crc = TblInfoPtr.Crc;
        CFE_ES_WriteToSysLog("GPS App: CRC: 0x%08lX\n\n", (unsigned long)Crc);
    }

    return;

} /* End of GPS_APP_GetCrc */
