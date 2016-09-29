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

#include "global.h"

struct Account {
	CString server;
	CString proxy;
	CString username;
	CString domain;
	int port;
	CString authID;
	CString password;
	int rememberPassword;
	CString displayName;
	CString srtp;
	CString transport;
	CString publicAddr;
	int publish;
	CString autoTransferExtension;
	int enableAutoTransfer;
	CString stun;
	int ice;
	int allowRewrite;
};

struct AccountSettings {

	int accountId;

	Account account;

	CString userAgent;
	int autoAnswer;
	CString denyIncoming;
	CString usersDirectory;
	int singleMode;
	int enableLocalAccount;
	int enableLog;
	int randomAnswerBox;
	CString ringingSound;
	CString audioInputDevice;
	CString audioOutputDevice;
	CString audioRingDevice;
	CString audioCodecs;
	int sourcePort;
	int vad;
	int ec;
	int forceCodec;
	CString videoCaptureDevice;
	CString videoCodec;
	int disableH264;
	CString bitrateH264;	
	int disableH263;
	CString bitrateH263;
	int localDTMF;

	CString updatesInterval;

	int activeTab;
	int alwaysOnTop;

	int mainX;
	int mainY;
	int mainW;
	int mainH;

	int messagesX;
	int messagesY;
	int messagesW;
	int messagesH;

	int callsWidth0;
	int callsWidth1;
	int callsWidth2;
	int callsWidth3;
	int callsWidth4;

	int volumeOutput;
	int volumeInput;
	
	CString iniFile;
	CString logFile;
	CString pathRoaming;
	int checkUpdatesTime;

	int hidden;
	CString cmdCallStart;
	CString cmdCallEnd;
	CString cmdIncomingCall;
	CString cmdCallAnswer;
	AccountSettings();
	void Init();
	bool AccountLoad(int id, Account *account);
	void AccountSave(int id, Account *account);
	void AccountDelete(int id);
	void SettingsSave();
};
extern AccountSettings accountSettings;
extern bool firstRun;
extern bool pj_ready;
