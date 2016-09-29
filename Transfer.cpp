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
#include "Transfer.h"
#include "mainDlg.h"
#include "langpack.h"

Transfer::Transfer(CWnd* pParent /*=NULL*/)
: CDialog(Transfer::IDD, pParent)
{
	Create (IDD, pParent);
}

Transfer::~Transfer(void)
{
	mainDlg->transferDlg = NULL;
}

int Transfer::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (langPack.rtl) {
		ModifyStyleEx(0,WS_EX_LAYOUTRTL);
	}
	return 0;
}

BOOL Transfer::OnInitDialog()
{
	CDialog::OnInitDialog();
	TranslateDialog(this->m_hWnd);

	return TRUE;
}

void Transfer::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	delete this;
}

BEGIN_MESSAGE_MAP(Transfer, CDialog)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, &Transfer::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &Transfer::OnBnClickedOk)
END_MESSAGE_MAP()


void Transfer::OnClose() 
{
	DestroyWindow();
}

void Transfer::OnBnClickedCancel()
{
	OnClose();
}

void Transfer::OnBnClickedOk()
{
	MessagesContact* messagesContactSelected = mainDlg->messagesDlg->GetMessageContact();
	if (messagesContactSelected->callId!=-1) {
		CString number;
		GetDlgItem(IDC_NUMBER)->GetWindowText(number);
		number.Trim();
		if (!number.IsEmpty()) {
			pj_str_t pj_uri = StrToPjStr ( GetSIPURI(number, true) );
			pjsua_call_xfer(messagesContactSelected->callId, &pj_uri, NULL);
			OnClose();
		}
	}
}
