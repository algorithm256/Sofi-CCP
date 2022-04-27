Attribute VB_Name = "PCCP"

'  PCCP.bas
'
'  ~~~~~~~~~~~~
'
'  PCAN-CCP API
'
'  ~~~~~~~~~~~~
'
'  ------------------------------------------------------------------
'  Author : Keneth Wagner
'  Last change: 07.12.2017 Wagner
'
'  Language: Visual Basic 6.0
'  ------------------------------------------------------------------
'
'  Copyright (C) 2017  PEAK-System Technik GmbH, Darmstadt
'  more Info at http://www.peak-system.com 
'
'///////////////////////////////////////////////////////////
' Inclusion of other needed files
'///////////////////////////////////////////////////////////

' Limit values 
' 
Public Const CCP_MAX_RCV_QUEUE As Long = &H7FFF                 ' Maximum count of asynchronous messages that the queue can retain

' Result and Error values
'
Public Const CCP_ERROR_ACKNOWLEDGE_OK As Long = &H0             '------- CCP CCP Result Level 1
Public Const CCP_ERROR_DAQ_OVERLOAD As Long = &H1
Public Const CCP_ERROR_CMD_PROCESSOR_BUSY As Long = &H10
Public Const CCP_ERROR_DAQ_PROCESSOR_BUSY As Long = &H11
Public Const CCP_ERROR_INTERNAL_TIMEOUT As Long = &H12
Public Const CCP_ERROR_KEY_REQUEST As Long = &H18
Public Const CCP_ERROR_SESSION_STS_REQUEST As Long = &H19
Public Const CCP_ERROR_COLD_START_REQUEST As Long = &H20        '------- CCP Result Level 2
Public Const CCP_ERROR_CAL_DATA_INIT_REQUEST As Long = &H21
Public Const CCP_ERROR_DAQ_LIST_INIT_REQUEST As Long = &H22
Public Const CCP_ERROR_CODE_UPDATE_REQUEST As Long = &H23
Public Const CCP_ERROR_UNKNOWN_COMMAND As Long = &H30           '------- CCP Result Level 3 (Errors)
Public Const CCP_ERROR_COMMAND_SYNTAX As Long = &H31
Public Const CCP_ERROR_PARAM_OUT_OF_RANGE As Long = &H32
Public Const CCP_ERROR_ACCESS_DENIED As Long = &H33
Public Const CCP_ERROR_OVERLOAD As Long = &H34
Public Const CCP_ERROR_ACCESS_LOCKED As Long = &H35
Public Const CCP_ERROR_NOT_AVAILABLE As Long = &H36
Public Const CCP_ERROR_PCAN As Long = &H80000000                '------- PCAN Error (&H1...&H7FFFFFFF)

Public Const CCP_RSM_NONE As Byte = &H0                         ' Any resource / Any protection
Public Const CCP_RSM_CALIBRATION As Byte = &H1                  ' Calibration available / protected
Public Const CCP_RSM_DATA_ADQUISITION As Byte = &H2             ' Data Adquisition available / protected
Public Const CCP_RSM_MEMORY_PROGRAMMING As Byte = &H40          ' Flashing available / protected

' Session Status
'
Public Const CCP_STS_CALIBRATING As Byte = &H0                  ' Calibration Status
Public Const CCP_STS_ACQUIRING As Byte = &H1                    ' Data acquisition Status
Public Const CCP_STS_RESUME_REQUEST As Byte = &H2               ' Request resuming 
Public Const CCP_STS_STORE_REQUEST As Byte = &H40               ' Request storing
Public Const CCP_STS_RUNNING As Byte = &H80                     ' Running Status

' Start/Stop Mode
'
Public Const CCP_SSM_STOP As Byte = 0                           ' Data Transmission Stop
Public Const CCP_SSM_START As Byte = 1                          ' Data Transmission Start
Public Const CCP_SSM_PREPARE_START As Byte = 2                  ' Prepare for start data transmission

'///////////////////////////////////////////////////////////
' Structure definitions
'///////////////////////////////////////////////////////////

' CCP_ExchangeId structure
'
Public Type TCCPExchangeData
	IdLength As Byte                                          ' Length of the Slave Device ID
	DataType As Byte                                          ' Data type qualifier of the Slave Device ID
	AvailabilityMask As Byte                                  ' Resource Availability Mask
	ProtectionMask As Byte	                                  ' Resource Protection Mask
End Type

' CCP_StartStopDataTransmission structure
'
Public Type TCCPStartStopData
	Mode As Byte                                              ' 0: Stop, 1: Start
	ListNumber  As Byte                                       ' DAQ list number to process
	LastODTNumber  As Byte                                    ' ODTs to be transmitted (from 0 to byLastODTNumber)
	EventChannel As Byte                                      ' Generic signal source for timing determination
	TransmissionRatePrescaler As Integer                      ' Transmission rate prescaler
End Type

' Slave Inforamtion used within the CCP_Connect/CCP_Test functions
'
Public Type TCCPSlaveData
	EcuAddress As Integer                                     ' Station address (Intel format)
	IdCRO As Long                                             ' CAN Id used for CRO packages (29 Bits = MSB set)
	IdDTO As Long                                             ' CAN Id used for DTO packages (29 Bits = MSB set)
	IntelFormat As Byte                                       ' Format used by the slave (True: Intel, False: Motorola)
End Type

' Represents an asynchronous message sent from a slave
'
Public Type TCCPMsg
	Source As Long                                            ' Handle of the connection that has received the message
	Length As Byte                                            ' Data length of the message
	Data(7) As Byte                                           ' Data bytes (max. 8)
End Type 


'///////////////////////////////////////////////////////////
' PCAN-CCP API function declarations
'///////////////////////////////////////////////////////////

''------------------------------
'' Extras
''------------------------------

''' <summary>
''' Initializes the CAN communication using PCAN-Basic API
''' </summary>
''' <param name="Channel">The handle of a PCAN Channel</param>
''' <param name="Btr0Btr1">The speed for the communication (BTR0BTR1 code)</param>
''' <param name="HwType">NON PLUG&PLAY: The type of hardware and operation mode</param>
''' <param name="IOPort">NON PLUG&PLAY: The I/O address for the parallel port</param>
''' <param name="Interupt">NON PLUG&PLAY: Interrupt number of the parallel port</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_InitializeChannel Lib "PCCP.DLL" _
    (ByVal Channel As Integer, _
    ByVal Btr0Btr1 As Integer, _
    Optional ByVal HwType As Byte = 0, _
    Optional ByVal IOPort As Long = 0, _
    Optional ByVal Interrupt As Integer = 0) As Long


''' <summary>
''' Uninitializes a PCAN Channels initialized by CCP_InitializeChannel
''' </summary>
''' <param name="Channel">The handle of a PCAN Channel</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_UninitializeChannel Lib "PCCP.DLL" _
    (ByVal Channel As Integer) As Long


''' <summary>
''' Reads a message from the receive queue of a PCAN-CCP connection.
''' </summary>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="Msg">Buffer for a message. See 'TCCPMsg' above</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_ReadMsg Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByRef Msg As TCCPMsg) As Long


''' <summary>
''' Resets the receive-queue of a PCAN-CCP connection.
''' </summary>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_Reset Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long) As Long

''------------------------------
'' Connection
''------------------------------

''' <summary>
''' Establishes a logical connection between a master applicaiton and a slave unit 
''' </summary>
''' <param name="Channel">The handle of an initialized PCAN Channel</param>
''' <param name="SlaveData">The description data of the slave to be connected</param>
''' <param name="CcpHandle">A buffer to return the handle of this Master/Channel/Slave connection</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_Connect Lib "PCCP.DLL" _
    (ByVal Channel As Integer, _
    ByRef SlaveData As TCCPSlaveData, _
    ByRef CcpHandle As Long, _
    ByVal TimeOut As Integer) As Long


''' <summary>
''' Logically disconnects a master application from a slave unit 
''' </summary>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="Temporary">Indicates if the disconnection should be temporary or not</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_Disconnect Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByVal Temporary As Byte, _
    ByVal TimeOut As Integer) As Long


''' <summary>
''' Tests if a slave is available 
''' </summary>
''' <param name="Channel">The handle of an initialized PCAN Channel</param>
''' <param name="SlaveData">The data of the slave to be checked for availability</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_Test Lib "PCCP.DLL" _
    (ByVal Channel As Integer, _
    ByRef SlaveData As TCCPSlaveData, _
    ByVal TimeOut As Integer) As Long

''------------------------------
'' Control + Configuration
''------------------------------

''' <summary>
''' Exchanges the CCP Version used by a master and a slave
''' </summary>
''' <remarks>Both buffers, Main and Release, are bidirectional, e.g. they are used by both 
''' Master and Slave. The master should call this function placing in these ufferrs its used 
''' version. After the function returns, these buffers contain the version used by the 
''' connected slave</remarks>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="Main">Buffer for the CCP Main version used.</param>
''' <param name="Release">Buffer for the CCP Release version used.</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_GetCcpVersion Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByRef Main As Byte, _
    ByRef Relase As Byte, _
    ByVal TimeOut As Integer) As Long


''' <summary>
''' Exchanges IDs between Master and Slave for automatic session configuration
''' </summary>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="ECUData">Slave ID and Resource Information buffer</param>
''' <param name="MasterData">Optional master data (ID)</param>
''' <param name="DataLength">Length of the master data</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_ExchangeId Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByRef ECUData As TCCPExchangeData, _
    ByRef MasterData As Byte, _
    ByVal DataLength As Long, _
    ByVal TimeOut As Integer) As Long


''' <summary>
''' Returns Seed data for a seed&key algorithm to unlock a slave functionality
''' </summary>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="Resource">The resource being asked. See 'Resource / Protection Mask' values above</param>
''' <param name="CurrentStatus">Current protection status of the asked resource</param>
''' <param name="Seed">Seed value for the seed&key algorithm</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_GetSeed Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByVal Resource As Byte, _
    ByRef CurrentStatus As Byte, _
    ByRef Seed As Byte, _
    ByVal TimeOut As Integer) As Long


''' <summary>
''' Unlocks the security protection of a resource within a connected slave.
''' </summary>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="KeyBuffer">A buffer with the key computed from a seed value obtained 
''' through the CCP_GetSeed function</param>
''' <param name="KeyLength">The length in bytes of the key buffer value</param>
''' <param name="Privileges">The current privileges status on the slave. 
''' See 'Resource / Protection Mask' values above"</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_Unlock Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByRef KeyBuffer As Byte, _
    ByVal KeyLength As Byte, _
    ByRef Privileges As Byte, _
    ByVal TimeOut As Integer) As Long


''' <summary>
''' Keeps the connected slave informed about the current state of the calibration session
''' </summary>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="Status">Current status bits. See 'Session Status' values above</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_SetSessionStatus Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByVal Status As Byte, _
    ByVal TimeOut As Integer) As Long


''' <summary>
''' Retrieves the information about the current state of the calibration session
''' </summary>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="Status">Current status bits. See 'Session Status' values above</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_GetSessionStatus Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByRef Status As Byte, _
    ByVal TimeOut As Integer) As Long

''------------------------------
'' Memory management
''------------------------------

''' <summary>
''' Initializes a base pointer for all following memory transfers.
''' </summary>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="UsedMTA">Memory Transfer Address (MTA) number (0 or 1)</param>
''' <param name="AddrExtension">address extension</param>
''' <param name="AddrExtension">Address</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_SetMemoryTransferAddress Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByVal UsedMTA As Byte, _
    ByVal AddrExtension As Byte, _
    ByVal Addr As Long, _
    ByVal TimeOut As Integer) As Long


''' <summary>
''' Copies a block of data into memory, starting at the current MTA0. 
''' </summary>
''' <remarks>MTA0 is post-incremented by the value of 'Size'</remarks>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="DataBytes">Buffer with the data to be transferred</param>
''' <param name="Size">Size of the data to be transferred, in bytes</param>
''' <param name="MTA0Ext">MTA0 extension after post-increment</param>
''' <param name="MTA0Addr">MTA0 Address after post-increment</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_Download Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByRef DataBytes As Byte, _
    ByVal Size As Byte, _
    ByRef MTA0Ext As Byte, _
    ByRef MTA0Addr As Long, _
    ByVal TimeOut As Integer) As Long


''' <summary>
''' Copies a block of 6 data bytes into memory, starting at the current MTA0. 
''' </summary>
''' <remarks>MTA0 is post-incremented by the value of 6</remarks>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="DataBytes">Buffer with the data to be transferred</param>
''' <param name="MTA0Ext">MTA0 extension after post-increment</param>
''' <param name="MTA0Addr">MTA0 Address after post-increment</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_Download_6 Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByRef DataBytes As Byte, _
    ByRef MTA0Ext As Byte, _
    ByRef MTA0Addr As Long, _
    ByVal TimeOut As Integer) As Long


''' <summary>
''' Retrieves a block of data starting at the current MTA0.  
''' </summary>
''' <remarks>MTA0 will be post-incremented with the value of size</remarks>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="Size">Size of the data to be retrieved, in bytes</param>
''' <param name="DataBytes">Buffer for the requested data</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_Upload Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByVal Size As Byte, _
    ByRef DataBytes As Byte, _
    ByVal TimeOut As Integer) As Long


''' <summary>
''' Retrieves a block of data.  
''' </summary>
''' <remarks>The amount of data is retrieved from the given address. The MTA0 
''' pointer \"remains unchanged\"</remarks>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="UploadSize">Size of the data to be retrieved, in bytes</param>
''' <param name="MTA0Ext">MTA0 extension for the upload</param>
''' <param name="MTA0Addr">MTA0 Address for the upload</param>
''' <param name="reqData">Buffer for the requested data</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_ShortUpload Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByVal UploadSize As Byte, _
    ByVal MTA0Ext As Byte, _
    ByVal MTA0Addr As Long, _
    ByRef reqData As Byte, _
    ByVal TimeOut As Integer) As Long


''' <summary>
''' Copies a block of data from the address MTA0 to the address MTA1 
''' </summary>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="SizeOfData">Number of bytes to be moved</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_Move Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByVal SizeOfData As Long, _
    ByVal TimeOut As Integer) As Long

''------------------------------
'' Calibration
''------------------------------

''' <summary>
''' ECU Implementation dependant. Sets the previously initialized MTA0 as the start of the 
''' current active calibration data page
''' </summary>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_SelectCalibrationDataPage Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByVal TimeOut As Integer) As Long


''' <summary>
''' Retrieves the start address of the calibration page that is currently active in the slave device
''' calibration data page that is selected as the currently active page
''' </summary>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="MTA0Ext">Buffer for the MTAO address extension</param>
''' <param name="MTA0Addr">Buffer for the MTA0 address pointer</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_GetActiveCalibrationPage Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByRef MTA0Ext As Byte, _
    ByRef MTA0Addr As Long, _
    ByVal TimeOut As Integer) As Long

''------------------------------
'' Data Adquisition
''------------------------------

''' <summary>
''' Retrieves the size of the specified DAQ List as the number of available Object
''' Descriptor Tables (ODTs) and clears the current list.
''' Optionally, sets a dedicated CAN-ID for the DAQ list.
''' </summary>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="ListNumber">DAQ List number</param>
''' <param name="DTOId">CAN identifier of DTO dedicated to the given 'ListNumber'</param>
''' <param name="Size">Buffer for the list size (Number of ODTs in the DAQ list)</param>
''' <param name="FirstPDI">Buffer for the first PID of the DAQ list</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_GetDAQListSize Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByVal ListNumber As Byte, _
    ByRef DTOId As Long, _
    ByRef Size As Byte, _
    ByRef FirstPDI As Byte, _
    ByVal TimeOut As Integer) As Long


''' <summary>
''' Initializes the DAQ List pointer for a subsequent write to a DAQ list.
''' </summary>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="ListNumber">DAQ List number</param>
''' <param name="ODTNumber">Object Descriptor Table number</param>
''' <param name="ElementNumber">Element number within the ODT</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_SetDAQListPointer Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByVal ListNumber As Byte, _
    ByVal ODTNumber As Byte, _
    ByVal ElementNumber As Byte, _
    ByVal TimeOut As Integer) As Long


''' <summary>
''' Writes one entry (DAQ element) to a DAQ list defined by the DAQ list pointer.
''' </summary>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="SizeElement">Size of the DAQ elements in bytes {1, 2, 4}</param>
''' <param name="AddrExtension">Address extension of DAQ element</param>
''' <param name="AddrDAQ">Address of a DAQ element</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_WriteDAQListEntry Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByVal SizeElement As Byte, _
    ByVal AddrExtension As Byte, _
    ByVal AddrDAQ As Long, _
    ByVal TimeOut As Integer) As Long


''' <summary>
''' Starts/Stops the data acquisition and/or prepares a synchronized start of the 
''' specified DAQ list
''' </summary>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="Data">Contains the data to be applied within the start/stop procedure. 
''' See 'TCCPStartStopData' structure above</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_StartStopDataTransmission Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByRef Data As TCCPStartStopData, _
    ByVal TimeOut As Integer) As Long


''' <summary>
''' Starts/Stops the periodic transmission of all DAQ lists. 
''' <remarks>Starts all DAQs configured as "prepare to start" with a previously 
''' CCP_StartStopDataTransmission call. Stops all DAQs, included those not started 
''' synchronized</remarks>
''' </summary>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="StartOrStop">True: Starts the configured DAQ lists. False: Stops all DAQ lists</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_StartStopSynchronizedDataTransmission Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByVal StartOrStop As Byte, _
    ByVal TimeOut As Integer) As Long

''------------------------------
'' Flash Programming
''------------------------------

''' <summary>
''' Erases non-volatile memory (FLASH EPROM) prior to reprogramming.
''' </summary>
''' <remarks>The MTA0 pointer points to the memory location to be erased</remarks>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="MemorySize">Memory size in bytes to be erased</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_ClearMemory Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByVal MemorySize As Long, _
    ByVal TimeOut As Integer) As Long


''' <summary>
''' Programms a block of data into non-volatile (FLASH, EPROM) memory, starting 
''' at the current MTA0. 
''' </summary>
''' <remarks>The MTA0 pointer is post-incremented by the value of 'Size'</remarks>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="Data">Buffer with the data to be programmed</param>
''' <param name="Size">Size of the 'Data' block to be programmed</param>
''' <param name="MTA0Ext">MTA0 extension after post-increment</param>
''' <param name="MTA0Addr">MTA0 Address after post-increment</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_Program Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByRef Data As Byte, _
    ByVal Size As Byte, _
    ByRef MTA0Ext As Byte, _
    ByRef MTA0Addr As Long, _
    ByVal TimeOut As Integer) As Long


''' <summary>
''' Programms a block of 6 data bytes into non-volatile (FLASH, EPROM) memory, 
''' starting at the current MTA0. 
''' </summary>
''' <remarks>The MTA0 pointer is post-incremented by 6</remarks>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="Data">Buffer with the data to be programmed</param>
''' <param name="MTA0Ext">MTA0 extension after post-increment</param>
''' <param name="MTA0Addr">MTA0 Address after post-increment</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_Program_6 Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByRef Data As Byte, _
    ByRef MTA0Ext As Byte, _
    ByRef MTA0Addr As Long, _
    ByVal TimeOut As Integer) As Long


''' <summary>
''' Calculates a checksum result of the memory block that is defined by MTA0 
''' (Memory Transfer Area Start address) and 'BlockSize'.
''' </summary>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="BlockSize">Block size in bytes</param>
''' <param name="ChecksumData">Checksum data (implementation specific)</param>
''' <param name="ChecksumSize">Size of the checksum data</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_BuildChecksum Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByVal BlockSize As Long, _
    ByRef ChecksumData As Byte, _
    ByRef ChecksumSize As Byte, _
    ByVal TimeOut As Integer) As Long

''------------------------------
'' Diagnostic
''------------------------------

''' <summary>
''' Executes a defined diagnostic procedure and sets the MTA0 pointer to the location 
''' from where the master can upload the requested information
''' </summary>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="DiagnosticNumber">Diagnostic service number</param>
''' <param name="Parameters">Parameters, if applicable</param>
''' <param name="ParametersLength">Length in bytes of the parameters passed within 'Parameters'</param>
''' <param name="ReturnLength">Length of the return information (to be uploaded)</param>
''' <param name="ReturnType">Data type qualifier of the return information</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_DiagnosticService Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByVal DiagnosticNumber As Integer, _
    ByRef Parameters As Byte, _
    ByVal ParametersLength As Byte, _
    ByRef ReturnLength As Byte, _
    ByRef ReturnType As Byte, _
    ByVal TimeOut As Integer) As Long


''' <summary>
''' Executes a defined action and sets the MTA0 pointer to the location 
''' from where the master can upload the requested information
''' </summary>
''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
''' <param name="ActionNumber">Action service number</param>
''' <param name="Parameters">Parameters, if applicable</param>
''' <param name="ParametersLength">Length in bytes of the parameters passed within 'Parameters'</param>
''' <param name="ReturnLength">Length of the return information (to be uploaded)</param>
''' <param name="ReturnType">Data type qualifier of the return information</param>
''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
''' <returns>A TCCPResult result code</returns>
Public Declare Function CCP_ActionService Lib "PCCP.DLL" _
    (ByVal CcpHandle As Long, _
    ByVal ActionNumber As Integer, _
    ByRef Parameters As Byte, _
    ByVal ParametersLength As Byte, _
    ByRef ReturnLength As Byte, _
    ByRef ReturnType As Byte, _
    ByVal TimeOut As Integer) As Long

' <summary>
' Converts an error code to its equivalent english text
' </summary>
' <param name="Error">"A TCCPResult error code"</param>
' <param name="Buffer">"Buffer for a null terminated char array (must be at least 256 in length)"</param>
' <returns>"A TCCPResult error code"</returns>
Public Declare Function CCP_GetErrorText Lib "PCCP.DLL" _
    (ByVal ErrorCode As Long, _
    ByVal Buffer As String) As Long