#include "StdAfx.h"
#include "SpectrometerModel.h"


CSpectrometerModel::CSpectrometerModel(void)
{
}

CSpectrometerModel::~CSpectrometerModel(void)
{
}

/** Retrieves the maximum intensity for the supplied spectrometer model */
double	CSpectrometerModel::GetMaxIntensity(const CString modelNumber){
	return GetMaxIntensity(GetModel(modelNumber));
}
double  CSpectrometerModel::GetMaxIntensity(const SPECTROMETER_MODEL modelNumber){
	switch(modelNumber){
		case S2000:			return 4095;
		case USB2000:		return 4095;
		case USB4000:		return 65535;
		case HR2000:		return 4095;
		case HR4000:		return 16535;
		case USB2000_PLUS:	return 65536;
		case HR2000_PLUS:	return 16535;
		case QE65000:		return 65535;
		default:			return 4095;
	}
}

/** Converts a SPECTROMETER_MODEL to a string item */
RETURN_CODE CSpectrometerModel::ToString(SPECTROMETER_MODEL model, CString &str){
	if(S2000 == model){
		str.Format("S2000");
		return SUCCESS;
	}
	if(USB2000 == model){
		str.Format("USB2000");
		return SUCCESS;
	}
	if(USB4000 == model){
		str.Format("USB4000");
		return SUCCESS;
	}
	if(HR2000 == model){
		str.Format("HR2000");
		return SUCCESS;
	}
	if(HR4000 == model){
		str.Format("HR4000");
		return SUCCESS;
	}
	if(QE65000 == model){
		str.Format("QE65000");
		return SUCCESS;
	}
	if(USB2000_PLUS == model){
		str.Format("USB2000+");
		return SUCCESS;
	}
	if(HR2000_PLUS == model){
		str.Format("HR2000+");
		return SUCCESS;
	}

	str.Format("Unknown");
	return FAIL;
}

/** Converts a string item to a SPECTROMETER_MODEL */
SPECTROMETER_MODEL CSpectrometerModel::GetModel(const CString &str){
	if(Equals(str, "S2000"))
		return S2000;
	if(Equals(str, "USB2000"))
		return USB2000;
	if(Equals(str, "HR2000"))
		return HR2000;
	if(Equals(str, "HR4000"))
		return HR4000;
	if(Equals(str, "USB4000"))
		return USB4000;
	if(Equals(str, "QE65000"))
		return QE65000;
	if(Equals(str, "USB2000+"))
		return USB2000_PLUS;
	if(Equals(str, "HR2000+"))
		return HR2000_PLUS;

	// not defined
	return UNKNOWN_SPECTROMETER;
}

int	CSpectrometerModel::GetNumSpectrometerModels(){
	return NUM_CONF_SPEC_MODELS;
}

/** Checks the format of the serial number and from this guesses the spectrometer
	model. */
SPECTROMETER_MODEL CSpectrometerModel::GuessSpectrometerModelFromSerial(const CString &serial){
	if(Equals(serial.Left(3), "D2J")){
		return S2000;
	}else if(Equals(serial.Left(3), "I2J")){
		return S2000;
	}else if(Equals(serial.Left(5), "USB2+")){
		return USB2000_PLUS;
	}else if(Equals(serial.Left(4), "USB2")){
		return USB2000;
	}else if(Equals(serial.Left(5), "USB4C")){
		return USB4000;
	}else if(Equals(serial.Left(4), "HR2+")){
		return HR2000_PLUS;
	}else if(Equals(serial.Left(3), "HR2")){
		return HR2000;
	}else if(Equals(serial.Left(3), "HR4")){
		return HR4000;
	}else if(Equals(serial.Left(2), "QE")){
		return QE65000;
	}
	
	return UNKNOWN_SPECTROMETER;
}
