//  PCCP.h
//
//  ~~~~~~~~~~~~
//
//  PCAN-CCP API
//
//  ~~~~~~~~~~~~
//
//  ------------------------------------------------------------------
//  Author : Keneth Wagner
//	Last change: 07.12.2017 Wagner
//
//  Language: ANSI-C
//  ------------------------------------------------------------------
//
//  Copyright (C) 2017  PEAK-System Technik GmbH, Darmstadt
//  more Info at http://www.peak-system.com 
//
#ifndef __PCCPH__
#define __PCCPH__

////////////////////////////////////////////////////////////
// Inclusion of other needed files
////////////////////////////////////////////////////////////

#ifndef __PCANBASICH__
#include "PCANBasic.h"							         // PCAN-Basic API
#endif

////////////////////////////////////////////////////////////
// Type definitions
////////////////////////////////////////////////////////////

#define TCCPHandle                             DWORD     // Represents a PCAN-CCP Connection Handle
#define TCCPResult                             DWORD     // Represent the PCAN CCP result and error codes 

// Limit values
//
#define CCP_MAX_RCV_QUEUE                      0x7FFF    // Maximum count of asynchronous messages that the queue can retain

// Result and Error values
//
#define CCP_ERROR_ACKNOWLEDGE_OK               0x00      //------- CCP CCP Result Level 1
#define CCP_ERROR_DAQ_OVERLOAD                 0x01
#define CCP_ERROR_CMD_PROCESSOR_BUSY           0x10
#define CCP_ERROR_DAQ_PROCESSOR_BUSY           0x11
#define CCP_ERROR_INTERNAL_TIMEOUT             0x12
#define CCP_ERROR_KEY_REQUEST                  0x18
#define CCP_ERROR_SESSION_STS_REQUEST          0x19
#define CCP_ERROR_COLD_START_REQUEST           0x20      //------- CCP Result Level 2
#define CCP_ERROR_CAL_DATA_INIT_REQUEST        0x21
#define CCP_ERROR_DAQ_LIST_INIT_REQUEST        0x22
#define CCP_ERROR_CODE_UPDATE_REQUEST          0x23
#define CCP_ERROR_UNKNOWN_COMMAND              0x30      //------- CCP Result Level 3 (Errors)
#define CCP_ERROR_COMMAND_SYNTAX               0x31
#define CCP_ERROR_PARAM_OUT_OF_RANGE           0x32
#define CCP_ERROR_ACCESS_DENIED                0x33
#define CCP_ERROR_OVERLOAD                     0x34
#define CCP_ERROR_ACCESS_LOCKED                0x35
#define CCP_ERROR_NOT_AVAILABLE                0x36
#define CCP_ERROR_PCAN                   0x80000000      //------- PCAN Error (0x1...0x7FFFFFFF)

// Resource / Protection Mask
//
#define CCP_RSM_NONE                           0x0       // Any resource / Any protection
#define CCP_RSM_CALIBRATION                    0x1       // Calibration available / protected
#define CCP_RSM_DATA_ADQUISITION               0x2       // Data Adquisition available / protected
#define CCP_RSM_MEMORY_PROGRAMMING             0x40      // Flashing available / protected

// Session Status
//
#define CCP_STS_CALIBRATING                    0x1       // Calibration Status
#define CCP_STS_ACQUIRING                      0x2       // Data acquisition Status
#define CCP_STS_RESUME_REQUEST                 0x4       // Request resuming 
#define CCP_STS_STORE_REQUEST                  0x40      // Request storing
#define CCP_STS_RUNNING                        0x80      // Running Status

// Start/Stop Mode
//
#define CCP_SSM_STOP                           0         // Data Transmission Stop
#define CCP_SSM_START                          1         // Data Transmission Start
#define CCP_SSM_PREPARE_START                  2         // Prepare for start data transmission

////////////////////////////////////////////////////////////
// Structure definitions
////////////////////////////////////////////////////////////

// CCP_ExchangeId structure
//
typedef struct 
{	
	BYTE IdLength;                                         // Length of the Slave Device ID
	BYTE DataType;                                         // Data type qualifier of the Slave Device ID
	BYTE AvailabilityMask;                                 // Resource Availability Mask
	BYTE ProtectionMask;	                               // Resource Protection Mask
}TCCPExchangeData;

// CCP_StartStopDataTransmission structure
//
typedef struct 
{
	BYTE Mode;                                             // 0: Stop, 1: Start
	BYTE ListNumber;                                       // DAQ list number to process
	BYTE LastODTNumber;                                    // ODTs to be transmitted (from 0 to byLastODTNumber)
	BYTE EventChannel;                                     // Generic signal source for timing determination
	WORD TransmissionRatePrescaler;                        // Transmission rate prescaler
}TCCPStartStopData;

// Slave Inforamtion used within the CCP_Connect/CCP_Test functions
typedef struct 
{
	WORD EcuAddress;                                       // Station address (Intel format)
	DWORD IdCRO;                                           // CAN Id used for CRO packages (29 Bits = MSB set)
	DWORD IdDTO;                                           // CAN Id used for DTO packages (29 Bits = MSB set)
	bool IntelFormat;                                      // Format used by the slave (True: Intel, False: Motorola)
}TCCPSlaveData;

// Represents an asynchronous message sent from a slave
//
typedef struct 
{
	TCCPHandle Source;                                      // Handle of the connection that has received the message
	BYTE Length;                                            // Data length of the message
	BYTE Data[8];                                           // Data bytes (max. 8)
}TCCPMsg;
	
#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////
// PCAN-CCP API function declarations
////////////////////////////////////////////////////////////

//------------------------------
// Extras
//------------------------------

/// <summary>
/// Initializes the CAN communication using PCAN-Basic API
/// </summary>
/// <param name="Channel">The handle of a PCAN Channel</param>
/// <param name="Btr0Btr1">The speed for the communication (BTR0BTR1 code)</param>
/// <param name="HwType">NON PLUG&PLAY: The type of hardware and operation mode</param>
/// <param name="IOPort">NON PLUG&PLAY: The I/O address for the parallel port</param>
/// <param name="Interupt">NON PLUG&PLAY: Interrupt number of the parallel port</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_InitializeChannel(
        TPCANHandle Channel, 
        TPCANBaudrate Btr0Btr1, 
        TPCANType HwType = 0,
        DWORD IOPort = 0, 
        WORD Interrupt = 0);


/// <summary>
/// Uninitializes a PCAN Channel initialized by CCP_InitializeChannel
/// </summary>
/// <param name="Channel">The handle of a PCAN Channel</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_UninitializeChannel(
	TPCANHandle Channel);


/// <summary>
/// Reads a message from the receive queue of a PCAN-CCP connection.
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="Msg">Buffer for a message. See 'TCCPMsg' above</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_ReadMsg(
        TCCPHandle CcpHandle, 
		TCCPMsg *Msg);


/// <summary>
/// Resets the receive-queue of a PCAN-CCP connection.
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_Reset(
        TCCPHandle CcpHandle);

//------------------------------
// Connection
//------------------------------

/// <summary>
/// Establishes a logical connection between a master applicaiton and a slave unit 
/// </summary>
/// <param name="Channel">The handle of an initialized PCAN Channel</param>
/// <param name="SlaveData">The description data of the slave to be connected</param>
/// <param name="CcpHandle">A buffer to return the handle of this Master/Channel/Slave connection</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_Connect(
        TPCANHandle Channel, 
		TCCPSlaveData* SlaveData, 
		TCCPHandle *CcpHandle,
		WORD TimeOut);


/// <summary>
/// Logically disconnects a master application from a slave unit 
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="Temporary">Indicates if the disconnection should be temporary or not</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_Disconnect(
        TCCPHandle CcpHandle, 
		bool Temporary,
		WORD TimeOut);


/// <summary>
/// Tests if a slave is available 
/// </summary>
/// <param name="Channel">The handle of an initialized PCAN Channel</param>
/// <param name="SlaveData">The data of the slave to be checked for availability</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_Test(
        TPCANHandle Channel, 
		TCCPSlaveData* SlaveData,
		WORD TimeOut);


//------------------------------
// Control + Configuration
//------------------------------

/// <summary>
/// Exchanges the CCP Version used by a master and a slave
/// </summary>
/// <remarks>Both buffers, Main and Release, are bidirectional, e.g. they are used by both 
/// Master and Slave. The master should call this function placing in these ufferrs its used 
/// version. After the function returns, these buffers contain the version used by the 
/// connected slave</remarks>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="Main">Buffer for the CCP Main version used.</param>
/// <param name="Release">Buffer for the CCP Release version used.</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_GetCcpVersion(
        TCCPHandle CcpHandle, 
		BYTE *Main, 
		BYTE *Relase,
		WORD TimeOut);


/// <summary>
/// Exchanges IDs between Master and Slave for automatic session configuration
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="ECUData">Slave ID and Resource Information buffer</param>
/// <param name="MasterData">Optional master data (ID)</param>
/// <param name="DataLength">Length of the master data</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_ExchangeId(
        TCCPHandle CcpHandle, 
		TCCPExchangeData *ECUData, 
		BYTE *MasterData, 
		int DataLength,
		WORD TimeOut);


/// <summary>
/// Returns Seed data for a seed&key algorithm to unlock a slave functionality
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="Resource">The resource being asked. See 'Resource / Protection Mask' values above</param>
/// <param name="CurrentStatus">Current protection status of the asked resource</param>
/// <param name="Seed">Seed value for the seed&key algorithm</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_GetSeed(
        TCCPHandle CcpHandle, 
		BYTE Resource, 
		bool *CurrentStatus, 
		BYTE *Seed,
		WORD TimeOut);


/// <summary>
/// Unlocks the security protection of a resource within a connected slave.
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="KeyBuffer">A buffer with the key computed from a seed value 
/// obtained through the CCP_GetSeed function</param>
/// <param name="KeyLength">The length in bytes of the key buffer value</param>
/// <param name="Privileges">The current privileges status on the slave. 
/// See 'Resource / Protection Mask' values above"</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_Unlock(
        TCCPHandle CcpHandle, 
		BYTE *KeyBuffer, 
		BYTE KeyLength, 
		BYTE *Privileges,
		WORD TimeOut);


/// <summary>
/// Keeps the connected slave informed about the current state of the calibration session
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="Status">Current status bits. See 'Session Status' values above</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_SetSessionStatus(
        TCCPHandle CcpHandle, 
		BYTE Status,
		WORD TimeOut);


/// <summary>
/// Retrieves the information about the current state of the calibration session
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="Status">Current status bits. See 'Session Status' values above</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_GetSessionStatus(
        TCCPHandle CcpHandle, 
		BYTE *Status,
		WORD TimeOut);


//------------------------------
// Memory management
//------------------------------

/// <summary>
/// Initializes a base pointer for all following memory transfers.
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="UsedMTA">Memory Transfer Address (MTA) number (0 or 1)</param>
/// <param name="AddrExtension">address extension</param>
/// <param name="AddrExtension">Address</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_SetMemoryTransferAddress(
        TCCPHandle CcpHandle, 
		BYTE UsedMTA, 
		BYTE AddrExtension, 
		DWORD Addr,
		WORD TimeOut); 


/// <summary>
/// Copies a block of data into memory, starting at the current MTA0. 
/// </summary>
/// <remarks>MTA0 is post-incremented by the value of 'Size'</remarks>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="DataBytes">Buffer with the data to be transferred</param>
/// <param name="Size">Size of the data to be transferred, in bytes</param>
/// <param name="MTA0Ext">MTA0 extension after post-increment</param>
/// <param name="MTA0Addr">MTA0 Address after post-increment</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_Download(
        TCCPHandle CcpHandle, 
		BYTE *DataBytes, 
		BYTE Size, 
		BYTE *MTA0Ext, 
		DWORD *MTA0Addr,
		WORD TimeOut);


/// <summary>
/// Copies a block of 6 data bytes into memory, starting at the current MTA0. 
/// </summary>
/// <remarks>MTA0 is post-incremented by the value of 6</remarks>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="DataBytes">Buffer with the data to be transferred</param>
/// <param name="MTA0Ext">MTA0 extension after post-increment</param>
/// <param name="MTA0Addr">MTA0 Address after post-increment</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_Download_6(
        TCCPHandle CcpHandle, 
		BYTE *DataBytes, 
		BYTE *MTA0Ext, 
		DWORD *MTA0Addr,
		WORD TimeOut);


/// <summary>
/// Retrieves a block of data starting at the current MTA0.  
/// </summary>
/// <remarks>MTA0 will be post-incremented with the value of size</remarks>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="Size">Size of the data to be retrieved, in bytes</param>
/// <param name="DataBytes">Buffer for the requested data</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_Upload(
        TCCPHandle CcpHandle, 
		BYTE Size, 
		BYTE *DataBytes,
		WORD TimeOut);


/// <summary>
/// Retrieves a block of data.  
/// </summary>
/// <remarks>The amount of data is retrieved from the given address. The MTA0 
/// pointer \"remains unchanged\"</remarks>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="UploadSize">Size of the data to be retrieved, in bytes</param>
/// <param name="MTA0Ext">MTA0 extension for the upload</param>
/// <param name="MTA0Addr">MTA0 Address for the upload</param>
/// <param name="reqData">Buffer for the requested data</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_ShortUpload(
        TCCPHandle CcpHandle, 
		BYTE UploadSize, 
		BYTE MTA0Ext, 
		DWORD MTA0Addr, 
		BYTE *reqData,
		WORD TimeOut);


/// <summary>
/// Copies a block of data from the address MTA0 to the address MTA1 
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="SizeOfData">Number of bytes to be moved</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_Move(
        TCCPHandle CcpHandle, 
		DWORD SizeOfData,
		WORD TimeOut);


//------------------------------
// Calibration
//------------------------------

/// <summary>
/// ECU Implementation dependant. Sets the previously initialized MTA0 as the start of the 
/// current active calibration data page
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_SelectCalibrationDataPage(
        TCCPHandle CcpHandle,
		WORD TimeOut);


/// <summary>
/// Retrieves the start address of the calibration page that is currently active in the slave device
/// calibration data page that is selected as the currently active page
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="MTA0Ext">Buffer for the MTAO address extension</param>
/// <param name="MTA0Addr">Buffer for the MTA0 address pointer</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_GetActiveCalibrationPage(
        TCCPHandle CcpHandle, 
		BYTE *MTA0Ext,
		DWORD *MTA0Addr,
		WORD TimeOut);


//------------------------------
// Data Adquisition
//------------------------------

/// <summary>
/// Retrieves the size of the specified DAQ List as the number of available Object
/// Descriptor Tables (ODTs) and clears the current list.
/// Optionally, sets a dedicated CAN-ID for the DAQ list.
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="ListNumber">DAQ List number</param>
/// <param name="DTOId">CAN identifier of DTO dedicated to the given 'ListNumber'</param>
/// <param name="Size">Buffer for the list size (Number of ODTs in the DAQ list)</param>
/// <param name="FirstPDI">Buffer for the first PID of the DAQ list</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_GetDAQListSize(
        TCCPHandle CcpHandle, 
		BYTE ListNumber, 
		DWORD *DTOId, 
		BYTE *Size, 
		BYTE *FirstPDI,
		WORD TimeOut);


/// <summary>
/// Initializes the DAQ List pointer for a subsequent write to a DAQ list.
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="ListNumber">DAQ List number</param>
/// <param name="ODTNumber">Object Descriptor Table number</param>
/// <param name="ElementNumber">Element number within the ODT</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_SetDAQListPointer(
        TCCPHandle CcpHandle, 
		BYTE ListNumber, 
		BYTE ODTNumber, 
		BYTE ElementNumber,
		WORD TimeOut);


/// <summary>
/// Writes one entry (DAQ element) to a DAQ list defined by the DAQ list pointer.
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="SizeElement">Size of the DAQ elements in bytes {1, 2, 4}</param>
/// <param name="AddrExtension">Address extension of DAQ element</param>
/// <param name="AddrDAQ">Address of a DAQ element</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_WriteDAQListEntry(
        TCCPHandle CcpHandle, 
		BYTE SizeElement, 
		BYTE AddrExtension, 
		DWORD AddrDAQ,
		WORD TimeOut);


/// <summary>
/// Starts/Stops the data acquisition and/or prepares a synchronized start of the 
/// specified DAQ list
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="Data">Contains the data to be applied within the start/stop procedure. 
/// See 'TCCPStartStopData' structure above</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_StartStopDataTransmission(
        TCCPHandle CcpHandle, 
		TCCPStartStopData *Data,
		WORD TimeOut);


/// <summary>
/// Starts/Stops the periodic transmission of all DAQ lists. 
/// <remarks>Starts all DAQs configured as "prepare to start" with a previously 
/// CCP_StartStopDataTransmission call. Stops all DAQs, included those not started 
/// synchronized</remarks>
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="StartOrStop">True: Starts the configured DAQ lists. False: Stops all DAQ lists</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_StartStopSynchronizedDataTransmission(
        TCCPHandle CcpHandle, 
		bool StartOrStop,
		WORD TimeOut);


//------------------------------
// Flash Programming
//------------------------------

/// <summary>
/// Erases non-volatile memory (FLASH EPROM) prior to reprogramming.
/// </summary>
/// <remarks>The MTA0 pointer points to the memory location to be erased</remarks>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="MemorySize">Memory size in bytes to be erased</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_ClearMemory(
        TCCPHandle CcpHandle, 
		DWORD MemorySize,
		WORD TimeOut);


/// <summary>
/// Programms a block of data into non-volatile (FLASH, EPROM) memory, starting 
/// at the current MTA0. 
/// </summary>
/// <remarks>The MTA0 pointer is post-incremented by the value of 'Size'</remarks>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="Data">Buffer with the data to be programmed</param>
/// <param name="Size">Size of the 'Data' block to be programmed</param>
/// <param name="MTA0Ext">MTA0 extension after post-increment</param>
/// <param name="MTA0Addr">MTA0 Address after post-increment</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_Program(
        TCCPHandle CcpHandle, 
		BYTE *Data, 
		BYTE Size, 
		BYTE *MTA0Ext, 
		DWORD *MTA0Addr,
		WORD TimeOut);

 
/// <summary>
/// Programms a block of 6 data bytes into non-volatile (FLASH, EPROM) memory, 
/// starting at the current MTA0. 
/// </summary>
/// <remarks>The MTA0 pointer is post-incremented by 6</remarks>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="Data">Buffer with the data to be programmed</param>
/// <param name="MTA0Ext">MTA0 extension after post-increment</param>
/// <param name="MTA0Addr">MTA0 Address after post-increment</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_Program_6(
        TCCPHandle CcpHandle, 
		BYTE *Data, 
		BYTE *MTA0Ext, 
		DWORD *MTA0Addr,
		WORD TimeOut);


/// <summary>
/// Calculates a checksum result of the memory block that is defined by MTA0 
/// (Memory Transfer Area Start address) and 'BlockSize'.
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="BlockSize">Block size in bytes</param>
/// <param name="ChecksumData">Checksum data (implementation specific)</param>
/// <param name="ChecksumSize">Size of the checksum data</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_BuildChecksum(
        TCCPHandle CcpHandle, 
		DWORD BlockSize, 
		BYTE *ChecksumData, 
		BYTE *ChecksumSize,
		WORD TimeOut);


//------------------------------
// Diagnostic
//------------------------------

/// <summary>
/// Executes a defined diagnostic procedure and sets the MTA0 pointer to the location 
/// from where the master can upload the requested information
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="DiagnosticNumber">Diagnostic service number</param>
/// <param name="Parameters">Parameters, if applicable</param>
/// <param name="ParametersLength">Length in bytes of the parameters passed within 'Parameters'</param>
/// <param name="ReturnLength">Length of the return information (to be uploaded)</param>
/// <param name="ReturnType">Data type qualifier of the return information</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_DiagnosticService(
        TCCPHandle CcpHandle, 
		WORD DiagnosticNumber, 
		BYTE *Parameters, 
		BYTE ParametersLength, 
		BYTE *ReturnLength, 
		BYTE *ReturnType,
		WORD TimeOut);


/// <summary>
/// Executes a defined action and sets the MTA0 pointer to the location 
/// from where the master can upload the requested information
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="ActionNumber">Action service number</param>
/// <param name="Parameters">Parameters, if applicable</param>
/// <param name="ParametersLength">Length in bytes of the parameters passed within 'Parameters'</param>
/// <param name="ReturnLength">Length of the return information (to be uploaded)</param>
/// <param name="ReturnType">Data type qualifier of the return information</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_ActionService(
        TCCPHandle CcpHandle, 
		WORD ActionNumber, 
		BYTE *Parameters, 
		BYTE ParametersLength, 
		BYTE *ReturnLength, 
		BYTE *ReturnType,
		WORD TimeOut);

/// <summary>
///Converts an error code to its equivalent english text
/// </summary>
/// <param name="errorCode">A TCCPResult result code</param>
/// <param name="textBuffer">Buffer for a null terminated char array (must be at least 256 in length)</param>
/// <returns>A TCCPResult result code</returns>
TCCPResult __stdcall CCP_GetErrorText(
		TCCPResult errorCode, 
		LPSTR textBuffer);

#ifdef __cplusplus
}
#endif
#endif