#pragma once

#include "../Common/Common.h"
#include "../Common/GPSData.h"
#include "../Common/ReferenceFile.h"

#include "../Evaluation/FitWindow.h"
#include "../Common/SpectrometerModel.h"

#ifndef _CCONFIGURATIONSETTINGS_H_
#define _CCONFIGURATIONSETTINGS_H_

/** The <b>CCConfigurationSetting</b> holds the configuration of the scanning Instruments
    that is found in the configuration.xml-file. */

class CConfigurationSetting
{
public:

	class CommunicationSetting{
	public:
		CommunicationSetting();
		~CommunicationSetting();

		/** Resets all values to default */
		void Clear();

		/** What type of connection is it to the instrument */
		int connectionType;

		// ----------- The settings for serial communication --------------
		/** The port number to use, only useful if connection type is serial */
		long port;

		/** The baudrate to use for communication, only useful if connection type is serial */
		long baudrate;

		/** Which type of flow control to use, only useful if connection type is serial */
		int flowControl;

		/** The timeout for communication */
		long timeout;

		/** The medium through which the communciation occurs.
				MEDIUM_CABLE corresponds to a cable,
				MEDIUM_FREEWAVE_SERIAL_MODEM corresponds to a Freewave radio modem. */
		int medium;

		// ----- The additional settings for the serial Freewave communication -----
		
		/** The RadioID OR callbook number */
		CString radioID;

		// ----------- The settings for FTP communication --------------

		/** The IP-number of the scanning instrument */
		BYTE ftpIP[4];

		/** The username at the scanning instrument */
		CString ftpUserName;

		/** The password at the scanning instrument */
		CString ftpPassword;

		/** The administrator-username at the scanning instrument */
		CString ftpAdminUserName;

		/** The administrator-password at the scanning instrument */
		CString ftpAdminPassword;
		
		// ----------- The general settings for the communication --------------

		/** How often the scanning Instrument should be queried for new data */
		long queryPeriod;

		/** Time to start sleeping */
		struct timeStruct sleepTime;

		/** Time to wake up */
		struct timeStruct wakeupTime;

		/** Assignment operator */
		CommunicationSetting &operator=(const CommunicationSetting &comm2);
	};

	class DarkSettings{
		public:
			DarkSettings();
			~DarkSettings();

		/** Resets all values to default */
		void Clear();

		/** The options for the how to get the dark.
			Can be: 0 - use measured (DEFAULT)
					1 - model of no measured is available
					2 - always model
					3 - the dark-spectrum is given by the user, do not model */
		DARK_SPEC_OPTION m_darkSpecOption;

		/** The offset-spectrum, only useful if 'm_darkSpecOption' is not 0.
				When this should be used is determined by 'm_offsetOption'.
				If 'm_darkSpecOption' is '3' then this is the dark-spectrum to use */
		CString m_offsetSpec;

		/** The option for how to use the offset-spectrum.
			Can be:	0 - always use measured
					1 - use the user supplied */
		DARK_MODEL_OPTION m_offsetOption;

		/** The dark-current spectrum, only useful if 'm_darkSpecOption' is not 0.
				When this should be used is determined by 'm_darkCurrentOption'. */
		CString m_darkCurrentSpec;

		/** The option for how to use the dark-current spectrum.
				Can be:	0 - always use measured
						1 - use the user supplied */
		DARK_MODEL_OPTION m_darkCurrentOption;

		/** Assignment operator */
		DarkSettings& operator=(const DarkSettings &dark2);
	};

	class SpectrometerChannelSetting{
		public:
			SpectrometerChannelSetting();
			~SpectrometerChannelSetting();

			/** Resets all values to default */
			void Clear();

			/** The size of the detector */
			unsigned int  detectorSize;

			/** The fit settings that are defined for this spectrometer */
			Evaluation::CFitWindow fitWindow;

				/** The settings for how to get the dark-spectrum */
				DarkSettings m_darkSettings;

				/** Assignment operator */
			SpectrometerChannelSetting& operator=(const SpectrometerChannelSetting &spec2);
	};

	class SpectrometerSetting{
		public:
			SpectrometerSetting();
			~SpectrometerSetting();

			/** Resets all values to default */
			void Clear();

			/** The serial number of this spectrometer */
			CString serialNumber;

			/** The model-number of this spectrometer */
			SPECTROMETER_MODEL	model;

			/** The number of channels defined in this spectrometer */
			unsigned char channelNum;

			SpectrometerChannelSetting channel[MAX_CHANNEL_NUM];

			/** Assignment operator */
			SpectrometerSetting& operator=(const SpectrometerSetting &spec2);
	};

	class WindSpeedMeasurementSetting{
		public:
			WindSpeedMeasurementSetting();
			~WindSpeedMeasurementSetting();

			/** Resets all values to default */
			void Clear();

			/** True if this system is supposed to make wind speed measurements automatically */
			bool	automaticWindMeasurements;

			/** The preferred interval between each measurement [in seconds] */
			int	interval;

			/** The preferred duration of each measurement [in seconds] */
			int	duration;

			/** Measurements will only be performed if the centre of the
					plume is within +-maxAngle from zenith */
			double maxAngle;

			/** Measurements will only be performed if the centre of the plume
					is relatively stable over the last 'stablePeriod' number of scans */
			int	stablePeriod;

			/** Measurements will only be performed if the peak-column (minus the offset)
					is larger than 'minPeakColumn' ppmm */
			double minPeakColumn;

			/** The desired angle [degrees] between the two measurement directions 
					OR the desired distance [meters] between the two measurements at plume-height.
					The value is an angle if > 0 and a distance if < 0
					*/
			double	desiredAngle;

			/** This is true if we should trust and use the values for 
					wind-direction and plume-height calculated by the program */
			int		useCalculatedPlumeParameters;

			/** */
			double SwitchRange;
			
			/** Assignment operator */
			WindSpeedMeasurementSetting& operator=(const WindSpeedMeasurementSetting &ws2);
	};

	class SetupChangeSetting{
	public:
		SetupChangeSetting();
		~SetupChangeSetting();

		/** Resets all values to default */
		void Clear();

		/** This is true if we should let the program change the setup of the 
				instrument automatically. This only works for Heidelberg (V-II) instruments */
		int		automaticSetupChange;

		/** This is true if we should trust and use the values for 
				wind-direction and plume-height calculated by the program */
		int		useCalculatedPlumeParameters;

		/** The tolerance for varying wind-directions. No changes will be done as long
				as the changes of the wind-direction is less than this value */
		double	windDirectionTolerance;

		/** How brave we are on using the scanner, can be either of; 
				CHANGEMODE_FAST or
				CHANGEMODE_SAFE 	*/
		int			mode;

		/** Assignment operator */
		SetupChangeSetting& operator=(const SetupChangeSetting &ws2);
	};

	class MotorSetting{
	public:
		MotorSetting();
		~MotorSetting();

		/** Resets all values to default */
		void Clear();

		/** The number of steps in one round */
		int		stepsPerRound;

		/** The motor-steps-compensation */
		int		motorStepsComp;

		/** Assignment operator */
		MotorSetting&  operator=(const MotorSetting &scanner2);
	};

	class ScanningInstrumentSetting{
		public:
			ScanningInstrumentSetting();
			~ScanningInstrumentSetting();

			/** Resets all values to default */
			void Clear();

			/** The volcano on which the instrument is measuring */
			CString volcano;

			/** The observatory which owns the scanning instrument */
			CString observatory;

			/** The site at which the scanning instrument is situated */
			CString site;

			/** The direction in which the instrument points (in degrees from north) */
			double  compass;

			/** The opening angle of the cone that the scanner measures in (in degrees).
			This is 90 degrees for the old scanner, and typically 30 or 45 degrees for the new. */
			double  coneAngle;

			/** The tilt of the system, in the direction of the scanner. */
			double tilt;

			/** The gps-coordinates for the scanning instrument */
			CGPSData  gps;

			/** The communication settings for the scanning instrument */
			CommunicationSetting comm;

			/** The spectrometer inside the scanning instrument */
			SpectrometerSetting spec[MAX_SPECTROMETERS_PER_SCANNER];

			/** The number of spectrometers configured */
			unsigned long specNum;

			/** The settings for automatic wind speed measurements */
			WindSpeedMeasurementSetting windSettings;

			/** The settings for automatically changing the parameters of the scan */
			SetupChangeSetting scSettings;

			/** The settings for the motor(s) */
			MotorSetting motor[2];

			/** The type of this instrument */
			INSTRUMENT_TYPE instrumentType;

			/** The type of the electronics box */
			ELECTRONICS_BOX electronicsBox;

			/** Assignment operator */
			ScanningInstrumentSetting&  operator=(const ScanningInstrumentSetting &scanner2);
	};

	/**FTP server's setting*/
	class CFTPSetting{

	public:
		CFTPSetting();
		~CFTPSetting();
		void SetFTPStatus(int status);
		CString ftpAddress;     // the ip-number of the FTP-server
		CString userName;       // the user name at the FTP-server
		CString password;       // the password at the FTP-server
		int     ftpStatus;      // not used?
		int     ftpStartTime;   // the time of day when to start uploading (seconds since midnight)
		int     ftpStopTime;    // the time of day when to stop uploading (seconds since midnight)
	};

	/** Settings for publishing the results on a web - page */
	class WebSettings{
	public:
		WebSettings();
		~WebSettings();
		int     publish;        // 0 if don't publish, otherwise publish
		CString localDirectory; // set to a local directory if the output is to be stored on the same computer
		CString imageFormat;    // the format of the images to save, can be .bmp, .gif, .jpg or .png
	};

	/** Settings for calling of external programs when receiving scans */
	class CExternalCallSettings{
	public:
		CExternalCallSettings();
		~CExternalCallSettings();
		CString			fullScanScript; // the path of one shell-command/excetuable file to exceute when receiving a complete scan
		CString			imageScript;    // the path of one shell-command/executable file to execute when one image has been generated
	};

	/** Settings for retrieval of the wind-field from external sources */
	class CWindFieldDataSettings{
	public:
		CWindFieldDataSettings();
		~CWindFieldDataSettings();
		CString		windFieldFile;					// the path and file-name of the file which is the source of the wind-field data
		long		windFileReloadInterval;	// the reload inteval of the wind-field fiel (in minutes) - zero corresponds to never reload the file
	};

public:
	CConfigurationSetting(void);
	~CConfigurationSetting(void);

	/** Resets all values to default */
	void Clear();

	// ----------------------------------------------------------------
	// -------------------- PUBLIC CONSTANTS --------------------------
	// ----------------------------------------------------------------
	static const int STARTUP_MANUAL     = 0;
	static const int STARTUP_AUTOMATIC  = 1;

	static const int CHANGEMODE_SAFE	= 0;
	static const int CHANGEMODE_FAST	= 1;

	// ----------------------------------------------------------------
	// ----------------------- PUBLIC DATA ----------------------------
	// ----------------------------------------------------------------

	/** The configured scanning instruments */
	ScanningInstrumentSetting scanner[MAX_NUMBER_OF_SCANNING_INSTRUMENTS];

	/** How many scanning instruments that are defined */
	unsigned long scannerNum;

	/** The output directory */
	CString outputDirectory;

	/** Settings for web - publishing the results */
	WebSettings webSettings;

	/** Startup method */
	int startup;

	/**The ftp server setting*/
	CFTPSetting ftpSetting;

	/** The settings for calling of external programs */
	CExternalCallSettings	externalSetting;

	/** The settings for retrieving the wind-field from external sources */
	CWindFieldDataSettings windSourceSettings;
};

#endif