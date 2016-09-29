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
#include "SettingsDlg.h"
#include "mainDlg.h"
#include "settings.h"
#include "Preview.h"
#include "langpack.h"

static BOOL prev;

SettingsDlg::SettingsDlg(CWnd* pParent /*=NULL*/)
: CDialog(SettingsDlg::IDD, pParent)
{
	Create (IDD, pParent);
	prev = FALSE;
}

SettingsDlg::~SettingsDlg(void)
{
	mainDlg->settingsDlg = NULL;
}

int SettingsDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (langPack.rtl) {
		ModifyStyleEx(0,WS_EX_LAYOUTRTL);
	}
	return 0;
}

BOOL SettingsDlg::OnInitDialog()
{
	CComboBox *combobox;
	CComboBox *combobox2;
	unsigned count;
	int i;

	CDialog::OnInitDialog();

	TranslateDialog(this->m_hWnd);
	combobox= (CComboBox*)GetDlgItem(IDC_AUTO_ANSWER);
	combobox->AddString(Translate(_T("No")));
	combobox->AddString(Translate(_T("All calls")));
	combobox->AddString(Translate(_T("SIP header")));
	combobox->SetCurSel(accountSettings.autoAnswer);

	combobox= (CComboBox*)GetDlgItem(IDC_DENY_INCOMING);
	combobox->AddString(Translate(_T("No")));
	combobox->AddString(Translate(_T("Different user")));
	combobox->AddString(Translate(_T("Different domain")));
	combobox->AddString(Translate(_T("Different user or domain")));
	combobox->AddString(Translate(_T("Different remote domain")));
	combobox->AddString(Translate(_T("All calls")));
	if (accountSettings.denyIncoming==_T("user"))
	{
		i=1;
	} else if (accountSettings.denyIncoming==_T("domain"))
	{
		i=2;
	} else if (accountSettings.denyIncoming==_T("address"))
	{
		i=3;
	} else if (accountSettings.denyIncoming==_T("rdomain"))
	{
		i=4;
	} else if (accountSettings.denyIncoming==_T("all"))
	{
		i=5;
	} else
	{
		i=0;
	}
	combobox->SetCurSel(i);

	GetDlgItem(IDC_DIRECTORY)->SetWindowText(accountSettings.usersDirectory);
	((CButton*)GetDlgItem(IDC_LOCAL_DTMF))->SetCheck(accountSettings.localDTMF);
	((CButton*)GetDlgItem(IDC_SINGLE_MODE))->SetCheck(accountSettings.singleMode);
	((CButton*)GetDlgItem(IDC_ENABLE_LOG))->SetCheck(accountSettings.enableLog);
	((CButton*)GetDlgItem(IDC_ANSWER_BOX_RANDOM))->SetCheck(accountSettings.randomAnswerBox);
	((CButton*)GetDlgItem(IDC_VAD))->SetCheck(accountSettings.vad);
	((CButton*)GetDlgItem(IDC_EC))->SetCheck(accountSettings.ec);
	((CButton*)GetDlgItem(IDC_FORCE_CODEC))->SetCheck(accountSettings.forceCodec);

	GetDlgItem(IDC_RINGING_SOUND)->SetWindowText(accountSettings.ringingSound);

	pjmedia_aud_dev_info aud_dev_info[128];
	count = 128;
	pjsua_enum_aud_devs(aud_dev_info, &count);

	combobox= (CComboBox*)GetDlgItem(IDC_MICROPHONE);
	combobox->AddString(Translate(_T("Default")));
	combobox->SetCurSel(0);

	for (unsigned i=0;i<count;i++)
	{
		if (aud_dev_info[i].input_count) {
			CString audDevName(aud_dev_info[i].name);
			combobox->AddString( audDevName );
			if (!accountSettings.audioInputDevice.Compare(audDevName))
			{
				combobox->SetCurSel(combobox->GetCount()-1);
			}
		}
	}
	combobox= (CComboBox*)GetDlgItem(IDC_SPEAKERS);
	combobox->AddString(Translate(_T("Default")));
	combobox->SetCurSel(0);
	combobox2= (CComboBox*)GetDlgItem(IDC_RING);
	combobox2->AddString(Translate(_T("Default")));
	combobox2->SetCurSel(0);
	for (unsigned i=0;i<count;i++)
	{
		if (aud_dev_info[i].output_count) {
			CString audDevName(aud_dev_info[i].name);
			combobox->AddString(audDevName);
			combobox2->AddString(audDevName);
			if (!accountSettings.audioOutputDevice.Compare(audDevName))
			{
				combobox->SetCurSel(combobox->GetCount()-1);
			}
			if (!accountSettings.audioRingDevice.Compare(audDevName))
			{
				combobox2->SetCurSel(combobox->GetCount()-1);
			}
		}
	}

	pjsua_codec_info codec_info[64];
	CListBox *listbox;
	CListBox *listbox2;
	listbox = (CListBox*)GetDlgItem(IDC_AUDIO_CODECS_ALL);
	listbox2 = (CListBox*)GetDlgItem(IDC_AUDIO_CODECS);

	CList<CString> disabledCodecsList;
	count = 64;
	pjsua_enum_codecs(codec_info, &count);
	for (unsigned i=0;i<count;i++)
	{
			POSITION pos = mainDlg->audioCodecList.Find(
				PjToStr(&codec_info[i].codec_id)
				);
			CString key = mainDlg->audioCodecList.GetNext(pos);
			CString value  = mainDlg->audioCodecList.GetNext(pos);
			if (codec_info[i].priority
				&& (!accountSettings.audioCodecs.IsEmpty() || StrStr(_T(_GLOBAL_CODECS_ENABLED),key))
				) {
				listbox2->AddString(value);
			} else {
				disabledCodecsList.AddTail(key);
			}	
	}
	POSITION pos = mainDlg->audioCodecList.GetHeadPosition();
	while (pos) {
		CString key = mainDlg->audioCodecList.GetNext(pos);
		CString value  = mainDlg->audioCodecList.GetNext(pos);
		if (disabledCodecsList.Find(key)) {
			listbox->AddString(value);
		}
	}

#ifdef _GLOBAL_VIDEO
	((CButton*)GetDlgItem(IDC_DISABLE_H264))->SetCheck(accountSettings.disableH264);
 	((CButton*)GetDlgItem(IDC_DISABLE_H263))->SetCheck(accountSettings.disableH263);
	if (accountSettings.bitrateH264.IsEmpty()) {
		const pj_str_t codec_id = {"H264", 4};
		pjmedia_vid_codec_param param;
		pjsua_vid_codec_get_param(&codec_id, &param);
		accountSettings.bitrateH264.Format(_T("%d"),param.enc_fmt.det.vid.max_bps/1000);
	}
	if (accountSettings.bitrateH263.IsEmpty()) {
		const pj_str_t codec_id = {"H263", 4};
		pjmedia_vid_codec_param param;
		pjsua_vid_codec_get_param(&codec_id, &param);
		accountSettings.bitrateH263.Format(_T("%d"),param.enc_fmt.det.vid.max_bps/1000);
	}
	GetDlgItem(IDC_BITRATE_264)->SetWindowText(accountSettings.bitrateH264);
	GetDlgItem(IDC_BITRATE_263)->SetWindowText(accountSettings.bitrateH263);

	combobox= (CComboBox*)GetDlgItem(IDC_VID_CAP_DEV);
	combobox->AddString(Translate(_T("Default")));
	combobox->SetCurSel(0);
	pjmedia_vid_dev_info vid_dev_info[64];
	count = 64;
	pjsua_vid_enum_devs(vid_dev_info, &count);
	for (unsigned i=0;i<count;i++)
	{
		if (vid_dev_info[i].fmt_cnt && (vid_dev_info[i].dir==PJMEDIA_DIR_ENCODING || vid_dev_info[i].dir==PJMEDIA_DIR_ENCODING_DECODING))
		{
			CString vidDevName(vid_dev_info[i].name);
			combobox->AddString(vidDevName);
			if (!accountSettings.videoCaptureDevice.Compare(vidDevName))
			{
				combobox->SetCurSel(combobox->GetCount()-1);
			}
		}
	}

	combobox= (CComboBox*)GetDlgItem(IDC_VIDEO_CODEC);
	combobox->AddString(Translate(_T("Default")));
	combobox->SetCurSel(0);
	count = 64;
	pjsua_vid_enum_codecs(codec_info, &count);
	for (unsigned i=0;i<count;i++)
	{
		combobox->AddString(PjToStr(&codec_info[i].codec_id));
		if (!accountSettings.videoCodec.Compare(PjToStr(&codec_info[i].codec_id)))
		{
			combobox->SetCurSel(combobox->GetCount()-1);
		}
	}
#endif

	((CButton*)GetDlgItem(IDC_ENABLE_LOCAL))->SetCheck(accountSettings.enableLocalAccount);

	combobox= (CComboBox*)GetDlgItem(IDC_UPDATES_INTERVAL);
	combobox->AddString(Translate(_T("Daily")));
	combobox->AddString(Translate(_T("Weekly")));
	combobox->AddString(Translate(_T("Monthly")));
	combobox->AddString(Translate(_T("Quarterly")));
	combobox->AddString(Translate(_T("Never")));
	if (accountSettings.updatesInterval==_T("daily"))
	{
		i=0;
	} else if (accountSettings.updatesInterval==_T("monthly"))
	{
		i=2;
	} else if (accountSettings.updatesInterval==_T("quarterly"))
	{
		i=3;
	} else if (accountSettings.updatesInterval==_T("never"))
	{
		i=4;
	} else
	{
		i=1;
	}
	combobox->SetCurSel(i);

	return TRUE;
}

void SettingsDlg::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	delete this;
}

BEGIN_MESSAGE_MAP(SettingsDlg, CDialog)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, &SettingsDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &SettingsDlg::OnBnClickedOk)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_MODIFY, &SettingsDlg::OnDeltaposSpinModify)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_ORDER, &SettingsDlg::OnDeltaposSpinOrder)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_RINGING_SOUND, &SettingsDlg::OnNMClickSyslinkRingingSound)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_AUTO_ANSWER, &SettingsDlg::OnNMClickSyslinkAutoAnswer)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_DENY_INCOMING, &SettingsDlg::OnNMClickSyslinkDenyIncoming)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_DIRECTORY, &SettingsDlg::OnNMClickSyslinkDirectory)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_LOCAL_DTMF, &SettingsDlg::OnNMClickSyslinkLocalDTMF)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_SINGLE_MODE, &SettingsDlg::OnNMClickSyslinkSingleMode)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_VAD, &SettingsDlg::OnNMClickSyslinkVAD)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_EC, &SettingsDlg::OnNMClickSyslinkEC)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_FORCE_CODEC, &SettingsDlg::OnNMClickSyslinkForceCodec)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_DISABLE_H264, &SettingsDlg::OnNMClickSyslinkDisableH264)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_DISABLE_H263, &SettingsDlg::OnNMClickSyslinkDisableH263)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_AUDIO_CODECS, &SettingsDlg::OnNMClickSyslinkAudioCodecs)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_ENABLE_LOG, &SettingsDlg::OnNMClickSyslinkEnableLog)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_ANSWER_BOX_RANDOM, &SettingsDlg::OnNMClickSyslinkRandomAnswerBox)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_DISABLE_LOCAL, &SettingsDlg::OnNMClickSyslinkEnableLocal)
#ifdef _GLOBAL_VIDEO
	ON_BN_CLICKED(IDC_PREVIEW, &SettingsDlg::OnBnClickedPreview)
#endif
	ON_BN_CLICKED(IDC_BROWSE, &SettingsDlg::OnBnClickedBrowse)
	ON_EN_CHANGE(IDC_RINGING_SOUND, &SettingsDlg::OnEnChangeRingingSound)
	ON_BN_CLICKED(IDC_DEFAULT, &SettingsDlg::OnBnClickedDefault)
END_MESSAGE_MAP()


void SettingsDlg::OnClose() 
{
	DestroyWindow();
}

void SettingsDlg::OnBnClickedCancel()
{
	OnClose();
}

void SettingsDlg::OnBnClickedOk()
{
	this->ShowWindow(SW_HIDE);
	mainDlg->PJDestroy();

	CComboBox *combobox;
	int i;
	combobox= (CComboBox*)GetDlgItem(IDC_AUTO_ANSWER);
	accountSettings.autoAnswer = combobox->GetCurSel();

	combobox= (CComboBox*)GetDlgItem(IDC_DENY_INCOMING);
	i = combobox->GetCurSel();
	switch (i) {
		case 1:
			accountSettings.denyIncoming=_T("user");
			break;
		case 2:
			accountSettings.denyIncoming=_T("domain");
			break;
		case 3:
			accountSettings.denyIncoming=_T("address");
			break;
		case 4:
			accountSettings.denyIncoming=_T("rdomain");
			break;
		case 5:
			accountSettings.denyIncoming=_T("all");
			break;
		default:
			accountSettings.denyIncoming=_T("");
	}

	GetDlgItem(IDC_DIRECTORY)->GetWindowText(accountSettings.usersDirectory);
	accountSettings.usersDirectory.Trim();
	accountSettings.localDTMF=((CButton*)GetDlgItem(IDC_LOCAL_DTMF))->GetCheck();
	accountSettings.singleMode=((CButton*)GetDlgItem(IDC_SINGLE_MODE))->GetCheck();
	accountSettings.enableLog=((CButton*)GetDlgItem(IDC_ENABLE_LOG))->GetCheck();
	accountSettings.randomAnswerBox=((CButton*)GetDlgItem(IDC_ANSWER_BOX_RANDOM))->GetCheck();
	accountSettings.vad=((CButton*)GetDlgItem(IDC_VAD))->GetCheck();
	accountSettings.ec=((CButton*)GetDlgItem(IDC_EC))->GetCheck();
	accountSettings.forceCodec =((CButton*)GetDlgItem(IDC_FORCE_CODEC))->GetCheck();

	GetDlgItem(IDC_MICROPHONE)->GetWindowText(accountSettings.audioInputDevice);
	if (accountSettings.audioInputDevice==Translate(_T("Default")))
	{
		accountSettings.audioInputDevice = _T("");
	}

	GetDlgItem(IDC_SPEAKERS)->GetWindowText(accountSettings.audioOutputDevice);
	if (accountSettings.audioOutputDevice==Translate(_T("Default")))
	{
		accountSettings.audioOutputDevice = _T("");
	}

	GetDlgItem(IDC_RING)->GetWindowText(accountSettings.audioRingDevice);
	if (accountSettings.audioRingDevice==Translate(_T("Default")))
	{
		accountSettings.audioRingDevice = _T("");
	}

	accountSettings.audioCodecs = _T("");
	CListBox *listbox2;
	listbox2 = (CListBox*)GetDlgItem(IDC_AUDIO_CODECS);
	for (unsigned i = 0; i < listbox2->GetCount(); i++)
	{
		CString value;
		listbox2->GetText(i, value);
		POSITION pos = mainDlg->audioCodecList.Find(value);
		if (pos) {
			mainDlg->audioCodecList.GetPrev(pos);
			CString key = mainDlg->audioCodecList.GetPrev(pos);
			accountSettings.audioCodecs += key + _T(" ");
		}
	}
	accountSettings.audioCodecs.Trim();

#ifdef _GLOBAL_VIDEO
	accountSettings.disableH264=((CButton*)GetDlgItem(IDC_DISABLE_H264))->GetCheck();
	accountSettings.disableH263=((CButton*)GetDlgItem(IDC_DISABLE_H263))->GetCheck();
	GetDlgItem(IDC_BITRATE_264)->GetWindowText(accountSettings.bitrateH264);
	if (!atoi(CStringA(accountSettings.bitrateH264))) {
		accountSettings.bitrateH264=_T("");
	}
	GetDlgItem(IDC_BITRATE_263)->GetWindowText(accountSettings.bitrateH263);
	if (!atoi(CStringA(accountSettings.bitrateH263))) {
		accountSettings.bitrateH263=_T("");
	}
	GetDlgItem(IDC_VID_CAP_DEV)->GetWindowText(accountSettings.videoCaptureDevice);
	if (accountSettings.videoCaptureDevice==Translate(_T("Default")))
	{
		accountSettings.videoCaptureDevice = _T("");
	}

	GetDlgItem(IDC_VIDEO_CODEC)->GetWindowText(accountSettings.videoCodec);
	if (accountSettings.videoCodec==Translate(_T("Default")))
	{
		accountSettings.videoCodec = _T("");
	}
#endif

	GetDlgItem(IDC_RINGING_SOUND)->GetWindowText(accountSettings.ringingSound);

	accountSettings.enableLocalAccount=((CButton*)GetDlgItem(IDC_ENABLE_LOCAL))->GetCheck();

	combobox= (CComboBox*)GetDlgItem(IDC_UPDATES_INTERVAL);
	i = combobox->GetCurSel();
	switch (i) {
		case 0:
			accountSettings.updatesInterval=_T("daily");
			break;
		case 2:
			accountSettings.updatesInterval=_T("monthly");
			break;
		case 3:
			accountSettings.updatesInterval=_T("quarterly");
			break;
		case 4:
			accountSettings.updatesInterval=_T("never");
			break;
		default:
			accountSettings.updatesInterval=_T("");
	}

	accountSettings.SettingsSave();
	mainDlg->PJCreate();
	mainDlg->PJAccountAdd();

	OnClose();
}

void SettingsDlg::OnBnClickedBrowse()
{
	CFileDialog dlgFile( TRUE, _T("wav"), 0, OFN_NOCHANGEDIR, _T("WAV Files (*.wav)|*.wav|") );
	if (dlgFile.DoModal()==IDOK) {
		CString cwd;
		LPTSTR ptr = cwd.GetBuffer(MAX_PATH);
		::GetCurrentDirectory(MAX_PATH, ptr);
		cwd.ReleaseBuffer();
		if ( cwd.MakeLower() + _T("\\") + dlgFile.GetFileName().MakeLower() == dlgFile.GetPathName().MakeLower() ) {
			GetDlgItem(IDC_RINGING_SOUND)->SetWindowText(dlgFile.GetFileName());
		} else {
			GetDlgItem(IDC_RINGING_SOUND)->SetWindowText(dlgFile.GetPathName());
		}
	}
}

void SettingsDlg::OnEnChangeRingingSound()
{
	CString str;
	GetDlgItem(IDC_RINGING_SOUND)->GetWindowText(str);
	GetDlgItem(IDC_DEFAULT)->EnableWindow(str.GetLength()>0);
}

void SettingsDlg::OnBnClickedDefault()
{
	GetDlgItem(IDC_RINGING_SOUND)->SetWindowText(_T(""));
}

void SettingsDlg::OnDeltaposSpinModify(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	CListBox *listbox;
	CListBox *listbox2;
	listbox = (CListBox*)GetDlgItem(IDC_AUDIO_CODECS_ALL);
	listbox2 = (CListBox*)GetDlgItem(IDC_AUDIO_CODECS);
	if (pNMUpDown->iDelta == -1) {
		//add
		int selected = listbox->GetCurSel();
		if (selected != LB_ERR) 
		{
			CString str;
			listbox->GetText(selected, str);
			listbox2->AddString(str);
			listbox->DeleteString(selected);
			listbox->SetCurSel( selected < listbox->GetCount() ? selected : selected-1 );
		}
	} else {
		//remove
		int selected = listbox2->GetCurSel();
		if (selected != LB_ERR) 
		{
			CString str;
			listbox2->GetText(selected, str);
			listbox->AddString(str);
			listbox2->DeleteString(selected);
			listbox2->SetCurSel( selected < listbox2->GetCount() ? selected : selected-1 );
		}
	}
	*pResult = 0;
}

void SettingsDlg::OnDeltaposSpinOrder(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	CListBox *listbox2;
	listbox2 = (CListBox*)GetDlgItem(IDC_AUDIO_CODECS);
	int selected = listbox2->GetCurSel();
	if (selected != LB_ERR) 
	{
		CString str;
		listbox2->GetText(selected, str);
		if (pNMUpDown->iDelta == -1) {
			//up
			if (selected > 0)
			{
				listbox2->DeleteString(selected);
				listbox2->InsertString(selected-1,str);
				listbox2->SetCurSel(selected-1);
			}
		} else {
			//down
			if (selected < listbox2->GetCount()-1)
			{
				listbox2->DeleteString(selected);
				listbox2->InsertString(selected+1,str);
				listbox2->SetCurSel(selected+1);
			}
		}
	}
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkRingingSound(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("ringingSound"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkAutoAnswer(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("autoAnswer"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkDenyIncoming(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("denyIncoming"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkDirectory(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("directory"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkLocalDTMF(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("soundEvents"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkSingleMode(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("singleMode"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkVAD(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("vad"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkEC(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("ec"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkForceCodec(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("forceCodec"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkDisableH264(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("disableH264"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkDisableH263(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("disableH263"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkAudioCodecs(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("audioCodecs"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkEnableLog(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("log"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkRandomAnswerBox(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("randomAnswerBox"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkEnableLocal(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("enableLocal"));
	*pResult = 0;
}

#ifdef _GLOBAL_VIDEO
void SettingsDlg::OnBnClickedPreview()
{
	CComboBox *combobox;
	combobox = (CComboBox*)GetDlgItem(IDC_VID_CAP_DEV);
	CString name;
	combobox->GetWindowText(name);
	if (!mainDlg->previewWin) {
		mainDlg->previewWin = new Preview(mainDlg);
	}
	mainDlg->previewWin->Start(mainDlg->VideoCaptureDeviceId(name));
}
#endif


