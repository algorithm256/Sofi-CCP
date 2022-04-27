//  PCCP.pas
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
//  Language: Pascal OO
//  ------------------------------------------------------------------
//
//  Copyright (C) 2017  PEAK-System Technik GmbH, Darmstadt
//  more Info at http://www.peak-system.com 
//
unit PCCP;

interface

////////////////////////////////////////////////////////////
// Inclusion of other needed units
////////////////////////////////////////////////////////////
uses
    PCANBasic;                                                // PCAN-Basic API

////////////////////////////////////////////////////////////
// Type definitions
////////////////////////////////////////////////////////////
type
    TCCPHandle = Longword;                                   // Represents a PCAN-CCP Connection Handle
    TCCPResult = Longword;                                   // Represent the PCAN CCP result and error codes 

const
	// Limit values 
	//
	CCP_MAX_RCV_QUEUE                       = $7FFF;         // Maximum count of asynchronous messages that the queue can retain
	
    // Result and Error values
    //
    CCP_ERROR_ACKNOWLEDGE_OK                = $00;           //------- CCP CCP Result Level 1
    CCP_ERROR_DAQ_OVERLOAD                  = $01;
    CCP_ERROR_CMD_PROCESSOR_BUSY            = $10;
    CCP_ERROR_DAQ_PROCESSOR_BUSY            = $11;
    CCP_ERROR_INTERNAL_TIMEOUT              = $12;
    CCP_ERROR_KEY_REQUEST                   = $18;
    CCP_ERROR_SESSION_STS_REQUEST           = $19;
    CCP_ERROR_COLD_START_REQUEST            = $20;           //------- CCP Result Level 2
    CCP_ERROR_CAL_DATA_INIT_REQUEST         = $21;
    CCP_ERROR_DAQ_LIST_INIT_REQUEST         = $22;
    CCP_ERROR_CODE_UPDATE_REQUEST           = $23;
    CCP_ERROR_UNKNOWN_COMMAND               = $30;           //------- CCP Result Level 3 (Errors)
    CCP_ERROR_COMMAND_SYNTAX                = $31;
    CCP_ERROR_PARAM_OUT_OF_RANGE            = $32;
    CCP_ERROR_ACCESS_DENIED                 = $33;
    CCP_ERROR_OVERLOAD                      = $34;
    CCP_ERROR_ACCESS_LOCKED                 = $35;
    CCP_ERROR_NOT_AVAILABLE                 = $36;
    CCP_ERROR_PCAN                          = $80000000;     //------- PCAN Error (0x1...0x7FFFFFFF)

    // Resource / Protection Mask
    //
    CCP_RSM_NONE                            = 0;             // Any resource / Any protection
    CCP_RSM_CALIBRATION                     = $01;           // Calibration available / protected
    CCP_RSM_DATA_ADQUISITION                = $02;           // Data Adquisition available / protected
    CCP_RSM_MEMORY_PROGRAMMING              = $40;           // Flashing available / protected

    // Session Status
    //
    CCP_STS_CALIBRATING                     = $01;           // Calibration Status
    CCP_STS_ACQUIRING                       = $02;           // Data acquisition Status
    CCP_STS_RESUME_REQUEST                  = $04;           // Request resuming
    CCP_STS_STORE_REQUEST                   = $40;           // Request storing
    CCP_STS_RUNNING                         = $80;           // Running Status

    // Start/Stop Mode
    //
    CCP_SSM_STOP                            = 0;             // Data Transmission Stop
    CCP_SSM_START                           = 1;             // Data Transmission Start
    CCP_SSM_PREPARE_START                   = 2;             // Prepare for start data transmission

////////////////////////////////////////////////////////////
// Structure definitions
////////////////////////////////////////////////////////////
///
type
    // CCP_ExchangeId structure
    //
    TCCPExchangeData = record
        IdLength : BYte;                                     // Length of the Slave Device ID
        DataType : Byte;                                     // Data type qualifier of the Slave Device ID
        AvailabilityMask : Byte;                             // Resource Availability Mask
        ProtectionMask : Byte;                               // Resource Protection Mask
    end;                                                     

    // CCP_StartStopDataTransmission structure
    //
    TCCPStartStopData = record
    	Mode : Byte;                                         // 0: Stop, 1: Start
	    ListNumber : Byte;                                   // DAQ list number to process
    	LastODTNumber : Byte;                                // ODTs to be transmitted (from 0 to byLastODTNumber)
	    EventChannel : Byte;                                 // Generic signal source for timing determination
    	TransmissionRatePrescaler : Word;                    // Transmission rate prescaler
    end;

    // Slave Inforamtion used within the CCP_Connect/CCP_Test functions
    //
    TCCPSlaveData = record
    	EcuAddress : Word;                                   // Station address (Intel format)
    	IdCRO : Longword;                                    // CAN Id used for CRO packages (29 Bits = MSB set)
    	IdDTO : Longword;                                    // CAN Id used for DTO packages (29 Bits = MSB set)
	    IntelFormat : Boolean;                               // Format used by the slave (True: Intel, False: Motorola)
    end;

    // Represents an asynchronous message sent from a slave
    //
    TCCPMsg = record
        Source: TCCPHandle;                                  // Handle of the connection that has received the message
        Length: Byte;                                        // Data length of the message
        Data: array[0..7] of Byte;                           // Data bytes (max. 8)
    end;

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
function CCP_InitializeChannel(
        Channel: TPCANHandle;
        Btr0Btr1: TPCANBaudrate;
        HwType: TPCANType;
        IOPort: LongWord;
        Interrupt: Word
        ): TCCPResult; stdcall;


/// <summary>
/// Uninitializes a PCAN Channel initialized by CCP_InitializeChannel
/// </summary>
/// <param name="Channel">The handle of a PCAN Channel</param>
/// <returns>A TCCPResult result code</returns>
function CCP_UninitializeChannel(
        Channel : TPCANHandle
        ): TCCPResult; stdcall;


/// <summary>
/// Reads a message from the receive queue of a PCAN-CCP connection.
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="Msg">Buffer for a message. See 'TCCPMsg' above</param>
/// <returns>A TCCPResult result code</returns>
function CCP_ReadMsg(
        CcpHandle : TCCPHandle;
        var Msg : TCCPMsg
        ): TCCPResult; stdcall;


/// <summary>
/// Resets the receive-queue of a PCAN-CCP connection.
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <returns>A TCCPResult result code</returns>
function CCP_Reset(
        CcpHandle : TCCPHandle
        ): TCCPResult; stdcall;

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
function CCP_Connect(
        Channel : TPCANHandle;
        var SlaveData : TCCPSlaveData;
        var CcpHandle : TCCPHandle;
        TimeOut : Word
        ):TCCPResult; stdcall;


/// <summary>
/// Logically disconnects a master application from a slave unit 
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="Temporary">Indicates if the disconnection should be temporary or not</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
function CCP_Disconnect(
        CcpHandle : TCCPHandle;
        Temporary : Boolean;
        TimeOut : Word
        ):TCCPResult; stdcall;


/// <summary>
/// Tests if a slave is available
/// </summary>
/// <param name="Channel">The handle of an initialized PCAN Channel</param>
/// <param name="SlaveData">The data of the slave to be checked for availability</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
function CCP_Test(
        Channel : TPCANHandle;
        var SlaveData : TCCPSlaveData;
        TimeOut : Word
        ):TCCPResult; stdcall;

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
function CCP_GetCcpVersion(
        CcpHandle : TCCPHandle;
        var Main : Byte;
        var Release : Byte;
        TimeOut : Word
        ):TCCPResult; stdcall;


/// <summary>
/// Exchanges IDs between Master and Slave for automatic session configuration
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="ECUData">Slave ID and Resource Information buffer</param>
/// <param name="MasterData">Optional master data (ID)</param>
/// <param name="DataLength">Length of the master data</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
function CCP_ExchangeId(
        CcpHandle : TCCPHandle;
        var ECUData : TCCPExchangeData;
        MasterData : Pointer;
        DataLength : Integer;
        TimeOut : Word
        ):TCCPResult; stdcall;


/// <summary>
/// Returns Seed data for a seed&key algorithm to unlock a slave functionality
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="Resource">The resource being asked. See 'Resource / Protection Mask' values above</param>
/// <param name="CurrentStatus">Current protection status of the asked resource</param>
/// <param name="Seed">Seed value for the seed&key algorithm</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
function CCP_GetSeed(
        CcpHandle : TCCPHandle;
        Resource : Byte;
        var CurrentStatus : Boolean;
        Seed : Pointer;
        TimeOut : Word
        ):TCCPResult; stdcall;


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
function CCP_Unlock(
        CcpHandle : TCCPHandle;
        KeyBuffer : Pointer;
        KeyLength : Byte;
        var Privileges : Byte;
        TimeOut : Word
        ):TCCPResult; stdcall;


/// <summary>
/// Keeps the connected slave informed about the current state of the calibration session
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="Status">Current status bits. See 'Session Status' values above</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>        
function CCP_SetSessionStatus(
        CcpHandle : TCCPHandle;
        Status : Byte;
        TimeOut : Word
        ):TCCPResult; stdcall;


/// <summary>
/// Retrieves the information about the current state of the calibration session
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="Status">Current status bits. See 'Session Status' values above</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>        
function CCP_GetSessionStatus(
        CcpHandle : TCCPHandle;
        var Status : Byte;
        TimeOut : Word
        ):TCCPResult; stdcall;

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
function CCP_SetMemoryTransferAddress(
        CcpHandle : TCCPHandle;
        UsedMTA : Byte;
        AddrExtension : Byte;
        Addr : Longword;
        TimeOut : Word
        ):TCCPResult; stdcall;


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
function CCP_Download(
        CcpHandle : TCCPHandle;
        DataBytes : Pointer;
        Size : Byte;
        var MTA0Ext : Byte;
        var MTA0Addr : Longword;
        TimeOut : Word
        ):TCCPResult; stdcall;


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
function CCP_Download_6(
        CcpHandle : TCCPHandle;
        DataBytes : Pointer;
        var MTA0Ext : Byte;
        var MTA0Addr : Longword;
        TimeOut : Word
        ):TCCPResult; stdcall;        


/// <summary>
/// Retrieves a block of data starting at the current MTA0.  
/// </summary>
/// <remarks>MTA0 will be post-incremented with the value of size</remarks>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="Size">Size of the data to be retrieved, in bytes</param>
/// <param name="DataBytes">Buffer for the requested data</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
function CCP_Upload(
        CcpHandle : TCCPHandle;
        Size : Byte;
        DataBytes : Pointer;
        TimeOut : Word
        ):TCCPResult; stdcall;


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
function CCP_ShortUpload(
        CcpHandle : TCCPHandle;
        UploadSize : Byte;
        MTA0Ext : Byte;
        MTA0Addr : Longword;
        ReqData : Pointer;
        TimeOut : Word
        ):TCCPResult; stdcall;


/// <summary>
/// Copies a block of data from the address MTA0 to the address MTA1 
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="SizeOfData">Number of bytes to be moved</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>        
function CCP_Move(
        CcpHandle : TCCPHandle;
        SizeOfData : Longword;
        TimeOut : Word
        ):TCCPResult; stdcall;

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
function CCP_SelectCalibrationDataPage(
        CcpHandle : TCCPHandle;
        TimeOut : Word
        ):TCCPResult; stdcall;


/// <summary>
/// Retrieves the start address of the calibration page that is currently active in the slave device
/// calibration data page that is selected as the currently active page
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="MTA0Ext">Buffer for the MTAO address extension</param>
/// <param name="MTA0Addr">Buffer for the MTA0 address pointer</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>        
function CCP_GetActiveCalibrationPage(
        CcpHandle : TCCPHandle;
        var MTA0Ext : Byte;
        var MTA0Addr : Longword;
        TimeOut : Word
        ):TCCPResult; stdcall;

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
function CCP_GetDAQListSize(
        CcpHandle : TCCPHandle;
        ListNumber : Byte;
        var DTOId : Longword;
        var Size : Byte;
        var FirstPDI : Byte;
        TimeOut : Word
        ):TCCPResult; stdcall;


/// <summary>
/// Initializes the DAQ List pointer for a subsequent write to a DAQ list.
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="ListNumber">DAQ List number</param>
/// <param name="ODTNumber">Object Descriptor Table number</param>
/// <param name="ElementNumber">Element number within the ODT</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
function CCP_SetDAQListPointer(
        CcpHandle : TCCPHandle;
        ListNumber : Byte;
        ODTNumber : Byte;
        ElementNumber : Byte;
        TimeOut : Word
        ):TCCPResult; stdcall;


/// <summary>
/// Writes one entry (DAQ element) to a DAQ list defined by the DAQ list pointer.
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="SizeElement">Size of the DAQ elements in bytes {1, 2, 4}</param>
/// <param name="AddrExtension">Address extension of DAQ element</param>
/// <param name="AddrDAQ">Address of a DAQ element</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>        
function CCP_WriteDAQListEntry(
        CcpHandle : TCCPHandle;
        SizeElement : Byte;
        AddrExtension : Byte;
        AddrDAQ : Longword;
        TimeOut : Word
        ):TCCPResult; stdcall;        


/// <summary>
/// Starts/Stops the data acquisition and/or prepares a synchronized start of the
/// specified DAQ list
/// </summary>
/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
/// <param name="Data">Contains the data to be applied within the start/stop procedure.
/// See 'TCCPStartStopData' structure above</param>
/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
/// <returns>A TCCPResult result code</returns>
function CCP_StartStopDataTransmission(
        CcpHandle : TCCPHandle;
        var Data : TCCPStartStopData;
        TimeOut : Word
        ):TCCPresult; stdcall;


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
function CCP_StartStopSynchronizedDataTransmission(
        CcpHandle : TCCPHandle;
        StartOrStop : Boolean;
        TimeOut : Word
        ):TCCPResult; stdcall;

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
function CCP_ClearMemory(
        CcpHandle : TCCPHandle;
        MemorySize : Longword;
        TimeOut : Word
        ):TCCPResult; stdcall;


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
function CCP_Program(
        CcpHandle : TCCPHandle;
        Data : Pointer;
        Size : Byte;
        var MTA0Ext : Byte;
        var MTA0Addr : Longword;
        TimeOut : Word
        ):TCCPResult; stdcall;


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
function CCP_Program_6(
        CcpHandle : TCCPHandle;
        Data : Pointer;
        var MTA0Ext : Byte;
        var MTA0Addr : Longword;
        TimeOut : Word
        ):TCCPResult; stdcall;


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
function CCP_BuildChecksum(
        CcpHandle : TCCPHandle;
        BlockSize : Longword;
        ChecksumData : Pointer;
        var ChecksumSize : Byte;
        TimeOut : Word
        ):TCCPResult; stdcall;

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
function CCP_DiagnosticService(
        CcpHandle : TCCPHandle;
        DiagnosticNumber : Word;
        Parameters : Pointer;
        ParametersLength : Byte;
        var ReturnLength : Byte;
        var ReturnType : Byte;
        TimeOut : Word
        ):TCCPResult; stdcall;


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
function CCP_ActionService(
        CcpHandle : TCCPHandle;
        ActionNumber : Word;
        Parameters : Pointer;
        ParametersLength : Byte;
        var ReturnLength : Byte;
        var ReturnType : Byte;
        TimeOut : Word
        ):TCCPResult; stdcall;

/// <summary>
/// Returns a descriptive text of a given TCCPResult error code
/// </summary>
/// <param name="Error">A TCCPResult error code</param>
/// <param name="StringBuffer">Buffer for the text (must be at least 256 in length)</param>
/// <returns>A TCCPResult error code</returns>
function CCP_GetErrorText(
    Error: TCCPResult;
    StringBuffer: PAnsiChar
    ): TCCPResult; stdcall;


implementation

uses SysUtils;

const DLL_Name = 'PCCP.DLL';

function CCP_InitializeChannel(Channel: TPCANHandle; Btr0Btr1: TPCANBaudrate; HwType: TPCANType; IOPort: LongWord; Interrupt: Word): TCCPResult; stdcall;
external DLL_Name;

function CCP_UninitializeChannel(Channel : TPCANHandle): TCCPResult; stdcall;
external DLL_Name;

function CCP_ReadMsg(CcpHandle : TCCPHandle; var Msg : TCCPMsg): TCCPResult; stdcall;
external DLL_Name;

function CCP_Reset(CcpHandle : TCCPHandle): TCCPResult; stdcall;
external DLL_Name;

function CCP_Connect(Channel : TPCANHandle; var SlaveData : TCCPSlaveData; var CcpHandle : TCCPHandle; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_Disconnect(CcpHandle : TCCPHandle; Temporary : Boolean; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_Test(Channel : TPCANHandle; var SlaveData : TCCPSlaveData; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_GetCcpVersion(CcpHandle : TCCPHandle; var Main : Byte; var Release : Byte; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_ExchangeId(CcpHandle : TCCPHandle; var ECUData : TCCPExchangeData; MasterData : Pointer; DataLength : Integer; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_GetSeed(CcpHandle : TCCPHandle; Resource : Byte; var CurrentStatus : Boolean; Seed : Pointer; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_Unlock(CcpHandle : TCCPHandle; KeyBuffer : Pointer; KeyLength : Byte; var Privileges : Byte; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_SetSessionStatus(CcpHandle : TCCPHandle; Status : Byte; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_GetSessionStatus(CcpHandle : TCCPHandle; var Status : Byte; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_SetMemoryTransferAddress(CcpHandle : TCCPHandle; UsedMTA : Byte; AddrExtension : Byte; Addr : Longword; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_Download(CcpHandle : TCCPHandle; DataBytes : Pointer; Size : Byte; var MTA0Ext : Byte; var MTA0Addr : Longword; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_Download_6(CcpHandle : TCCPHandle; DataBytes : Pointer; var MTA0Ext : Byte; var MTA0Addr : Longword; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_Upload(CcpHandle : TCCPHandle; Size : Byte; DataBytes : Pointer; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_ShortUpload(CcpHandle : TCCPHandle; UploadSize : Byte; MTA0Ext : Byte; MTA0Addr : Longword; ReqData : Pointer; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_Move(CcpHandle : TCCPHandle; SizeOfData : Longword; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_SelectCalibrationDataPage(CcpHandle : TCCPHandle; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_GetActiveCalibrationPage(CcpHandle : TCCPHandle; var MTA0Ext : Byte; var MTA0Addr : Longword; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_GetDAQListSize(CcpHandle : TCCPHandle; ListNumber : Byte; var DTOId : Longword; var Size : Byte; var FirstPDI : Byte; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_SetDAQListPointer(CcpHandle : TCCPHandle; ListNumber : Byte; ODTNumber : Byte; ElementNumber : Byte; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_WriteDAQListEntry(CcpHandle : TCCPHandle; SizeElement : Byte; AddrExtension : Byte; AddrDAQ : Longword; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_StartStopDataTransmission(CcpHandle : TCCPHandle; var Data : TCCPStartStopData; TimeOut : Word):TCCPresult; stdcall;
external DLL_Name;

function CCP_StartStopSynchronizedDataTransmission(CcpHandle : TCCPHandle; StartOrStop : Boolean; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_ClearMemory(CcpHandle : TCCPHandle; MemorySize : Longword; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_Program(CcpHandle : TCCPHandle; Data : Pointer; Size : Byte; var MTA0Ext : Byte; var MTA0Addr : Longword; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_Program_6(CcpHandle : TCCPHandle; Data : Pointer; var MTA0Ext : Byte; var MTA0Addr : Longword; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_BuildChecksum(CcpHandle : TCCPHandle; BlockSize : Longword; ChecksumData : Pointer; var ChecksumSize : Byte; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_DiagnosticService(CcpHandle : TCCPHandle; DiagnosticNumber : Word; Parameters : Pointer; ParametersLength : Byte; var ReturnLength : Byte; var ReturnType : Byte; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_ActionService(CcpHandle : TCCPHandle; ActionNumber : Word; Parameters : Pointer; ParametersLength : Byte; var ReturnLength : Byte; var ReturnType : Byte; TimeOut : Word):TCCPResult; stdcall;
external DLL_Name;

function CCP_GetErrorText(Error: TCCPResult; StringBuffer: PAnsiChar): TCCPResult; stdcall;
external DLL_Name;

end.
