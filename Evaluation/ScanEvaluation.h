#pragma once

#include "Evaluation.h"
#include "ScanResult.h"

#include "../Common/Common.h"
#include "../Common/Spectra/ScanFileHandler.h"
#include "../Common/LogFileWriter.h"
#include "../Configuration/Configuration.h"

namespace Evaluation
{

	/** 
		An object of the <b>CScanEvaluation</b>-class handles the evaluation of one
		scan.
	*/
	class CScanEvaluation
	{

	public:
		/** Default constructor */
		CScanEvaluation(void);

		/** Default destructor */
		~CScanEvaluation(void);

		/** If the contents of m_pause is true then the current thread will 
			be suspended between each iteration. Useful for reevaluation as it
			lets the user have a look at each fitted spectrum before continuing. */
		int *m_pause;

		/** The state of the evaluation. If m_sleeping is true then the
			thread is sleeping and needs to be woken up. */
		bool *m_sleeping;

		/** if pView != NULL then after the evaluation of a spectrum, a 'WM_EVAL_SUCCESS'
			message will be sent to pView. */
		CWnd *pView;

		/** If the pView != NULL then these spectra will be filled in after each evaluation
			and the address of the first spectrum in the array will be sent 
			with the 'WM_EVAL_SUCCESS' message to the pView window. 
			The first spectrum in the array defines the last read spectrum. 
			The second spectrum is the residual of the fit, 
			The third spectrum is the fitted polynomial, and the following
				MAX_N_REFERENCES + 1 spectra are the scaled reference spectra used in the fit. */
		CSpectrum m_spec[MAX_N_REFERENCES + 4];

		/** The evaluation results from the last scan evaluated */
		CScanResult *m_result;

		/** Called to evaluate one scan.
				@return the number of spectra evaluated. */
		long EvaluateScan(const CString &scanfile, CEvaluation *evaluator, bool *fRun = NULL, const CConfigurationSetting::DarkSettings *darkSettings = NULL);

		/** Setting the option for how to get the sky spectrum.
			@param skySpecPath - if not null and skyOption == SKY_USER, then this string will be used
				as sky-spectrum. 
			If skyOption==SKY_USER and skySpecPath ==NULL then SKY_FIRST will be
				used instead*/
		void SetOption_Sky(SKY_OPTION skyOption, long skyIndex, const CString *skySpecPath = NULL);

		/** Setting the option for which spectra to ignore */
		void SetOption_Ignore(IgnoreOption lowerLimit, IgnoreOption upperLimit);

		/** Setting the option for wheather the spectra are averaged or not. */
		void SetOption_AveragedSpectra(bool averaged);
	private:

		// ----------------------- PRIVATE METHODS ---------------------------

		/** This returns the sky spectrum that is to be used in the fitting. */
		RETURN_CODE GetSky(FileHandler::CScanFileHandler *scan, CSpectrum &sky);

		/** This returns the dark spectrum that is to be used in the fitting. 
			@param scan - the scan-file handler from which to get the dark spectrum
			@param spec - the spectrum for which the dark spectrum should be retrieved
			@param dark - will on return be filled with the dark spectrum 
			@param darkSettings - the settings for how to get the dark spectrum from this spectrometer */
		RETURN_CODE GetDark(FileHandler::CScanFileHandler *scan, const CSpectrum &spec, CSpectrum &dark, const CConfigurationSetting::DarkSettings *darkSettings = NULL);

		/** checks the spectrum to the settings and returns 'true' if the spectrum should not be evaluated */
		bool Ignore(const CSpectrum &spec, const CFitWindow window);

		/** This function updates the 'm_residual' and 'm_fitResult' spectra
			and sends the 'WM_EVAL_SUCCESS' message to the pView-window. */
		void ShowResult(const CSpectrum &spec, const CEvaluation *eval, long curSpecIndex, long specNum);

		/** Includes the sky spectrum into the fitting. The dark-spectrum
			should already have been removed from the 'sky' */
		bool IncludeSkySpecInFit(CEvaluation *eval, const CSpectrum &skySpectrum, CFitWindow &window);

		/** Finds the optimum shift and squeeze for an evaluated scan 
					by looking at the spectrum with the highest absorption of the evaluated specie
					and evaluate it with shift and squeeze free */
		void FindOptimumShiftAndSqueeze(CEvaluation *eval, FileHandler::CScanFileHandler *scan, CScanResult *result);

		/** Finds the optimum shift and squeeze for an scan by evaluating
			with a solar-reference spectrum and studying the shift of the 
			Fraunhofer-lines. 
			@param eval - the evaluator to use for the evaluation. On successful determination
				of the shift and squeeze then the shift and squeeze of the reference-files
				in the CEvaluation-objects CFitWindow will be fixed to the optimum value found
			@param scan - a handle to the spectrum file. */
		void FindOptimumShiftAndSqueeze_Fraunhofer(CEvaluation *eval, FileHandler::CScanFileHandler *scan);

		// ------------------------ THE PARAMETERS FOR THE EVALUATION ------------------
		/** This is the options for the sky spectrum */
		SKY_OPTION  m_skyOption;
		long        m_skyIndex;
		CString     m_userSkySpectrum; // the sky-spectrum, only used if m_skyOption is SKY_USER

		/** The options for which spectra to ignore */
		IgnoreOption m_ignore_Lower;
		IgnoreOption m_ignore_Upper;

		/** This is the fit region */
		long m_fitLow;
		long m_fitHigh;

		/** True if the spectra are averaged, not summed */
		bool m_averagedSpectra;

		/** Remember the index of the spectrum with the highest absorption, to be able to
			adjust the shift and squeeze with it later */
		int m_indexOfMostAbsorbingSpectrum;

		/** how many spectra there are in the current scan-file (for showing the progress) */
		long m_prog_SpecNum;

		/** which spectrum we are on in the current scan-file (for showing the progress) */
		long m_prog_SpecCur;
	};
}