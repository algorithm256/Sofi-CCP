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
using namespace System;
using namespace System::Text;
using namespace System::Runtime::InteropServices;

////////////////////////////////////////////////////////////
// Type definitions
////////////////////////////////////////////////////////////
#include ".\PCANBasicCLR.h"
using namespace Peak::Can::Basic;
#define TCCPHandle                             System::UInt32     // Represents a PCAN-CCP Connection Handle

namespace Peak
{
	namespace Can
	{
		namespace Ccp
		{
			#pragma region Enumerations
			/// <summary>
			/// Start/Stop Mode
			/// </summary>
			public enum class TCCPStartStopMode : Byte
			{
				CCP_SSM_STOP = 0,
				CCP_SSM_START = 1,
				CCP_SSM_PREPARE_START = 2,
			};

			/// <summary>
			/// Resource / Protection Mask
			/// </summary>
			[Flags]
			public enum class TCCPResourceMask : Byte
			{
				CCP_RSM_NONE = 0,
				CCP_RSM_CALIBRATION = 0x1,
				CCP_RSM_DATA_ADQUISITION = 0x2,
				CCP_RSM_MEMORY_PROGRAMMING = 0x40,
			};

			/// <summary>
			/// Result and Error values
			/// </summary>
			public enum class TCCPResult : UInt32
			{
				CCP_ERROR_ACKNOWLEDGE_OK = 0x00,            //------- CCP CCP Result Level 1
				CCP_ERROR_DAQ_OVERLOAD = 0x01,
				CCP_ERROR_CMD_PROCESSOR_BUSY = 0x10,
				CCP_ERROR_DAQ_PROCESSOR_BUSY = 0x11,
				CCP_ERROR_INTERNAL_TIMEOUT = 0x12,
				CCP_ERROR_KEY_REQUEST = 0x18,
				CCP_ERROR_SESSION_STS_REQUEST = 0x19,
				CCP_ERROR_COLD_START_REQUEST = 0x20,        //------- CCP Result Level 2
				CCP_ERROR_CAL_DATA_INIT_REQUEST = 0x21,
				CCP_ERROR_DAQ_LIST_INIT_REQUEST = 0x22,
				CCP_ERROR_CODE_UPDATE_REQUEST = 0x23,
				CCP_ERROR_UNKNOWN_COMMAND = 0x30,           //------- CCP Result Level 3 (Errors)
				CCP_ERROR_COMMAND_SYNTAX = 0x31,
				CCP_ERROR_PARAM_OUT_OF_RANGE = 0x32,
				CCP_ERROR_ACCESS_DENIED = 0x33,
				CCP_ERROR_OVERLOAD = 0x34,
				CCP_ERROR_ACCESS_LOCKED= 0x35,
				CCP_ERROR_NOT_AVAILABLE = 0x36,

				CCP_ERROR_PCAN = 0x80000000,                 //-------- PCAN-Basic Error FLAG (value & 0x7FFFFFFF)
			};

			/// <summary>
			/// Session Status
			/// </summary>
			public enum class TCCPSessionStatus : Byte
			{
				CCP_STS_CALIBRATING = 1,
				CCP_STS_ACQUIRING = 2,
				CCP_STS_RESUME_REQUEST = 4,
				CCP_STS_STORE_REQUEST = 64,
				CCP_STS_RUNNING = 128,
			};

			/// <summary>
			/// Category of an error TCCPResult
			/// </summary>
			public enum class TCCPErrorCategory : Byte
			{
				NotDefined = 0,
				Warning = 1,
				Spurious = 2,
				Resolvable = 3,
				Unresolvable = 4,
			};
			#pragma endregion

			#pragma region Structures
			/// <summary>
			/// Help structure for better handling the return value of 
			/// the API functions
			/// </summary>
			[StructLayout(LayoutKind::Auto, Size = 4)]
			public value struct CCPResult
			{
				private:
					[MarshalAs(UnmanagedType::U4)]
					UInt32 m_pccpResult;

				public: CCPResult(TCCPResult result)
					{
						m_pccpResult = (UInt32)result;
					}

				private: bool IsCanError()
					{
						return (TCCPResult)(m_pccpResult & (UInt32)TCCPResult::CCP_ERROR_PCAN) == TCCPResult::CCP_ERROR_PCAN;
					}

				private: TCCPErrorCategory GetErrorCategory()
					{
						Byte valueTest;

						valueTest = (!IsCanError() ? (((Byte)CCP & 0xF0) >> 4) : 0);
						return (TCCPErrorCategory)valueTest;
					}

				public: property TCCPResult CCP
					{
						TCCPResult get() { return IsCanError() ? TCCPResult::CCP_ERROR_PCAN : (TCCPResult)(m_pccpResult & 0xFF); }
					}

				public: property TPCANStatus PCAN
					{
						TPCANStatus get() { return (TPCANStatus)(m_pccpResult & 0x7FFFFFFF); }
					}

				public: property TCCPErrorCategory ErrorCategory
					{
						TCCPErrorCategory get() { return GetErrorCategory(); }
					}
			};

			/// <summary>
			/// CCP_StartStopDataTransmission structure
			/// </summary>
			[StructLayout(LayoutKind::Sequential)]
			public value struct TCCPStartStopData
			{
				[MarshalAs(UnmanagedType::U1)]
				TCCPStartStopMode Mode;
				Byte ListNumber;
				Byte LastODTNumber;
				Byte EventChannel;
				UInt16 TransmissionRatePrescaler;
			};

			/// <summary>
			/// CCP_ExchangeId structure
			/// </summary>
			[StructLayout(LayoutKind::Sequential)]
			public value struct TCCPExchangeData
			{
				Byte IdLength;
				Byte DataType;
				[MarshalAs(UnmanagedType::U1)]
				TCCPResourceMask AvailabilityMask;
				[MarshalAs(UnmanagedType::U1)]
				TCCPResourceMask ProtectionMask;
			};

			/// <summary>
			/// Slave Information used within the CCP_Connect/CCP_Test functions
			/// </summary>
			[StructLayout(LayoutKind::Sequential)]
			public value struct TCCPSlaveData
			{   
				UInt16 EcuAddress;
				UInt32 IdCRO;
				UInt32 IdDTO;
				[MarshalAs(UnmanagedType::U1)]
				bool IntelFormat;
			};

			/// <summary>
			/// Represents an asynchronous message sent from a slave
			/// </summary>
			[StructLayout(LayoutKind::Sequential)]
			public value struct TCCPMsg
			{
				TCCPHandle Source;
				Byte Length;
				[MarshalAs(UnmanagedType::ByValArray, SizeConst=8)]
				array<Byte>^ Data;
			};
			#pragma endregion

			#pragma region PCAN-CCP Class
			public ref class CCPApi abstract sealed
			{
				public:
				/// <summary>
				/// Maximum count of asynchronous messages that the queue can retain
				/// </summary>
				static const int CCP_MAX_RCV_QUEUE = 0x7FFF;

				/// <summary>
				/// Initializes the CAN communication using PCAN-Basic API
				/// </summary>
				/// <param name="Channel">The handle of a PCAN Channel</param>
				/// <param name="Btr0Btr1">The speed for the communication (BTR0BTR1 code)</param>
				/// <param name="HwType">NON PLUG&PLAY: The type of hardware and operation mode</param>
				/// <param name="IOPort">NON PLUG&PLAY: The I/O address for the parallel port</param>
				/// <param name="Interrupt">NON PLUG&PLAY: Interrupt number of the parallel port</param>
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_InitializeChannel")]
				static TCCPResult InitializeChannel(
					[MarshalAs(UnmanagedType::U2)]
					TPCANHandle Channel,
					[MarshalAs(UnmanagedType::U2)]
					TPCANBaudrate Btr0Btr1,
					[MarshalAs(UnmanagedType::U1)]
					TPCANType HwType, 
					UInt32 IOPort, 
					UInt16 Interrupt);

				/// <summary>
				/// Initializes the CAN communication using PCAN-Basic API
				/// </summary>
				/// <param name="Channel">The handle of a PCAN Channel</param>
				/// <param name="Btr0Btr1">The speed for the communication (BTR0BTR1 code)</param>
				/// <returns>A CCPResult result code</returns>
				static TCCPResult InitializeChannel(
					TPCANHandle Channel,
					TPCANBaudrate Btr0Btr1)
				{
					return InitializeChannel(Channel, Btr0Btr1, (TPCANType)0, 0, 0);
				}

				/// <summary>
				/// Uninitializes a PCAN Channels initialized by CCP_InitializeChannel
				/// </summary>
				/// <param name="Channel">The handle of a PCAN Channel</param>
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_UninitializeChannel")]
				static TCCPResult UninitializeChannel(
					[MarshalAs(UnmanagedType::U2)]
					TPCANHandle Channel);

				/// <summary>
				/// Reads a message from the receive queue of a PCAN-CCP connection.
				/// </summary>
				/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
				/// <param name="Msg">Buffer for a message. See 'TCCPMsg' above</param>
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_ReadMsg")]
				static TCCPResult ReadMsg(
					TCCPHandle CcpHandle,
					TCCPMsg %Msg);

				/// <summary>
				/// Resets the receive-queue of a PCAN-CCP connection.
				/// </summary>
				/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_Reset")]
				static TCCPResult Reset(
					TCCPHandle CcpHandle);

				/// <summary>
				/// Establishes a logical connection between a master applicaiton and a slave unit 
				/// </summary>
				/// <param name="Channel">The handle of an initialized PCAN Channel</param>
				/// <param name="SlaveData">The description data of the slave to be connected</param>
				/// <param name="CcpHandle">A buffer to return the handle of this Master/Channel/Slave connection</param>
				/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_Connect")]
				static TCCPResult Connect(
					[MarshalAs(UnmanagedType::U2)]
					TPCANHandle Channel,
					TCCPSlaveData %SlaveData,
					TCCPHandle %CcpHandle,
					UInt16 TimeOut);

				/// <summary>
				/// Logically disconnects a master application from a slave unit 
				/// </summary>
				/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
				/// <param name="Temporary">Indicates if the disconnection should be temporary or not</param>
				/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_Disconnect")]
				static TCCPResult Disconnect(
					TCCPHandle CcpHandle,
					bool Temporary,
					UInt16 TimeOut);

				/// <summary>
				/// Tests if a slave is available 
				/// </summary>
				/// <param name="Channel">The handle of an initialized PCAN Channel</param>
				/// <param name="SlaveData">The data of the slave to be checked for availability</param>
				/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_Test")]
				static TCCPResult Test(
					[MarshalAs(UnmanagedType::U2)]
					TPCANHandle Channel,
					TCCPSlaveData %SlaveData,
					UInt16 TimeOut);

				/// <summary>
				/// Exchanges the CCP Version used by a master and a slave
				/// </summary>
				/// <remarks>Both buffers, Main and Release, are bidirectional, e.g. they are used by both 
				/// Master and Slave. The master should call this function placing in these ufferrs its used 
				/// version. After the function returns, these buffers contain the version used by the 
				/// connected slave</remarks>
				/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
				/// <param name="Main">Buffer for the CCP Main version used</param>
				/// <param name="Release">Buffer for the CCP Release version used</param>
				/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_GetCcpVersion")]
				static TCCPResult GetCcpVersion(
					TCCPHandle CcpHandle,
					Byte %Main,
					Byte %Release,
					UInt16 TimeOut);

				/// <summary>
				/// Exchanges IDs between Master and Slave for automatic session configuration
				/// </summary>
				/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
				/// <param name="ECUData">Slave ID and Resource Information buffer</param>
				/// <param name="MasterData">Optional master data (ID)</param>
				/// <param name="DataLength">Length of the master data</param>
				/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_ExchangeId")]
				static TCCPResult ExchangeId(
					TCCPHandle CcpHandle,
					TCCPExchangeData %ECUData,
					[MarshalAs(UnmanagedType::LPArray, SizeParamIndex = 3)]
					array<Byte>^ MasterData,
					int DataLength,
					UInt16 TimeOut);
	
				/// <summary>
				/// Exchanges IDs between Master and Slave for automatic session configuration
				/// </summary>
				/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
				/// <param name="ECUData">Slave ID and Resource Information buffer</param>
				/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
				/// <returns>A CCPResult result code</returns>
				static TCCPResult ExchangeId(
					TCCPHandle CcpHandle,
					TCCPExchangeData %ECUData,
					UInt16 TimeOut)
				{
					return ExchangeId(CcpHandle, ECUData, gcnew array<Byte>(0), 0, TimeOut);
				}

				/// <summary>
				/// Returns Seed data for a seed&key algorithm to unlock a slave functionality
				/// </summary>
				/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
				/// <param name="Resource">The resource being asked. See 'Resource / Protection Mask' values above</param>
				/// <param name="CurrentStatus">Current protection status of the asked resource</param>
				/// <param name="Seed">Seed value for the seed&key algorithm</param>
				/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_GetSeed")]
				static TCCPResult GetSeed(
					TCCPHandle CcpHandle, 
					Byte Resource,
					bool %CurrentStatus, 
					[MarshalAs(UnmanagedType::LPArray)]
					array<Byte>^ Seed,
					UInt16 TimeOut);

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
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_Unlock")]
				static TCCPResult Unlock(
					TCCPHandle CcpHandle,
					[MarshalAs(UnmanagedType::LPArray, SizeParamIndex = 2)]
					array<Byte>^ KeyBuffer,
					Byte KeyLength,
					[MarshalAs(UnmanagedType::U1)]
					TCCPResourceMask %Privileges,
					UInt16 TimeOut);

				/// <summary>
				/// Keeps the connected slave informed about the current state of the calibration session
				/// </summary>
				/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
				/// <param name="Status">Current status bits. See 'Session Status' values above</param>
				/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_SetSessionStatus")]
				static TCCPResult SetSessionStatus(
					TCCPHandle CcpHandle, 
					[MarshalAs(UnmanagedType::U1)]
					TCCPSessionStatus Status,
					UInt16 TimeOut);

				/// <summary>
				/// Retrieves the information about the current state of the calibration session
				/// </summary>
				/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
				/// <param name="Status">Current status bits. See 'Session Status' values above</param>
				/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_GetSessionStatus")]
				static TCCPResult GetSessionStatus(
					TCCPHandle CcpHandle,
					[MarshalAs(UnmanagedType::U1)]
					TCCPSessionStatus %Status,
					UInt16 TimeOut);

				/// <summary>
				/// Initializes a base pointer for all following memory transfers.
				/// </summary>
				/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
				/// <param name="UsedMTA">Memory Transfer Address (MTA) number (0 or 1)</param>
				/// <param name="AddrExtension">Address extension</param>
				/// <param name="Addr">Address</param>
				/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_SetMemoryTransferAddress")]
				static TCCPResult SetMemoryTransferAddress(
					TCCPHandle CcpHandle,
					Byte UsedMTA,
					Byte AddrExtension, 
					UInt32 Addr,
					UInt16 TimeOut);

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
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_Download")]
				static TCCPResult Download(
					TCCPHandle CcpHandle,
					[MarshalAs(UnmanagedType::LPArray, SizeParamIndex = 2)]
					array<Byte>^ DataBytes, 
					Byte Size, 
					Byte %MTA0Ext, 
					UInt32 %MTA0Addr,
					UInt16 TimeOut);

				/// <summary>
				/// Copies a block of 6 data bytes into memory, starting at the current MTA0. 
				/// </summary>
				/// <remarks>MTA0 is post-incremented by the value of 6</remarks>
				/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
				/// <param name="DataBytes">Buffer with the data to be transferred</param>
				/// <param name="MTA0Ext">MTA0 extension after post-increment</param>
				/// <param name="MTA0Addr">MTA0 Address after post-increment</param>
				/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_Download_6")]
				static TCCPResult Download_6(
					TCCPHandle CcpHandle,
					[MarshalAs(UnmanagedType::LPArray, SizeConst=6)]
					array<Byte>^ DataBytes, 
					Byte %MTA0Ext, 
					UInt32 %MTA0Addr,
					UInt16 TimeOut);

				/// <summary>
				/// Retrieves a block of data starting at the current MTA0.  
				/// </summary>
				/// <remarks>MTA0 will be post-incremented with the value of size</remarks>
				/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
				/// <param name="Size">Size of the data to be retrieved, in bytes</param>
				/// <param name="DataBytes">Buffer for the requested data</param>
				/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_Upload")]
				static TCCPResult Upload(
					TCCPHandle CcpHandle, 
					Byte Size,
					[MarshalAs(UnmanagedType::LPArray, SizeParamIndex = 1)]
					array<Byte>^ DataBytes,
					UInt16 TimeOut);

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
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_ShortUpload")]
				static TCCPResult ShortUpload(
					TCCPHandle CcpHandle, 
					Byte UploadSize,
					Byte MTA0Ext,
					UInt32 MTA0Addr,
					[MarshalAs(UnmanagedType::LPArray, SizeParamIndex = 1)]
					array<Byte>^ reqData,
					UInt16 TimeOut);

				/// <summary>
				/// Copies a block of data from the address MTA0 to the address MTA1 
				/// </summary>
				/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
				/// <param name="SizeOfData">Number of bytes to be moved</param>
				/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_Move")]
				static TCCPResult Move(
					TCCPHandle CcpHandle, 
					UInt32 SizeOfData,
					UInt16 TimeOut);

				/// <summary>
				/// ECU Implementation dependant. Sets the previously initialized MTA0 as the start of the 
				/// current active calibration data page
				/// </summary>
				/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
				/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_SelectCalibrationDataPage")]
				static TCCPResult SelectCalibrationDataPage(
					TCCPHandle CcpHandle,
					UInt16 TimeOut);

				/// <summary>
				/// Retrieves the start address of the calibration page that is currently active in the slave device
				/// calibration data page that is selected as the currently active page
				/// </summary>
				/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
				/// <param name="MTA0Ext">Buffer for the MTAO address extension</param>
				/// <param name="MTA0Addr">Buffer for the MTA0 address pointer</param>
				/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_GetActiveCalibrationPage")]
				static TCCPResult GetActiveCalibrationPage(
					TCCPHandle CcpHandle, 
					Byte %MTA0Ext, 
					UInt32 %MTA0Addr,
					UInt16 TimeOut);

				/// <summary>
				/// Retrieves the size of the specified DAQ List as the number of available Object
				/// Descriptor Tables (ODTs) and clears the current list.
				/// Optionally, sets a dedicated CAN-ID for the DAQ list.
				/// </summary>
				/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
				/// <param name="ListNumber">DAQ List number</param>
				/// <param name="DTOId">CAN identifier of DTO dedicated to the given 'ListNumber'</param>
				/// <param name="Size">Buffer for the list size (Number of ODTs in the DAQ list)</param>
				/// <param name="FirstPID">Buffer for the first PID of the DAQ list</param>
				/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_GetDAQListSize")]
				static TCCPResult GetDAQListSize(
					TCCPHandle CcpHandle, 
					Byte ListNumber, 
					UInt32 %DTOId, 
					Byte %Size, 
					Byte %FirstPID,
					UInt16 TimeOut);

				[DllImport("PCCP.dll", EntryPoint = "CCP_GetDAQListSize")]
				static TCCPResult GetDAQListSize(
					TCCPHandle CcpHandle,
					Byte listNumber,
					UIntPtr^ ptr,
					Byte %size,
					Byte %FirstPID,
					UInt16 TimeOut);

				/// <summary>
				/// Retrieves the size of the specified DAQ List as the number of available Object
				/// Descriptor Tables (ODTs) and clears the current list.
				/// Optionally, sets a dedicated CAN-ID for the DAQ list.
				/// </summary>
				/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
				/// <param name="ListNumber">DAQ List number</param>
				/// <param name="Size">Buffer for the list size (Number of ODTs in the DAQ list)</param>
				/// <param name="FirstPID">Buffer for the first PID of the DAQ list</param>
				/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
				/// <returns>A CCPResult result code</returns>
				static TCCPResult GetDAQListSize(
					TCCPHandle CcpHandle,
					Byte ListNumber,
					Byte %Size,
					Byte %FirstPID,
					UInt16 TimeOut)
				{
					return GetDAQListSize(CcpHandle, ListNumber, UIntPtr::Zero, Size, FirstPID, TimeOut);
				}

				/// <summary>
				/// Initializes the DAQ List pointer for a subsequent write to a DAQ list.
				/// </summary>
				/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
				/// <param name="ListNumber">DAQ List number</param>
				/// <param name="ODTNumber">Object Descriptor Table number</param>
				/// <param name="ElementNumber">Element number within the ODT</param>
				/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_SetDAQListPointer")]
				static TCCPResult SetDAQListPointer(
					TCCPHandle CcpHandle, 
					Byte ListNumber, 
					Byte ODTNumber, 
					Byte ElementNumber,
					UInt16 TimeOut);

				/// <summary>
				/// Writes one entry (DAQ element) to a DAQ list defined by the DAQ list pointer.
				/// </summary>
				/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
				/// <param name="SizeElement">Size of the DAQ elements in bytes {1, 2, 4}</param>
				/// <param name="AddrExtension">Address extension of DAQ element</param>
				/// <param name="AddrDAQ">Address of a DAQ element</param>
				/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_WriteDAQListEntry")]
				static TCCPResult WriteDAQListEntry(
					TCCPHandle CcpHandle, 
					Byte SizeElement, 
					Byte AddrExtension, 
					UInt32 addrDAQ,
					UInt16 TimeOut);

				/// <summary>
				/// Starts/Stops the data acquisition and/or prepares a synchronized start of the 
				/// specified DAQ list
				/// </summary>
				/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
				/// <param name="Data">Contains the data to be applied within the start/stop procedure. 
				/// See 'TCCPStartStopData' structure above</param>
				/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_StartStopDataTransmission")]
				static TCCPResult StartStopDataTransmission(
					TCCPHandle CcpHandle, 
					TCCPStartStopData %Data,
					UInt16 TimeOut);

				/// <summary>
				/// Starts/Stops the periodic transmission of all DAQ lists. 
				/// <remarks>Starts all DAQs configured as "prepare to start" with a previously 
				/// CCP_StartStopDataTransmission call. Stops all DAQs, included those not started 
				/// synchronized</remarks>
				/// </summary>
				/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
				/// <param name="StartOrStop">True: Starts the configured DAQ lists. False: Stops all DAQ lists</param>
				/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_StartStopSynchronizedDataTransmission")]
				static TCCPResult StartStopSynchronizedDataTransmission(
					TCCPHandle CcpHandle,
					bool StartOrStop,
					UInt16 TimeOut);

				/// <summary>
				/// Erases non-volatile memory (FLASH EPROM) prior to reprogramming.
				/// </summary>
				/// <remarks>The MTA0 pointer points to the memory location to be erased</remarks>
				/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
				/// <param name="MemorySize">Memory size in bytes to be erased</param>
				/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_ClearMemory")]
				static TCCPResult ClearMemory(
					TCCPHandle CcpHandle, 
					UInt32 MemorySize,
					UInt16 TimeOut);

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
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_Program")]
				static TCCPResult Program(
					TCCPHandle CcpHandle,
					[MarshalAs(UnmanagedType::LPArray, SizeParamIndex = 2)]
					array<Byte>^ Data, 
					Byte Size, 
					Byte %MTA0Ext, 
					UInt32 %MTA0Addr,
					UInt16 TimeOut);


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
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_Program_6")]
				static TCCPResult Program_6(
					TCCPHandle CcpHandle,
					[MarshalAs(UnmanagedType::LPArray, SizeConst = 6)]
					array<Byte>^ Data, 
					Byte %MTA0Ext, 
					UInt32 %MTA0Addr,
					UInt16 TimeOut);

				/// <summary>
				/// Calculates a checksum result of the memory block that is defined by MTA0 
				/// (Memory Transfer Area Start address) and 'BlockSize'.
				/// </summary>
				/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
				/// <param name="BlockSize">Block size in bytes</param>
				/// <param name="ChecksumData">Checksum data (implementation specific)</param>
				/// <param name="ChecksumSize">Size of the checksum data</param>
				/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_BuildChecksum")]
				static TCCPResult BuildChecksum(
					TCCPHandle CcpHandle, 
					UInt32 BlockSize,
					[MarshalAs(UnmanagedType::LPArray, SizeParamIndex = 3)]
					array<Byte>^ ChecksumData, 
					Byte %ChecksumSize,
					UInt16 TimeOut);

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
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_DiagnosticService")]
				static TCCPResult DiagnosticService(
					TCCPHandle CcpHandle, 
					UInt16 DiagnosticNumber,
					[MarshalAs(UnmanagedType::LPArray, SizeParamIndex = 3)]
					array<Byte>^ Parameters, 
					Byte ParamsLength, 
					Byte %ReturnLength, 
					Byte %ReturnType,
					UInt16 TimeOut);

				/// <summary>
				/// Executes a defined diagnostic procedure and sets the MTA0 pointer to the location 
				/// from where the master can upload the requested information
				/// </summary>
				/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
				/// <param name="DiagnosticNumber">Diagnostic service number</param>
				/// <param name="ReturnLength">Length of the return information (to be uploaded)</param>
				/// <param name="ReturnType">Data type qualifier of the return information</param>
				/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
				/// <returns>A CCPResult result code</returns>
				static TCCPResult DiagnosticService(
					TCCPHandle CcpHandle,
					UInt16 DiagnosticNumber,
					Byte %ReturnLength,
					Byte %ReturnType,
					UInt16 TimeOut)
				{
					return DiagnosticService(CcpHandle, DiagnosticNumber, gcnew array<Byte>(0), 0, ReturnLength, ReturnType, TimeOut);
				}

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
				/// <returns>A CCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_ActionService")]
				static TCCPResult ActionService(
					TCCPHandle CcpHandle, 
					UInt16 ActionNumber,
					[MarshalAs(UnmanagedType::LPArray, SizeParamIndex = 3)]
					array<Byte>^ Parameters, 
					Byte ParamsLength, 
					Byte %ReturnLength, 
					Byte %ReturnType,
					UInt16 TimeOut);

				/// <summary>
				/// Executes a defined action and sets the MTA0 pointer to the location 
				/// from where the master can upload the requested information
				/// </summary>
				/// <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
				/// <param name="ActionNumber">Action service number</param>
				/// <param name="ReturnLength">Length of the return information (to be uploaded)</param>
				/// <param name="ReturnType">Data type qualifier of the return information</param>
				/// <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
				/// <returns>A CCPResult result code</returns>
				static TCCPResult ActionService(
					TCCPHandle CcpHandle,
					UInt16 ActionNumber,
					Byte %ReturnLength,
					Byte %ReturnType,
					UInt16 TimeOut)
				{
					return ActionService(CcpHandle, ActionNumber, gcnew array<Byte>(0), 0, ReturnLength, ReturnType, TimeOut);
				}

				/// <summary>
				/// Returns a descriptive text of a given TCCPResult error code
				/// </summary>
				/// <param name="anError">A TCCPResult result code</param>
				/// <param name="textBuffer">Action service number</param>
				/// <returns>A TCCPResult result code</returns>
				[DllImport("PCCP.dll", EntryPoint = "CCP_GetErrorText")]
				static TCCPResult GetErrorText(
					TCCPResult anError, 
					StringBuilder^ textBuffer);
			};
			#pragma endregion
		}
	}
}
