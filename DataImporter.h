#pragma once

#include "Communication/FTPCom.h"

/** The class <b>CDataImporter</b> is the main thread in the NovacViewer.
	This is the class that takes care of downloading data from the FTP - server
	and sends it to the interface.
*/
class CDataImporter : public CWinThread
{
	DECLARE_DYNCREATE(CDataImporter)

protected:
	CDataImporter();           // protected constructor used by dynamic creation
	virtual ~CDataImporter();

public:
	bool m_fInitialized;
	
	CString m_volcano;

	virtual BOOL InitInstance();
	virtual int ExitInstance();

protected:
	DECLARE_MESSAGE_MAP()
	
	// --------------- PROTECTED VARIABLES ---------------------
	/** The ftp-connection */
	Communication::CFTPCom* m_ftp;

	CString m_ftpServer;
	CString m_ftpUserName;
	CString m_ftpPassword;


	/** The path to the executable */
	CString m_exePath;

	/** Timer */
	UINT_PTR m_nTimerID;
	
	/** The directory we're to get the data from */
	CString m_remoteDirectory;
	
	/** The directory that we download the data to */
	CString m_localDataDirectory;
	
	/** The list of files that we have downloaded so far */
	CList <CString, CString&> m_downloadedEvalLogs;

	/** The list of dates when data has been uploaded to the 
		server for the volcano that we are looking at. 
		This is a list of the directories under the volcano
		directory. */
	CList <CString, CString &> m_dateDirectories;

public:

	// ----------------------------------------------------------------------
	// --------------------- PUBLIC METHODS ---------------------------------
	// ----------------------------------------------------------------------

	/** */
	afx_msg void OnTimer(UINT nIDEvent, LPARAM lp);
	afx_msg void OnChangedDate(UINT nIDEvent, LPARAM lp);
	
	/** Downloads all the .txt files in the current 'm_remoteDirectory' 
		This will also clear the list 'm_downloadedEvalLogs'
	*/
	void DownloadAllEvalLogs();
	
	/** Downloads the new .txt files in the current 'm_remoteDirectory' */
	void DownloadNewEvalLogs();

private:

	// ----------------------------------------------------------------------
	// --------------------- PRIVATE METHODS --------------------------------
	// ----------------------------------------------------------------------
	
	/** Sends the message to the window that a new file has been 
		downloaded. */
	void SendMessageOnDownloadedFile(const CString &fileName);
	
	/** Checks if this file is an evaluation log file.
		The judgement is made from the file-name only. */
	bool IsEvaluationLogFile(const CString &fileName);
	
	/** Clears the local directory of old eval-log files */
	bool RemoveOldFiles(const CString &localDir);
	
	/** Takes a small eval-log and appends it to the end
		of a larger eval-log */
	void AppendToLargeEvalLog(const CString &localFileName);
};


