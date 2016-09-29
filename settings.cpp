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

#include "stdafx.h"
#include "settings.h"
#include "Crypto.h"

using namespace MFC;

AccountSettings accountSettings;
bool firstRun;
bool pj_ready;

void AccountSettings::Init()
{
	CString str;
	LPTSTR ptr;

	accountId = 0;
	//--
	logFile.Format(_T("%s.log"), _T(_GLOBAL_NAME));
	iniFile.Format(_T("%s.ini"), _T(_GLOBAL_NAME));
	pathRoaming=_T("");
	if (pathRoaming.IsEmpty()) {

		CFileStatus rStatus;
		CRegKey regKey;
		CString pathInstaller;
		CString rab;
		ULONG pnChars;
		rab.Format(_T("Software\\%s"), _T(_GLOBAL_NAME));
		if (regKey.Open(HKEY_LOCAL_MACHINE,rab,KEY_READ)==ERROR_SUCCESS) {
			ptr = pathInstaller.GetBuffer(255);
			pnChars = 256;
			regKey.QueryStringValue(NULL,ptr,&pnChars);
			pathInstaller.ReleaseBuffer();
			regKey.Close();
		}

		CString pathCurrent;
		ptr = pathCurrent.GetBuffer(MAX_PATH);
		::GetCurrentDirectory(MAX_PATH, ptr);
		pathCurrent.ReleaseBuffer();

		CString appDataRoaming;
		ptr = appDataRoaming.GetBuffer(MAX_PATH);
		SHGetSpecialFolderPath(
			0,
			ptr, 
			CSIDL_APPDATA,
			FALSE ); 
		appDataRoaming.ReleaseBuffer();
		appDataRoaming.AppendFormat(_T("\\%s\\"),_T(_GLOBAL_NAME));

		if (!pathInstaller.IsEmpty() && pathInstaller.CompareNoCase(pathCurrent) == 0) {
			// installer
			CString appDataLocal;
			ptr = appDataLocal.GetBuffer(MAX_PATH);
			SHGetSpecialFolderPath(
				0,
				ptr, 
				CSIDL_LOCAL_APPDATA, 
				FALSE ); 
			appDataLocal.ReleaseBuffer();
			appDataLocal.AppendFormat(_T("\\%s\\"),_T(_GLOBAL_NAME));

			CreateDirectory(appDataLocal, NULL);
			CreateDirectory(appDataRoaming, NULL);

			logFile = appDataLocal + logFile;
			pathRoaming = appDataRoaming;
		} else {
			// portable
			pathRoaming = pathCurrent + _T("\\");
			logFile = pathRoaming + logFile;
			CString contactsFile = _T("Contacts.xml");
			CopyFile(appDataRoaming + iniFile, pathRoaming + iniFile,  TRUE);
			CopyFile(appDataRoaming + contactsFile, pathRoaming + contactsFile,  TRUE);
		}

		iniFile = pathRoaming + iniFile;
		
		firstRun = true;
		if (CFile::GetStatus(iniFile, rStatus)) {
			firstRun = false;
		}

	}
	//--

	ptr = updatesInterval.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("updatesInterval"), NULL, ptr, 256, iniFile);
	updatesInterval.ReleaseBuffer();
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("checkUpdatesTime"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	checkUpdatesTime = atoi(CStringA(str));

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("enableLocalAccount"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	enableLocalAccount=str==_T("1")?1:0;

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("autoAnswer"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	autoAnswer=str==_T("1")?1:(str==_T("2")?2:0);

	ptr = userAgent.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("userAgent"), NULL, ptr, 256, iniFile);
	userAgent.ReleaseBuffer();

	ptr = denyIncoming.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("denyIncoming"), NULL, ptr, 256, iniFile);
	denyIncoming.ReleaseBuffer();

	ptr = usersDirectory.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("usersDirectory"), NULL, ptr, 256, iniFile);
	usersDirectory.ReleaseBuffer();

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("localDTMF"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	localDTMF=str==_T("0")?0:1;

	ptr = ringingSound.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("ringingSound"), NULL, ptr, 256, iniFile);
	ringingSound.ReleaseBuffer();
	ptr = audioInputDevice.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("audioInputDevice"), NULL, ptr, 256, iniFile);
	audioInputDevice.ReleaseBuffer();
	ptr = audioOutputDevice.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("audioOutputDevice"), NULL, ptr, 256, iniFile);
	audioOutputDevice.ReleaseBuffer();
	ptr = audioRingDevice.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("audioRingDevice"), NULL, ptr, 256, iniFile);
	audioRingDevice.ReleaseBuffer();
	
	ptr = audioCodecs.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("audioCodecs"), NULL, ptr, 256, iniFile);
	audioCodecs.ReleaseBuffer();

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("sourcePort"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	sourcePort = atoi(CStringA(str));

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("VAD"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	vad = str == "1" ? 1 : 0;

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("EC"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	ec = str==_T("1")?1:0;

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("forceCodec"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	forceCodec = str == "1" ? 1 : 0;

#ifdef _GLOBAL_VIDEO
	ptr = videoCaptureDevice.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("videoCaptureDevice"), NULL, ptr, 256, iniFile);
	videoCaptureDevice.ReleaseBuffer();
	ptr = videoCodec.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("videoCodec"), NULL, ptr, 256, iniFile);
	videoCodec.ReleaseBuffer();
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("disableH264"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	disableH264 = str == "1" ? 1 : 0;
	ptr = bitrateH264.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("bitrateH264"), NULL, ptr, 256, iniFile);
	bitrateH264.ReleaseBuffer();
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("disableH263"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	disableH263 = str == "1" ? 1 : 0;
	ptr = bitrateH263.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("bitrateH263"), NULL, ptr, 256, iniFile);
	bitrateH263.ReleaseBuffer();
#endif

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("mainX"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	mainX = atoi(CStringA(str));
	
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("mainY"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	mainY = atoi(CStringA(str));

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("mainW"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	mainW = atoi(CStringA(str));
	
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("mainH"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	mainH = atoi(CStringA(str));

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("messagesX"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	messagesX = atoi(CStringA(str));
	
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("messagesY"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	messagesY = atoi(CStringA(str));
	
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("messagesW"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	messagesW = atoi(CStringA(str));
	
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("messagesH"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	messagesH = atoi(CStringA(str));

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("callsWidth0"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	callsWidth0 = atoi(CStringA(str));

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("callsWidth1"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	callsWidth1 = atoi(CStringA(str));

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("callsWidth2"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	callsWidth2 = atoi(CStringA(str));

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("callsWidth3"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	callsWidth3 = atoi(CStringA(str));

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("callsWidth4"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	callsWidth4 = atoi(CStringA(str));

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("volumeOutput"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	volumeOutput = str.IsEmpty()?100:atoi(CStringA(str));

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("volumeInput"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	volumeInput = str.IsEmpty()?100:atoi(CStringA(str));

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("activeTab"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	activeTab = atoi(CStringA(str));

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("alwaysOnTop"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	alwaysOnTop = atoi(CStringA(str));

	ptr = cmdCallStart.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("cmdCallStart"), NULL, ptr, 256, iniFile);
	cmdCallStart.ReleaseBuffer();

	ptr = cmdCallEnd.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("cmdCallEnd"), NULL, ptr, 256, iniFile);
	cmdCallEnd.ReleaseBuffer();

	ptr = cmdIncomingCall.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("cmdIncomingCall"), NULL, ptr, 256, iniFile);
	cmdIncomingCall.ReleaseBuffer();

	ptr = cmdCallAnswer.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("cmdCallAnswer"), NULL, ptr, 256, iniFile);
	cmdCallAnswer.ReleaseBuffer();

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("enableLog"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	enableLog=str==_T("1")?1:0;

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("randomAnswerBox"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	randomAnswerBox=str==_T("1")?1:0;

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("singleMode"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	singleMode=str==_T("0")?0:1;

	hidden = 0;

	//--
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("accountId"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	if (str.IsEmpty()) {
		if (AccountLoad(-1,&account)) {
			accountId = 1;
			WritePrivateProfileString(_T("Settings"),_T("accountId"), _T("1"), iniFile);
		}
	} else {
		accountId=atoi(CStringA(str));
		if (!accountId && !enableLocalAccount) {
			accountId = 1;
		}
		if (accountId>0) {
			if (!AccountLoad(accountId,&account)) {
				accountId = 0;
			}
		}
	}

}

AccountSettings::AccountSettings()
{
	Init();
}

void AccountSettings::AccountDelete(int id)
{
	CString section;
	section.Format(_T("Account%d"),id);
	WritePrivateProfileStruct(section, NULL, NULL, 0, iniFile);
}

bool AccountSettings::AccountLoad(int id, Account *account)
{
	CString str;
	CString rab;
	LPTSTR ptr;

	CString section;
	if (id==-1) {
		section = _T("Settings");
	} else {
		section.Format(_T("Account%d"),id);
	}

	ptr = account->server.GetBuffer(255);
	GetPrivateProfileString(section,_T("server"), NULL, ptr, 256, iniFile);
	account->server.ReleaseBuffer();

	ptr = account->proxy.GetBuffer(255);
	GetPrivateProfileString(section,_T("proxy"), NULL, ptr, 256, iniFile);
	account->proxy.ReleaseBuffer();

	ptr = account->domain.GetBuffer(255);
	GetPrivateProfileString(section,_T("domain"), NULL, ptr, 256, iniFile);
	account->domain.ReleaseBuffer();


	CString usernameLocal;
	CString passwordLocal;

	ptr = usernameLocal.GetBuffer(255);
	GetPrivateProfileString(section,_T("username"), NULL, ptr, 256, iniFile);
	usernameLocal.ReleaseBuffer();

	ptr = passwordLocal.GetBuffer(255);
	GetPrivateProfileString(section,_T("password"), NULL, ptr, 256, iniFile);
	passwordLocal.ReleaseBuffer();
	if (!passwordLocal.IsEmpty()) {
		CByteArray arPassword;
		String2Bin(passwordLocal, &arPassword);
		CCrypto crypto;
		if (crypto.DeriveKey((LPCTSTR)_GLOBAL_KEY)) {
			try {			
				if (!crypto.Decrypt(arPassword,passwordLocal)) {
					//--decode from old format
					ptr = str.GetBuffer(255);
					GetPrivateProfileString(section,_T("passwordSize"), NULL, ptr, 256, iniFile);
					str.ReleaseBuffer();
					if (!str.IsEmpty()) {
						int size = atoi(CStringA(str));
						arPassword.SetSize(size>0?size:16);
						GetPrivateProfileStruct(section,_T("password"), arPassword.GetData(),arPassword.GetSize(), iniFile);
						crypto.Decrypt(arPassword,passwordLocal);
					}
					//--end decode from old format
					if (crypto.Encrypt(passwordLocal,arPassword)) {
						WritePrivateProfileString(section,_T("password"), Bin2String(&arPassword), iniFile);
						//--delete old format addl.data
						WritePrivateProfileString(section,_T("passwordSize"),NULL,iniFile);
					}
				}
			} catch (CArchiveException *e) {
			}
		}
	}

	account->rememberPassword = passwordLocal.GetLength()?1:0;


	account->username = usernameLocal;
	account->password = passwordLocal;


	ptr = account->authID.GetBuffer(255);
	GetPrivateProfileString(section,_T("authID"), NULL, ptr, 256, iniFile);
	account->authID.ReleaseBuffer();

	ptr = account->displayName.GetBuffer(255);
	GetPrivateProfileString(section,_T("displayName"), NULL, ptr, 256, iniFile);
	account->displayName.ReleaseBuffer();

	ptr = account->srtp.GetBuffer(255);
	GetPrivateProfileString(section,_T("SRTP"), NULL, ptr, 256, iniFile);
	account->srtp.ReleaseBuffer();

	ptr = account->transport.GetBuffer(255);
	GetPrivateProfileString(section,_T("transport"), NULL, ptr, 256, iniFile);
	account->transport.ReleaseBuffer();

	ptr = account->publicAddr.GetBuffer(255);
	GetPrivateProfileString(section,_T("publicAddr"), NULL, ptr, 256, iniFile);
	account->publicAddr.ReleaseBuffer();

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(section,_T("publish"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	account->publish=str==_T("1")?1:0;


	ptr = account->stun.GetBuffer(255);
	GetPrivateProfileString(section,_T("STUN"), NULL, ptr, 256, iniFile);
	account->stun.ReleaseBuffer();

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(section,_T("ICE"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	account->ice=str==_T("1")?1:0;

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(section,_T("allowRewrite"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	account->allowRewrite=str==_T("1")?1:0;

	bool sectionExists = IniSectionExists(section,iniFile);

	if (id==-1) {
		// delete old
		WritePrivateProfileString(section,_T("server"), NULL, iniFile);
		WritePrivateProfileString(section,_T("proxy"), NULL, iniFile);
		WritePrivateProfileString(section,_T("SRTP"), NULL, iniFile);
		WritePrivateProfileString(section,_T("transport"), NULL, iniFile);
		WritePrivateProfileString(section,_T("publicAddr"), NULL, iniFile);
		WritePrivateProfileString(section,_T("publish"), NULL, iniFile);
		WritePrivateProfileString(section,_T("STUN"), NULL, iniFile);
		WritePrivateProfileString(section,_T("ICE"), NULL, iniFile);
		WritePrivateProfileString(section,_T("allowRewrite"), NULL, iniFile);
		WritePrivateProfileString(section,_T("domain"), NULL, iniFile);
		WritePrivateProfileString(section,_T("authID"), NULL, iniFile);
		WritePrivateProfileString(section,_T("username"), NULL, iniFile);
		WritePrivateProfileString(section,_T("passwordSize"), NULL, iniFile);
		WritePrivateProfileString(section,_T("password"), NULL, iniFile);
		WritePrivateProfileString(section,_T("id"), NULL, iniFile);
		WritePrivateProfileString(section,_T("displayName"), NULL, iniFile);
		// save new
		//if (!account->domain.IsEmpty() && !account->username.IsEmpty()) {
		if (sectionExists && !account->domain.IsEmpty()) {
			AccountSave(1, account);
		}
	}
	//return !account->domain.IsEmpty() && !account->username.IsEmpty();
	return  sectionExists && !account->domain.IsEmpty();
}

void AccountSettings::AccountSave(int id, Account *account)
{
	CString section;
	section.Format(_T("Account%d"),id);

	WritePrivateProfileString(section,_T("server"),account->server,iniFile);

	WritePrivateProfileString(section,_T("proxy"),account->proxy,iniFile);

	WritePrivateProfileString(section,_T("domain"),account->domain,iniFile);

	CString usernameLocal;
	CString passwordLocal;

	usernameLocal = account->username;
	passwordLocal = account->password;

	if (!account->rememberPassword) {
		WritePrivateProfileString(section,_T("username"),_T(""),iniFile);
		WritePrivateProfileString(section,_T("password"),_T(""),iniFile);
	}

	if (account->rememberPassword) {
		WritePrivateProfileString(section,_T("username"),usernameLocal,iniFile);
	CCrypto crypto;
	CByteArray arPassword;
	if (crypto.DeriveKey((LPCTSTR)_GLOBAL_KEY)
		&& crypto.Encrypt(passwordLocal,arPassword)
		) {
			WritePrivateProfileString(section,_T("password"), Bin2String(&arPassword), iniFile);
	} else {
		WritePrivateProfileString(section,_T("password"), passwordLocal, iniFile);
	}
	}

	WritePrivateProfileString(section,_T("authID"),account->authID,iniFile);

	WritePrivateProfileString(section,_T("displayName"),account->displayName,iniFile);

	WritePrivateProfileString(section,_T("transport"),account->transport,iniFile);

	WritePrivateProfileString(section,_T("STUN"),account->stun,iniFile);

	WritePrivateProfileString(section,_T("publicAddr"),account->publicAddr,iniFile);
	WritePrivateProfileString(section,_T("SRTP"),account->srtp,iniFile);
	WritePrivateProfileString(section,_T("publish"),account->publish?_T("1"):_T("0"),iniFile);
	WritePrivateProfileString(section,_T("ICE"),account->ice?_T("1"):_T("0"),iniFile);
	WritePrivateProfileString(section,_T("allowRewrite"),account->allowRewrite?_T("1"):_T("0"),iniFile);
}

void AccountSettings::SettingsSave()
{
	CString str;

	str.Format(_T("%d"),accountId);
	WritePrivateProfileString(_T("Settings"),_T("accountId"),str,iniFile);

	WritePrivateProfileString(_T("Settings"),_T("enableLocalAccount"),enableLocalAccount?_T("1"):_T("0"),iniFile);

	WritePrivateProfileString(_T("Settings"),_T("enableLog"),enableLog?_T("1"):_T("0"),iniFile);

	WritePrivateProfileString(_T("Settings"),_T("randomAnswerBox"),randomAnswerBox?_T("1"):_T("0"),iniFile);

	WritePrivateProfileString(_T("Settings"),_T("updatesInterval"),updatesInterval,iniFile);
	str.Format(_T("%d"),checkUpdatesTime);
	WritePrivateProfileString(_T("Settings"),_T("checkUpdatesTime"),str,iniFile);

	WritePrivateProfileString(_T("Settings"),_T("autoAnswer"),autoAnswer==1?_T("1"):(autoAnswer==2?_T("2"):_T("0")),iniFile);
	WritePrivateProfileString(_T("Settings"),_T("denyIncoming"),denyIncoming,iniFile);

	WritePrivateProfileString(_T("Settings"),_T("usersDirectory"),usersDirectory,iniFile);

	WritePrivateProfileString(_T("Settings"),_T("singleMode"),singleMode?_T("1"):_T("0"),iniFile);

	WritePrivateProfileString(_T("Settings"),_T("localDTMF"),localDTMF?_T("1"):_T("0"),iniFile);
	WritePrivateProfileString(_T("Settings"),_T("ringingSound"),ringingSound,iniFile);
	WritePrivateProfileString(_T("Settings"),_T("audioInputDevice"),_T("\"")+audioInputDevice+_T("\""),iniFile);
	WritePrivateProfileString(_T("Settings"),_T("audioOutputDevice"),_T("\"")+audioOutputDevice+_T("\""),iniFile);
	WritePrivateProfileString(_T("Settings"),_T("audioRingDevice"),_T("\"")+audioRingDevice+_T("\""),iniFile);
	WritePrivateProfileString(_T("Settings"),_T("audioCodecs"),audioCodecs,iniFile);
	WritePrivateProfileString(_T("Settings"),_T("VAD"),vad?_T("1"):_T("0"),iniFile);
	WritePrivateProfileString(_T("Settings"),_T("EC"),ec?_T("1"):_T("0"),iniFile);
	WritePrivateProfileString(_T("Settings"),_T("forceCodec"),forceCodec?_T("1"):_T("0"),iniFile);
#ifdef _GLOBAL_VIDEO
	WritePrivateProfileString(_T("Settings"),_T("videoCaptureDevice"),_T("\"")+videoCaptureDevice+_T("\""),iniFile);
	WritePrivateProfileString(_T("Settings"),_T("videoCodec"),videoCodec,iniFile);
	WritePrivateProfileString(_T("Settings"),_T("disableH264"),disableH264?_T("1"):_T("0"),iniFile);
	WritePrivateProfileString(_T("Settings"),_T("bitrateH264"),bitrateH264,iniFile);
	WritePrivateProfileString(_T("Settings"),_T("disableH263"),disableH263?_T("1"):_T("0"),iniFile);
	WritePrivateProfileString(_T("Settings"),_T("bitrateH263"),bitrateH263,iniFile);
#endif

	str.Format(_T("%d"),mainX);
	WritePrivateProfileString(_T("Settings"),_T("mainX"),str,iniFile);

	str.Format(_T("%d"),mainY);
	WritePrivateProfileString(_T("Settings"),_T("mainY"),str,iniFile);

	str.Format(_T("%d"),mainW);
	WritePrivateProfileString(_T("Settings"),_T("mainW"),str,iniFile);

	str.Format(_T("%d"),mainH);
	WritePrivateProfileString(_T("Settings"),_T("mainH"),str,iniFile);

	str.Format(_T("%d"),messagesX);
	WritePrivateProfileString(_T("Settings"),_T("messagesX"),str,iniFile);

	str.Format(_T("%d"),messagesY);
	WritePrivateProfileString(_T("Settings"),_T("messagesY"),str,iniFile);

	str.Format(_T("%d"),messagesW);
	WritePrivateProfileString(_T("Settings"),_T("messagesW"),str,iniFile);

	str.Format(_T("%d"),messagesH);
	WritePrivateProfileString(_T("Settings"),_T("messagesH"),str,iniFile);

	str.Format(_T("%d"),callsWidth0);
	WritePrivateProfileString(_T("Settings"),_T("callsWidth0"),str,iniFile);
		
	str.Format(_T("%d"),callsWidth1);
	WritePrivateProfileString(_T("Settings"),_T("callsWidth1"),str,iniFile);

	str.Format(_T("%d"),callsWidth2);
	WritePrivateProfileString(_T("Settings"),_T("callsWidth2"),str,iniFile);

	str.Format(_T("%d"),callsWidth3);
	WritePrivateProfileString(_T("Settings"),_T("callsWidth3"),str,iniFile);

	str.Format(_T("%d"),callsWidth4);
	WritePrivateProfileString(_T("Settings"),_T("callsWidth4"),str,iniFile);

	str.Format(_T("%d"),volumeOutput);
	WritePrivateProfileString(_T("Settings"),_T("volumeOutput"),str,iniFile);

	str.Format(_T("%d"),volumeInput);
	WritePrivateProfileString(_T("Settings"),_T("volumeInput"),str,iniFile);

	str.Format(_T("%d"),activeTab);
	WritePrivateProfileString(_T("Settings"),_T("activeTab"),str,iniFile);

	str.Format(_T("%d"),alwaysOnTop);
	WritePrivateProfileString(_T("Settings"),_T("alwaysOnTop"),str,iniFile);
	
	WritePrivateProfileString(_T("Settings"),_T("cmdCallStart"),cmdCallStart,iniFile);
	WritePrivateProfileString(_T("Settings"),_T("cmdCallEnd"),cmdCallEnd,iniFile);
	WritePrivateProfileString(_T("Settings"),_T("cmdIncomingCall"),cmdIncomingCall,iniFile);
	WritePrivateProfileString(_T("Settings"),_T("cmdCallAnswer"),cmdCallAnswer,iniFile);
}
