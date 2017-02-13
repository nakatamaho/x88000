////////////////////////////////////////////////////////////
// X88000 Printer Dialog
//
// Written by Manuke

////////////////////////////////////////////////////////////
// include

#include "StdHeader.h"

#include "X88PrinterDlg.h"

#include "X88PrinterCopyDlg.h"
#include "X88PrinterPaperDelDlg.h"
#include "X88PrinterPaperFeedDlg.h"
#include "X88PrinterDrawer.h"
#include "X88FileDlg.h"
#include "X88Resource.h"
#include "X88Utility.h"

using namespace NX88Utility;

////////////////////////////////////////////////////////////
// implementation of CX88PrinterDlg

////////////////////////////////////////////////////////////
// attribute

// preview window

CX88PrinterPreviewWnd CX88PrinterDlg::m_wndPreview;

// target printer

CParallelPrinter* CX88PrinterDlg::m_pTargetPrinter = NULL;

// changed paper

bool CX88PrinterDlg::m_bChangedPaper;

// popupped list of combobox

bool CX88PrinterDlg::m_bPopuppedComboList;

////////////////////////////////////////////////////////////
// create & destroy

// standard constructor

CX88PrinterDlg::CX88PrinterDlg(CParallelPrinter* pPrinter) :
	CX88Dialog(NULL),
	m_pPrinter(pPrinter)
{
	m_bChangedPaper = false;
	m_bPopuppedComboList = false;
}

// destructor

CX88PrinterDlg::~CX88PrinterDlg() {
}

////////////////////////////////////////////////////////////
// implementation

#ifdef X88_GUI_WINDOWS

// dialog procedure

BOOL CALLBACK CX88PrinterDlg::DlgProc(
	HWND hdlg, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	BOOL bResult = TRUE;
	switch (nMessage) {
	case WM_INITDIALOG:
		{ // dummy block
			SendDlgItemMessage(
				hdlg, IDC_PRINTER_PAPER,
				CB_SETEXTENDEDUI, TRUE, 0);
			HWND hStatic = GetDlgItem(hdlg, IDC_PRINTER_PREVIEW);
			if (hStatic != NULL) {
				HWND hLabel = GetWindow(hStatic, GW_HWNDPREV);
				RECT rectLimit;
				GetWindowRect(hStatic, &rectLimit);
				ScreenToClient(hdlg, (POINT*)&rectLimit);
				ScreenToClient(hdlg, (POINT*)&rectLimit+1);
				DestroyWindow(hStatic);
				m_wndPreview.SetTargetPrinter(m_pTargetPrinter);
				m_wndPreview.Create(
					rectLimit.left, rectLimit.top,
					rectLimit.right, rectLimit.bottom,
					hdlg, hLabel, IDC_PRINTER_PREVIEW);
			}
			Centering(hdlg);
			SetChildren(hdlg, true, true);
		}
		break;
	case WM_DESTROY:
		m_wndPreview.Destroy();
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
		case IDCANCEL:
			EndDialog(hdlg, LOWORD(wParam));
			break;
		case IDC_PRINTER_PAPER:
			switch (HIWORD(wParam)) {
			case CBN_DROPDOWN:
				m_bPopuppedComboList = true;
				m_bChangedPaper = false;
				break;
			case CBN_SELENDOK:
			case CBN_SELENDCANCEL:
				m_bPopuppedComboList = false;
				if (m_bChangedPaper) {
					m_bChangedPaper = false;
					DoPrinterPaper(hdlg);
				}
				break;
			case CBN_SELCHANGE:
				if (m_bPopuppedComboList) {
					m_bChangedPaper = true;
				} else {
					DoPrinterPaper(hdlg);
				}
				break;
			}
			break;
		case IDC_PRINTER_PAPER_CENTER:
			if (HIWORD(wParam) == BN_CLICKED) {
				DoPrinterPaperCenter(hdlg);
			}
			break;
		case IDC_PRINTER_PAGE:
			if (HIWORD(wParam) == EN_KILLFOCUS) {
				DoPrinterPage(hdlg);
			}
			break;
		case IDC_PRINTER_COPY:
			if (HIWORD(wParam) == BN_CLICKED) {
				DoPrinterCopy(hdlg);
			}
			break;
		case IDC_PRINTER_PAPER_DELETE:
			if (HIWORD(wParam) == BN_CLICKED) {
				DoPrinterPaperDelete(hdlg);
			}
			break;
		case IDC_PRINTER_PAPER_FEED:
			if (HIWORD(wParam) == BN_CLICKED) {
				DoPrinterPaperFeed(hdlg);
			}
			break;
		case IDC_PRINTER_RESET:
			if (HIWORD(wParam) == BN_CLICKED) {
				DoPrinterReset(hdlg);
			}
			break;
		}
		break;
	case WM_VSCROLL:
		switch (GetDlgCtrlID((HWND)lParam)) {
		case IDC_PRINTER_PAGE_SPIN:
			DoPrinterPage(hdlg);
			break;
		}
		break;
	default:
		bResult = FALSE;
		break;
	}
	return bResult;
}

#elif defined(X88_GUI_GTK)

// dialog procedure(initialize & dispose)

bool CX88PrinterDlg::DlgProc(
	GtkWidget* pDialog,
	bool bInitialize,
	int nID)
{
	if (bInitialize) {
		if (!BuildDialog(pDialog, nID)) {
			return false;
		}
		m_wndPreview.SetTargetPrinter(m_pTargetPrinter);
		m_wndPreview.Create(
			0, 0,
			0, 0,
			GetDlgItem(pDialog, NX88Resource::IDC_PRINTER_PREVIEW_ALIGN),
			GetDlgItem(pDialog, IDC_PRINTER_PREVIEW),
			0);
		SetChildren(pDialog, true, false);
		GtkWidget* pcomboPaper = GetDlgItem(pDialog, IDC_PRINTER_PAPER);
		if (pcomboPaper != NULL) {
			g_signal_connect(
				pcomboPaper, "changed",
				G_CALLBACK(OnChangedSignalPaperCombo),
				pDialog);
		}
		g_signal_connect(
			GetDlgItem(pDialog, IDC_PRINTER_PAPER_CENTER), "toggled",
			G_CALLBACK(OnToggledSignalPaperCenter),
			pDialog);
		GtkWidget* pspinPage = GetDlgItem(pDialog, IDC_PRINTER_PAGE_SPIN);
		if (pspinPage != NULL) {
			GtkAdjustment* pAdjustment =
				gtk_spin_button_get_adjustment(
					GTK_SPIN_BUTTON(pspinPage));
			g_signal_connect(
				pAdjustment, "value_changed",
				G_CALLBACK(OnValueChangedSignalPageNo),
				pDialog);
		}
		g_signal_connect(
			GetDlgItem(pDialog, IDC_PRINTER_COPY), "clicked",
			G_CALLBACK(OnClickedSignalPrinterOperate),
			pDialog);
		g_signal_connect(
			GetDlgItem(pDialog, IDC_PRINTER_PAPER_DELETE), "clicked",
			G_CALLBACK(OnClickedSignalPrinterOperate),
			pDialog);
		g_signal_connect(
			GetDlgItem(pDialog, IDC_PRINTER_PAPER_FEED), "clicked",
			G_CALLBACK(OnClickedSignalPrinterOperate),
			pDialog);
		g_signal_connect(
			GetDlgItem(pDialog, IDC_PRINTER_RESET), "clicked",
			G_CALLBACK(OnClickedSignalPrinterOperate),
			pDialog);
	}
	return true;
}

// paper combobox changed signal

void CX88PrinterDlg::OnChangedSignalPaperCombo(
	GtkWidget* /*pWidget*/, gpointer pData)
{
	GtkWidget* pDialog = GTK_WIDGET(pData);
	DoPrinterPaper(pDialog);
}

// paper center toggled signal

void CX88PrinterDlg::OnToggledSignalPaperCenter(
	GtkToggleButton* /*pToggleButton*/, gpointer pData)
{
	GtkWidget* pDialog = GTK_WIDGET(pData);
	DoPrinterPaperCenter(pDialog);
}

// page number changed signal

void CX88PrinterDlg::OnValueChangedSignalPageNo(
	GtkAdjustment* /*pAdjustment*/, gpointer pData)
{
	GtkWidget* pDialog = GTK_WIDGET(pData);
	DoPrinterPage(pDialog);
}

// printer operation clicked signal

void CX88PrinterDlg::OnClickedSignalPrinterOperate(
	GtkButton* pButton, gpointer pData)
{
	GtkWidget* pDialog = GTK_WIDGET(pData);
	int nID = GetDlgItemID(
		pDialog, GTK_WIDGET(pButton));
	switch (nID) {
	case IDC_PRINTER_COPY:
		DoPrinterCopy(pDialog);
		break;
	case IDC_PRINTER_PAPER_DELETE:
		DoPrinterPaperDelete(pDialog);
		break;
	case IDC_PRINTER_PAPER_FEED:
		DoPrinterPaperFeed(pDialog);
		break;
	case IDC_PRINTER_RESET:
		DoPrinterReset(pDialog);
		break;
	}
}

#endif // X88_GUI

////////////////////////////////////////////////////////////
// operation

// set children

void CX88PrinterDlg::SetChildren(
	CX88WndHandle hdlg,
	bool bPageReset, bool bPageRebuild)
{
	SetStaticText(
		GetDlgItem(hdlg, IDC_PRINTER_NAME),
		NX88Resource::GetParallelDeviceNameFromID(
			m_pTargetPrinter->GetDeviceID()));
	bool bComboSet = (GetComboBoxItemCount(
		GetDlgItem(hdlg, IDC_PRINTER_PAPER)) <= 0);
	if (bComboSet) {
		ClearComboBox(
			GetDlgItem(hdlg, IDC_PRINTER_PAPER));
	}
	int nPaperCount = m_pTargetPrinter->GetSelectablePaperCount(),
		nCurrentIndex = 0;
	for (int nIndex = 0; nIndex < nPaperCount; nIndex++) {
		const char* psszPaperName = "";
		int nPaper = m_pTargetPrinter->GetSelectablePaper(nIndex);
		if (nPaper == m_pTargetPrinter->GetSelectedPaper()) {
			nCurrentIndex = nIndex;
		}
		if (bComboSet) {
			switch (nPaper) {
			case CParallelPrinter::PAPER_C10:
				psszPaperName = "10 inch continuous";
				break;
			case CParallelPrinter::PAPER_C15:
				psszPaperName = "15 inch continuous";
				break;
			case CParallelPrinter::PAPER_A5V:
				psszPaperName = "A5 Portrait";
				break;
			case CParallelPrinter::PAPER_A5H:
				psszPaperName = "A5 Landscape";
				break;
			case CParallelPrinter::PAPER_A4V:
				psszPaperName = "A4 Portrait";
				break;
			case CParallelPrinter::PAPER_A4H:
				psszPaperName = "A4 Landscape";
				break;
			case CParallelPrinter::PAPER_A3V:
				psszPaperName = "A3 Portrait";
				break;
			case CParallelPrinter::PAPER_A3H:
				psszPaperName = "A3 Landscape";
				break;
			case CParallelPrinter::PAPER_B5V:
				psszPaperName = "B5 Portrait";
				break;
			case CParallelPrinter::PAPER_B5H:
				psszPaperName = "B5 Landscape";
				break;
			case CParallelPrinter::PAPER_B4V:
				psszPaperName = "B4 Portrait";
				break;
			case CParallelPrinter::PAPER_B4H:
				psszPaperName = "B4 Landscape";
				break;
			case CParallelPrinter::PAPER_HAGAKIV:
				psszPaperName = "Postcard Portrait";
				break;
			case CParallelPrinter::PAPER_HAGAKIH:
				psszPaperName = "Postcard Landscape";
				break;
			}
			AddComboBoxItem(
				GetDlgItem(hdlg, IDC_PRINTER_PAPER),
				RSTR(psszPaperName));
		}
	}
	SetComboBoxSel(
		GetDlgItem(hdlg, IDC_PRINTER_PAPER),
		nCurrentIndex);
	CX88WndHandle hwndPaperCenter = GetDlgItem(
		hdlg, IDC_PRINTER_PAPER_CENTER);
	if (hwndPaperCenter != NULL) {
		EnableControl(
			hwndPaperCenter,
			m_pTargetPrinter->IsAblePaperCentering()?
				TRUE: FALSE);
		SetCheckButtonChecked(
			hwndPaperCenter,
			m_pTargetPrinter->IsPaperCentering());
	}
	int nPageCount = max_value(
			m_pTargetPrinter->GetCurrentPage()+1,
			(int)m_pTargetPrinter->size()),
		nCurPage = 0;
	if (!bPageReset) {
		nCurPage = m_wndPreview.GetTargetPage();
		if (nCurPage < 0) {
			nCurPage = 0;
		} else if (nCurPage >= nPageCount) {
			nCurPage = nPageCount-1;
		}
	}
	SetSpinButtonRange(
		GetDlgItem(hdlg, IDC_PRINTER_PAGE_SPIN),
		1, nPageCount);
	SetSpinButtonValue(
		GetDlgItem(hdlg, IDC_PRINTER_PAGE),
		GetDlgItem(hdlg, IDC_PRINTER_PAGE_SPIN),
		nCurPage+1);
	SetStaticText(
		GetDlgItem(hdlg, IDC_PRINTER_PAGE_MAX),
		FormatStr("/%d", nPageCount));
	m_wndPreview.SetTargetPage(nCurPage);
	if (bPageRebuild) {
		m_wndPreview.RebuildPage();
	}
}

// change paper

void CX88PrinterDlg::DoPrinterPaper(CX88WndHandle hdlg) {
	int nIndex = GetComboBoxSel(
			GetDlgItem(hdlg, IDC_PRINTER_PAPER)),
		nPaper = CParallelPrinter::PAPER_NONE;
	if (nIndex >= 0) {
		nPaper = m_pTargetPrinter->GetSelectablePaper(nIndex);
		if (nPaper == m_pTargetPrinter->GetSelectedPaper()) {
			nPaper = CParallelPrinter::PAPER_NONE;
		}
	}
	if ((nPaper != CParallelPrinter::PAPER_NONE) &&
		(m_pTargetPrinter->size() > 0))
	{
		if (ShowMessageBox(
				hdlg,
				MB_ICONEXCLAMATION, MB_OKCANCEL,
				RSTR("Select Paper"),
				RSTR("Delete paper?")) != IDOK)
		{
			nPaper = CParallelPrinter::PAPER_NONE;
		}
	}
	if ((nPaper != CParallelPrinter::PAPER_NONE) &&
		m_pTargetPrinter->SelectPaper(nPaper))
	{
		m_wndPreview.ChangePaper();
		SetChildren(hdlg, true, true);
	} else {
		SetChildren(hdlg, false, false);
	}
}

// set paper to center

void CX88PrinterDlg::DoPrinterPaperCenter(CX88WndHandle hdlg) {
	bool bCentering = IsCheckButtonChecked(
		GetDlgItem(hdlg, IDC_PRINTER_PAPER_CENTER));
	if (m_pTargetPrinter->IsAblePaperCentering() &&
		(bCentering != m_pTargetPrinter->IsPaperCentering()))
	{
		m_pTargetPrinter->SetPaperCentering(bCentering);
		SetChildren(hdlg, false, true);
	}
}

// change page number

void CX88PrinterDlg::DoPrinterPage(CX88WndHandle hdlg) {
	int nPage = GetSpinButtonValue(
			GetDlgItem(hdlg, IDC_PRINTER_PAGE),
			GetDlgItem(hdlg, IDC_PRINTER_PAGE_SPIN))-1,
		nPageCount = max_value(
			m_pTargetPrinter->GetCurrentPage()+1,
			(int)m_pTargetPrinter->size());
	if (nPage < 0) {
		nPage = 0;
	} else if (nPage >= nPageCount) {
		nPage = nPageCount-1;
	}
	if (nPage != m_wndPreview.GetTargetPage()) {
		m_wndPreview.SetTargetPage(nPage);
		SetChildren(hdlg, false, true);
	}
}

// copy

void CX88PrinterDlg::DoPrinterCopy(CX88WndHandle hdlg) {
	CX88PrinterCopyDlg dlgCopy(hdlg);
	dlgCopy.SetPageCount(
		max_value(
			m_pTargetPrinter->GetCurrentPage()+1,
			(int)m_pTargetPrinter->size()));
	dlgCopy.SetCopyPageNo(
		m_wndPreview.GetTargetPage());
	dlgCopy.SetExistSheetFeeder(
		m_pTargetPrinter->IsExistSheetFeeder());
	if (dlgCopy.DoModal() != IDOK) {
		return;
	}
	CX88PrinterDrawer prtdrawer;
	int nPage = dlgCopy.GetCopyPageNo();

#ifdef X88_GUI_WINDOWS

	if (dlgCopy.IsCopyMetafile()) {
		HENHMETAFILE hEnhMeta = NULL;
		HGLOBAL hWinMeta = NULL;
		HDC hdc = GetDC(hdlg);
		if (hdc != NULL) {
			int nDPI = m_pTargetPrinter->GetDPI(),
				nPaperWidth = m_pTargetPrinter->GetPaperWidth(),
				nPaperHeight = m_pTargetPrinter->GetPaperHeight();
			RECT rectBound;
			rectBound.left = 0;
			rectBound.top = 0;
			rectBound.right = (nPaperWidth*2540+nDPI-1)/nDPI;
			rectBound.bottom = (nPaperHeight*2540+nDPI-1)/nDPI;
			if (dlgCopy.GetZoomOut() > 0) {
				rectBound.right >>= dlgCopy.GetZoomOut();
				rectBound.bottom >>= dlgCopy.GetZoomOut();
			}
			POINT ptViewportExt;
			ptViewportExt.x = rectBound.right;
			ptViewportExt.y = rectBound.bottom;
			SetMapMode(hdc, MM_HIMETRIC);
			LPtoDP(hdc, &ptViewportExt, 1);
			ptViewportExt.x = abs(ptViewportExt.x);
			ptViewportExt.y = abs(ptViewportExt.y);
			// stop uploading Windows-metafile(^^;)
			for (int i = 0; i < 1; i++) {
				HDC hdcMeta;
				if (i == 0) {
					// enhanced-metafile
					rectBound.right++;
					rectBound.bottom++;
					hdcMeta = CreateEnhMetaFile(
						hdc, NULL, &rectBound, "X88000\0PC-PR201\0");
					rectBound.right--;
					rectBound.bottom--;
					if (hdcMeta != NULL) {
						SetMapMode(hdcMeta, MM_ANISOTROPIC);
						SetViewportOrgEx(
							hdcMeta,
							0, 0,
							NULL);
						SetViewportExtEx(
							hdcMeta,
							ptViewportExt.x, ptViewportExt.y,
							NULL);
					}
				} else {
					// Windows-metafile
					hdcMeta = CreateMetaFile(NULL);
				}
				if (hdcMeta == NULL) {
					continue;
				}
				SetWindowOrgEx(
					hdcMeta,
					0, 0,
					NULL);
				SetWindowExtEx(
					hdcMeta,
					nPaperWidth, nPaperHeight,
					NULL);
				if (dlgCopy.IsDrawBorder()) {
					prtdrawer.DrawPaperBorder(
						hdcMeta,
						m_pTargetPrinter, nPage,
						NULL);
				}
				if (dlgCopy.IsDrawSprocketHole()) {
					prtdrawer.DrawSprocketHole(
						hdcMeta,
						m_pTargetPrinter, nPage,
						NULL);
				}
				prtdrawer.DrawPrinterObject(
					hdcMeta,
					m_pTargetPrinter, nPage,
					NULL);
				if (i == 0) {
					// enhanced-metafile
					hEnhMeta = CloseEnhMetaFile(hdcMeta);
				} else {
					// Windows-metafile
					HMETAFILE hMeta = CloseMetaFile(hdcMeta);
					if (hMeta != NULL) {
						bool bSet = false;
						hWinMeta = GlobalAlloc(GHND, sizeof(METAFILEPICT));
						if (hWinMeta != NULL) {
							METAFILEPICT* pWinMeta =
								(METAFILEPICT*)GlobalLock(hWinMeta);
							if (pWinMeta != NULL) {
								pWinMeta->mm = MM_ANISOTROPIC;
								pWinMeta->xExt = rectBound.right;
								pWinMeta->yExt = rectBound.bottom;
								pWinMeta->hMF = hMeta;
								GlobalUnlock(hWinMeta);
								bSet = true;
							}
						}
						if (!bSet) {
							DeleteMetaFile(hMeta);
						}
					}
				}
			}
			ReleaseDC(hdlg, hdc);
		}
		if ((hEnhMeta != NULL) ||
			(hWinMeta != NULL))
		{
			if (OpenClipboard(hdlg)) {
				EmptyClipboard();
				if (hEnhMeta != NULL) {
					SetClipboardData(CF_ENHMETAFILE, hEnhMeta);
				}
				if (hWinMeta != NULL) {
					SetClipboardData(CF_METAFILEPICT, hWinMeta);
				}
				CloseClipboard();
			} else {
				if (hEnhMeta != NULL) {
					DeleteEnhMetaFile(hEnhMeta);
				}
				if (hWinMeta != NULL) {
					METAFILEPICT* pWinMeta =
						(METAFILEPICT*)GlobalLock(hWinMeta);
					if (pWinMeta != NULL) {
						DeleteMetaFile(pWinMeta->hMF);
						GlobalUnlock(hWinMeta);
					}
					GlobalFree(hWinMeta);
				}
			}
		}
	} else {
		std::string jstrText;
		prtdrawer.ExtractText(
			m_pTargetPrinter, nPage,
			jstrText, true);
		HGLOBAL hgText = GlobalAlloc(GHND, jstrText.size()+1);
		if (hgText != NULL) {
			char* pszBuffer = (char*)GlobalLock(hgText);
			if (pszBuffer != NULL) {
				strcpy(pszBuffer, jstrText.c_str());
				GlobalUnlock(hgText);
				if (OpenClipboard(hdlg)) {
					EmptyClipboard();
					SetClipboardData(CF_TEXT, hgText);
					CloseClipboard();
				} else {
					GlobalFree(hgText);
				}
			}
		}
	}

#elif defined(X88_GUI_GTK)

#ifdef X88_PRINTER_DRAW_POSTSCRIPT

	CX88FileDlg dlgFile(
		hdlg,
		false,
		RSTR(
			dlgCopy.IsCopyMetafile()?
				"Write PostScript":
				"Write Text"),
		RSTR(
			dlgCopy.IsCopyMetafile()?
				"PostScript(ps)|*.ps||":
				"Text(txt)|*.txt||"),
		dlgCopy.IsCopyMetafile()?
			"ps":
			"txt");
	if (dlgFile.DoModal() != IDOK) {
		return;
	}
	FILE* pFile = NULL;
	try {
		pFile = fopen(dlgFile.GetFileName().c_str(), "w");
		if (pFile == NULL) {
			throw 1;
		}
		if (dlgCopy.IsCopyMetafile()) {
			if (!prtdrawer.InitializePostScript(
					pFile,
					m_pTargetPrinter,
					dlgCopy.GetZoomOut()))
			{
				throw 11;
			}
			if (dlgCopy.IsDrawBorder()) {
				if (!prtdrawer.DrawPaperBorderToPostScript(
						pFile,
						m_pTargetPrinter, nPage))
				{
					throw 12;
				}
			}
			if (dlgCopy.IsDrawSprocketHole()) {
				if (!prtdrawer.DrawSprocketHoleToPostScript(
						pFile,
						m_pTargetPrinter, nPage))
				{
					throw 13;
				}
			}
			if (!prtdrawer.DrawPrinterObjectToPostScript(
					pFile,
					m_pTargetPrinter, nPage))
			{
				throw 14;
			}
			if (!prtdrawer.FinalizePostScript(
					pFile,
					m_pTargetPrinter))
			{
				throw 15;
			}
		} else {
			std::string jstrText;
			prtdrawer.ExtractText(
				m_pTargetPrinter, nPage,
				jstrText, true);
			std::string lstrText =
				ConvUTF8toLOC(ConvSJIStoUTF8(jstrText));
			if (fwrite(
					lstrText.c_str(), 1, lstrText.size(), pFile) !=
						lstrText.size())
			{
				throw 22;
			}
		}
		if (ferror(pFile) != 0) {
			throw 31;
		}
		if (fflush(pFile) != 0) {
			throw 32;
		}
	} catch (int /* nError*/) {
		ShowMessageBox(
			hdlg,
			MB_ICONEXCLAMATION, MB_OK,
			RSTR("Printing Image"),
			RSTR("An error occurred while writing."));
	}
	if (pFile != NULL) {
		fclose(pFile);
	}

#elif defined(X88_PRINTER_DRAW_CAIRO)

	CX88FileDlg dlgFile(
		hdlg,
		false,
		RSTR(
			dlgCopy.IsCopyMetafile()?
				"Write PDF":
				"Write Text"),
		RSTR(
			dlgCopy.IsCopyMetafile()?
				"PDF(pdf)|*.pdf||":
				"Text(txt)|*.txt||"),
		dlgCopy.IsCopyMetafile()?
			"pdf":
			"txt");
	if (dlgFile.DoModal() != IDOK) {
		return;
	}
	if (dlgCopy.IsCopyMetafile()) {
		cairo_surface_t* pCairoSutface = NULL;
		cairo_t* pCairo = NULL;
		try {
			int nZoomOut = dlgCopy.GetZoomOut(),
				nDPI = m_pTargetPrinter->GetDPI(),
				nWidth = m_pTargetPrinter->GetPaperWidth(),
				nHeight = m_pTargetPrinter->GetPaperHeight();
			double fWidthX = (nWidth*72.0)/(nDPI << nZoomOut),
				fHeightX = (nHeight*72.0)/(nDPI << nZoomOut);
			pCairoSutface = cairo_pdf_surface_create(
				dlgFile.GetFileName().c_str(),
				fWidthX, fHeightX);
			if (pCairoSutface == NULL) {
				throw 1;
			}
			pCairo = cairo_create(pCairoSutface);
			cairo_scale(
				pCairo,
				fWidthX/nWidth, fHeightX/nHeight);
			if (dlgCopy.IsDrawBorder()) {
				prtdrawer.DrawPaperBorderToCairo(
					pCairo,
					m_pTargetPrinter, nPage);
			}
			if (dlgCopy.IsDrawSprocketHole()) {
				prtdrawer.DrawSprocketHoleToCairo(
					pCairo,
					m_pTargetPrinter, nPage);
			}
			prtdrawer.DrawPrinterObjectToCairo(
				pCairo,
				m_pTargetPrinter, nPage);
		} catch (int /* nError*/) {
			ShowMessageBox(
				hdlg,
				MB_ICONEXCLAMATION, MB_OK,
				RSTR("Printing Image"),
				RSTR("An error occurred while writing."));
		}
		if (pCairo != NULL) {
			cairo_destroy(pCairo);
		}
		if (pCairoSutface != NULL) {
			cairo_surface_destroy(pCairoSutface);
		}
	} else {
		FILE* pFile = NULL;
		try {
			pFile = fopen(dlgFile.GetFileName().c_str(), "w");
			if (pFile == NULL) {
				throw 1;
			}
			std::string jstrText;
			prtdrawer.ExtractText(
				m_pTargetPrinter, nPage,
				jstrText, true);
			std::string lstrText =
				ConvUTF8toLOC(ConvSJIStoUTF8(jstrText));
			if (fwrite(
					lstrText.c_str(), 1, lstrText.size(), pFile) !=
						lstrText.size())
			{
				throw 22;
			}
			if (ferror(pFile) != 0) {
				throw 31;
			}
			if (fflush(pFile) != 0) {
				throw 32;
			}
		} catch (int /* nError*/) {
			ShowMessageBox(
				hdlg,
				MB_ICONEXCLAMATION, MB_OK,
				RSTR("Printing Image"),
				RSTR("An error occurred while writing."));
		}
		if (pFile != NULL) {
			fclose(pFile);
		}
	}

#endif // X88_PRINTER_DRAW

#endif // X88_GUI

}

// delete paper

void CX88PrinterDlg::DoPrinterPaperDelete(CX88WndHandle hdlg) {
	CX88PrinterPaperDelDlg dlgPaperDel(hdlg);
	dlgPaperDel.SetPageCount(
		max_value(
			m_pTargetPrinter->GetCurrentPage()+1,
			(int)m_pTargetPrinter->size()));
	dlgPaperDel.SetDelPageNo(
		m_wndPreview.GetTargetPage());
	if (dlgPaperDel.DoModal() == IDOK) {
		std::string gstrMessage;
		if (!dlgPaperDel.IsDelAllPages()) {
			gstrMessage = FormatStr(
				RSTR("Delete page %d?"),
				dlgPaperDel.GetDelPageNo()+1);
		} else {
			gstrMessage = RSTR("Delete all pages?");
		}
		if (ShowMessageBox(
				hdlg,
				MB_ICONEXCLAMATION, MB_OKCANCEL,
				RSTR("Delete Paper"),
				gstrMessage) == IDOK)
		{
			if (!dlgPaperDel.IsDelAllPages()) {
				m_pTargetPrinter->DeletePage(
					dlgPaperDel.GetDelPageNo());
			} else {
				m_pTargetPrinter->DeleteAllPages();
			}
			SetChildren(hdlg, false, true);
		}
	}
}

// feed paper

void CX88PrinterDlg::DoPrinterPaperFeed(CX88WndHandle hdlg) {
	CX88PrinterPaperFeedDlg dlgPaperFeed(hdlg);
	if (dlgPaperFeed.DoModal() == IDOK) {
		switch (dlgPaperFeed.GetPaperFeedUnit()) {
		case CX88PrinterPaperFeedDlg::UNIT_PAGE:
			m_pTargetPrinter->PaperFeed(
				true, 1);
			break;
		case CX88PrinterPaperFeedDlg::UNIT_LINE:
			m_pTargetPrinter->PaperFeed(
				false, dlgPaperFeed.GetLineFeedCount()*12);
			break;
		case CX88PrinterPaperFeedDlg::UNIT_POINT:
			m_pTargetPrinter->PaperFeed(
				false, dlgPaperFeed.GetPointFeedCount());
			break;
		}
		m_wndPreview.SetTargetPage(
			m_pTargetPrinter->GetCurrentPage());
		SetChildren(hdlg, false, true);
	}
}

// reset

void CX88PrinterDlg::DoPrinterReset(CX88WndHandle hdlg) {
	if (ShowMessageBox(
			hdlg,
			MB_ICONEXCLAMATION, MB_OKCANCEL,
			RSTR("Printer"),
			RSTR("Reset Printer?")) == IDOK)
	{
		m_pTargetPrinter->Reset();
		SetChildren(hdlg, true, true);
	}
}

// create modal dialog

int CX88PrinterDlg::DoModal() {
	m_pTargetPrinter = m_pPrinter;
	return ExecuteDialog(
		IDD_PRINTER,
		GetParentWndHandle(),
		DlgProc);
}
