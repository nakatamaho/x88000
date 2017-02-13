////////////////////////////////////////////////////////////
// X88000 Printer Dialog
//
// Written by Manuke

#ifndef X88PrinterDlg_DEFINED
#define X88PrinterDlg_DEFINED

////////////////////////////////////////////////////////////
// declare

class CX88PrinterDlg;

////////////////////////////////////////////////////////////
// include

#include "ParallelPrinter.h"

#include "X88Dialog.h"
#include "X88PrinterPreviewWnd.h"

////////////////////////////////////////////////////////////
// declaration of CX88PrinterDlg

class CX88PrinterDlg :
	public CX88Dialog
{
// attribute
protected:
	// printer
	CParallelPrinter* m_pPrinter;
	// preview window
	static CX88PrinterPreviewWnd m_wndPreview;
	// target printer
	static CParallelPrinter* m_pTargetPrinter;
	// changed paper
	static bool m_bChangedPaper;
	// combobox list popupped
	static bool m_bPopuppedComboList;

// create & destroy
public:
	// standard constructor
	CX88PrinterDlg(CParallelPrinter* pPrinter);
	// destructor
	virtual ~CX88PrinterDlg();

// implementation
protected:

#ifdef X88_GUI_WINDOWS

	// dialog procedure
	static BOOL CALLBACK DlgProc(
		HWND hdlg, UINT nMessage, WPARAM wParam, LPARAM lParam);

#elif defined(X88_GUI_GTK)

	// dialog procedure(initialize & dispose)
	static bool DlgProc(
		GtkWidget* pDialog,
		bool bInitialize,
		int nID);
	// paper combobox changed signal
	static void OnChangedSignalPaperCombo(
		GtkWidget* pWidget, gpointer pData);
	// paper center toggled signal
	static void OnToggledSignalPaperCenter(
		GtkToggleButton* pToggleButton, gpointer pData);
	// page number changed signal
	static void OnValueChangedSignalPageNo(
		GtkAdjustment* pAdjustment, gpointer pData);
	// printer operation clicked signal
	static void OnClickedSignalPrinterOperate(
		GtkButton* pButton, gpointer pData);

#endif // X88_GUI

// operation
protected:
	// set children
	static void SetChildren(
		CX88WndHandle hdlg,
		bool bPageReset, bool bPageRebuild);
	// change paper
	static void DoPrinterPaper(CX88WndHandle hdlg);
	// set paper to center
	static void DoPrinterPaperCenter(CX88WndHandle hdlg);
	// change page number
	static void DoPrinterPage(CX88WndHandle hdlg);
	// copy
	static void DoPrinterCopy(CX88WndHandle hdlg);
	// delete paper
	static void DoPrinterPaperDelete(CX88WndHandle hdlg);
	// feed paper
	static void DoPrinterPaperFeed(CX88WndHandle hdlg);
	// reset
	static void DoPrinterReset(CX88WndHandle hdlg);

public:
	// create modal dialog
	virtual int DoModal();
};

#endif // X88PrinterDlg_DEFINED
