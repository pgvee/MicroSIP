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

#include "define.h"
#include <pjsua-lib/pjsua.h>
#include <pjsua-lib/pjsua_internal.h>

enum EUserWndMessages
{
	UM_FIRST_USER_MSG = (WM_USER + 0x100 + 1),

	UM_NOTIFYICON,

	UM_CREATE_RINGING,
	UM_CALL_ANSWER,
	UM_CALL_HANGUP,
	UM_ON_ACCOUNT,
	UM_ON_REG_STATE2,
	UM_ON_CALL_STATE,
	UM_ON_CALL_TRANSFER_STATUS,
	UM_ON_CALL_MEDIA_STATE,
	UM_ON_PAGER,
	UM_ON_PAGER_STATUS,
	UM_ON_BUDDY_STATE,
	UM_ON_PLAYER_PLAY,
	UM_ON_PLAYER_STOP,
	UM_SHELL_EXECUTE,
	UM_SET_PANE_TEXT,
	UM_USERS_DIRECTORY,
	UM_ON_BALANCE_PLAIN,
	UM_ON_BALANCE_OPTIONS,
	
	IDT_TIMER_IDLE,
	IDT_TIMER_TONE,
	IDT_TIMER_BALANCE,
	IDT_TIMER_INIT_RINGIN,
	IDT_TIMER_CALL,
	IDT_TIMER_CONTACTS_BLINK,
	IDT_TIMER_DIRECTORY,
	IDT_TIMER_SAVE,
	IDT_TIMER_SWITCH_DEVICES,
	IDT_TIMER_HEADSET,
	
	UM_CLOSETAB,
	UM_DBLCLICKTAB,
	UM_QUERYTAB,
};

enum {MSIP_MESSAGE_TYPE_LOCAL, MSIP_MESSAGE_TYPE_REMOTE, MSIP_MESSAGE_TYPE_SYSTEM};
enum {MSIP_TRANSPORT_AUTO, MSIP_TRANSPORT_TCP, MSIP_TRANSPORT_TLS};
enum {MSIP_CALL_OUT, MSIP_CALL_IN, MSIP_CALL_MISS};
enum {MSIP_SOUND_CUSTOM,MSIP_SOUND_MESSAGE_IN, MSIP_SOUND_MESSAGE_OUT, MSIP_SOUND_HANGUP, MSIP_SOUND_RINGIN, MSIP_SOUND_RINGIN2, MSIP_SOUND_RINGOUT};

struct SIPURI {
	CString user;
	CString domain;
	CString name;
};

struct Contact {
	CString number;
	CString name;
	BOOL presence;
	BOOL directory;
	time_t presenceTime;
	BOOL ringing;
	int image;
	BOOL candidate;
	Contact():presenceTime(0),ringing(FALSE),image(0),candidate(FALSE){}
};

struct MessagesContact {
	CString name;
	CString number;
	CString messages;
	CString message;
	CString lastSystemMessage;
	CTime lastSystemMessageTime;
	pjsua_call_id callId;
	int mediaStatus;
	int adjustmentRx;
	MessagesContact():mediaStatus(PJSUA_CALL_MEDIA_ERROR){}
};

struct Call {
	int key;
	CString id;
	CString name;
	CString number;
	int type;
	int time;
	int duration;
	CString info;
};

struct call_tonegen_data
{
   pj_pool_t          *pool;
   pjmedia_port       *tonegen;
   pjsua_conf_port_id  toneslot;
};

struct call_user_data
{
	call_tonegen_data *tonegen_data;
	call_user_data():tonegen_data(NULL)
		{}
};

extern struct call_tonegen_data *tone_gen;
extern int transport;
extern pjsua_acc_id account;
extern pjsua_acc_id account_local;

CString GetErrorMessage(pj_status_t status);
BOOL ShowErrorMessage(pj_status_t status);
BOOL IsIP(CString host);
CString RemovePort(CString domain);
void ParseSIPURI(CString in, SIPURI* out);
CString PjToStr(const pj_str_t* str, BOOL utf = FALSE);
pj_str_t StrToPjStr(CString str);
char* StrToPj(CString str);
CString Utf8DecodeUni(CStringA str);
CStringA UnicodeToAnsi(CString str);
CString AnsiToUnicode(CStringA str);
CString XMLEntityDecode(CString str);
CString XMLEntityEncode(CString str);
void OpenURL(CString url);
CString GetDuration(int sec, bool zero = false);
void AddTransportSuffix(CString &str);
CString GetSIPURI(CString str, bool isSimple = false, bool isLocal = false);
bool SelectSIPAccount(CString number, pjsua_acc_id &acc_id, pj_str_t &pj_uri);
CString FormatNumber(CString number);
bool IniSectionExists(CString section, CString iniFile);
CString Bin2String(CByteArray *ca);
void String2Bin(CString str, CByteArray *res);

struct call_tonegen_data *call_init_tonegen(pjsua_call_id call_id);
BOOL call_play_digit(pjsua_call_id call_id, const char *digits);
void call_deinit_tonegen(pjsua_call_id call_id);
void destroyDTMFPlayer(
  HWND hwnd,
  UINT uMsg,
  UINT_PTR idEvent,
  DWORD dwTime
						   ) ;
void call_hangup_fast(pjsua_call_id call_id,pjsua_call_info *p_call_info = NULL);

unsigned call_get_count_noincoming();
void call_hangup_all_noincoming(bool onHold=false);
void call_hold_all_except(pjsua_call_id call_id = PJSUA_INVALID_ID);

void OpenHelp(CString code);

typedef struct {
	HWND hWnd;
	UINT message;
	CString url;
	DWORD statusCode;
	CString headers;
	CStringA body;
} URLGetAsyncData;
void URLGetAsync(CString url, HWND hWnd=0, UINT message=0);

CStringA urlencode(CStringA str);
CStringA char2hex(char dec);

