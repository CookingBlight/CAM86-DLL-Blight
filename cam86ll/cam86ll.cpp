#include "cam86ll.h"

char LowByte(WORD var) {
	BYTE* p_low_byte = (BYTE*)&var;
	return *p_low_byte;
}

char HighByte(WORD var) {
	BYTE* p_high_byte = (BYTE*)&var + 1;
	return *p_high_byte;
}

WORD SwapBytes(WORD var) {
	BYTE low_byte = LowByte(var);
	BYTE high_byte = HighByte(var);
	WORD result = 0;
	BYTE* p_low_byte = (BYTE*)&result;
	BYTE* p_high_byte = (BYTE*)&result + 1;
	*p_low_byte = high_byte;
	*p_high_byte = low_byte;
	return result;
}

int Qbuf() {
	FT_STATUS ft_status = FT_OK;
	
	ft_status = FT_GetQueueStatus(ft_handle_a, &ft_q_bytes);
	if (ft_status != FT_OK) {

	}

	int result = ft_q_bytes;
	return result;
}

void SSPI() {
	int b;
	DWORD n = 100;
	FT_STATUS ft_status = FT_OK;

	memset(ft_out_buffer, kPort_Initial, n);

	for (int j = 0; j <= 2; j++) {
		b = si_in[j];
		for (int i = 0; i <= 7; i++) {
			ft_out_buffer[2 * i + 1 + 16 * j] += 0x20;
			if ((b & 0x80) == 0x80) {
				ft_out_buffer[2 * i + 16 * j] += 0x80;
				ft_out_buffer[2 * i + 1 + 16 * j] += 0x80;
			}
			b = b * 2;
		}
	}
	if (!error_write_flag) {
		DWORD write_result;
		ft_status = FT_Write(ft_handle_b, ft_out_buffer, n, &write_result);
		if (ft_status != FT_OK) {
#ifdef DEBUG
			debug_log.Add_Code("SSPI - ft_status", ft_status, Debug_Logger::kLevel_1, Debug_Logger::kNew_Line_Disable);
#endif
			error_write_flag = true;
		}
		if (write_result != n) {
#ifdef DEBUG
			debug_log.Add_Code("SSPI - write_result", write_result, Debug_Logger::kLevel_1, Debug_Logger::kNew_Line_Disable);
#endif
			error_write_flag = true;
		}
	}
	return;
}

void SSPO() {
	WORD b;
	WORD n = 100;
	DWORD byte_count = 0;
	DWORD byte_expected = n;
	FT_STATUS ft_status = FT_OK;

	if (!error_write_flag) {
		ft_status = FT_Read(ft_handle_b, ft_in_buffer, byte_expected, &byte_count);
		if (ft_status != FT_OK) {
#ifdef DEBUG
			debug_log.Add_Code("SSPO - ft_status", ft_status, Debug_Logger::kLevel_1, Debug_Logger::kNew_Line_Disable);
#endif
			error_read_flag = true;
		}
	}

	if (byte_count != byte_expected) {
#ifdef DEBUG
		debug_log.Add_Code("SSPO - byte_count", byte_count, Debug_Logger::kLevel_1, Debug_Logger::kNew_Line_Disable);
#endif
		error_read_flag = true;
	}

	b = 0;
	for (int i = 0; i <= 15; i++) {
		b = b * 2;
		if ((ft_in_buffer[i + 1 + 8] & 0x40) != 0) {
			b++;
		}
	}

	si_out = b;
	return;
}

void SpiComm(BYTE comm, WORD param) {
	FT_STATUS ft_status = FT_OK;

	ft_status = FT_Purge(ft_handle_b, FT_PURGE_TX);
	if (ft_status != FT_OK) {
#ifdef DEBUG
		debug_log.Add_Code("SpiComm - FT_Purge Out", ft_status, Debug_Logger::kLevel_1, Debug_Logger::kNew_Line_Disable);
#endif
	}

	ft_status = FT_Purge(ft_handle_b, FT_PURGE_RX);
	if (ft_status != FT_OK) {
#ifdef DEBUG
		debug_log.Add_Code("SpiComm - FT_Purge In", ft_status, Debug_Logger::kLevel_1, Debug_Logger::kNew_Line_Disable);
#endif
	}

	si_in[0] = comm;
	si_in[1] = HighByte(param);
	si_in[2] = LowByte(param);
	SSPI();
	SSPO();
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	return;
}

void ReadThread() {
	DWORD byte_count = 0;
	DWORD bytes_to_read = kol_byte;
	FT_STATUS ft_status = FT_OK;

	if (!error_write_flag) {
		ft_status = FT_Read(ft_handle_a, ft_in_buffer, bytes_to_read, &byte_count);
		if (ft_status != FT_OK) {
			error_read_flag = true;
		}
	}
	if (byte_count != bytes_to_read) {
		error_read_flag = true;
		if (!error_write_flag) {

			ft_status = FT_Purge(ft_handle_a, FT_PURGE_TX);
			if (ft_status != FT_OK) {
				
			}

			ft_status = FT_Purge(ft_handle_a, FT_PURGE_RX);
			if (ft_status != FT_OK) {
				
			}
		}
	}
	else {
		if (m_bin == 0) {
			for (int y = 0; y <= (m_delta_y-1); y++) { // Was y <= m_delta_y - 1.
				for (int x = 0; x <= 1499; x++) {
					image_buffer[2 * x + 0 + (2 * (y + m_yn) + 0) * 3000] = SwapBytes(ft_in_buffer[4 * x + 4 + y * 6004]);
					image_buffer[2 * x + 0 + (2 * (y + m_yn) + 1) * 3000] = SwapBytes(ft_in_buffer[4 * x + 5 + y * 6004]);
					image_buffer[2 * x + 1 + (2 * (y + m_yn) + 1) * 3000] = SwapBytes(ft_in_buffer[4 * x + 6 + y * 6004]);
					image_buffer[2 * x + 1 + (2 * (y + m_yn) + 0) * 3000] = SwapBytes(ft_in_buffer[4 * x + 7 + y * 6004]);
				}
			}
		}
		else {
			for (int y = 0; y <= (m_delta_y-1); y++) { // Was y <= m_delta_y - 1.
				for (int x = 0; x <= 1498; x++) {
					image_buffer[2 * x + 0 + (2 * (y + m_yn) + 0) * 3000] = SwapBytes(ft_in_buffer[x + 7 + y * 1504]);
					image_buffer[2 * x + 0 + (2 * (y + m_yn) + 1) * 3000] = SwapBytes(ft_in_buffer[x + 7 + y * 1504]);
					image_buffer[2 * x + 1 + (2 * (y + m_yn) + 1) * 3000] = SwapBytes(ft_in_buffer[x + 7 + y * 1504]);
					image_buffer[2 * x + 1 + (2 * (y + m_yn) + 0) * 3000] = SwapBytes(ft_in_buffer[x + 7 + y * 1504]);
				}
				int x = 1499;
				image_buffer[2 * x + 0 + (2 * (y + m_yn) + 0) * 3000] = SwapBytes(ft_in_buffer[x + 6 + y * 1504]);
				image_buffer[2 * x + 0 + (2 * (y + m_yn) + 1) * 3000] = SwapBytes(ft_in_buffer[x + 6 + y * 1504]);
				image_buffer[2 * x + 1 + (2 * (y + m_yn) + 1) * 3000] = SwapBytes(ft_in_buffer[x + 6 + y * 1504]);
				image_buffer[2 * x + 1 + (2 * (y + m_yn) + 0) * 3000] = SwapBytes(ft_in_buffer[x + 6 + y * 1504]);
			}
		}
	}

	camera_state = kCamera_Idle;

	// check if we need to update the CameraSetTemp value, i.e. if the set value changed while the sensor was read.
	if (target_temp_dirty) {
		cameraSetTemp(target_temp_cache);
		target_temp_dirty = false;
	}

	if (cooler_on_dirty) {
		if (cooler_on_cache) {
			cameraCoolingOn();
		}
		else {
			cameraCoolingOff();
		}
		cooler_on_dirty = false;
	}

	// Discard image if sensorClearing was required (Bias frame before exposure).
	if (sensor_clear) {
		image_ready = false;
		sensor_clear = false;
	}
	else {
		image_ready = true;
	}
}

void CALLBACK ExposureTimer(PVOID lpParam, BOOLEAN TimerOrWaitFired) {
	ReadFrame();
	return;
}

void AD9826(char adr, WORD val) {
	const int kol = 64;
	BYTE dan[kol];
	memset(dan, kPort_Initial, kol);
	int i;
	FT_STATUS ft_status = FT_OK;

	// Create SL signal.
	for (i = 1; i <= 32; i++) {
		dan[i] = dan[i] & 0xfe;
	}

	// Create SCK pulses.
	for (i = 0; i <= 15; i++) {
		dan[2 * i + 2] += 2; // dan[2 * i + 2] + 2;
	}

	// Create data adressing.
	if ((adr & 4) == 4) {
		dan[3] += 4;
		dan[4] += 4;
	}

	if ((adr & 2) == 2) {
		dan[5] += 4;
		dan[6] += 4;
	}

	if ((adr & 1) == 1) {
		dan[7] += 4;
		dan[8] += 4;
	}

	// Create data.
	if ((val & 256) == 256) {
		dan[15] += 4;
		dan[16] += 4;
	}

	if ((val & 128) == 128) {
		dan[17] += 4;
		dan[18] += 4;
	}

	if ((val & 64) == 64) {
		dan[19] += 4;
		dan[20] += 4;
	}

	if ((val & 32) == 32) {
		dan[21] += 4;
		dan[22] += 4;
	}

	if ((val & 16) == 16) {
		dan[23] += 4;
		dan[24] += 4;
	}

	if ((val & 8) == 8) {
		dan[25] += 4;
		dan[26] += 4;
	}

	if ((val & 4) == 4) {
		dan[27] += 4;
		dan[28] += 4;
	}

	if ((val & 2) == 2) {
		dan[29] += 4;
		dan[30] += 4;
	}

	if ((val & 1) == 1) {
		dan[31] += 4;
		dan[32] += 4;
	}

	if (!error_write_flag) {
		DWORD write_result;
		ft_status = FT_Write(ft_handle_b, dan, kol, &write_result);
		if (ft_status != FT_OK) {
			error_write_flag = true;
#ifdef DEBUG
			debug_log.Add_Code("AD9826 - FT_Write - ft_status", ft_status, Debug_Logger::kLevel_1, Debug_Logger::kNew_Line_Disable);
#endif
		}
		if (write_result != kol) {
			error_write_flag = true;
#ifdef DEBUG
			debug_log.Add_Code("AD9826 - FT_Write - write_result", write_result, Debug_Logger::kLevel_1, Debug_Logger::kNew_Line_Disable);
#endif
		}
	}
	return;
}

void ReadFrame() {
	camera_state = kCamera_Reading;
	FT_STATUS ft_status = FT_OK;

	ft_status = FT_Purge(ft_handle_a, FT_PURGE_TX);
	if (ft_status != FT_OK) {
#ifdef DEBUG
		debug_log.Add_Code("ReadFrame - Purge", ft_status, Debug_Logger::kLevel_1, Debug_Logger::kNew_Line_Disable);
#endif
	}

	std::thread co(&ReadThread);
	co.detach();
	SpiComm(kCommand_Read_Frame, 0);
	return;
}

void CameraSensorClearFull() {
	WORD exposure = 0;
	error_read_flag = false;
	image_ready = false;
	m_yn = 0;
	SpiComm(kCommand_Set_ROI_Start_Y, m_yn);
	m_delta_y = kCamera_Height / 2;
	SpiComm(kCommand_Set_ROI_Num_Y, m_delta_y);

	// Use 2x2 binning to increase the reading speed.
	// The image will be deleted anyway.
	kol_byte = m_delta_y * 3008;
	// Binning.
	SpiComm(kCommand_Set_Binning, 1);
	m_bin = 1;

	SpiComm(kCommand_Set_Exposure, exposure);

	camera_state = kCamera_Exposing;

	exposure_exp = 0;
	ReadFrame();

	// ReadFrame will clear the sensor_clear flag once it is done. This does however lock the
	// main thread. Not sure how to do this otherwise.
	while (sensor_clear) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	// Now exit to do the proper exposure.
	return;
}

BOOL cameraSetGain(int val) {
	AD9826(3, val);
	return true;
}

BOOL cameraSetOffset(int val) {
	int x = abs(2 * val);
	if (val < 0) {
		x += 256;
	}
	AD9826(6, x);
	return true;
}

BOOL cameraConnect() {
	is_connected = false;
	bool ft_flag = true;
	error_write_flag = false;
	sensor_temp_cache = 0;
	target_temp_cache = 0;
	target_temp_dirty = false;
	cooler_on_cache = false;
	cooler_on_dirty = false;
	cooler_power_cache = 0;
	firmware_version_cache = 0;
	char serial_a[20] = "CAM86A";
	char serial_b[20] = "CAM86B";
	FT_STATUS ft_status = FT_OK;

	if (ft_flag) {
		ft_status = FT_OpenEx(serial_a, FT_OPEN_BY_SERIAL_NUMBER, &ft_handle_a);
		if (ft_status != FT_OK) {
			ft_flag = false;
#ifdef DEBUG
			debug_log.Add_Code("cameraConnect - Serial A", ft_status, Debug_Logger::kLevel_0, Debug_Logger::kNew_Line_Disable);
#endif
		}
	}

	if (ft_flag) {
		ft_status = FT_OpenEx(serial_b, FT_OPEN_BY_SERIAL_NUMBER, &ft_handle_b);
		if (ft_status != FT_OK) {
			ft_flag = false;
#ifdef DEBUG
			debug_log.Add_Code("cameraConnect - Serial B", ft_status, Debug_Logger::kLevel_0, Debug_Logger::kNew_Line_Disable);
#endif
		}
	}

	if (ft_flag) {
		ft_status = FT_SetBitMode(ft_handle_b, 0xbf, FT_BITMODE_SYNC_BITBANG);
		if (ft_status != FT_OK) {
			ft_flag = false;
#ifdef DEBUG
			debug_log.Add_Code("cameraConnect - Set bitmode", ft_status, Debug_Logger::kLevel_0, Debug_Logger::kNew_Line_Disable);
#endif
		}
	}

	if (ft_flag) {
		ft_status = FT_SetBaudRate(ft_handle_b, ft_handle_b_baud);
		if (ft_status != FT_OK) {
			ft_flag = false;
#ifdef DEBUG
			debug_log.Add_Code("cameraConnect - Set baudrate", ft_status, Debug_Logger::kLevel_0, Debug_Logger::kNew_Line_Disable);
#endif
		}

		ft_status = FT_SetLatencyTimer(ft_handle_b, 2);
		if (ft_status != FT_OK) {
			ft_flag = false;
#ifdef DEBUG
			debug_log.Add_Code("cameraConnect - Set latency timer B", ft_status, Debug_Logger::kLevel_0, Debug_Logger::kNew_Line_Disable);
#endif
		}

		ft_status = FT_SetLatencyTimer(ft_handle_a, 2);
		if (ft_status != FT_OK) {
			ft_flag = false;
#ifdef DEBUG
			debug_log.Add_Code("cameraConnect - Set latency timer A", ft_status, Debug_Logger::kLevel_0, Debug_Logger::kNew_Line_Disable);
#endif
		}

		ft_status = FT_SetTimeouts(ft_handle_a, 6000, 100);
		if (ft_status != FT_OK) {
			ft_flag = false;
#ifdef DEBUG
			debug_log.Add_Code("cameraConnect - Set timeouts A", ft_status, Debug_Logger::kLevel_0, Debug_Logger::kNew_Line_Disable);
#endif
		}

		ft_status = FT_SetTimeouts(ft_handle_b, 100, 100);
		if (ft_status != FT_OK) {
			ft_flag = false;
#ifdef DEBUG
			debug_log.Add_Code("cameraConnect - Set timeouts B", ft_status, Debug_Logger::kLevel_0, Debug_Logger::kNew_Line_Disable);
#endif
		}

		ft_status = FT_SetUSBParameters(ft_handle_a, 65536,0);
		if (ft_status != FT_OK) {
			ft_flag = false;
#ifdef DEBUG
			debug_log.Add_Code("cameraConnect - Set USB parameters", ft_status, Debug_Logger::kLevel_0, Debug_Logger::kNew_Line_Disable);
#endif
		}

		ft_status = FT_Purge(ft_handle_a, FT_PURGE_TX);
		if (ft_status != FT_OK) {
			ft_flag = false;
#ifdef DEBUG
			debug_log.Add_Code("cameraConnect - Purge A In", ft_status, Debug_Logger::kLevel_0, Debug_Logger::kNew_Line_Disable);
#endif
		}

		ft_status = FT_Purge(ft_handle_a, FT_PURGE_RX);
		if (ft_status != FT_OK) {
			ft_flag = false;
#ifdef DEBUG
			debug_log.Add_Code("cameraConnect - Purge A Out", ft_status, Debug_Logger::kLevel_0, Debug_Logger::kNew_Line_Disable);
#endif
		}

		ft_status = FT_Purge(ft_handle_b, FT_PURGE_TX);
		if (ft_status != FT_OK) {
			ft_flag = false;
#ifdef DEBUG
			debug_log.Add_Code("cameraConnect - Purge B In", ft_status, Debug_Logger::kLevel_0, Debug_Logger::kNew_Line_Disable);
#endif
		}

		ft_status = FT_Purge(ft_handle_b, FT_PURGE_RX);
		if (ft_status != FT_OK) {
			ft_flag = false;
#ifdef DEBUG
			debug_log.Add_Code("cameraConnect - Purge B Out", ft_status, Debug_Logger::kLevel_0, Debug_Logger::kNew_Line_Disable);
#endif
		}

		// Set the default pin state for port B.
		DWORD write_result;
		BYTE portb_priming = kPort_Initial;
		ft_status = FT_Write(ft_handle_b, &portb_priming, 1, &write_result);
		if (ft_status != FT_OK) {
			ft_flag = false;
#ifdef DEBUG
			debug_log.Add_Code("cameraConnect - FT_Write - ft_status", ft_status, Debug_Logger::kLevel_1, Debug_Logger::kNew_Line_Disable);
#endif
		}
		if (write_result != 1) {
			ft_flag = false;
#ifdef DEBUG
			debug_log.Add_Code("cameraConnect - FT_Write - write_result", write_result, Debug_Logger::kLevel_1, Debug_Logger::kNew_Line_Disable);
#endif
		}

		// Clear all data to the AD9826 signal processor.
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		/*
		AD9826 Configuration.
		D8 - 0
		D7 - 1	Input Range: 4V.
		D6 - 1	Internal VREF: ON
		D5 - 0	3 Channel Mode: OFF
		D4 - 1	CDS Operation: CDS Mode
		D3 - 1	Input Clamp Bias: 4V
		D2 - 0	Power-Down: OFF (Normal)
		D1 - 0
		D0 - 0	Output Mode: 2 Byte
		*/
		AD9826(0, 0xd8);

		/*
		AD9826 MUX Configuration.
		D8 - 0
		D7 - 1	MUX Order: RGB
		D6 - 0	Channel Select RED: OFF
		D5 - 1	Channel Select GREEN: ON
		D4 - 0	Channel Select BLUE: OFF
		D3 - 0
		D2 - 0
		D1 - 0
		D0 - 0
		*/
		AD9826(1, 0xa0);

		cameraSetGain(0);

		cameraSetOffset(-6);

		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		SpiComm(kCommand_Init_MCU, 0);

		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		ft_status = FT_Purge(ft_handle_a, FT_PURGE_TX);
		if (ft_status != FT_OK) {
			ft_flag = false;
#ifdef DEBUG
			debug_log.Add_Code("cameraConnect - Purge A In", ft_status, Debug_Logger::kLevel_0, Debug_Logger::kNew_Line_Disable);
#endif
		}

		m_bin = 0;
	}

	is_connected = ft_flag;
	error_read_flag = false;
	camera_state = kCamera_Idle;
	image_ready = false;

	if (!ft_flag) {
		camera_state = kCamera_Error;
#ifdef DEBUG
		debug_log.Add_Code("cameraConnect - camera_state", camera_state, Debug_Logger::kLevel_0, Debug_Logger::kNew_Line_Disable);
#endif
	}

	return is_connected;
}

BOOL cameraDisconnect() {
	bool ft_op_flag = true;
	FT_STATUS ft_status = FT_OK;

	ft_status = FT_Close(ft_handle_a);
	if (ft_status != FT_OK) {
		ft_op_flag = false;
#ifdef DEBUG
		debug_log.Add_Code("cameraDisconnect - Close A", ft_status, Debug_Logger::kLevel_0, Debug_Logger::kNew_Line_Disable);
#endif
	}

	ft_status = FT_Close(ft_handle_b);
	if (ft_status != FT_OK) {
		ft_op_flag = false;
#ifdef DEBUG
		debug_log.Add_Code("cameraDisconnect - Close B", ft_status, Debug_Logger::kLevel_0, Debug_Logger::kNew_Line_Disable);
#endif
	}

	is_connected = !ft_op_flag;
	return ft_op_flag;
}

BOOL cameraIsConnected() {
	return is_connected;
}

BOOL cameraStartExposure(int bin, int start_x, int start_y, int num_x, int num_y, double duration, BOOL light) {
	// num_x is not used, but is kept to keep the driver somewhat compatible with the ascom driver.
	int exposure;

	if (sensor_clear) {
		CameraSensorClearFull();
	}

	error_read_flag = false;
	image_ready = false;

	m_yn = start_y / 2;
	SpiComm(kCommand_Set_ROI_Start_Y, m_yn);

	m_delta_y = num_y / 2;
	SpiComm(kCommand_Set_ROI_Num_Y, m_delta_y);

	if (bin == 2) {
		kol_byte = m_delta_y * 3008;
		SpiComm(kCommand_Set_Binning, 1);
		m_bin = 1;
	}
	else {
		kol_byte = m_delta_y * 12008;
		SpiComm(kCommand_Set_Binning, 0);
		m_bin = 0;
	}

	exposure = round(duration * 1000.0);
	if (exposure > 1000) {
		exposure = 1001;
	}

	SpiComm(kCommand_Set_Exposure, exposure);

	camera_state = kCamera_Exposing;

	if (duration > 1.0) {
		// Long exposure. Exposure time is handled by the driver.
		SpiComm(kCommand_Shift_3, 0); // Drain pixels and start the exposure.
		std::this_thread::sleep_for(std::chrono::milliseconds(40));

		SpiComm(kCommand_Clear_Frame, 0);
		std::this_thread::sleep_for(std::chrono::milliseconds(180));

		// Turn off 15V to reduce noise/bloom.
		SpiComm(kCommand_Off_15V, 0);

		// Exposure has been set to 1001ms with a 180ms delay and a command delay = -1.2s.
		exposure_exp = round(1000.0 * (duration - 1.2));

		if (exposure_exp <= 0) {
			// Frame needs to be read out immediatly.
			exposure_exp = 0;
			ReadFrame();
		}
		else {
			// Set a timer for a frame read once exposure is done.
			if (!CreateTimerQueueTimer(&exposure_timer, NULL, ExposureTimer, NULL, exposure_exp, 0, WT_EXECUTEONLYONCE)) {
#ifdef DEBUG
				debug_log.Add_Message("cameraStartExposure", "Could not create timer.", Debug_Logger::kLevel_0, Debug_Logger::kNew_Line_Disable);
#endif
			}
		}
	}
	else {
		// Short exposure. Exposure time is handled by the firmware.
		exposure_exp = 0;
		ReadFrame();
	}
	return true;
}

BOOL cameraStopExposure() {
	if (!DeleteTimerQueueTimer(NULL, exposure_timer, NULL)) {
#ifdef DEBUG
		debug_log.Add_Message("cameraStopExposure", "Could not delete timer!", Debug_Logger::kLevel_0, Debug_Logger::kNew_Line_Disable);
#endif
	}

	if (camera_state == kCamera_Exposing) {

		ReadFrame();
	}
	return true;
}

int cameraGetCameraState() {
	if (!error_write_flag) {
		return camera_state;
	}
	else {
		return kCamera_Error;
	}
}

BOOL cameraGetImageReady() {
	return image_ready;
}

int* cameraGetImage() {
	camera_state = kCamera_Idle;
	return image_buffer;
}

int cameraGetError() {
	int result = 0;
	if (error_write_flag) {
		result += 2;
#ifdef DEBUG
		debug_log.Add_Code("cameraGetError - error_wire_flag", error_write_flag, Debug_Logger::kLevel_0, Debug_Logger::kNew_Line_Disable);
#endif
	}
	if (error_read_flag) {
		result += 1;
#ifdef DEBUG
		debug_log.Add_Code("cameraGetError - error_read_flag", error_read_flag, Debug_Logger::kLevel_0, Debug_Logger::kNew_Line_Disable);
#endif
	}
	return result;
}

double cameraGetTemp() {
	double temp;

	if (camera_state == kCamera_Reading || camera_state == kCamera_Download) {
		return sensor_temp_cache;
	}
	else {
		SpiComm(kCommand_Get_CCD_Temp, 0);
		temp = (si_out - kTemperature_Offset) / 10.0;
		if (temp > kMax_Error_Temperature || temp < kMin_Error_Temperature) {
			temp = sensor_temp_cache;
		}
		sensor_temp_cache = temp;
		return temp;
	}
}

BOOL cameraSetTemp(double temp) {
	if (camera_state == kCamera_Reading || camera_state == kCamera_Download) {
		target_temp_cache = temp;
		target_temp_dirty = true;
	}
	else {
		WORD d0 = kTemperature_Offset + round(temp * 10.0);
		SpiComm(kCommand_Set_Target_Temp, d0);
		target_temp_dirty = false;
	}
	return true;
}

double cameraGetSetTemp() {
	double temp;
	if (camera_state == kCamera_Reading || camera_state == kCamera_Download) {
		return target_temp_cache;
	}
	else {
		SpiComm(kCommand_Get_Target_Temp, 0);
		temp = (si_out - kTemperature_Offset) / 10.0;
		if (temp > kMax_Error_Temperature || temp < kMin_Error_Temperature) {
			temp = target_temp_cache;
		}
		target_temp_cache = temp;
		return temp;
	}
}

BOOL cameraCoolingOn() {
	if (camera_state == kCamera_Reading || camera_state == kCamera_Download) {
		cooler_on_cache = true;
		cooler_on_dirty = true;
	}
	else {
		SpiComm(kCommand_On_Off_Cooler, 1);
	}
	return true;
}

BOOL cameraCoolingOff() {
	if (camera_state == kCamera_Reading || camera_state == kCamera_Download) {
		cooler_on_cache = false;
		cooler_on_dirty = true;
	}
	else {
		SpiComm(kCommand_On_Off_Cooler, 0);
	}
	return true;
}

BOOL cameraGetCoolerOn() {
	if (camera_state == kCamera_Reading || camera_state == kCamera_Download) {
		return cooler_on_cache;
	}
	else {
		SpiComm(kCommand_Get_Cooler_Status, 0);
		if (si_out == kTrue_Inv_Prot) {
			cooler_on_cache = true;
			return true;
		}
		else if (si_out == kFalse_Inv_Prot) {
			cooler_on_cache = false;
			return false;
		}
		else {
			return cooler_on_cache;
		}
	}
}

double cameraGetCoolerPower() {
	double power;
	if (camera_state == kCamera_Reading || camera_state == kCamera_Download) {
		return cooler_power_cache;
	}
	else {
		SpiComm(kCommand_Get_Cooler_Power, 0);
		if ((si_out >> 8) == (kHigh_Mask_Prot >> 8)) {
			power = (si_out & 0x00ff) / 2.55;
		}
		else {
			power = cooler_power_cache;
		}
		cooler_power_cache = power;
		return power;
	}
}

BOOL cameraSetReadingTime(int val) {
	SpiComm(kCommand_Set_Delay_Per_Line, val);
	return true;
}

BYTE cameraGetFirmwareVersion() {
	if (camera_state == kCamera_Reading || camera_state == kCamera_Download) {
		return firmware_version_cache;
	}
	else {
		SpiComm(kCommand_Get_Version, 0);
		firmware_version_cache = si_out & 0xff;
		return firmware_version_cache;
	}
}

BOOL cameraSetCoolerDuringReading(BYTE val) {
	SpiComm(kCommand_Set_Cooler_On_During_Read, val);
	return true;
}

BYTE cameraGetLLDriverVersion() {
	return kSoftware_LLDriver_Version;
}

BOOL cameraSetBiasBeforeExposure(BYTE val) {
	sensor_clear = val;
	return true;
}

double cameraGetTempDHT() {
	if (camera_state == kCamera_Reading || camera_state == kCamera_Download) {
		return temp_dht_cache;
	}
	else {
		SpiComm(kCommand_Get_Case_Temp, 0);
		temp_dht_cache = (si_out - kTemperature_Offset) / 10.0;
		return temp_dht_cache;
	}
}

double cameraGetHumidityDHT() {
	if (camera_state == kCamera_Reading || camera_state == kCamera_Download) {
		return humidity_dht_cache;
	}
	else {
		SpiComm(kCommand_Get_Case_Humidity, 0);
		humidity_dht_cache = si_out / 10.0;
		return humidity_dht_cache;
	}
}

BOOL cameraSetCoolingStartingPowerPercentage(int val) {
	SpiComm(kCommand_Set_Cooler_Power_Start, val);
	cooling_start_power_percentage_cache = val;
	return true;
}

BOOL cameraSetCoolingMaximumPowerPercentage(int val) {
	SpiComm(kCommand_Set_Cooler_Power_Max, val);
	cooling_maximum_power_percentage_cache = val;
	return true;
}

int cameraGetCoolingStartingPowerPercentage() {
	if (camera_state == kCamera_Reading || camera_state == kCamera_Download) {
		return cooling_start_power_percentage_cache;
	}
	else {
		SpiComm(kCommand_Get_Cooler_Power_Start, 0);
		cooling_start_power_percentage_cache = si_out;
		return cooling_start_power_percentage_cache;
	}
}

int cameraGetCoolingMaximumPowerPercentage() {
	if (camera_state == kCamera_Reading || camera_state == kCamera_Download) {
		return cooling_maximum_power_percentage_cache;
	}
	else {
		SpiComm(kCommand_Get_Cooler_Power_Max, 0);
		cooling_maximum_power_percentage_cache = si_out;
		return cooling_maximum_power_percentage_cache;
	}
}

BOOL cameraSetPIDproportionalGain(double val) {
	WORD temp_val = trunc(val * 100.0);
	SpiComm(kCommand_Set_PID_KP, temp_val);
	kp_cache = val;
	return true;
}

BOOL cameraSetPIDintegralGain(double val) {
	WORD temp_val = trunc(val * 100.0);
	SpiComm(kCommand_Set_PID_KI, temp_val);
	ki_cache = val;
	return true;
}

BOOL cameraSetPIDderivativeGain(double val) {
	WORD temp_val = trunc(val * 100.0);
	SpiComm(kCommand_Set_PID_KD, temp_val);
	kd_cache = val;
	return true;
}

BOOL cameraGetPIDGainLow(BYTE cmd) {
	SpiComm(cmd, 0);
	return si_out;
}

BOOL cameraGetPIDGainHigh(BYTE cmd) {
	SpiComm(cmd, 0);
	return si_out;
}

double cameraGetPIDGain(double cache_var, BYTE cmd_low, BYTE cmd_high) {
	BYTE temp[4];
	WORD temp_low;
	WORD temp_high;
	float temp_float;

	if (camera_state == kCamera_Reading || camera_state == kCamera_Download) {
		return cache_var;
	}
	else {
		temp_low = cameraGetPIDGainLow(cmd_low);
		temp_high = cameraGetPIDGainHigh(cmd_high);

		temp[0] = LowByte(temp_low);
		temp[1] = HighByte(temp_low);
		temp[2] = LowByte(temp_high);
		temp[3] = HighByte(temp_high);

		// Convert the byte array to a single float.
		memcpy_s(&temp_float, sizeof(temp_float), temp, sizeof(temp));

		cache_var = temp_float;
		return cache_var;
	}
}

double cameraGetPIDproportionalGain() {
	return cameraGetPIDGain(kp_cache, kCommand_Get_PID_KP_LW, kCommand_Get_PID_KP_HW);
}

double cameraGetPIDintegralGain() {
	return cameraGetPIDGain(ki_cache, kCommand_Get_PID_KI_LW, kCommand_Get_PID_KI_HW);
}

double cameraGetPIDderivativeGain() {
	return cameraGetPIDGain(kd_cache, kCommand_Get_PID_KD_LW, kCommand_Get_PID_KD_HW);
}