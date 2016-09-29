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
#include "global.h"
#include "settings.h"
#include "utf.h"
#include "langpack.h"
#include <afxinet.h>

#ifdef UNICODE
#define CF_TEXT_T CF_UNICODETEXT
#else
#define CF_TEXT_T CF_TEXT
#endif

struct call_tonegen_data *tone_gen = NULL;
int transport;
pjsua_acc_id account;
pjsua_acc_id account_local;

CString GetErrorMessage(pj_status_t status)
{
	CStringA str;
	char *buf = str.GetBuffer(PJ_ERR_MSG_SIZE-1);
	pj_strerror(status, buf, PJ_ERR_MSG_SIZE);
	str.ReleaseBuffer();
	int i = str.ReverseFind( '(' );
	if (i!=-1) {
		str = str.Left(i-1);
	}
	if (str == "Invalid Request URI" || str == "Invalid URI") {
		str = "Invalid number";
	}
	return Translate(CString(str).GetBuffer());
}

BOOL ShowErrorMessage(pj_status_t status)
{
	if (status!=PJ_SUCCESS) {
		AfxMessageBox(GetErrorMessage(status));
		return TRUE;
	} else {
		return FALSE;
	}
}

CString RemovePort(CString domain)
{
	int pos = domain.Find(_T(":"));
	if (pos != -1) {
		return domain.Mid(0,pos);
	} else {
		return domain;
	}
}

BOOL IsIP(CString host)
{
	CStringA hostA(host);
	char *pHost = hostA.GetBuffer();
	unsigned long ulAddr  = inet_addr(pHost);
	if (ulAddr !=INADDR_NONE && ulAddr != INADDR_ANY) {
		struct in_addr antelope;
		antelope.S_un.S_addr = ulAddr;
		if (strcmp(inet_ntoa(antelope),pHost)==0) {
			return TRUE;
		}
	}
	return FALSE;
}

void ParseSIPURI(CString in, SIPURI* out)
{
	//	tone_gen.toneslot = -1;
	//	tone_gen = NULL;

	// "WEwewew rewewe" <sip:qqweqwe@qwerer.com;qweqwe=rrr>
	out->domain = _T("");
	out->user = _T("");
	out->name = _T("");

	int start = in.Find( _T("sip:") );
	int end;
	if (start>0)
	{
		out->name = in.Left(start);
		out->name.Trim(_T(" \" <"));
		if (!out->name.CompareNoCase(_T("unknown")))
		{
			out->name = _T("");
		}
	}
	if (start>=0)
	{
		start+=4;
	} else {
		start = 0;
	}

	end = in.Find( _T("@"), start );
	if (end>=0)
	{
		out->user=in.Mid(start,end-start);
		start=end+1;
	}
	end = in.Find( _T(";"), start );
	if (end<0)
	{
		end = in.Find( _T(">"), start );
	}
	if (end>=0)
	{
		out->domain=in.Mid(start,end-start);
	} else {
		out->domain=in.Mid(start);
	}
}

CString PjToStr(const pj_str_t* str, BOOL utf)
{
	CStringA rab;
	rab.Format("%.*s", str->slen, str->ptr);
	if (utf)
	{
#ifdef _UNICODE
		WCHAR* msg;
		Utf8DecodeCP(rab.GetBuffer(), CP_ACP, &msg);
		return msg;
#else
		return Utf8DecodeCP(rab.GetBuffer(), CP_ACP, NULL);
#endif
	} else 
	{
		return CString(rab);
	}
}

pj_str_t StrToPjStr(CString str)
{
	return pj_str(StrToPj(str));
}

char* StrToPj(CString str)
{
#ifdef _UNICODE
	return Utf8EncodeUcs2(str.GetBuffer());
#else
	return Utf8EncodeCP(str.GetBuffer(), CP_ACP);
#endif
}

CString Utf8DecodeUni(CStringA str)
{
#ifdef _UNICODE
	LPTSTR msg;
	Utf8DecodeCP(str.GetBuffer(), CP_ACP, &msg);
	return msg;
#else
	return Utf8DecodeCP(str.GetBuffer(), CP_ACP, NULL);
#endif
}

CStringA UnicodeToAnsi(CString str)
{
	CStringA res; 
	int nCount = str.GetLength();            
	for( int nIdx =0; nIdx < nCount; nIdx++ )      
	{          
		res+=str[nIdx];
	}
	return res;
}

CString AnsiToUnicode(CStringA str)
{
	CString res; 
	int nCount = str.GetLength();            
	for( int nIdx =0; nIdx < nCount; nIdx++ )      
	{          
		res+=str[nIdx];
	}
	return res;
}

CString XMLEntityDecode(CString str)
{
	str.Replace(_T("&lt;"),_T("<"));
	str.Replace(_T("&gt;"),_T(">"));
	str.Replace(_T("&quot;"),_T("\""));
	str.Replace(_T("&amp;"),_T("&"));
	return str;
}

CString XMLEntityEncode(CString str)
{
	str.Replace(_T("&"),_T("&amp;"));
	str.Replace(_T("<"),_T("&lt;"));
	str.Replace(_T(">"),_T("&gt;"));
	str.Replace(_T("\""),_T("&quot;"));
	return str;
}

void OpenURL(CString url)
{
	CString param;
	param.Format(_T("url.dll,FileProtocolHandler %s"),url);
	ShellExecute(NULL, NULL, _T("rundll32.exe"), param, NULL, SW_SHOWNORMAL);
}

CString GetDuration(int sec, bool zero)
{
	CString duration;
	if (sec || zero) {
		int h,m,s;
		s = sec;
		h = s/3600;
		s = s%3600;
		m = s/60;
		s = s%60;
		if (h) {
			duration.Format(_T("%d:%02d:%02d"),h,m,s);
		} else {
			duration.Format(_T("%d:%02d"),m,s);
		}
	}
	return duration;
}

void AddTransportSuffix(CString &str)
{
	switch (transport)
	{ 
	case MSIP_TRANSPORT_TCP:
		str.Append(_T(";transport=tcp"));
		break;
	case MSIP_TRANSPORT_TLS:
		str.Append(_T(";transport=tls"));
		break;
	}
}

CString GetSIPURI(CString str, bool isSimple, bool isLocal)
{
	CString rab = str;
	rab.MakeLower();
	int pos = rab.Find(_T("sip:"));
	if (pos==-1)
	{
		str=_T("sip:")+str;
	}
	if (!isLocal) {
		pos = str.Find(_T("@"));
		if (accountSettings.accountId && pos==-1) {
			str.Append(_T("@")+accountSettings.account.domain);
		}
	}
	if (str.GetAt(str.GetLength()-1)=='>')
	{
		str = str.Left(str.GetLength()-1);
		if (!isSimple) {
			if (!isLocal || !accountSettings.accountId) {
				AddTransportSuffix(str);
			}
		}
		str += _T(">");
	} else {
		if (!isSimple) {
			if (!isLocal || !accountSettings.accountId) {
				AddTransportSuffix(str);
			}
		}
		str = _T("<") + str + _T(">");
	}
	return str;
}

bool SelectSIPAccount(CString number, pjsua_acc_id &acc_id, pj_str_t &pj_uri)
{
	SIPURI sipuri;
	ParseSIPURI(number, &sipuri);
	if (pjsua_acc_is_valid(account) && pjsua_acc_is_valid(account_local)) {
		acc_id = account;
		if (accountSettings.account.domain != sipuri.domain) {
			int pos = sipuri.domain.Find(_T(":"));
			CString domainWithoutPort = RemovePort(sipuri.domain);
			if (domainWithoutPort.CompareNoCase(_T("localhost"))==0 || IsIP(domainWithoutPort)) {
				acc_id = account_local;
			}
		}

	} else if (pjsua_acc_is_valid(account)) {
		acc_id = account;
	} else if (pjsua_acc_is_valid(account_local)) {
		acc_id = account_local;
	} else {
		return false;
	}
	pj_uri = StrToPjStr ( GetSIPURI(number, acc_id == account_local, acc_id == account_local) );
	return true;
}

CString FormatNumber(CString number) {
	CString numberFormated = number;
	pjsua_acc_id acc_id;
	pj_str_t pj_uri;
	bool isLocal = SelectSIPAccount(number,acc_id,pj_uri) && acc_id == account_local;
	if (!isLocal) {
		BOOL isDigits = TRUE;
		for (int i=0;i<number.GetLength();i++)
		{
			if ( (number[i]>'9' || number[i]<'0') && number[i]!='*' && number[i]!='#' && number[i]!='.' && number[i]!='-' && number[i]!='(' && number[i]!=')' && number[i]!=' ' && number[0]!='+')
			{
				isDigits = FALSE;
				break;
			}
		}
		if (isDigits) {
			numberFormated.Remove('.');
			numberFormated.Remove('-');
			numberFormated.Remove('(');
			numberFormated.Remove(')');
			numberFormated.Remove(' ');
		}
	}
	return GetSIPURI(numberFormated,true,isLocal);
}

bool IniSectionExists(CString section, CString iniFile)
{
	CString str;
	LPTSTR ptr = str.GetBuffer(3);
	int result = GetPrivateProfileString(section,NULL, NULL, ptr, 3, iniFile);
	str.ReleaseBuffer();
	return result;
}

void OpenHelp(CString code)
{
	CString url = _T(_GLOBAL_HELP_WEBSITE);
	url.Append(_T("#"));
	OpenURL(url+code);
}

struct call_tonegen_data *call_init_tonegen(pjsua_call_id call_id)
{
	pj_pool_t *pool;
	struct call_tonegen_data *cd;
	pjsua_call_info ci;

	if (call_id !=-1 ) {
		pjsua_call_get_info(call_id, &ci);

		if (ci.media_status != PJSUA_CALL_MEDIA_ACTIVE)
			return NULL;
	}

	pool = pjsua_pool_create("mycall", 512, 512);
	cd = PJ_POOL_ZALLOC_T(pool, struct call_tonegen_data);
	cd->pool = pool;

	pjmedia_tonegen_create(cd->pool, 8000, 1, 160, 16, 0, &cd->tonegen);
	pjsua_conf_add_port(cd->pool, cd->tonegen, &cd->toneslot);

	if (call_id !=-1 ) {
		pjsua_conf_connect(cd->toneslot, ci.conf_slot);
	}
	if (accountSettings.localDTMF) {
		pjsua_conf_connect(cd->toneslot, 0);
	}

	if (call_id !=-1 ) {
			call_user_data *user_data = (call_user_data *)pjsua_call_get_user_data(call_id);
			if (!user_data) {
				user_data = new call_user_data();
				pjsua_call_set_user_data(call_id, user_data);
			}
			user_data->tonegen_data = cd;
	}
	return cd;
}

static UINT_PTR destroyDTMFPlayerTimer = NULL;

void destroyDTMFPlayer(
  HWND hwnd,
  UINT uMsg,
  UINT_PTR idEvent,
  DWORD dwTime
						   ) 
{
		if (!tone_gen || !pjmedia_tonegen_is_busy(tone_gen->tonegen)) {
			if (destroyDTMFPlayerTimer) {
				KillTimer(NULL,destroyDTMFPlayerTimer);
				destroyDTMFPlayerTimer = NULL;
			}
			call_deinit_tonegen(-1);
		}
}

BOOL call_play_digit(pjsua_call_id call_id, const char *digits)
{
	pjmedia_tone_digit d[16];
	unsigned i, count = strlen(digits);
	struct call_tonegen_data *cd;

	if (call_id !=-1 ) {
			call_user_data *user_data = (call_user_data *)pjsua_call_get_user_data(call_id);
			if (user_data && user_data->tonegen_data) {
				cd = user_data->tonegen_data;
			} else {
				cd = NULL;
			}
	} else {
		cd = tone_gen;
	}
	if (!cd)
		cd = call_init_tonegen(call_id);
	if (!cd) 
		return FALSE;
	if (call_id == -1 ) {
		tone_gen = cd;
	}

	if (count > PJ_ARRAY_SIZE(d))
		count = PJ_ARRAY_SIZE(d);

	pj_bzero(d, sizeof(d));
	for (i=0; i<count; ++i) {
		d[i].digit = digits[i];
		if (call_id !=-1 ) {
			d[i].on_msec = 260;
			d[i].off_msec = 200;
		} else {
			d[i].on_msec = 120;
			d[i].off_msec = 50;
		}
		d[i].volume = 0;
	}

	pjmedia_tonegen_play_digits(cd->tonegen, count, d, 0);
	if (call_id == -1 ) {
		if (destroyDTMFPlayerTimer) {
			KillTimer(NULL,destroyDTMFPlayerTimer);
		}
		destroyDTMFPlayerTimer = SetTimer(NULL,NULL,5000,(TIMERPROC)destroyDTMFPlayer);
	}
	return TRUE;
}

void call_deinit_tonegen(pjsua_call_id call_id)
{
	struct call_tonegen_data *cd;
	call_user_data *user_data = NULL;

	if (call_id !=-1 ) {
		user_data = (call_user_data *)pjsua_call_get_user_data(call_id);
		if (user_data && user_data->tonegen_data) {
			cd = user_data->tonegen_data;
		} else {
			cd = NULL;
		}
	} else {
		cd = tone_gen;
	}
	if (!cd)
		return;

	pjsua_conf_remove_port(cd->toneslot);
	pjmedia_port_destroy(cd->tonegen);
	pj_pool_release(cd->pool);

	if (call_id !=-1 ) {
		if (user_data) {
			user_data->tonegen_data = NULL;
		}
	} else {
		tone_gen = NULL;
	}
}

void call_hangup_fast(pjsua_call_id call_id,pjsua_call_info *p_call_info)
{
	pjsua_call_info call_info;
	if (!p_call_info) {
		if (pjsua_call_get_info(call_id, &call_info) == PJ_SUCCESS) {
			p_call_info = &call_info;
		}
	}
	if (p_call_info) {
		if (p_call_info->state==PJSIP_INV_STATE_CALLING
			|| (p_call_info->role==PJSIP_ROLE_UAS && p_call_info->state==PJSIP_INV_STATE_CONNECTING)
			) {
				pjsua_call *call = &pjsua_var.calls[call_id];
				pjsip_tx_data *tdata = NULL;
				// Generate an INVITE END message
				if (pjsip_inv_end_session(call->inv, 487, NULL, &tdata) != PJ_SUCCESS || !tdata) {
					pjsip_inv_terminate(call->inv,487,PJ_TRUE);
				} else {
					// Send that END request
					if (pjsip_endpt_send_request(pjsua_get_pjsip_endpt(), tdata, -1, NULL, NULL) != PJ_SUCCESS) {
						pjsip_inv_terminate(call->inv,487,PJ_TRUE);
					}
				}
				return;
		}
	}
	pjsua_call_hangup(call_id, 0, NULL, NULL);
}

unsigned call_get_count_noincoming()
{
	unsigned noincoming_count = 0;
	pjsua_call_id call_ids[PJSUA_MAX_CALLS];
	unsigned count = PJSUA_MAX_CALLS;
	if (pjsua_enum_calls ( call_ids, &count)==PJ_SUCCESS)  {
		for (unsigned i = 0; i < count; ++i) {
			pjsua_call_info call_info;
			pjsua_call_get_info(call_ids[i], &call_info);
			if (call_info.role!=PJSIP_ROLE_UAS || (call_info.state!=PJSIP_INV_STATE_INCOMING && call_info.state!=PJSIP_INV_STATE_EARLY)) {
				noincoming_count++;
			}
		}
	}
	return noincoming_count;
}

void call_hangup_all_noincoming(bool onHold)
{
	pjsua_call_id call_ids[PJSUA_MAX_CALLS];
	unsigned count = PJSUA_MAX_CALLS;
	if (pjsua_enum_calls ( call_ids, &count)==PJ_SUCCESS)  {
		for (unsigned i = 0; i < count; ++i) {
			pjsua_call_info call_info;
			pjsua_call_get_info(call_ids[i], &call_info);
			if (call_info.role!=PJSIP_ROLE_UAS || (call_info.state!=PJSIP_INV_STATE_INCOMING && call_info.state!=PJSIP_INV_STATE_EARLY)) {
				if (onHold && call_info.media_status == PJSUA_CALL_MEDIA_LOCAL_HOLD) {
					continue;
				}
				call_hangup_fast(call_ids[i]);
			}
		}
	}
}

void call_hold_all_except(pjsua_call_id call_id)
{
	pjsua_call_id call_ids[PJSUA_MAX_CALLS];
	unsigned count = PJSUA_MAX_CALLS;
	if (pjsua_enum_calls ( call_ids, &count)==PJ_SUCCESS)  {
		for (unsigned i = 0; i < count; ++i) {
			if (call_id == PJSUA_INVALID_ID || call_ids[i] != call_id ) {
				pjsua_call_info call_info;
				pjsua_call_get_info(call_ids[i], &call_info);
				if (call_info.media_cnt>0) {
					if (call_info.media_status != PJSUA_CALL_MEDIA_LOCAL_HOLD && call_info.media_status != PJSUA_CALL_MEDIA_NONE) {
						pjsua_call_set_hold(call_info.id, NULL);
					}
				}
			}
		}
	}
}

CStringA urlencode(CStringA str)
{
    CStringA escaped;
	int max = str.GetLength();
    for(int i=0; i<max; i++)
    {
		const char chr = str.GetAt(i);
        if ( (48 <= chr && chr <= 57) ||//0-9
             (65 <= chr && chr <= 90) ||//abc...xyz
             (97 <= chr && chr <= 122) || //ABC...XYZ
             (chr=='~' || chr=='!' || chr=='*' || chr=='(' || chr==')' || chr=='\'')
        )
        {
			escaped.AppendFormat("%c",chr);
        }
        else
        {
            escaped.Append("%");
            escaped.Append(char2hex(chr));//converts char 255 to string "ff"
        }
    }
    return escaped;
}

CStringA char2hex( char dec )
{
    char dig1 = (dec&0xF0)>>4;
    char dig2 = (dec&0x0F);
    if ( 0<= dig1 && dig1<= 9) dig1+=48;    //0,48inascii
    if (10<= dig1 && dig1<=15) dig1+=97-10; //a,97inascii
    if ( 0<= dig2 && dig2<= 9) dig2+=48;
    if (10<= dig2 && dig2<=15) dig2+=97-10;

    CStringA r;
    r.AppendFormat ("%c", dig1);
    r.AppendFormat ("%c", dig2);
    return r;
}

static DWORD WINAPI URLGetAsyncThread( LPVOID lpParam ) 
{
	URLGetAsyncData *data = (URLGetAsyncData *)lpParam;
	if (!data->url.IsEmpty()) {
		CInternetSession session;
		try {
			CHttpFile* pFile;
			pFile = (CHttpFile*)session.OpenURL(data->url, NULL, INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE);
			if (pFile) {
				pFile->QueryInfoStatusCode(data->statusCode);
				CStringA buf;
				char pBuf[256];
				int i;
				do {
					i = pFile->Read(pBuf,255);
					pBuf[i] = 0;
					data->body.AppendFormat("%s",pBuf);
				} while (i>0);
				//--
				pFile->QueryInfo(
					HTTP_QUERY_RAW_HEADERS_CRLF,
					data->headers
					);
				pFile->Close();
			}
			session.Close();
		} catch (CInternetException *e) {
			data->statusCode = 0;
		}
	}
	if (data->message) {
		SendMessage(data->hWnd,data->message,(WPARAM)data,0);
	}
	delete data;
	return 0;
}

void URLGetAsync(CString url, HWND hWnd, UINT message)
{
	HANDLE hThread;
	URLGetAsyncData *data = new URLGetAsyncData();
	data->hWnd = hWnd;
	data->message = message;
	data->statusCode = 0;
	data->url = url;
	if (!CreateThread(NULL,0, URLGetAsyncThread, data, 0, NULL)) {
		data->url.Empty();
		URLGetAsyncThread(data);
	}
}

CString Bin2String(CByteArray *ca)
{
	CString res;
	int k=ca->GetSize();
	for(int i=0;i<k;i++) {
		unsigned char ch=ca->GetAt(i);
		res.AppendFormat(_T("%02x"),ca->GetAt(i));
	}
	return res;
}

void String2Bin(CString str, CByteArray *res)
{
	res->RemoveAll();
	int k=str.GetLength();
	CStringA rab;
	for(int i=0;i<str.GetLength();i+=2) {
		rab = CStringA(str.Mid(i,2));
		char *p = NULL;
		unsigned long bin = strtoul(rab.GetBuffer(), &p, 16);
		res->Add(bin);
	}
}

