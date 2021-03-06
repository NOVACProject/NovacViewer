#include "StdAfx.h"
#include "windfilereader.h"

using namespace FileHandler;

CWindFileReader::CWindFileReader(void)
{
	m_containsWindDirection = false;
	m_containsWindSpeed			= false;
	m_containsPlumeHeight		= false;
}

CWindFileReader::~CWindFileReader(void)
{
}

/** Reads the wind file */
RETURN_CODE CWindFileReader::ReadWindFile(){
	CWindFieldAndPlumeHeight windfield; // <-- the next wind-field to insert
	char dateStr[] = _T("date"); // this string only exists in the header line.
	char sourceStr[] = _T("source");
	char szLine[8192];
	MET_SOURCE windFieldSource = MET_USER; // the source for the wind-information

	// Lock this object to make sure that now one else tries to read
	//	data from this object while we are reading
	CSingleLock singleLock(&m_critSect);
	singleLock.Lock();

	if(singleLock.IsLocked()){

		// If no evaluation log selected, quit
		if(strlen(m_windFile) <= 1){
			singleLock.Unlock(); // open up this object again
			return FAIL;
		}

		// Open the wind log
		FILE *f = fopen(m_windFile, "r");
		if(NULL == f){
			singleLock.Unlock(); // open up this object again
			return FAIL;
		}

		// Reset prior knowledge of the contents of each column
		ResetColumns();

		// Read the file, one line at a time
		while(fgets(szLine, 8192, f)){
			// ignore empty lines
			if(strlen(szLine) < 2){
				continue;
			}

			// ignore comment lines
			if(szLine[0] == '#' || szLine[0] == '%')
				continue;

			// convert the string to all lower-case letters
			for(unsigned int it = 0; it < strlen(szLine); ++it){
				szLine[it] = tolower(szLine[it]);
			}

			// If this is the line saying the source of the information...
			if(strstr(szLine, sourceStr)){
				ParseSourceString(szLine, windFieldSource);
			}

			// if this is a header-line then parse it
			if(NULL != strstr(szLine, dateStr)){
				ParseFileHeader(szLine);
				continue;
			}

			// Split the scan information up into tokens and parse them. 
			char* szToken = (char*)(LPCSTR)szLine;
			int curCol = -1;
			while(szToken = strtok(szToken, " \t")){
				++curCol;

				// First check the time
				if(curCol == m_col.time){
					int fValue1, fValue2, fValue3;
					int nValues;
					if(strstr(szToken, ":"))
						nValues = sscanf(szToken, "%d:%d:%d", &fValue1, &fValue2, &fValue3);
					else
						nValues = sscanf(szToken, "%d.%d.%d", &fValue1, &fValue2, &fValue3);
					if(nValues == 2){
						windfield.SetTime(fValue1, fValue2, 0);
					}else{
						windfield.SetTime(fValue1, fValue2, fValue3);
					}
					szToken = NULL;
					continue;
				}

				// Then check the date
				if(curCol == m_col.date){
					int fValue1, fValue2, fValue3;
					int nValues;
					if(strstr(szToken, "_"))
						nValues = sscanf(szToken, "%d_%d_%d", &fValue1, &fValue2, &fValue3);
					else
						nValues = sscanf(szToken, "%d.%d.%d", &fValue1, &fValue2, &fValue3);
					if(nValues == 2){
						windfield.SetDate(fValue1, fValue2, 0);
					}else{
						windfield.SetDate(fValue1, fValue2, fValue3);
					}
					szToken = NULL;
					continue;
				}

				// The wind-direction
				if(curCol == m_col.windDirection){
					double fValue;
					int nValues = sscanf(szToken, "%lf", &fValue);
					windfield.SetWindDirection(fValue, windFieldSource);
					szToken = NULL;
					continue;
				}

				// The wind-speed
				if(curCol == m_col.windSpeed){
					double fValue;
					int nValues = sscanf(szToken, "%lf", &fValue);
					windfield.SetWindSpeed(fValue, windFieldSource);
					szToken = NULL;
					continue;
				}

				// The plume height
				if(curCol == m_col.plumeHeight){
					double fValue;
					int nValues = sscanf(szToken, "%lf", &fValue);
					windfield.SetPlumeHeight(fValue, windFieldSource);
					szToken = NULL;
					continue;
				}

				// parse the next token...
				szToken = NULL;
			}//end while(szToken = strtok(szToken, " \t"))

			// insert the recently read wind-field into the list
			m_windRecord.InsertWindField(windfield);
		}

		// close the file
		fclose(f);

		// Remember to open up this object again
		singleLock.Unlock();
	}

	// all is ok..
	return SUCCESS;
}

/** Returns an interpolation from the most recently read in
		wind-field */
RETURN_CODE CWindFileReader::InterpolateWindField(const CDateTime desiredTime, CWindFieldAndPlumeHeight &desiredWindField){
	RETURN_CODE ret = FAIL;

	// Lock this object to make sure that we are not reading in data
	//	and returning a handle to the wind-field record at the same time
	CSingleLock singleLock(&m_critSect);
	singleLock.Lock();

	if(singleLock.IsLocked()){
	
		// Get the interpolated wind-field
		ret = m_windRecord.InterpolateWindField(desiredTime, desiredWindField);

		// Remember to open up this object again
		singleLock.Unlock();
	}

	return ret;
}

/** Writes the contents of this object to a new wind file */
RETURN_CODE CWindFileReader::WriteWindFile(const CString fileName){

	// not implemented yet
	return FAIL;
}

/** Reads the header line for the file and retrieves which 
    column represents which value. */
void CWindFileReader::ParseFileHeader(const char szLine[8192]){
	char date[]			= _T("date");
	char time[]			= _T("time");
	char speed[]		= _T("speed");
	char speed2[]		= _T("ws");
	char speedErr[]		= _T("wserr");
	char direction[]	= _T("direction");
	char direction2[]	= _T("dir");
	char direction3[]	= _T("wd");
	char directionErr[]	= _T("wderr");
	char height[]		= _T("plumeheight");
	char height2[]		= _T("height");
	char height3[]		= _T("ph");
	char heightErr[]	= _T("pherr");
	int curCol = -1;

	// reset some old information
	ResetColumns();

	// Tokenize the string and see which column belongs to which what value
	char str[8192];
	if(szLine[0] == '#')
		strncpy(str, szLine+1, 8191*sizeof(char));
	else
		strncpy(str, szLine, 8192*sizeof(char));
	char* szToken = (char*)(LPCSTR)str;

	while(szToken = strtok(szToken, "\t")){
		++curCol;

		// The time
		if(0 == strnicmp(szToken, time, strlen(time))){
			m_col.time = curCol;
			szToken = NULL;
			continue;
		}

		// The date
		if(0 == strnicmp(szToken, date, strlen(date))){
			m_col.date = curCol;
			szToken = NULL;
			continue;
		}

		// The speed
		if(0 == strnicmp(szToken, speed, strlen(speed)) || 0 == strnicmp(szToken, speed2, strlen(speed2))){
			m_col.windSpeed = curCol;
			m_containsWindSpeed = true;
			szToken = NULL;
			continue;
		}

		// The uncertainty in the speed
		if(0 == strnicmp(szToken, speedErr, strlen(speedErr))){
			m_col.windSpeedError = curCol;
			szToken = NULL;
			continue;
		}

		// The direction
		if(0 == strnicmp(szToken, direction, strlen(direction)) ||
			 0 == strnicmp(szToken, direction2, strlen(direction2)) ||
			 0 == strnicmp(szToken, direction3, strlen(direction3))){

			m_col.windDirection = curCol;
			m_containsWindDirection = true;
			szToken = NULL;
			continue;
		}

		// The uncertainty in the direction
		if(0 == strnicmp(szToken, directionErr, strlen(directionErr))){
			m_col.windDirectionError = curCol;
			szToken = NULL;
			continue;
		}

		// The plume height
		if(0 == strnicmp(szToken, height,  strlen(height)) ||
			 0 == strnicmp(szToken, height2, strlen(height2)) ||
			 0 == strnicmp(szToken, height3, strlen(height3))){

			m_col.plumeHeight = curCol;
			m_containsPlumeHeight = true;
			szToken = NULL;
			continue;
		}

		// The uncertainty in the plume height
		if(0 == strnicmp(szToken, heightErr, strlen(heightErr))){
			m_col.plumeHeightError = curCol;
			szToken = NULL;
			continue;
		}

	}

	// Make sure that columns which are not found are not in the 
	//	log-columns structure.
	if(!m_containsWindDirection)
		m_col.windDirection = -1;
	if(!m_containsWindSpeed)
		m_col.windSpeed = -1;


	// done!!!
	return;
}

/** Resets the information about which column data is stored in */
void CWindFileReader::ResetColumns(){
	m_col.date					= 0;
	m_col.time					= 1;
	m_col.windDirection	= 2;
	m_col.windSpeed			= 3;
	m_col.plumeHeight		= -1; // not in the first generation of wind-field files...

	// Assume that the file does not contain either the wind-direction, wind-speed or the plume height
	m_containsWindDirection = false;
	m_containsWindSpeed			= false;
	m_containsPlumeHeight		= false;
}

/** Returns the number of points in the database */
long CWindFileReader::GetRecordNum(){
	return this->m_windRecord.GetRecordNum();
}

/** Parses the section containing the source of the wind-field.
	The string should be converted to lower-case characters before
		calling this function */
void CWindFileReader::ParseSourceString(const char szLine[8192], MET_SOURCE &source){
	source = MET_USER; // general assumption...

	if(strstr(szLine, "user")){
		source = MET_USER;
	}else if(strstr(szLine, "default")){
		source = MET_DEFAULT;
	}else if(strstr(szLine, "ecmwf_forecast")){
		source = MET_ECMWF_FORECAST;
	}else if(strstr(szLine, "ecmwf_analysis")){
		source = MET_ECMWF_ANALYSIS;
	}else if(strstr(szLine, "triangulation")){
		source = MET_GEOMETRY_CALCULATION;
	}
}
