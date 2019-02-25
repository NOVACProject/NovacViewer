// DataImporter.cpp : implementation file
//

#include "stdafx.h"
#include "NovacViewer.h"
#include "DataImporter.h"
#include "Common/Common.h"
#include "VolcanoInfo.h"
#include "Common/EvaluationLogFileHandler.h"
#include "Evaluation/ScanResult.h"

extern CVolcanoInfo g_volcanoes;
extern int			g_volcano;		// the index of the chosen volcano
extern CFormView	*pView;			// <-- the main window
CDateTime			g_selectedDate;

IMPLEMENT_DYNCREATE(CDataImporter, CWinThread)

CDataImporter::CDataImporter()
{
	m_fInitialized = false;
	m_ftp = NULL;
	m_nTimerID = 0;	
}

CDataImporter::~CDataImporter()
{
	m_fInitialized = false;
	
	this->m_downloadedEvalLogs.RemoveAll();
	
}

BOOL CDataImporter::InitInstance()
{
	CList <CString, CString &> volcanoDirectories;
	CList <CString, CString &> fileNames;
	bool foundDirectory = false;
	long nLoops;
	CString folderName, localConfigurationFileName, userMessage;
	m_ftpServer.Format("");
	m_ftpUserName.Format("");
	m_ftpPassword.Format("");
	CDateTime selectedDate, today;
	Common common;
	common.GetExePath();
	
	// Get the path to this executable
	this->m_exePath.Format(common.m_exePath);
	
	m_localDataDirectory.Format("%s\\Temp\\", m_exePath);
	
	localConfigurationFileName.Format("%sconfiguration.xml", m_exePath);

	// Connect to the server and enter the directory where the data is supposed to be
	m_ftp = new Communication::CFTPCom();

	// Get the list of available volcanoes
	if(!m_ftp->Connect(m_ftpServer, m_ftpUserName, m_ftpPassword)){
		MessageBox(NULL, "Cannot connect to FTP-server", "Connection failed.", MB_OK);
		exit(1);
	}

	// Get the list of volcanoDirectories...
	m_ftp->GetFileList(volcanoDirectories);

	// If we cannot find out volcano in the list of volcanoDirectories, then quit
	POSITION p = volcanoDirectories.GetHeadPosition();
	while(p != NULL){
		CString &directoryName = volcanoDirectories.GetNext(p);
		if(Equals(directoryName, g_volcanoes.m_simpleName[g_volcano])){
			foundDirectory = true;
			break;
		}
	}
	if(!foundDirectory){
		MessageBox(NULL, "No data from selected volcano is present at the FTP-server", "No data to show", MB_OK);
		m_ftp->Disconnect();
		exit(1);
	}

	// Go into the correct sub-directory
	m_ftp->EnterFolder(g_volcanoes.m_simpleName[g_volcano]);

	// Get the list of dates that are available
	m_ftp->GetFileList(m_dateDirectories);

	// Find the most recently uploaded configuration.xml file 
	selectedDate.SetToNow();
	bool foundConfigurationXml = false;
	nLoops = 0;
	while(!foundConfigurationXml){
		if(nLoops++ > 300){
			MessageBox(NULL, "Failed to find configuration.xml at FTP-server", "No data to show", MB_OK);
			m_ftp->Disconnect();
			exit(1);
		}
		
		// configuration.xml is only uploaded on days which are
		//	dividiable by seven
		int nDays = selectedDate.day % 7;
		while(nDays > 0){
			selectedDate.DecrementOneDay();
			--nDays;
		}

		// enter the folder with the specified date and get the list of files
		folderName.Format("%04d.%02d.%02d", selectedDate.year, selectedDate.month, selectedDate.day);
		
		// Make sure that a folder with this name exists...
		POSITION dPos = m_dateDirectories.GetHeadPosition();
		bool foundDateDirectory = false;
		while(dPos != NULL){
			CString &dateDir = m_dateDirectories.GetNext(dPos);
			if(Equals(dateDir, folderName)){
				foundDateDirectory = true;
				dPos = NULL;
			}
		}
		if(!foundDateDirectory){
			selectedDate.DecrementOneDay();
			continue;
		}
		
		// Get the list of files in this directory
		m_ftp->EnterFolder(folderName);
		m_ftp->GetFileList(fileNames);
		
		// See if we can find an .xml file in the list
		POSITION fPos = fileNames.GetHeadPosition();
		while(fPos != NULL){
			CString &fileName = fileNames.GetNext(fPos);
			if(Equals(fileName.Right(4), ".xml")){
				if(m_ftp->DownloadAFile(fileName, localConfigurationFileName)){
					foundConfigurationXml = true;
					fPos				  = NULL;
				}
			}
		}
		
		// go to an earlier date
		selectedDate.DecrementOneDay();
		
		m_ftp->EnterFolder("..");
	}

	m_fInitialized = true;

	// Set the remote working directory
	g_selectedDate.SetToNow();
	bool fFoundLatestDirectory = false;
	while(!fFoundLatestDirectory){
		// enter the folder with the specified date and get the list of files
		folderName.Format("%04d.%02d.%02d", g_selectedDate.year, g_selectedDate.month, g_selectedDate.day);

		POSITION dPos = m_dateDirectories.GetHeadPosition();
		while(dPos != NULL){
			CString &dateDir = m_dateDirectories.GetNext(dPos);
			if(Equals(folderName, dateDir)){
				fFoundLatestDirectory = true;
				break;
			}
		}
		
		if(!fFoundLatestDirectory){
			userMessage.Format("No data uploaded %04d.%02d.%02d. Decrementing selected date", g_selectedDate.year, g_selectedDate.month, g_selectedDate.day);
			ShowMessage(userMessage);
					
			g_selectedDate.DecrementOneDay();
			
			if(g_selectedDate.year == 2004){
				MessageBox(NULL, "Failed set latest working day", "No data to show", MB_OK);
				m_ftp->Disconnect();
				exit(1);
			}
		}
	}
	today.SetToNow();
	if(!(g_selectedDate == today)){
		userMessage.Format("No data uploaded later than %04d.%02d.%02d, viewing data from this date.", g_selectedDate.year, g_selectedDate.month, g_selectedDate.day);
		MessageBox(NULL, userMessage, "Info", MB_TOPMOST | MB_SETFOREGROUND | MB_OK);
	}

	// Set the directory to download data from
	m_remoteDirectory.Format("%s/%04d.%02d.%02d", g_volcanoes.m_simpleName[g_volcano], g_selectedDate.year, g_selectedDate.month, g_selectedDate.day);

	// Set a timer to wake up every minute
	m_nTimerID = ::SetTimer(NULL, 0, 1 * 60 * 1000, NULL);

	// Don't keep the connection open all the time...
	m_ftp->Disconnect();
	
	return TRUE;
}

int CDataImporter::ExitInstance()
{
	if(0 != m_nTimerID){
		KillTimer(NULL, m_nTimerID);
	}

	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CDataImporter, CWinThread)
	ON_THREAD_MESSAGE(WM_TIMER,				OnTimer)
	ON_THREAD_MESSAGE(WM_CHANGED_DATE,		OnChangedDate)
END_MESSAGE_MAP()


// CDataImporter message handlers

void CDataImporter::OnTimer(UINT nIDEvent, LPARAM lp){
	if(m_downloadedEvalLogs.GetCount() == 0){
		// Download all the eval-logs found in the given directory
		DownloadAllEvalLogs();
	}else{
		DownloadNewEvalLogs();
	}
}
void CDataImporter::OnChangedDate(UINT nIDEvent, LPARAM lp){
	// Make sure that the view is up to date with the data...
	pView->PostMessage(WM_CHANGED_DATE, 0, 0);


	// re-start with downloading all old eval-logs
	DownloadAllEvalLogs();
}

/** Downloads all the .txt files in the given directory */
void CDataImporter::DownloadAllEvalLogs(){
	CList <CString, CString &> fileNames;
	CString localFileName, dateStr, remoteDir, userMessage;
	CDateTime today, iterator;
	
	// the day we're looking for
	dateStr.Format("%02d%02d%02d", g_selectedDate.year % 100, g_selectedDate.month, g_selectedDate.day);

	// today
	today.SetToNow();

	// Clear out the list of already downloaded files 
	m_downloadedEvalLogs.RemoveAll();

	// Make sure that we have a directory to download to...
	CreateDirectoryStructure(m_localDataDirectory);
	
	// Remove all old files in m_localDataDirectory
	RemoveOldFiles(m_localDataDirectory);

	// Connect
	if(!m_ftp->Connect(m_ftpServer, m_ftpUserName, m_ftpPassword)){
		MessageBox(NULL, "Cannot connect to FTP-server", "Connection failed.", MB_OK);
		return;
	}
	
	// Loop through the different directories where we could find data
	//	from the given date.
	iterator = g_selectedDate;
	while(1){
		remoteDir.Format("/%s/%04d.%02d.%02d", g_volcanoes.m_simpleName[g_volcano], iterator.year, iterator.month, iterator.day);
	
		// Check if this directory exists at all...
		POSITION dPos = m_dateDirectories.GetHeadPosition();
		bool foundDateDirectory = false;
		while(dPos != NULL){
			CString &dateDir = m_dateDirectories.GetNext(dPos);
			if(Equals(dateDir, remoteDir.Right(10))){
				foundDateDirectory = true;
				dPos = NULL;
			}
		}
		if(foundDateDirectory){
			userMessage.Format("Checking directory %s", remoteDir);
			ShowMessage(userMessage);
			
			// Go to the correct directory
			m_ftp->EnterFolder(remoteDir);
		
			// Get the list of files here
			m_ftp->GetFileList(fileNames);
			
			// Download all the eval-logs
			POSITION fPos = fileNames.GetHeadPosition();
			while(fPos != NULL){
				CString &fileName = fileNames.GetNext(fPos);
				
				if(!IsEvaluationLogFile(fileName)){
					continue;
				}
				if(-1 == fileName.Find(dateStr))
					continue;

				// download the file
				localFileName.Format("%s\\%s", m_localDataDirectory, fileName);
				if(m_ftp->DownloadAFile(fileName, localFileName)){
					m_downloadedEvalLogs.AddTail(fileName);
					
					AppendToLargeEvalLog(localFileName);
					
					SendMessageOnDownloadedFile(localFileName);
				}
			}

			// Go back to the parent directory and try with another directory
			m_ftp->EnterFolder("..");			
		}else{
			// what to do if the date directory does not exist...
		}
		
		if(iterator.year > today.year)
			break;
		if(iterator.year == today.year && iterator.month == today.month && iterator.day == today.day)
			break; // quit the loop
		iterator.IncrementOneDay();
	}
	
	// Close the connection
	m_ftp->Disconnect();
	
	ShowMessage("All old evaluation log files downloaded");
}

/** Downloads the new .txt files in the given directory */
void CDataImporter::DownloadNewEvalLogs(){
	CList <CString, CString &> fileNames;
	CString localFileName, dateStr;

	// the day we're looking for
	dateStr.Format("%02d%02d%02d", g_selectedDate.year % 100, g_selectedDate.month, g_selectedDate.day);

	// Connect
	if(!m_ftp->Connect(m_ftpServer, m_ftpUserName, m_ftpPassword)){
		MessageBox(NULL, "Cannot connect to FTP-server", "Connection failed.", MB_OK);
		return;
	}
	
	// Go to the correct directory
	m_ftp->EnterFolder(m_remoteDirectory);
	
	// Get the list of files here
	m_ftp->GetFileList(fileNames);
	
	// Download all the eval-logs
	POSITION fPos = fileNames.GetHeadPosition();
	while(fPos != NULL){
		CString &fileName = fileNames.GetNext(fPos);
		
		if(!IsEvaluationLogFile(fileName)){
			continue;			
		}
		if(-1 == fileName.Find(dateStr))
			continue;

		// check if we have already downloaded this file...
		bool alreadyDownloaded = false;
		POSITION ePos = m_downloadedEvalLogs.GetHeadPosition();
		while(ePos != NULL){
			CString &downloadedFile = m_downloadedEvalLogs.GetNext(ePos);
			if(Equals(downloadedFile, fileName)){
				alreadyDownloaded = true;
				ePos = NULL;
			}
		}
		if(alreadyDownloaded)
			continue;

		// download the file
		localFileName.Format("%s\\%s", m_localDataDirectory, fileName);
		if(m_ftp->DownloadAFile(fileName, localFileName)){
			m_downloadedEvalLogs.AddTail(fileName);
			SendMessageOnDownloadedFile(localFileName);
		}
	}	
	
	// Close the connection
	m_ftp->Disconnect();
	
	ShowMessage("Done updating file list");
}

/** Sends the message to the window that a new file has been 
	downloaded. */
void CDataImporter::SendMessageOnDownloadedFile(const CString &fileName){
	FileHandler::CEvaluationLogFileHandler reader;
	reader.m_evaluationLog.Format(fileName);
	static CString serialNumber[256];
	static int srnIndex = 0;
	CDateTime date;
	int channel;
	
	if(!FileHandler::CEvaluationLogFileHandler::GetInfoFromFileName(fileName, date, serialNumber[srnIndex], channel))
		return;
	
	if(SUCCESS != reader.ReadEvaluationLog())
		return;
		
	Evaluation::CScanResult *result = new Evaluation::CScanResult();
	*result = reader.m_scan[0];
	
	pView->PostMessage(WM_EVAL_SUCCESS, (WPARAM)&(serialNumber[srnIndex]), (LPARAM)result);

	srnIndex = (srnIndex + 1) % 255;	
}


/** Checks if this file is an evaluation log file.
	The judgement is made from the file-name only. */
bool CDataImporter::IsEvaluationLogFile(const CString &fileName){
	CString serial;
	CDateTime startTime;
	int nUnderScores = 0;
	int pos = -1;
	int channel;

	if(!Equals(fileName.Right(4), ".txt"))
		return false;
		
	// there should be 3 underscores in the file-name, no more no less.
	while(-1 != (pos = fileName.Find("_", pos+1))){
		++nUnderScores;
	}
	if(nUnderScores != 3)
		return false;
	
	if(fileName.GetLength() > 30)
		return false;
	if(fileName.GetLength() < 19)
		return false;
		
	if(false == FileHandler::CEvaluationLogFileHandler::GetInfoFromFileName(fileName, startTime, serial, channel))
		return false;

	if(-1 != fileName.Find("Flux")){
		return false;
	}else if(-1 != fileName.Find("Geometry")){
		return false;
	}else  if(-1 != fileName.Find("Debug")){
		return false;
	}		
	
	return true;	
}


/** Clears the local directory of old eval-log files */
bool CDataImporter::RemoveOldFiles(const CString &localDir){
	WIN32_FIND_DATA FindFileData;
	char fileToFind[MAX_PATH];
	CString fileName;
		
	// Find all .txt-files in the specified directory

	sprintf(fileToFind, "%s\\*.txt", localDir);

	// Search for the file
	HANDLE hFile = FindFirstFile(fileToFind, &FindFileData);

	if(hFile == INVALID_HANDLE_VALUE)
		return false; // no files found

	do{
		if(IsEvaluationLogFile(FindFileData.cFileName) || (0 != strstr(FindFileData.cFileName, "EvalLog_"))){
			fileName.Format("%s\\%s", localDir, FindFileData.cFileName);
			DeleteFile(fileName);
		}
	}while(0 != FindNextFile(hFile, &FindFileData));

	FindClose(hFile);
	
	return true;
}

/** Takes a small eval-log and appends it to the end
	of a larger eval-log */
void CDataImporter::AppendToLargeEvalLog(const CString &localFileName){
	CString serial, largeEvalLog, fileName, directory;
	CDateTime date;
	char buffer[8192];
	int nCharsRead = 0;
	int channel;
	
	// separate the name into a file-name and a directory
	fileName.Format(localFileName);
	directory.Format(localFileName);
	Common::GetFileName(fileName);
	Common::GetDirectory(directory);
	
	FileHandler::CEvaluationLogFileHandler::GetInfoFromFileName(fileName, date, serial, channel);

	// Create the large eval-log
	largeEvalLog.Format("%s\\EvalLog_%s_%02d%02d%02d.txt", directory, serial, date.year % 100, date.month, date.day);

	// open the two files
	FILE *f_in = fopen(localFileName, "r");
	if(f_in == NULL)
		return;
	FILE *f_out = fopen(largeEvalLog, "a");
	if(f_out == NULL)
		return;

	// copy the contents from f_in to f_out
	while(0 < (nCharsRead = fread(buffer, sizeof(char), 8191, f_in))){
		fwrite(buffer, sizeof(char), nCharsRead, f_out);
	}
	fprintf(f_out, "\n");

	// remember to close the files
	fclose(f_in);
	fclose(f_out);
}
