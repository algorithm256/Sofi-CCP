'///////////////////////////////////////////////////////////////////////////////
'  PCCP.cs
'
'  ~~~~~~~~~~~~
'
'  PCAN-CCP API
'
'  ~~~~~~~~~~~~
'
'  ------------------------------------------------------------------
'  Author : Keneth Wagner
'	Last change: 07.12.2018 Wagner
'
'  Language: C# 1.0
' ------------------------------------------------------------------
'
' Copyright (C) 2018  PEAK-System Technik GmbH, Darmstadt
'  more Info at http://www.peak-system.com 
'///////////////////////////////////////////////////////////////////////////////
Imports System
Imports System.Text
Imports System.Runtime.InteropServices

'///////////////////////////////////////////////////////////
' Inclusion of other needed stuff
'///////////////////////////////////////////////////////////
Imports Peak.Can.Basic
Imports TPCANHandle = System.UInt16
Imports TCCPHandle = System.UInt32

Namespace Peak.Can.Ccp

#Region "Enumerations"
    ''' <summary>
    ''' Represents a PCAN status/error code
    ''' </summary>    
    Public Enum TCCPStartStopMode As Byte
        CCP_SSM_STOP = 0
        CCP_SSM_START = 1
        CCP_SSM_PREPARE_START = 2
    End Enum

    ''' <summary>
    ''' Resource / Protection Mask
    ''' </summary>
    ''' <remarks></remarks>
    <Flags()> _
    Public Enum TCCPResourceMask As Byte
        CCP_RSM_NONE = 0
        CCP_RSM_CALIBRATION = &H1
        CCP_RSM_DATA_ADQUISITION = &H2
        CCP_RSM_MEMORY_PROGRAMMING = &H40
    End Enum

    ''' <summary>
    ''' Result and Error values
    ''' </summary>
    ''' <remarks></remarks>
    Public Enum TCCPResult As Integer
        CCP_ERROR_ACKNOWLEDGE_OK = &H0            '------- CCP CCP Result Level 1
        CCP_ERROR_DAQ_OVERLOAD = &H1
        CCP_ERROR_CMD_PROCESSOR_BUSY = &H10
        CCP_ERROR_DAQ_PROCESSOR_BUSY = &H11
        CCP_ERROR_INTERNAL_TIMEOUT = &H12
        CCP_ERROR_KEY_REQUEST = &H18
        CCP_ERROR_SESSION_STS_REQUEST = &H19
        CCP_ERROR_COLD_START_REQUEST = &H20        '------- CCP Result Level 2
        CCP_ERROR_CAL_DATA_INIT_REQUEST = &H21
        CCP_ERROR_DAQ_LIST_INIT_REQUEST = &H22
        CCP_ERROR_CODE_UPDATE_REQUEST = &H23
        CCP_ERROR_UNKNOWN_COMMAND = &H30           '------- CCP Result Level 3 (Errors)
        CCP_ERROR_COMMAND_SYNTAX = &H31
        CCP_ERROR_PARAM_OUT_OF_RANGE = &H32
        CCP_ERROR_ACCESS_DENIED = &H33
        CCP_ERROR_OVERLOAD = &H34
        CCP_ERROR_ACCESS_LOCKED = &H35
        CCP_ERROR_NOT_AVAILABLE = &H36

        CCP_ERROR_PCAN = &H80000000                 '-------- PCAN-Basic Error FLAG (value AND &H7FFFFFFF)
    End Enum

    ''' <summary>
    ''' Session Status
    ''' </summary>
    ''' <remarks></remarks>
    Public Enum TCCPSessionStatus As Byte
        CCP_STS_CALIBRATING = 1
        CCP_STS_ACQUIRING = 2
        CCP_STS_RESUME_REQUEST = 4
        CCP_STS_STORE_REQUEST = 64
        CCP_STS_RUNNING = 128
    End Enum

    ''' <summary>
    ''' Category of an error TCCPResult
    ''' </summary>
    ''' <remarks></remarks>
    Public Enum TCCPErrorCategory As Byte
        NotDefined = 0
        Warning = 1
        Spurious = 2
        Resolvable = 3
        Unresolvable = 4
    End Enum
#End Region

#Region "Structures"
    ''' <summary>
    ''' Help structure for better handling the return value of 
    ''' the API functions
    ''' </summary>
    <StructLayout(LayoutKind.Auto, Size:=4)> _
    Public Structure CCPResult
        <MarshalAs(UnmanagedType.U4)> _
        Private m_pccpResult As UInteger

        Public Sub New(ByVal result As TCCPResult)
            m_pccpResult = result
        End Sub

        Private Function IsCanError() As Boolean
            Return ((m_pccpResult And TCCPResult.CCP_ERROR_PCAN) = TCCPResult.CCP_ERROR_PCAN)
        End Function

        Public Function GetErrorCategory() As TCCPErrorCategory
            Dim valueTest As Byte

            If IsCanError() Then
                valueTest = 0
            Else
                valueTest = CCP And &HF0
                valueTest = valueTest >> 4
            End If
            Return valueTest
        End Function

        Public ReadOnly Property CCP() As TCCPResult
            Get
                If IsCanError() Then
                    Return TCCPResult.CCP_ERROR_PCAN
                Else
                    Return (m_pccpResult And &HFF)
                End If
            End Get
        End Property

        Public ReadOnly Property PCAN() As TPCANStatus
            Get
                Return (m_pccpResult And &H7FFFFFFF)
            End Get
        End Property

        Public ReadOnly Property ErrorCategory() As TCCPErrorCategory
            Get
                Return GetErrorCategory()
            End Get
        End Property
    End Structure

    ''' <summary>
    ''' CCP_StartStopDataTransmission structure
    ''' </summary>
    <StructLayout(LayoutKind.Sequential)> _
    Public Structure TCCPStartStopData
        <MarshalAs(UnmanagedType.U1)> _
        Public Mode As TCCPStartStopMode
        Public ListNumber As Byte
        Public LastODTNumber As Byte
        Public EventChannel As Byte
        Public TransmissionRatePrescaler As UShort
    End Structure

    ''' <summary>
    ''' CCP_ExchangeId structure
    ''' </summary>
    <StructLayout(LayoutKind.Sequential)> _
    Public Structure TCCPExchangeData
        Public IdLength As Byte
        Public DataType As Byte
        <MarshalAs(UnmanagedType.U1)> _
        Public AvailabilityMask As TCCPResourceMask
        <MarshalAs(UnmanagedType.U1)> _
        Public ProtectionMask As TCCPResourceMask
    End Structure

    ''' <summary>
    ''' Slave Information used within the CCP_Connect/CCP_Test functions
    ''' </summary>
    <StructLayout(LayoutKind.Sequential)> _
    Public Structure TCCPSlaveData
        Public EcuAddress As UShort
        Public IdCRO As UInteger
        Public IdDTO As UInteger
        <MarshalAs(UnmanagedType.U1)> _
        Public IntelFormat As Boolean
    End Structure

    ''' <summary>
    ''' Represents an asynchronous message sent from a slave
    ''' </summary>
    <StructLayout(LayoutKind.Sequential)> _
    Public Structure TCCPMsg
        Public Source As TCCPHandle
        Public Length As Byte
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=8)> _
        Public Data As Byte()
    End Structure
#End Region

#Region "PCAN-CCP Class"
    ''' <summary>
    ''' PCAN-CCP API class implementation
    ''' </summary>
    Public NotInheritable Class CCPApi
        ''' <summary>
        ''' Maximum count of asynchronous messages that the queue can retain
        ''' </summary>
        Public Const CCP_MAX_RCV_QUEUE As Integer = &H7FFF

        ''' <summary>
        ''' Initializes the CAN communication using PCAN-Basic API
        ''' </summary>
        ''' <param name="Channel">The handle of a PCAN Channel</param>
        ''' <param name="Btr0Btr1">The speed for the communication (BTR0BTR1 code)</param>
        ''' <param name="HwType">NON PLUG-and-PLAY: The type of hardware and operation mode</param>
        ''' <param name="IOPort">NON PLUG-and-PLAY: The I/O address for the parallel port</param>
        ''' <param name="Interrupt">NON PLUG-and-PLAY: Interrupt number of the parallel port</param>
        ''' <returns>A CCPResult result code</returns>
        <DllImport("PCCP.dll", EntryPoint:="CCP_InitializeChannel")> _
        Public Shared Function InitializeChannel( _
            <MarshalAs(UnmanagedType.U2)> _
            ByVal Channel As TPCANHandle, _
            <MarshalAs(UnmanagedType.U2)> _
            ByVal Btr0Btr1 As TPCANBaudrate, _
            <MarshalAs(UnmanagedType.U1)> _
            ByVal HwType As TPCANType, _
            ByVal IOPort As UInt32, _
            ByVal Interrupt As UInt16) As TCCPResult
        End Function

        ''' <summary>
        ''' Initializes the CAN communication using PCAN-Basic API
        ''' </summary>
        ''' <param name="Channel">The handle of a PCAN Channel</param>
        ''' <param name="Btr0Btr1">The speed for the communication (BTR0BTR1 code)</param>
        ''' <returns>A CCPResult result code</returns>
        Public Shared Function InitializeChannel( _
            ByVal Channel As TPCANHandle, _
            ByVal Btr0Btr1 As TPCANBaudrate) As TCCPResult
            Return InitializeChannel(Channel, Btr0Btr1, 0, 0, 0)
        End Function

        ''' <summary>
        ''' Uninitializes a PCAN Channels initialized by CCP_InitializeChannel
        ''' </summary>
        ''' <param name="Channel">The handle of a PCAN Channel</param>
        ''' <returns>A CCPResult result code</returns>
        <DllImport("PCCP.dll", EntryPoint:="CCP_UninitializeChannel")> _
        Public Shared Function UninitializeChannel( _
            <MarshalAs(UnmanagedType.U2)> _
            ByVal Channel As TPCANHandle) As TCCPResult
        End Function

        ''' <summary>
        ''' Reads a message from the receive queue of a PCAN-CCP connection.
        ''' </summary>
        ''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="MessageBuffer">Buffer for a message. See 'TCCPMsg' above</param>
        ''' <returns>A CCPResult result code</returns>
        <DllImport("PCCP.dll", EntryPoint:="CCP_ReadMsg")> _
        Public Shared Function ReadMsg( _
            ByVal CcpHandle As TCCPHandle, _
            ByRef MessageBuffer As TCCPMsg) As TCCPResult
        End Function

        ''' <summary>
        ''' Resets the receive-queue of a PCAN-CCP connection.
        ''' </summary>
        ''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
        ''' <returns>A CCPResult result code</returns>
        <DllImport("PCCP.dll", EntryPoint:="CCP_Reset")> _
        Public Shared Function Reset( _
            ByVal CcpHandle As TCCPHandle) As TCCPResult
        End Function

        ''' <summary>
        ''' Establishes a logical connection between a master applicaiton and a slave unit 
        ''' </summary>
        ''' <param name="Channel">The handle of an initialized PCAN Channel</param>
        ''' <param name="SlaveData">The description data of the slave to be connected</param>
        ''' <param name="CcpHandle">A buffer to return the handle of this Master/Channel/Slave connection</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        <DllImport("PCCP.dll", EntryPoint:="CCP_Connect")> _
        Public Shared Function Connect( _
            <MarshalAs(UnmanagedType.U2)> _
            ByVal Channel As TPCANHandle, _
            ByRef SlaveData As TCCPSlaveData, _
            ByRef CcpHandle As TCCPHandle, _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Logically disconnects a master application from a slave unit
        ''' </summary>
        ''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="Temporary">Indicates if the disconnection should be temporary or not</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        <DllImport("PCCP.dll", EntryPoint:="CCP_Disconnect")> _
        Public Shared Function Disconnect( _
            ByVal CcpHandle As TCCPHandle, _
            ByVal Temporary As Boolean, _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Tests if a slave is available
        ''' </summary>
        ''' <param name="Channel">The handle of an initialized PCAN Channel</param>
        ''' <param name="SlaveData">The data of the slave to be checked for availability</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        <DllImport("PCCP.dll", EntryPoint:="CCP_Test")> _
        Public Shared Function Test( _
            <MarshalAs(UnmanagedType.U2)> _
            ByVal Channel As TPCANHandle, _
            ByRef SlaveData As TCCPSlaveData, _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Exchanges the CCP Version used by a master and a slave
        ''' </summary>
        ''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="Main">Buffer for the CCP Main version used</param>
        ''' <param name="Release">Buffer for the CCP Release version used</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        ''' <remarks>Both buffers, Main and Release, are bidirectional, e.g. they are used by both 
        ''' Master and Slave. The master should call this function placing in these ufferrs its used 
        ''' version. After the function returns, these buffers contain the version used by the 
        ''' connected slave</remarks>
        <DllImport("PCCP.dll", EntryPoint:="CCP_GetCcpVersion")> _
        Public Shared Function GetCcpVersion( _
            ByVal CcpHandle As TCCPHandle, _
            <[In](), Out()> _
            ByRef Main As Byte, _
            <[In](), Out()> _
            ByRef Release As Byte, _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Exchanges IDs between Master and Slave for automatic session configuration
        ''' </summary>
        ''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="ECUData">Slave ID and Resource Information buffer</param>
        ''' <param name="MasterData">Optional master data (ID)</param>
        ''' <param name="DataLength">Length of the master data</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        <DllImport("PCCP.dll", EntryPoint:="CCP_ExchangeId")> _
        Public Shared Function ExchangeId( _
            ByVal CcpHandle As TCCPHandle, _
            ByRef ECUData As TCCPExchangeData, _
            <MarshalAs(UnmanagedType.LPArray, SizeParamIndex:=3)> _
            ByVal MasterData As Byte(), _
            ByVal DataLength As Integer, _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Exchanges IDs between Master and Slave for automatic session configuration
        ''' </summary>
        ''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="ECUData">Slave ID and Resource Information buffer</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        Public Shared Function ExchangeId( _
            ByVal CcpHandle As TCCPHandle, _
            ByRef ECUData As TCCPExchangeData, _
            ByVal TimeOut As UShort) As TCCPResult
            Return ExchangeId(CcpHandle, ECUData, Array.CreateInstance(GetType(Byte), 0), 0, TimeOut)
        End Function

        ''' <summary>
        ''' Returns Seed data for a seed and key algorithm to unlock a slave functionality
        ''' </summary>
        ''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="Resource">The resource being asked. See 'Resource / Protection Mask' values above</param>
        ''' <param name="CurrentStatus">Current protection status of the asked resource</param>
        ''' <param name="Seed">Seed value for the seed and key algorithm</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        <DllImport("PCCP.dll", EntryPoint:="CCP_GetSeed")> _
        Public Shared Function GetSeed( _
            ByVal CcpHandle As TCCPHandle, _
            ByVal Resource As Byte, _
            ByRef CurrentStatus As Boolean, _
            <MarshalAs(UnmanagedType.LPArray)> _
            ByVal Seed As Byte(), _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Unlocks the security protection of a resource within a connected slave.
        ''' </summary>
        ''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="KeyBuffer">A buffer with the key computed from a seed value 
        ''' obtained through the CCP_GetSeed function</param>
        ''' <param name="KeyLength">The length in bytes of the key buffer value</param>
        ''' <param name="Privileges">The current privileges status on the slave. 
        ''' See 'Resource / Protection Mask' values above</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        <DllImport("PCCP.dll", EntryPoint:="CCP_Unlock")> _
        Public Shared Function Unlock( _
            ByVal CcpHandle As TCCPHandle, _
            <MarshalAs(UnmanagedType.LPArray, SizeParamIndex:=2)> _
            ByVal KeyBuffer As Byte(), _
            ByVal KeyLength As Byte, _
            <MarshalAs(UnmanagedType.U1)> _
            ByRef Privileges As TCCPResourceMask, _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Keeps the connected slave informed about the current state of the calibration session
        ''' </summary>
        ''' <param name="Ccphandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="Status">Current status bits. See 'Session Status' values above</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        <DllImport("PCCP.dll", EntryPoint:="CCP_SetSessionStatus")> _
        Public Shared Function SetSessionStatus( _
            ByVal Ccphandle As TCCPHandle, _
            <MarshalAs(UnmanagedType.U1)> _
            ByVal Status As TCCPSessionStatus, _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Retrieves the information about the current state of the calibration session
        ''' </summary>
        ''' <param name="Ccphandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="Status">Current status bits. See 'Session Status' values above</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        <DllImport("PCCP.dll", EntryPoint:="CCP_GetSessionStatus")> _
        Public Shared Function GetSessionStatus( _
            ByVal Ccphandle As TCCPHandle, _
            <MarshalAs(UnmanagedType.U1)> _
            ByRef Status As TCCPSessionStatus, _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Initializes a base pointer for all following memory transfers.
        ''' </summary>
        ''' <param name="Ccphandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="UsedMTA">Memory Transfer Address (MTA) number (0 or 1)</param>
        ''' <param name="AddrExtension">Address extension</param>
        ''' <param name="Addr">Address</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        <DllImport("PCCP.dll", EntryPoint:="CCP_SetMemoryTransferAddress")> _
        Public Shared Function SetMemoryTransferAddress( _
            ByVal Ccphandle As TCCPHandle, _
            ByVal UsedMTA As Byte, _
            ByVal AddrExtension As Byte, _
            ByVal Addr As UInteger, _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Copies a block of data into memory, starting at the current MTA0.
        ''' </summary>
        ''' <param name="Ccphandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="DataBytes">Buffer with the data to be transferred</param>
        ''' <param name="Size">Size of the data to be transferred, in bytes</param>
        ''' <param name="MTA0Ext">MTA0 extension after post-increment</param>
        ''' <param name="MTA0Addr">MTA0 Address after post-increment</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        ''' <remarks>MTA0 is post-incremented by the value of 'Size'</remarks>
        <DllImport("PCCP.dll", EntryPoint:="CCP_Download")> _
        Public Shared Function Download( _
            ByVal Ccphandle As TCCPHandle, _
            <MarshalAs(UnmanagedType.LPArray, SizeParamIndex:=2)> _
            ByVal DataBytes As Byte(), _
            ByVal Size As Byte, _
            ByRef MTA0Ext As Byte, _
            ByRef MTA0Addr As UInteger, _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Copies a block of 6 data bytes into memory, starting at the current MTA0. 
        ''' </summary>
        ''' <param name="Ccphandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="DataBytes">Buffer with the data to be transferred</param>
        ''' <param name="MTA0Ext">MTA0 extension after post-increment</param>
        ''' <param name="MTA0Addr">MTA0 Address after post-increment</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        ''' <remarks>MTA0 is post-incremented by the value of 6</remarks>
        <DllImport("PCCP.dll", EntryPoint:="CCP_Download_6")> _
        Public Shared Function Download_6( _
            ByVal Ccphandle As TCCPHandle, _
            <MarshalAs(UnmanagedType.LPArray, SizeConst:=6)> _
            ByVal DataBytes As Byte(), _
            ByRef MTA0Ext As Byte, _
            ByRef MTA0Addr As UInteger, _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Retrieves a block of data starting at the current MTA0.
        ''' </summary>
        ''' <param name="Ccphandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="Size">Size of the data to be retrieved, in bytes</param>
        ''' <param name="DataBytes">Buffer for the requested data</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        ''' <remarks>MTA0 will be post-incremented with the value of size</remarks>
        <DllImport("PCCP.dll", EntryPoint:="CCP_Upload")> _
        Public Shared Function Upload( _
            ByVal Ccphandle As TCCPHandle, _
            ByVal Size As Byte, _
            <MarshalAs(UnmanagedType.LPArray, SizeParamIndex:=1)> _
            ByVal DataBytes As Byte(), _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Retrieves a block of data. 
        ''' </summary>
        ''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="UploadSize">Size of the data to be retrieved, in bytes</param>
        ''' <param name="MTA0Ext">MTA0 extension for the upload</param>
        ''' <param name="MTA0Addr">MTA0 Address for the upload</param>
        ''' <param name="reqData">Buffer for the requested data</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        ''' <remarks>The amount of data is retrieved from the given address. The MTA0 
        ''' pointer \"remains unchanged\"</remarks>
        <DllImport("PCCP.dll", EntryPoint:="CCP_ShortUpload")> _
        Public Shared Function ShortUpload( _
            ByVal CcpHandle As TCCPHandle, _
            ByVal UploadSize As Byte, _
            ByVal MTA0Ext As Byte, _
            ByVal MTA0Addr As UInteger, _
            <MarshalAs(UnmanagedType.LPArray, SizeParamIndex:=1)> _
            ByVal reqData As Byte(), _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Copies a block of data from the address MTA0 to the address MTA1 
        ''' </summary>
        ''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="SizeOfData">Number of bytes to be moved</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        <DllImport("PCCP.dll", EntryPoint:="CCP_Move")> _
        Public Shared Function Move( _
            ByVal CcpHandle As TCCPHandle, _
            ByVal SizeOfData As UInteger, _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' ECU Implementation dependant. Sets the previously initialized MTA0 as the start of the 
        ''' current active calibration data page
        ''' </summary>
        ''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        <DllImport("PCCP.dll", EntryPoint:="CCP_SelectCalibrationDataPage")> _
        Public Shared Function SelectCalibrationDataPage( _
            ByVal CcpHandle As TCCPHandle, _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Retrieves the start address of the calibration page that is currently active in the slave device
        ''' calibration data page that is selected as the currently active page
        ''' </summary>
        ''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="MTA0Ext">Buffer for the MTAO address extension</param>
        ''' <param name="MTA0Addr">Buffer for the MTA0 address pointer</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        <DllImport("PCCP.dll", EntryPoint:="CCP_GetActiveCalibrationPage")> _
        Public Shared Function GetActiveCalibrationPage( _
            ByVal CcpHandle As TCCPHandle, _
            ByRef MTA0Ext As Byte, _
            ByRef MTA0Addr As UInteger, _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Retrieves the size of the specified DAQ List as the number of available Object
        ''' Descriptor Tables (ODTs) and clears the current list.
        ''' Optionally, sets a dedicated CAN-ID for the DAQ list.
        ''' </summary>
        ''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="ListNumber">DAQ List number</param>
        ''' <param name="DTOId">CAN identifier of DTO dedicated to the given 'ListNumber'</param>
        ''' <param name="Size">Buffer for the list size (Number of ODTs in the DAQ list)</param>
        ''' <param name="FirstPID">Buffer for the first PID of the DAQ list</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        <DllImport("PCCP.dll", EntryPoint:="CCP_GetDAQListSize")> _
        Public Shared Function GetDAQListSize( _
            ByVal CcpHandle As TCCPHandle, _
            ByVal ListNumber As Byte, _
            ByRef DTOId As UInteger, _
            ByRef Size As Byte, _
            ByRef FirstPID As Byte, _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        <DllImport("PCCP.dll", EntryPoint:="CCP_GetDAQListSize")> _
        Private Shared Function GetDAQListSize( _
            ByVal CcpHandle As TCCPHandle, _
            ByVal ListNumber As Byte, _
            ByVal ptr As IntPtr, _
            ByRef Size As Byte, _
            ByRef FirstPID As Byte, _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Retrieves the size of the specified DAQ List as the number of available Object
        ''' Descriptor Tables (ODTs) and clears the current list.
        ''' Optionally, sets a dedicated CAN-ID for the DAQ list.
        ''' </summary>
        ''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="ListNumber">DAQ List number</param>
        ''' <param name="Size">Buffer for the list size (Number of ODTs in the DAQ list)</param>
        ''' <param name="FirstPID">Buffer for the first PID of the DAQ list</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        Public Shared Function GetDAQListSize( _
            ByVal CcpHandle As TCCPHandle, _
            ByVal ListNumber As Byte, _
            ByRef Size As Byte, _
            ByRef FirstPID As Byte, _
            ByVal TimeOut As UShort) As TCCPResult
            Return GetDAQListSize(CcpHandle, ListNumber, IntPtr.Zero, Size, FirstPID, TimeOut)
        End Function

        ''' <summary>
        ''' Initializes the DAQ List pointer for a subsequent write to a DAQ list.
        ''' </summary>
        ''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="ListNumber">DAQ List number</param>
        ''' <param name="ODTNumber">Object Descriptor Table number</param>
        ''' <param name="ElementNumber">Element number within the ODT</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        <DllImport("PCCP.dll", EntryPoint:="CCP_SetDAQListPointer")> _
        Public Shared Function SetDAQListPointer( _
            ByVal CcpHandle As TCCPHandle, _
            ByVal ListNumber As Byte, _
            ByVal ODTNumber As Byte, _
            ByVal ElementNumber As Byte, _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Writes one entry (DAQ element) to a DAQ list defined by the DAQ list pointer.
        ''' </summary>
        ''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="SizeElement">Size of the DAQ elements in bytes {1, 2, 4}</param>
        ''' <param name="AddrExtension">Address extension of DAQ element</param>
        ''' <param name="AddrDAQ">Address of a DAQ element</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        <DllImport("PCCP.dll", EntryPoint:="CCP_WriteDAQListEntry")> _
        Public Shared Function WriteDAQListEntry( _
            ByVal CcpHandle As TCCPHandle, _
            ByVal SizeElement As Byte, _
            ByVal AddrExtension As Byte, _
            ByVal AddrDAQ As UInteger, _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Starts/Stops the data acquisition and/or prepares a synchronized start of the 
        ''' specified DAQ list
        ''' </summary>
        ''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="Data">Contains the data to be applied within the start/stop procedure. 
        ''' See 'TCCPStartStopData' structure above</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        <DllImport("PCCP.dll", EntryPoint:="CCP_StartStopDataTransmission")> _
        Public Shared Function StartStopDataTransmission( _
            ByVal CcpHandle As TCCPHandle, _
            ByRef Data As TCCPStartStopData, _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Starts/Stops the periodic transmission of all DAQ lists. 
        ''' </summary>
        ''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="StartOrStop">True: Starts the configured DAQ lists. False: Stops all DAQ lists</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        ''' <remarks>Starts all DAQs configured as "prepare to start" with a previously 
        ''' CCP_StartStopDataTransmission call. Stops all DAQs, included those not started 
        ''' synchronized</remarks>
        <DllImport("PCCP.dll", EntryPoint:="CCP_StartStopSynchronizedDataTransmission")> _
        Public Shared Function StartStopSynchronizedDataTransmission( _
            ByVal CcpHandle As TCCPHandle, _
            ByVal StartOrStop As Boolean, _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Erases non-volatile memory (FLASH EPROM) prior to reprogramming.
        ''' </summary>
        ''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="MemorySize">Memory size in bytes to be erased</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        ''' <remarks>The MTA0 pointer points to the memory location to be erased</remarks>
        <DllImport("PCCP.dll", EntryPoint:="CCP_ClearMemory")> _
        Public Shared Function ClearMemory( _
            ByVal CcpHandle As TCCPHandle, _
            ByVal MemorySize As UInteger, _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Programms a block of data into non-volatile (FLASH, EPROM) memory, starting 
        ''' at the current MTA0.
        ''' </summary>
        ''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="Data">Buffer with the data to be programmed</param>
        ''' <param name="Size">Size of the 'Data' block to be programmed</param>
        ''' <param name="MTA0Ext">MTA0 extension after post-increment</param>
        ''' <param name="MTA0Addr">MTA0 Address after post-increment</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        ''' <remarks>The MTA0 pointer is post-incremented by the value of 'Size'</remarks>
        <DllImport("PCCP.dll", EntryPoint:="CCP_Program")> _
        Public Shared Function Program( _
            ByVal CcpHandle As TCCPHandle, _
            <MarshalAs(UnmanagedType.LPArray, SizeParamIndex:=2)> _
            ByVal Data As Byte(), _
            ByVal Size As Byte, _
            ByRef MTA0Ext As Byte, _
            ByRef MTA0Addr As UInteger, _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Programms a block of 6 data bytes into non-volatile (FLASH, EPROM) memory, 
        ''' starting at the current MTA0.
        ''' </summary>
        ''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="Data">Buffer with the data to be programmed</param>
        ''' <param name="MTA0Ext">MTA0 extension after post-increment</param>
        ''' <param name="MTA0Addr">MTA0 Address after post-increment</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        ''' <remarks>The MTA0 pointer is post-incremented by 6</remarks>
        <DllImport("PCCP.dll", EntryPoint:="CCP_Program_6")> _
        Public Shared Function Program_6( _
            ByVal CcpHandle As TCCPHandle, _
            <MarshalAs(UnmanagedType.LPArray, SizeConst:=6)> _
            ByVal Data As Byte(), _
            ByRef MTA0Ext As Byte, _
            ByRef MTA0Addr As UInteger, _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Calculates a checksum result of the memory block that is defined by MTA0 
        ''' (Memory Transfer Area Start address) and 'BlockSize'.
        ''' </summary>
        ''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="BlockSize">Block size in bytes</param>
        ''' <param name="ChecksumData">Checksum data (implementation specific)</param>
        ''' <param name="checksumSize">Size of the checksum data</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        <DllImport("PCCP.dll", EntryPoint:="CCP_BuildChecksum")> _
        Public Shared Function BuildChecksum( _
            ByVal CcpHandle As TCCPHandle, _
            ByVal BlockSize As UInteger, _
            <MarshalAs(UnmanagedType.LPArray, SizeParamIndex:=3)> _
            ByVal ChecksumData As Byte(), _
            ByRef checksumSize As Byte, _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Executes a defined diagnostic procedure and sets the MTA0 pointer to the location 
        ''' from where the master can upload the requested information
        ''' </summary>
        ''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="DiagnosticNumber">Diagnostic service number</param>
        ''' <param name="Parameters">Parameters, if applicable</param>
        ''' <param name="ParamsLength">Length in bytes of the parameters passed within 'Parameters'</param>
        ''' <param name="ReturnLength">Length of the return information (to be uploaded)</param>
        ''' <param name="ReturnType">Data type qualifier of the return information</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        <DllImport("PCCP.dll", EntryPoint:="CCP_DiagnosticService")> _
        Public Shared Function DiagnosticService( _
            ByVal CcpHandle As TCCPHandle, _
            ByVal DiagnosticNumber As UShort, _
            <MarshalAs(UnmanagedType.LPArray, SizeParamIndex:=3)> _
            ByVal Parameters As Byte(), _
            ByVal ParamsLength As Byte, _
            ByRef ReturnLength As Byte, _
            ByRef ReturnType As Byte, _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Executes a defined diagnostic procedure and sets the MTA0 pointer to the location 
        ''' from where the master can upload the requested information
        ''' </summary>
        ''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="DiagnosticNumber">Diagnostic service number</param>
        ''' <param name="ReturnLength">Length of the return information (to be uploaded)</param>
        ''' <param name="ReturnType">Data type qualifier of the return information</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        Public Shared Function DiagnosticService( _
            ByVal CcpHandle As TCCPHandle, _
            ByVal DiagnosticNumber As UShort, _
            ByRef ReturnLength As Byte, _
            ByRef ReturnType As Byte, _
            ByVal TimeOut As UShort) As TCCPResult
            Return DiagnosticService(CcpHandle, DiagnosticNumber, Array.CreateInstance(GetType(Byte), 0), 0, ReturnLength, ReturnType, TimeOut)
        End Function

        ''' <summary>
        ''' Executes a defined action and sets the MTA0 pointer to the location 
        ''' from where the master can upload the requested information
        ''' </summary>
        ''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="ActionNumber">Action service number</param>
        ''' <param name="Parameters">Parameters, if applicable</param>
        ''' <param name="ParamsLength">Length in bytes of the parameters passed within 'Parameters'</param>
        ''' <param name="ReturnLength">Length of the return information (to be uploaded)</param>
        ''' <param name="ReturnType">Data type qualifier of the return information</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        <DllImport("PCCP.dll", EntryPoint:="CCP_ActionService")> _
        Public Shared Function ActionService( _
            ByVal CcpHandle As TCCPHandle, _
            ByVal ActionNumber As UShort, _
            <MarshalAs(UnmanagedType.LPArray, SizeParamIndex:=3)> _
            ByVal Parameters As Byte(), _
            ByVal ParamsLength As Byte, _
            ByRef ReturnLength As Byte, _
            ByRef ReturnType As Byte, _
            ByVal TimeOut As UShort) As TCCPResult
        End Function

        ''' <summary>
        ''' Executes a defined action and sets the MTA0 pointer to the location 
        ''' from where the master can upload the requested information
        ''' </summary>
        ''' <param name="CcpHandle">The handle of a PCAN-CCP connection</param>
        ''' <param name="ActionNumber">Action service number</param>
        ''' <param name="ReturnLength">Length of the return information (to be uploaded)</param>
        ''' <param name="ReturnType">Data type qualifier of the return information</param>
        ''' <param name="TimeOut">Wait time (millis) for ECU response. Zero(0) to use the default time</param>
        ''' <returns>A CCPResult result code</returns>
        Public Shared Function ActionService( _
            ByVal CcpHandle As TCCPHandle, _
            ByVal ActionNumber As UShort, _
            ByRef ReturnLength As Byte, _
            ByRef ReturnType As Byte, _
            ByVal TimeOut As UShort) As TCCPResult
            Return ActionService(CcpHandle, ActionNumber, Array.CreateInstance(GetType(Byte), 0), 0, ReturnLength, ReturnType, TimeOut)
        End Function

        ''' <summary>
        ''' Returns a descriptive text of a given TCCPResult error code
        ''' </summary>
        ''' <param name="anError">A TCCPResult error code</param>
        ''' <param name="StringBuffer">Buffer for the text (must be at least 256 in length)</param>
        ''' <returns>A TCCPResult error code</returns>
        <DllImport("PCCP.dll", EntryPoint:="CCP_GetErrorText")> _
        Public Shared Function GetErrorText( _
            ByVal anError As TCCPResult, _
            ByVal StringBuffer As StringBuilder) As TCCPResult
        End Function
    End Class
#End Region
End Namespace