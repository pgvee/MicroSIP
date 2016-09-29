/* 
 * Copyright (C) 2011-2016 MicroSIP (http://www.microsip.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#include "StdAfx.h"
#include "MessagesDlg.h"
#include "microsip.h"
#include "mainDlg.h"
#include "settings.h"
#include "Transfer.h"
#include "langpack.h"

static DWORD __stdcall MEditStreamOutCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	CString sThisWrite;
	sThisWrite.GetBufferSetLength(cb);

	CString *psBuffer = (CString *)dwCookie;

	for (int i=0;i<cb;i++)
	{
		sThisWrite.SetAt(i,*(pbBuff+i));
	}

	*psBuffer += sThisWrite;

	*pcb = sThisWrite.GetLength();
	sThisWrite.ReleaseBuffer();
	return 0;
}

static DWORD __stdcall MEditStreamInCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
    CString *psBuffer = (CString*)dwCookie;
 
    if (cb > psBuffer->GetLength()) cb = psBuffer->GetLength();
 
    for (int i = 0; i < cb; i++)
    {
        *(pbBuff + i) = psBuffer->GetAt(i);
    }
 
    *pcb = cb;
    *psBuffer = psBuffer->Mid(cb);
 
    return 0;
}

MessagesDlg::MessagesDlg(CWnd* pParent /*=NULL*/)
: CBaseDialog(MessagesDlg::IDD, pParent)
{
	this->m_hWnd = NULL;
	Create (IDD, pParent);
}

MessagesDlg::~MessagesDlg(void)
{
}

int MessagesDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (langPack.rtl) {
		ModifyStyleEx(0,WS_EX_LAYOUTRTL);
	}
	return 0;
}

void MessagesDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, tabComponent);
}

BOOL MessagesDlg::OnInitDialog()
{
	CBaseDialog::OnInitDialog();

	AutoMove(IDC_TAB,0,0,100,0);
	AutoMove(IDC_LAST_CALL,100,0,0,0);
	AutoMove(IDC_CLOSE_ALL,100,0,0,0);
	AutoMove(IDC_HOLD,100,0,0,0);
	AutoMove(IDC_ATTENDED_TRANSFER,100,0,0,0);
	AutoMove(IDC_TRANSFER,100,0,0,0);
	AutoMove(IDC_END,100,0,0,0);
	AutoMove(IDC_LIST,0,0,100,80);
	AutoMove(IDC_MESSAGE,0,80,100,20);

	lastCall = NULL;
	tab = &tabComponent;
	
	HICON m_hIcon = theApp.LoadIcon(IDR_MAINFRAME);
	SetIcon(m_hIcon, FALSE);
	
	TranslateDialog(this->m_hWnd);

#ifndef _GLOBAL_VIDEO
	GetDlgItem(IDC_VIDEO_CALL)->ShowWindow(SW_HIDE);
#endif

	CRichEditCtrl* richEdit = (CRichEditCtrl*)GetDlgItem(IDC_MESSAGE);
	richEdit->SetEventMask(richEdit->GetEventMask() | ENM_KEYEVENTS);

	CRichEditCtrl* richEditList = (CRichEditCtrl*)GetDlgItem(IDC_LIST);
    richEditList->SetEventMask(richEdit->GetEventMask() | ENM_MOUSEEVENTS);
	richEditList->SetUndoLimit(0);

	CFont* font = this->GetFont();
	LOGFONT lf;
	font->GetLogFont(&lf);
	lf.lfHeight = 16;
	_tcscpy(lf.lfFaceName, _T("Arial"));
	fontList.CreateFontIndirect(&lf);
	richEditList->SetFont(&fontList);
	lf.lfHeight = 18;
	fontMessage.CreateFontIndirect(&lf);
	richEdit->SetFont(&fontMessage);


	para.cbSize=sizeof(PARAFORMAT2);
	para.dwMask = PFM_STARTINDENT | PFM_LINESPACING | PFM_SPACEBEFORE | PFM_SPACEAFTER;
	para.dxStartIndent=100;
	para.dySpaceBefore=100;
	para.dySpaceAfter=0;
	para.bLineSpacingRule = 5;
	para.dyLineSpacing = 22;

	m_hIconHold = (HICON)LoadImage(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(IDI_HOLD),
		IMAGE_ICON, 0, 0, LR_SHARED );
	((CButton*)GetDlgItem(IDC_HOLD))->SetIcon(m_hIconHold);

	m_hIconAttendedTransfer = (HICON)LoadImage(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(IDI_ATTENDED_TRANSFER),
		IMAGE_ICON, 0, 0, LR_SHARED );
	((CButton*)GetDlgItem(IDC_ATTENDED_TRANSFER))->SetIcon(m_hIconAttendedTransfer);

	m_hIconTransfer = (HICON)LoadImage(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(IDI_TRANSFER),
		IMAGE_ICON, 0, 0, LR_SHARED );
	((CButton*)GetDlgItem(IDC_TRANSFER))->SetIcon(m_hIconTransfer);

	menuAttendedTransfer.CreatePopupMenu();

	return TRUE;
}

void MessagesDlg::OnDestroy()
{
	mainDlg->messagesDlg = NULL;
	CBaseDialog::OnDestroy();
}


void MessagesDlg::PostNcDestroy()
{
	delete this;
}

BEGIN_MESSAGE_MAP(MessagesDlg, CBaseDialog)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_MOVE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_CLOSEALLTABS,OnCloseAllTabs)
	ON_COMMAND(ID_GOTOLASTTAB,OnGoToLastTab)
	ON_COMMAND(ID_COPY,OnCopy)
	ON_COMMAND(ID_SELECT_ALL,OnSelectAll)
	ON_COMMAND_RANGE(ID_ATTENDED_TRANSFER_RANGE,ID_ATTENDED_TRANSFER_RANGE+99,OnAttendedTransfer)
	ON_BN_CLICKED(IDCANCEL, &MessagesDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &MessagesDlg::OnBnClickedOk)
	ON_NOTIFY(EN_MSGFILTER, IDC_MESSAGE, &MessagesDlg::OnEnMsgfilterMessage)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &MessagesDlg::OnTcnSelchangeTab)
	ON_NOTIFY(TCN_SELCHANGING, IDC_TAB, &MessagesDlg::OnTcnSelchangingTab)
	ON_MESSAGE(WM_CONTEXTMENU,OnContextMenu)
	ON_MESSAGE(UM_CLOSETAB, &MessagesDlg::OnCloseTab)
	ON_BN_CLICKED(IDC_CALL_END, &MessagesDlg::OnBnClickedCallEnd)
	ON_BN_CLICKED(IDC_VIDEO_CALL, &MessagesDlg::OnBnClickedVideoCall)
	ON_BN_CLICKED(IDC_HOLD, &MessagesDlg::OnBnClickedHold)
	ON_BN_CLICKED(IDC_TRANSFER, &MessagesDlg::OnBnClickedTransfer)
	ON_BN_CLICKED(IDC_ATTENDED_TRANSFER, &MessagesDlg::OnBnClickedAttendedTransfer)
	ON_BN_CLICKED(IDC_END, &MessagesDlg::OnBnClickedEnd)
	ON_BN_CLICKED(IDC_CLOSE_ALL, &MessagesDlg::OnBnClickedCloseAll)
	ON_BN_CLICKED(IDC_LAST_CALL, &MessagesDlg::OnBnClickedLastCall)
END_MESSAGE_MAP()

LRESULT MessagesDlg::OnContextMenu(WPARAM wParam,LPARAM lParam)
{
	int x = GET_X_LPARAM(lParam); 
	int y = GET_Y_LPARAM(lParam); 
	POINT pt = { x, y };
	RECT rc;
	if (x!=-1 || y!=-1) {
		ScreenToClient(&pt);
		GetClientRect(&rc); 
		if (!PtInRect(&rc, pt)) {
			x = y = -1;
		} 
	} else {
		::ClientToScreen((HWND)wParam, &pt);
		x = 10+pt.x;
		y = 10+pt.y;
	}
	if (x!=-1 || y!=-1) {
			CMenu menu;
			menu.LoadMenu(IDR_MENU_TABS);
			CMenu* tracker = menu.GetSubMenu(0);
			TranslateMenu(tracker->m_hMenu);
			tracker->TrackPopupMenu( 0, x, y, this );
			return TRUE;
	}
	return DefWindowProc(WM_CONTEXTMENU,wParam,lParam);
}

void MessagesDlg::OnClose() 
{
	call_hangup_all_noincoming();
	this->ShowWindow(SW_HIDE);
}

void MessagesDlg::OnMove(int x, int y)
{
	if (IsWindowVisible() && !IsZoomed() && !IsIconic()) {
		CRect cRect;
		GetWindowRect(&cRect);
		accountSettings.messagesX = cRect.left;
		accountSettings.messagesY = cRect.top;
		mainDlg->AccountSettingsPendingSave();
	}
}

void MessagesDlg::OnSize(UINT type, int w, int h)
{
	CBaseDialog::OnSize(type, w, h);
	if (this->IsWindowVisible() && type == SIZE_RESTORED) {
		CRect cRect;
		GetWindowRect(&cRect);
		accountSettings.messagesW = cRect.Width();
		accountSettings.messagesH = cRect.Height();
		mainDlg->AccountSettingsPendingSave();
	}
}

void MessagesDlg::OnBnClickedCancel()
{
	OnClose();
}

void MessagesDlg::OnBnClickedOk()
{
}

MessagesContact* MessagesDlg::AddTab(CString number, CString name, BOOL activate, pjsua_call_info *call_info, BOOL notShowWindow, BOOL ifExists)
{
	MessagesContact* messagesContact;

	SIPURI sipuri;
	ParseSIPURI(number, &sipuri);
	if (accountSettings.accountId && RemovePort(accountSettings.account.domain) == RemovePort(sipuri.domain) ) {
		sipuri.domain = accountSettings.account.domain;
	}

	number = (sipuri.user.GetLength() ? sipuri.user + _T("@") : _T("")) + sipuri.domain;

	LONG exists = -1;
	for (int i=0; i < tab->GetItemCount(); i++)
	{
		messagesContact = GetMessageContact(i);

		CString compareNumber = messagesContact->number;
		if (messagesContact->number == number || compareNumber == number) {
			exists=i;
			if (call_info)
			{
				if (messagesContact->callId != -1) {
					if (messagesContact->callId != call_info->id) {
						if (call_info->state != PJSIP_INV_STATE_DISCONNECTED) {
							mainDlg->PostMessage(UM_CALL_ANSWER, (WPARAM)call_info->id, -486);
						}
						return NULL;
					}
				} else {
					messagesContact->callId = call_info->id;
				}
			}
			break;
		}
	}
	if (exists==-1)
	{
		if (ifExists)
		{
			return NULL;
		}
		if (!name.GetLength()) {
			name = mainDlg->pageContacts->GetNameByNumber(number);
			if (!name.GetLength()) {
				if (!sipuri.name.GetLength())
				{
					name = (sipuri.domain == accountSettings.account.domain ? sipuri.user : number);
				} else 
				{
					name = sipuri.name + _T(" (") + (sipuri.domain == accountSettings.account.domain ? sipuri.user : number) + _T(")");
				}
			}
		}
		messagesContact = new MessagesContact();
		messagesContact->callId = call_info ? call_info->id : -1;
		messagesContact->number = number;
		messagesContact->name = name;
		
		TCITEM item;
		item.mask = TCIF_PARAM | TCIF_TEXT;
		name.Format(_T("   %s  "), name);
		item.pszText=name.GetBuffer();
		item.cchTextMax=0;
		item.lParam = (LPARAM)messagesContact;
		exists = tab->InsertItem(tab->GetItemCount(),&item);
		if (tab->GetCurSel() == exists)
		{
			OnChangeTab(call_info);
		}
	} else
	{
		if (tab->GetCurSel() == exists && call_info)
		{
			UpdateCallButton(messagesContact->callId != -1, call_info);
		}
	}
	//if (tab->GetCurSel() != exists && (activate || !IsWindowVisible()))
	if (tab->GetCurSel() != exists && activate)
	{
		long result;
		OnTcnSelchangingTab(NULL, &result);
		tab->SetCurSel(exists);
		OnChangeTab(call_info);
	}
	if (!IsWindowVisible()) {
		if (!notShowWindow) 
		{
			if (!accountSettings.hidden) {
				ShowWindow(SW_SHOW);
				CRichEditCtrl* richEdit = (CRichEditCtrl*)GetDlgItem(IDC_MESSAGE);
				GotoDlgCtrl(richEdit);
			}
		}
	}
	return messagesContact;
}

void MessagesDlg::OnChangeTab(pjsua_call_info *p_call_info)
{
	tab->HighlightItem(tab->GetCurSel(),FALSE);

	MessagesContact* messagesContact = GetMessageContact();
	SetWindowText(messagesContact->name);

	if (messagesContact->callId != -1) {
		pjsua_call_info call_info;
		if (!p_call_info) {
			pjsua_call_get_info(messagesContact->callId, &call_info);
			p_call_info = &call_info;
		}
		UpdateCallButton(TRUE, p_call_info);
		if (accountSettings.singleMode
			&&(p_call_info->role==PJSIP_ROLE_UAC ||
				(p_call_info->role==PJSIP_ROLE_UAS &&
				(p_call_info->state == PJSIP_INV_STATE_CONFIRMED
				|| p_call_info->state == PJSIP_INV_STATE_CONNECTING)
				))
			) {
			SIPURI sipuri;
			ParseSIPURI(messagesContact->number, &sipuri);
			mainDlg->pageDialer->SetNumber(!sipuri.user.IsEmpty() && sipuri.domain == accountSettings.account.domain ? sipuri.user : messagesContact->number);
		}
	} else {
		UpdateCallButton();
		if (accountSettings.singleMode) {
			mainDlg->pageDialer->Clear();
		}
	}

	CRichEditCtrl* richEditList = (CRichEditCtrl*)GetDlgItem(IDC_LIST);
	CRichEditCtrl* richEdit = (CRichEditCtrl*)GetDlgItem(IDC_MESSAGE);

	CString messages = messagesContact->messages;
	EDITSTREAM es;
	es.dwCookie = (DWORD) &messages;
	es.pfnCallback = MEditStreamInCallback; 
	richEditList->StreamIn(SF_RTF, es);

	richEditList->PostMessage(WM_VSCROLL, SB_BOTTOM, 0);
	richEdit->SetWindowText(messagesContact->message);

	int nEnd = richEdit->GetTextLengthEx(GTL_NUMCHARS);
	richEdit->SetSel(nEnd, nEnd);
}

void MessagesDlg::OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnChangeTab();
	*pResult = 0;
}


void MessagesDlg::OnTcnSelchangingTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	CRichEditCtrl* richEdit = (CRichEditCtrl*)GetDlgItem(IDC_MESSAGE);
	CString str;
	int len = richEdit->GetWindowTextLength();
	LPTSTR ptr = str.GetBuffer(len);
	richEdit->GetWindowText(ptr,len+1);
	str.ReleaseBuffer();

	MessagesContact* messagesContact = GetMessageContact();
	messagesContact->message = str;
	*pResult = 0;
}

LRESULT  MessagesDlg::OnCloseTab(WPARAM wParam,LPARAM lParam)
{
	int i=wParam;
	CloseTab(i);
	return TRUE;
}

BOOL MessagesDlg::CloseTab(int i, BOOL safe)
{
	int curSel = tab->GetCurSel();

	MessagesContact* messagesContact = GetMessageContact(i);
	if (messagesContact->callId != -1)
	{
		if (safe) {
			return FALSE;
		}
		call_hangup_fast(messagesContact->callId);
	}
	delete messagesContact;
	tab->DeleteItem(i);
	int count = tab->GetItemCount();
	if (!count) {
		GetDlgItem(IDC_LIST)->SetWindowText(_T(""));
		GetDlgItem(IDC_MESSAGE)->SetWindowText(_T(""));
		OnClose();
	} else  {
		tab->SetCurSel( curSel < count ? curSel: count-1 );
		OnChangeTab();
	}
	return TRUE;
}

pjsua_call_id MessagesDlg::CallMake(CString number, bool hasVideo, pj_status_t *pStatus)
{
	pjsua_acc_id acc_id;
	pj_str_t pj_uri;
	if (!SelectSIPAccount(number,acc_id,pj_uri)) {
		Account dummy;
		*pStatus = accountSettings.AccountLoad(1,&dummy) ? PJSIP_EAUTHACCDISABLED : PJSIP_EAUTHACCNOTFOUND;
		return PJSUA_INVALID_ID;
	}

	if (accountSettings.singleMode) {
		call_hangup_all_noincoming();
	} else {
		call_hold_all_except();
	}
#ifdef _GLOBAL_VIDEO
	if (hasVideo) {
		mainDlg->createPreviewWin();
	}
#endif

	mainDlg->SetSoundDevice(mainDlg->audio_output);

	pjsua_call_id call_id;

	pjsua_call_setting call_setting;
	pjsua_call_setting_default(&call_setting);
	call_setting.flag = 0;
	call_setting.vid_cnt=hasVideo ? 1 : 0;

	/* testing autoanswer
	pjsua_msg_data msg_data;
	pjsip_generic_string_hdr subject;
    pj_str_t hvalue, hname;
	pjsua_msg_data_init(&msg_data);
	hname = pj_str("X-AUTOANSWER");
	hvalue = pj_str("TRUE");
	pjsip_generic_string_hdr_init2 (&subject, &hname, &hvalue);
    pj_list_push_back(&msg_data.hdr_list, &subject);

	pj_status_t status = pjsua_call_make_call(
		acc_id,
		&pj_uri,
		&call_setting,
		NULL,
		&msg_data,
		&call_id);
	*/

	pj_status_t status = pjsua_call_make_call(
		acc_id,
		&pj_uri,
		&call_setting,
		NULL,
		NULL,
		&call_id);
	if (pStatus) {
		*pStatus = status;
	}
	return status == PJ_SUCCESS ? call_id : PJSUA_INVALID_ID;
}

void MessagesDlg::CallStart(bool hasVideo)
{
	MessagesContact* messagesContact = GetMessageContact();
	pj_status_t status;
	pjsua_call_id call_id = CallMake(messagesContact->number,hasVideo, &status);
	if (call_id!=PJSUA_INVALID_ID) {
		messagesContact->callId = call_id;
		UpdateCallButton(TRUE);
	} else {
		if (status != PJ_ERESOLVE) {
			CString message = GetErrorMessage(status);
			AddMessage(messagesContact,message);
			if (accountSettings.singleMode) {
				AfxMessageBox(message);
			}
		}
	}
}

void MessagesDlg::OnBnClickedCallEnd()
{
	MessagesContact* messagesContact = GetMessageContact();
	if (messagesContact->callId == -1)
	{
		CallStart();
	}
}

void MessagesDlg::OnEndCall(pjsua_call_info *call_info)
{
	for (int i = 0; i < tab->GetItemCount(); i++)
	{
		MessagesContact* messagesContact = GetMessageContact(i);
		if (messagesContact->callId == call_info->id)
		{
			lastCall = messagesContact;
			messagesContact->callId = -1;
			if (tab->GetCurSel()==i)
			{
				UpdateCallButton(FALSE, call_info);
			}
			break;
		}
	}
}

void MessagesDlg::UpdateCallButton(BOOL active, pjsua_call_info *call_info)
{
	GetDlgItem(IDC_CALL_END)->ShowWindow(active? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_END)->ShowWindow(!active? SW_HIDE : SW_SHOW);
#ifdef _GLOBAL_VIDEO
	GetDlgItem(IDC_VIDEO_CALL)->ShowWindow(active? SW_HIDE : SW_SHOW);
#endif
	UpdateHoldButton(call_info);
}

void MessagesDlg::UpdateHoldButton(pjsua_call_info *call_info)
{
	MessagesContact* messagesContact = GetMessageContact();
	if (messagesContact) {
		bool transferHide = false;
		bool holdHide = false;
		CButton* buttonHold = (CButton*)GetDlgItem(IDC_HOLD);
		CButton* buttonTransfer = (CButton*)GetDlgItem(IDC_TRANSFER);
		CButton* buttonAttendedTransfer = (CButton*)GetDlgItem(IDC_ATTENDED_TRANSFER);
		CButton* buttonHoldDialer = (CButton*)mainDlg->pageDialer->GetDlgItem(IDC_HOLD);
		CButton* buttonTransferDialer = (CButton*)mainDlg->pageDialer->GetDlgItem(IDC_TRANSFER);
		if (messagesContact->callId != -1) {
			if (call_info && messagesContact->callId == call_info->id) {
				if (call_info->state == PJSIP_INV_STATE_EARLY ||
					call_info->state == PJSIP_INV_STATE_CONNECTING ||
					call_info->state == PJSIP_INV_STATE_CONFIRMED) {
						buttonTransfer->ShowWindow(SW_SHOW);
						buttonAttendedTransfer->ShowWindow(SW_SHOW);
						buttonTransferDialer->EnableWindow(TRUE);
				} else {
					transferHide = true;
				}
				if (call_info->media_cnt>0) {
					if (call_info->media_status == PJSUA_CALL_MEDIA_ACTIVE
						|| call_info->media_status == PJSUA_CALL_MEDIA_REMOTE_HOLD
						) {
							buttonHold->ShowWindow(SW_SHOW);
							buttonHold->SetCheck(BST_UNCHECKED);
							buttonHoldDialer->EnableWindow(TRUE);
							buttonHoldDialer->SetCheck(BST_UNCHECKED);
					} else if (call_info->media_status == PJSUA_CALL_MEDIA_LOCAL_HOLD
						|| call_info->media_status == PJSUA_CALL_MEDIA_NONE) {
							buttonHold->ShowWindow(SW_SHOW);
							buttonHold->SetCheck(BST_CHECKED);
							buttonHoldDialer->EnableWindow(TRUE);
							buttonHoldDialer->SetCheck(BST_CHECKED);
					} else {
						holdHide = true;
					}
				} else {
					holdHide = true;
				}
			}
		} else {
			transferHide = true;
			holdHide = true;
		}
		if (transferHide) {
					buttonTransfer->ShowWindow(SW_HIDE);
					buttonAttendedTransfer->ShowWindow(SW_HIDE);
					buttonTransferDialer->EnableWindow(FALSE);
		}
		if (holdHide) {
			buttonHold->ShowWindow(SW_HIDE);
			buttonHold->SetCheck(BST_UNCHECKED);
			buttonHoldDialer->EnableWindow(FALSE);
			buttonHoldDialer->SetCheck(BST_UNCHECKED);
		}
	}
}

void MessagesDlg::Call(BOOL hasVideo)
{
	if (!accountSettings.singleMode || !call_get_count_noincoming())
	{
		MessagesContact* messagesContact = GetMessageContact();
		if (messagesContact->callId == -1)
		{
			CallStart(hasVideo);
		}
	} else {
		mainDlg->GotoTab(0);
	}
}

void MessagesDlg::AddMessage(MessagesContact* messagesContact, CString message, int type, BOOL blockForeground)
{
	CTime tm = CTime::GetCurrentTime();

	if (type == MSIP_MESSAGE_TYPE_SYSTEM) {
		if ( messagesContact->lastSystemMessage == message && messagesContact->lastSystemMessageTime > tm.GetTime()-2) {
			messagesContact->lastSystemMessageTime = tm;
			return;
		}
		messagesContact->lastSystemMessage = message;
		messagesContact->lastSystemMessageTime = tm;
	} else if (!messagesContact->lastSystemMessage.IsEmpty()) {
		messagesContact->lastSystemMessage.Empty();
	}

	if (IsWindowVisible() && !blockForeground) {
		SetForegroundWindow();
	}
	CRichEditCtrl richEdit;
	MessagesContact* messagesContactSelected = GetMessageContact();

	CRichEditCtrl *richEditList = (CRichEditCtrl *)GetDlgItem(IDC_LIST);
	if (messagesContactSelected != messagesContact) {
		CRect rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = 300;
		rect.bottom = 300;
		richEdit.Create(ES_MULTILINE | ES_READONLY | ES_NUMBER | WS_VSCROLL, rect, this, NULL);
		richEdit.SetFont(&fontList);

		CString messages = messagesContact->messages;
		EDITSTREAM es;
		es.dwCookie = (DWORD) &messages;
		es.pfnCallback = MEditStreamInCallback; 
		richEdit.StreamIn(SF_RTF, es);

		richEditList = &richEdit;
	}

	if (messagesContact->messages.IsEmpty()) {
		richEditList->SetSel(0,-1);
		richEditList->SetParaFormat(para);
	}

	COLORREF color;
	CString name;
	if (type==MSIP_MESSAGE_TYPE_LOCAL) {
		color = RGB (0,0,0);
		if (!accountSettings.account.displayName.IsEmpty()) {
			name = accountSettings.account.displayName;
		}
	} else if (type==MSIP_MESSAGE_TYPE_REMOTE) {
		color = RGB (21,101,206);
		name = messagesContact->name;
		int pos = name.Find(_T(" ("));
		if (pos==-1) {
			pos = name.Find(_T("@"));
		}
		if (pos!=-1) {
			name = name.Mid(0,pos);
		}
	}

	int nBegin;
	CHARFORMAT cf;
	CString str;

	CString time = tm.Format(_T("%X"));

	nBegin = richEditList->GetTextLengthEx(GTL_NUMCHARS);
	richEditList->SetSel(nBegin, nBegin);
	str.Format(_T("[%s]  "),time);
	richEditList->ReplaceSel( str );
	cf.dwMask = CFM_BOLD | CFM_COLOR | CFM_SIZE;
	cf.crTextColor = RGB (131,131,131);
	cf.dwEffects = 0;
	cf.yHeight = 160;
	richEditList->SetSel(nBegin,-1);
	richEditList->SetSelectionCharFormat(cf);

	if (type != MSIP_MESSAGE_TYPE_SYSTEM) {
		cf.yHeight = 200;
	}
	if (name.GetLength()) {
		nBegin = richEditList->GetTextLengthEx(GTL_NUMCHARS);
		richEditList->SetSel(nBegin, nBegin);
		richEditList->ReplaceSel( name + _T(": "));
		cf.dwMask = CFM_BOLD | CFM_COLOR | CFM_SIZE;
		cf.crTextColor = color;
		cf.dwEffects = CFE_BOLD;
		richEditList->SetSel(nBegin,-1);
		richEditList->SetSelectionCharFormat(cf);
	}

	nBegin = richEditList->GetTextLengthEx(GTL_NUMCHARS);
	richEditList->SetSel(nBegin, nBegin);
	richEditList->ReplaceSel(message+_T("\r\n"));
	cf.dwMask = CFM_BOLD | CFM_COLOR | CFM_SIZE;

	cf.crTextColor = type == MSIP_MESSAGE_TYPE_SYSTEM ? RGB (131, 131, 131) : color;
	cf.dwEffects = 0;

	richEditList->SetSel(nBegin,-1);
	richEditList->SetSelectionCharFormat(cf);

	if (messagesContactSelected == messagesContact)	{
		richEditList->PostMessage(WM_VSCROLL, SB_BOTTOM, 0);
	} else {
		for (int i = 0; i < tab->GetItemCount(); i++) {
			if (messagesContact == GetMessageContact(i))
			{
				tab->HighlightItem(i, TRUE);
				break;
			}
		}
	}

	str=_T("");
	EDITSTREAM es;
	es.dwCookie = (DWORD) &str;
	es.pfnCallback = MEditStreamOutCallback; 
	richEditList->StreamOut(SF_RTF, es);
	messagesContact->messages=str;
}

void MessagesDlg::OnEnMsgfilterMessage(NMHDR *pNMHDR, LRESULT *pResult)
{
	MSGFILTER *pMsgFilter = reinterpret_cast<MSGFILTER *>(pNMHDR);

	if (pMsgFilter->msg == WM_CHAR) {
		if ( pMsgFilter->wParam == VK_RETURN ) {
			CRichEditCtrl* richEdit = (CRichEditCtrl*)GetDlgItem(IDC_MESSAGE);
			CString message;
			int len = richEdit->GetWindowTextLength();
			LPTSTR ptr = message.GetBuffer(len);
			richEdit->GetWindowText(ptr,len+1);
			message.ReleaseBuffer();
			message.Trim();
			if (message.GetLength()) {
				MessagesContact* messagesContact = GetMessageContact();
				if (SendMessage (messagesContact,message) ) {
					richEdit->SetWindowText(_T(""));
					GotoDlgCtrl(richEdit);
					AddMessage(messagesContact, message, MSIP_MESSAGE_TYPE_LOCAL);
					if (accountSettings.localDTMF) {
						mainDlg->onPlayerPlay(MSIP_SOUND_MESSAGE_OUT,0);
					}
				}
			}
			*pResult= 1;
			return;
		}
	}
	*pResult = 0;
}

BOOL MessagesDlg::SendMessage(MessagesContact* messagesContact, CString message, CString number)
{
	message.Trim();
	if (message.GetLength()) {
		pjsua_acc_id acc_id;
		pj_str_t pj_uri;
		pj_status_t status;
		if (SelectSIPAccount(messagesContact?messagesContact->number:number,acc_id,pj_uri)) {
			pj_str_t pj_message = StrToPjStr ( message );
			status = pjsua_im_send( acc_id, &pj_uri, NULL, &pj_message, NULL, NULL );
		} else {
			Account dummy;
			status = accountSettings.AccountLoad(1,&dummy) ? PJSIP_EAUTHACCDISABLED : PJSIP_EAUTHACCNOTFOUND;
		}
		if ( status != PJ_SUCCESS ) {
			if (messagesContact) {
				CString message = GetErrorMessage(status);
				AddMessage(messagesContact,message);
			}
		} else {
			return TRUE;
		}
	}
	return FALSE;
}

MessagesContact* MessagesDlg::GetMessageContact(int i)
{
	if (i ==-1) {
		i = tab->GetCurSel();
	}
	if (i != -1) {
		TCITEM item;
		item.mask = TCIF_PARAM;
		tab->GetItem(i, &item);
		return (MessagesContact*) item.lParam;
	}
	return NULL;
}
void MessagesDlg::OnBnClickedVideoCall()
{
	CallStart(true);
}

void MessagesDlg::OnBnClickedHold()
{
	MessagesContact* messagesContactSelected = GetMessageContact();
	if (messagesContactSelected->callId!=-1) {
		pjsua_call_info info;
		pjsua_call_get_info(messagesContactSelected->callId,&info);
		if (info.media_cnt>0) {
			if (info.media_status == PJSUA_CALL_MEDIA_LOCAL_HOLD || info.media_status == PJSUA_CALL_MEDIA_NONE) {
				call_hold_all_except(messagesContactSelected->callId);
				pjsua_call_reinvite(messagesContactSelected->callId, PJSUA_CALL_UNHOLD, NULL);
			} else {
				pjsua_call_set_hold(messagesContactSelected->callId, NULL);
			}
		}
	}
}

void MessagesDlg::OnBnClickedTransfer()
{
	if (!mainDlg->transferDlg)
	{
		mainDlg->transferDlg = new Transfer(this);
	}
	mainDlg->transferDlg->SetForegroundWindow();
}

void MessagesDlg::OnBnClickedAttendedTransfer()
{
	MessagesContact* messagesContact = GetMessageContact();
	CPoint point;    
	GetCursorPos(&point);

	while (menuAttendedTransfer.DeleteMenu(0, MF_BYPOSITION));
	
	pjsua_call_id call_ids[PJSUA_MAX_CALLS];
	unsigned calls_count = PJSUA_MAX_CALLS;
	int pos = 0;
	if (pjsua_enum_calls ( call_ids, &calls_count)==PJ_SUCCESS)  {
		for (unsigned i = 0; i < calls_count; ++i) {
			pjsua_call_info call_info_curr;
			pjsua_call_get_info(call_ids[i], &call_info_curr);
			if (call_info_curr.state == PJSIP_INV_STATE_EARLY ||
				call_info_curr.state == PJSIP_INV_STATE_CONNECTING ||
				call_info_curr.state == PJSIP_INV_STATE_CONFIRMED) {
					SIPURI sipuri_curr;
					ParseSIPURI(PjToStr(&call_info_curr.remote_info, TRUE), &sipuri_curr);
					if (call_info_curr.id != messagesContact->callId) {
						CString str = !sipuri_curr.name.IsEmpty()?sipuri_curr.name:(!sipuri_curr.user.IsEmpty()?sipuri_curr.user:(sipuri_curr.domain));
						menuAttendedTransfer.InsertMenu(pos, MF_BYPOSITION, ID_ATTENDED_TRANSFER_RANGE+pos, str);
						MENUITEMINFO mii;
						mii.cbSize = sizeof (MENUITEMINFO);
						mii.fMask = MIIM_DATA;
						mii.dwItemData = call_info_curr.id;
						menuAttendedTransfer.SetMenuItemInfo(pos,&mii,TRUE);
						pos++;
					}
			}
		}
	}
	if (pos==1) {
		OnAttendedTransfer(ID_ATTENDED_TRANSFER_RANGE);
	} else if (pos > 1){
		menuAttendedTransfer.TrackPopupMenu( 0, point.x, point.y, this );
	}
}

void MessagesDlg::OnBnClickedEnd()
{
	MessagesContact* messagesContact = GetMessageContact();
	call_hangup_fast(messagesContact->callId);
}

void MessagesDlg::OnCloseAllTabs()
{
	int i = 0;
	while (i < tab->GetItemCount()) {
		if (CloseTab(i,TRUE)) {
			i = 0;
		} else {
			i++;
		}
	}
}

void MessagesDlg::OnGoToLastTab()
{
	int i = 0;
	BOOL found = FALSE;
	int lastCallIndex = -1;
	while (i < tab->GetItemCount())	{
		MessagesContact* messagesContact = GetMessageContact(i);
		if (messagesContact->callId != -1) {
			found = TRUE;
			if (tab->GetCurSel() != i) {
				long result;
				OnTcnSelchangingTab(NULL, &result);
				tab->SetCurSel(i);
				OnChangeTab();
				break;
			}
		}
		if (messagesContact == lastCall) {
			lastCallIndex = i;
		}
		i++;
	}
	if (!found && lastCallIndex!=-1) {
		if (tab->GetCurSel() != lastCallIndex) {
			long result;
			OnTcnSelchangingTab(NULL, &result);
			tab->SetCurSel(lastCallIndex);
			OnChangeTab();
		}
	}
}

int MessagesDlg::GetCallDuration()
{
	int duration = -1;
	pjsua_call_info call_info;
	int i = 0;
	while (i < tab->GetItemCount()) {
		MessagesContact* messagesContact = GetMessageContact(i);
		if (messagesContact->callId != -1) {
			if (pjsua_call_get_info(messagesContact->callId, &call_info)==PJ_SUCCESS) {
				if (call_info.state == PJSIP_INV_STATE_CONFIRMED) {
					duration = call_info.connect_duration.sec;
				}
			}
		}
		i++;
	}
	return duration;
}

void MessagesDlg::OnCopy()
{
	CRichEditCtrl* richEditList = (CRichEditCtrl*)GetDlgItem(IDC_LIST);
	richEditList->Copy();
}

void MessagesDlg::OnSelectAll()
{
	CRichEditCtrl* richEditList = (CRichEditCtrl*)GetDlgItem(IDC_LIST);
	richEditList->SetSel(0,-1);
}

void MessagesDlg::OnAttendedTransfer(UINT nID)
{
	int pos = nID - ID_ATTENDED_TRANSFER_RANGE;

	MENUITEMINFO mii;
	mii.cbSize = sizeof (MENUITEMINFO);
	mii.fMask = MIIM_DATA;
	menuAttendedTransfer.GetMenuItemInfo(pos, &mii, TRUE);

	MessagesContact* messagesContact = GetMessageContact();
	if (messagesContact->callId != -1) {
		pjsua_call_xfer_replaces(mii.dwItemData,messagesContact->callId,0,0);
	}
}

void MessagesDlg::OnBnClickedCloseAll()
{
	OnCloseAllTabs();
}

void MessagesDlg::OnBnClickedLastCall()
{
	OnGoToLastTab();
}
