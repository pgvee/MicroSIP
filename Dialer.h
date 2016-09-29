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

#pragma once

#include "resource.h"
#include "const.h"
#include "BaseDialog.h"
#include "ButtonDialer.h"

enum DialerActions {
	ACTION_CALL, ACTION_VIDEO_CALL, ACTION_MESSAGE
};

class Dialer :
	public CBaseDialog
{
	CFont m_font;
	CFont m_font_balance;

	CButtonDialer m_ButtonDialer1;
	CButtonDialer m_ButtonDialer2;
	CButtonDialer m_ButtonDialer3;
	CButtonDialer m_ButtonDialer4;
	CButtonDialer m_ButtonDialer5;
	CButtonDialer m_ButtonDialer6;
	CButtonDialer m_ButtonDialer7;
	CButtonDialer m_ButtonDialer8;
	CButtonDialer m_ButtonDialer9;
	CButtonDialer m_ButtonDialer0;
	CButtonDialer m_ButtonDialerStar;
    CButtonDialer m_ButtonDialerGrate;
    CButtonDialer m_ButtonDialerDelete;
    CButtonDialer m_ButtonDialerPlus;
    CButtonDialer m_ButtonDialerClear;

	HICON m_hIconMuteOutput;
	HICON m_hIconMutedOutput;
	HICON m_hIconMuteInput;
	HICON m_hIconMutedInput;

	BOOL muteOutput;
	BOOL muteInput;
	
public:

	HICON m_hIconHold;
	HICON m_hIconTransfer;
#ifdef _GLOBAL_VIDEO
	HICON m_hIconVideo;
#endif
	HICON m_hIconMessage;
	Dialer(CWnd* pParent = NULL);	// standard constructor
	~Dialer();
	enum { IDD = IDD_DIALER };

	void Input(CString digits, BOOL disableDTMF = FALSE);
	void DTMF(CString digits, BOOL noLocalDTMF = FALSE);
	void DialedClear();
	void DialedLoad();
	void DialedSave(CComboBox *combobox);
	void SetNumber(CString  number, int callsCount = -1);
	void UpdateCallButton(BOOL forse = FALSE, int callsCount = -1);
	void Action(DialerActions action);
	void Clear(bool update=true);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedCall();
#ifdef _GLOBAL_VIDEO
	afx_msg void OnBnClickedVideoCall();
#endif
	afx_msg void OnBnClickedMessage();
	afx_msg void OnBnClickedHold();
	afx_msg void OnBnClickedTransfer();
	afx_msg void OnBnClickedEnd();
	afx_msg void OnBnClickedMuteOutput();
	afx_msg void OnBnClickedMuteInput();
	afx_msg void OnCbnEditchangeComboAddr();
	afx_msg void OnCbnSelchangeComboAddr();
	afx_msg void OnBnClickedKey1();
	afx_msg void OnBnClickedKey2();
	afx_msg void OnBnClickedKey3();
	afx_msg void OnBnClickedKey4();
	afx_msg void OnBnClickedKey5();
	afx_msg void OnBnClickedKey6();
	afx_msg void OnBnClickedKey7();
	afx_msg void OnBnClickedKey8();
	afx_msg void OnBnClickedKey9();
	afx_msg void OnBnClickedKeyStar();
	afx_msg void OnBnClickedKey0();
	afx_msg void OnBnClickedKeyGrate();
	afx_msg void OnBnClickedKeyPlus();
	afx_msg void OnBnClickedClear();
	afx_msg void OnBnClickedDelete();
	afx_msg void OnRButtonUp( UINT nFlags, CPoint pt );
	afx_msg void OnLButtonUp( UINT nFlags, CPoint pt );
	afx_msg void OnMouseMove(UINT nFlags, CPoint pt );
	afx_msg void OnVScroll( UINT, UINT, CScrollBar* );
	virtual BOOL PreTranslateMessage(MSG* pMsg);

};
