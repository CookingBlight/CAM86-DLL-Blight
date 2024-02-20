// --------------------------------------------------------------------------------
// ASCOM Camera driver low-level interaction library for cam86 v.0.9.7
// Edit Log:
// Date Who Vers Description
// ----------- --- ----- ---------------------------------------------------------
// 28-aug-2016 VSS 0.1 Initial release (code obtained from grim)
//
// 1-feb-2017 Luka Pravica 0.2L
//     - Update code to match the newer cam86_view
//     - Add function to set camera reading time
//     - if camera is not idle cameraGetCoolerPower, cameraGetCoolerOn and
//       cameraGetTemp return cashed values to prevent image corruption while
//       downloading images
// 3-feb-2017 Luka Pravica 0.3L
//     - Add function to set if TEC cooling should be on or off during frame reading
//     - Add function to read firmware version
//     - Add function to read version of this DLL
// 19-feb-2017 Luka Pravica 0.4L
//     - Add bias-before-exposure option
// 2-mar-2017 Luka Pravica 0.5L
//     - Add humidity functions by Gilmanov Rim
//     - Add controls for maximum and starting TEC power
// 8-mar-2017 Luka Pravica 0.6L
//     - Fix bug where timer stops working if time is greater than 999s
//       by using multiple restarts of the timer for every 900s
// 11-mar-2017 Luka Pravica 0.7L
//     - Fix bug in long exposures (over 900s)
//     - Add code to debug to file (must be disabled in production version)
// 17-mar-2017 Luka Pravica 0.8L
//     - Add code to set the KP proportional gain
// 20-mar-2017 Luka Pravica 0.9L
//     - Add code to read Cooler min, cooler max and Pk
// 21-mar-2017 Luka Pravica 0.9.1L
//     - Fix bugs in Pk reading
// 27-Sep-2017 Luka Pravica 0.9.2L
//     - Add caching of setCCDtemperature Get/Set and CoolerOn Get/Set during frame reads to prevent the white line bug
// 10-Nov-2017 Oscar Casanova 0.9.3
//     - Kp value sent is multiplied by 100 instead of 1000 to be compatible with new proportional control
// 24-Nov-2017 Oscar Casanova 0.9.4
//     - Add camera[Set|Get]PIDProportionalGain
//           camera[Set|Get]PIDIntegralGain
//           camera[Set|Get]PIDDerivativeGain
//       to be compatible with full PID implementation
// 03-Feb-2018 Luka Pravica 0.9.5
//     - Improve debugging outputs
//     - Add delays after some commands to fix problems where commands are being sent too quickly to ATMega and getting lost
// 17-Feb-2018 Luka Pravica 0.9.6
//     - Remove delays, move them to the main driver
// 21-Jan-2023 Tommy Ramberg 0.9.7
//     - Converted driver to C++. This also requires an update to the ASCOM driver as type definitions are not the same.
//     - This makes this dll incompatible with earlier ASCOM drivers and software.
//     - Calling convention is cdecl with ANSI charset.
//     - Changed exposure timer to timer queues.
//
// --------------------------------------------------------------------------------

/*  Copyright © 2017 Gilmanov Rim, Vakulenko Sergiyand Luka Pravica, Tommy Ramberg

This program is free software : you can redistribute itand /or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.If not, see < http://www.gnu.org/licenses/>.
*/

// Debug logging will generate a log file in the same folder as the application using the driver.
// #define DEBUG

#pragma once
#include <chrono>
#include <thread>
#include "ftd2xx.h" //Current version in use: FTDI D2XX - V2.12.36.4
#include "cam86ll-api.h"
#include <atomic>
#ifdef DEBUG
#include "debug_logger.h"
#endif // DEBUG

#ifdef DEBUG
Debug_Logger debug_log("CAM86_Logfile.txt", true, 6, 8, 2);
#endif // DEBUG


const WORD kSoftware_LLDriver_Version = 97;

FT_HANDLE ft_handle_a;
FT_HANDLE ft_handle_b;
DWORD ft_q_bytes; // This is not used in the original driver.
WORD ft_in_buffer[0x1000000];
BYTE ft_out_buffer[0x10000];
ULONG ft_handle_b_baud = 20000;
std::atomic_bool error_write_flag = false;
std::atomic_bool error_read_flag = false;

BYTE firmware_version_cache = 0;

const BYTE kPort_Initial = 0x11; // SL High, RES High, The rest is low.
BYTE si_in[4]; // Defined as array[0..3] in the original driver.
WORD si_out;

std::atomic_int camera_state = 0;
std::atomic_bool is_connected = false;
std::atomic_bool sensor_clear = false;
std::atomic_bool image_ready = false;
const int kCamera_Width = 3000;
const int kCamera_Height = 2000;
int image_buffer[kCamera_Width * kCamera_Height];
int kol_byte;
std::atomic_int m_bin; // Binning flag.
std::atomic_ushort m_delta_y; // Number of Y lines to read.
std::atomic_ushort m_yn; // Start of Y image data.

HANDLE exposure_timer = NULL;
DWORD exposure_exp;

std::atomic_bool cooler_on_dirty = false;
std::atomic_ushort cooler_on_cache = false;
double cooler_power_cache = 0;
std::atomic_bool target_temp_dirty = false;
double target_temp_cache = 0; // Should be protected!
double sensor_temp_cache = 0;
const WORD kTemperature_Offset = 1280;
const double kMin_Error_Temperature = -120.0;
const double kMax_Error_Temperature = 120.0;
const WORD kTrue_Inv_Prot = 0xaa55;
const WORD kFalse_Inv_Prot = 0x55aa;
const WORD kHigh_Mask_Prot = 0xaa00;
double temp_dht_cache = -128.0;
double humidity_dht_cache = -1;
int cooling_start_power_percentage_cache = -1;
int cooling_maximum_power_percentage_cache = 101;
double kp_cache = 0.0;
double ki_cache = 0.0;
double kd_cache = 0.0;

// Spi commands.
enum {
	kCommand_Read_Frame = 0x1b,
	kCommand_Shift_3 = 0x2b,
	kCommand_Off_15V = 0x3b,
	kCommand_Set_ROI_Start_Y = 0x4b,
	kCommand_Set_ROI_Num_Y = 0x5b,
	kCommand_Set_Exposure = 0x6b,
	kCommand_Set_Binning = 0x8b,
	kCommand_On_Off_Cooler = 0x9b,
	kCommand_Set_Target_Temp = 0xab,
	kCommand_Clear_Frame = 0xcb,
	kCommand_Init_MCU = 0xdb,
	kCommand_Set_Delay_Per_Line = 0xeb,
	kCommand_Set_Cooler_On_During_Read = 0xfb,
	kCommand_Set_Cooler_Power_Start = 0x0a,
	kCommand_Set_Cooler_Power_Max = 0x1a,
	kCommand_Set_PID_KP = 0x2a,
	kCommand_Set_PID_KI = 0x3a,
	kCommand_Set_PID_KD = 0x4a,
	kCommand_Get_Case_Temp = 0xf1,
	kCommand_Get_Case_Humidity = 0xf2,
	kCommand_Get_CCD_Temp = 0xbf,
	kCommand_Get_Target_Temp = 0xbe,
	kCommand_Get_Cooler_Status = 0xbd,
	kCommand_Get_Cooler_Power = 0xbc,
	kCommand_Get_Version = 0xbb,
	kCommand_Get_Cooler_Power_Start = 0xba,
	kCommand_Get_Cooler_Power_Max = 0xb9,
	kCommand_Get_PID_KP_LW = 0xb8,
	kCommand_Get_PID_KP_HW = 0xb7,
	kCommand_Get_PID_KI_LW = 0xb6,
	kCommand_Get_PID_KI_HW = 0xb5,
	kCommand_Get_PID_KD_LW = 0xb4,
	kCommand_Get_PID_KD_HW = 0xb3,
	kCommand_Test_Interrupt_Start = 0xaa,
	kCommand_Test_Interrupt_Fetch = 0xab
};

// Functions normal to Delphi.
char LowByte(WORD shortVar);
char HighByte(WORD shortVar);
WORD SwapBytes(WORD shortVar);

int Qbuf(); // This function is not used in the original driver.
void SSPI(); // Synchronous Serial Protocol In. Sends data to the mcu.
void SSPO(); // Synchronous Serial Protocol Out. Reads data from the mcu.
void SpiComm(BYTE comm, WORD param); // Send mcu command.
void ReadThread(); // This reads the image data in its own thread.
void AD9826(char adr, WORD val); //AD9826 communication. Offset, gain, channels and setup.
void ReadFrame();
void CameraSensorClearFull();
void CALLBACK ExposureTimer(PVOID lpParam, BOOLEAN TimerOrWaitFired);