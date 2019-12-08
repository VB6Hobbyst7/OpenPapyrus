// CHZN.CPP
// Copyright (c) A.Sobolev 2019
// @codepage UTF-8
// Реализация интерфейса к сервисам честный знак
//
#include <pp.h>
#pragma hdrstop
//#include <wininet.h>

class ChZnInterface {
public:
	enum {
		qAuth = 1,
		qToken,
		qDocOutcome,
		qCurrentUserInfo,
		qGetIncomeDocList
	};
	SLAPI  ChZnInterface()
	{
	}
	SLAPI ~ChZnInterface()
	{
	}
	enum {
		urloptHttps   = 0x0001,
		urloptSandBox = 0x0002
	};

	struct InitBlock {
		InitBlock() : GuaID(0)
		{
		}
		PPID   GuaID;
		PPGlobalUserAccPacket GuaPack;
		SString CliAccsKey;
		SString CliSecret;
		SString CliIdent;
		SString Cn; // CN субъекта сертификата электронной подписи (PPTAG_GUA_CERTSUBJCN)
		SString CryptoProPath;
		//
		SString Token;
	};

	int    SLAPI SetupInitBlock(PPID guaID, InitBlock & rBlk)
	{
		int    ok = 1;
		PPObjGlobalUserAcc gua_obj;
		THROW(gua_obj.GetPacket(guaID, &rBlk.GuaPack) > 0);
		rBlk.GuaPack.TagL.GetItemStr(PPTAG_GUA_ACCESSKEY, rBlk.CliAccsKey);
		rBlk.GuaPack.TagL.GetItemStr(PPTAG_GUA_SECRET, rBlk.CliSecret);
		rBlk.GuaPack.TagL.GetItemStr(PPTAG_GUA_LOGIN, rBlk.CliIdent); // Отпечаток открытого ключа 
			// Сертификат в реестре находится в "сертификаты-текущий пользователь/личное/реестр/сертификаты". 
			// Требуется в доверенные еще внести сертификат Крипто-Про (в инструкции по быстрому старту про это есть). 
		rBlk.GuaPack.TagL.GetItemStr(PPTAG_GUA_CERTSUBJCN, rBlk.Cn);
		if(rBlk.Cn.NotEmptyS()) {
			//rBlk.Cn.ReplaceStr("\"", " ", 0);
			//rBlk.Cn.ReplaceStr("  ", " ", 0);
			rBlk.Cn.Transf(CTRANSF_INNER_TO_OUTER);
		}
		rBlk.GuaID = guaID;
		{
			PPIniFile ini_file;
			ini_file.Get(PPINISECT_PATH, PPINIPARAM_PATH_CRYPTOPRO, rBlk.CryptoProPath);
		}
		CATCHZOK
		return ok;
	}
	int    SLAPI GetSign(const InitBlock & rIb, const void * pData, size_t dataLen, SString & rResultBuf)
	{
		int    ok = -1;
		rResultBuf.Z();
		if(pData && dataLen) {
			if(rIb.CryptoProPath.NotEmpty() && IsDirectory(rIb.CryptoProPath) && rIb.Cn.NotEmpty()) {
				SString temp_buf;
				//"C:\Program Files\Crypto Pro\CSP\csptest" -sfsign -sign -in barcode-tobacco.7z –out barcode-tobacco.sign -my "ООО ЛУИЗА ПЛЮС" -detached -base64 –add -silent
				temp_buf.Cat(rIb.CryptoProPath).SetLastSlash().Cat("csptest.exe");
				if(fileExists(temp_buf)) {
					SString in_file_name;
					SString out_file_name;
					SString cn;
					PPMakeTempFileName("chzn", "st", 0, in_file_name);
					PPMakeTempFileName("chzn", "sf", 0, out_file_name);
					{
						SFile f_in(in_file_name, SFile::mWrite|SFile::mBinary);
						THROW_SL(f_in.IsValid());
						THROW_SL(f_in.Write(pData, dataLen));
					}
					{
						cn = rIb.Cn;
						cn.ReplaceStr("\"", " ", 0);
						cn.ReplaceStr("  ", " ", 0);
					}
					temp_buf.Space().Cat("-sfsign").Space().Cat("-sign").Space().Cat("-in").Space().Cat(in_file_name).Space().
						Cat("-out").Space().Cat(out_file_name).Space().Cat("-my").Space().CatQStr(cn).Space().Cat("-detached").Space().
						Cat("-base64").Space().Cat("-add");
					STempBuffer cmd_line((temp_buf.Len() + 32) * sizeof(TCHAR));
					STARTUPINFO si;
					DWORD exit_code = 0;
					PROCESS_INFORMATION pi;
					MEMSZERO(si);
					si.cb = sizeof(si);
					MEMSZERO(pi);
					strnzcpy(static_cast<TCHAR *>(cmd_line.vptr()), SUcSwitch(temp_buf), cmd_line.GetSize() / sizeof(TCHAR));
					int    r = ::CreateProcess(0, static_cast<TCHAR *>(cmd_line.vptr()), 0, 0, FALSE, 0, 0, 0, &si, &pi);
					if(!r) {
						SLS.SetOsError(0);
						CALLEXCEPT_PP(PPERR_SLIB);
					}
					WaitForSingleObject(pi.hProcess, INFINITE); // Wait until child process exits.
					{
						GetExitCodeProcess(pi.hProcess, &exit_code);
						if(exit_code == 0) {
							SFile f_result(out_file_name, SFile::mRead);
							THROW_SL(f_result.IsValid());
							while(f_result.ReadLine(temp_buf)) {
								temp_buf.Chomp();
								rResultBuf.Cat(temp_buf);
							}
							ok = 1;
						}
					}
				}
			}
		}
		CATCHZOK
		return ok;
	}
	SString & SLAPI MakeTargetUrl(int query, const InitBlock & rIb, SString & rResult) const
	{
		//rResult = "http://api.sb.mdlp.crpt.ru";
		//(rResult = (oneof2(query, qAuth, qToken) ? "http" : "https")).Cat("://");
		(rResult = (oneof2(query, qAuth, qToken) ? "http" : "http")).Cat("://");
		if(rIb.GuaPack.Rec.Flags & PPGlobalUserAcc::fSandBox) {
			rResult.Cat("api.sb.mdlp.crpt.ru");
		}
		else {
			rResult.Cat("api.mdlp.crpt.ru");
		}
		rResult.SetLastDSlash().Cat("api/v1").SetLastDSlash();
		switch(query) {
			case qAuth:  rResult.Cat("auth"); break;
			case qToken: rResult.Cat("token"); break;
			case qDocOutcome: rResult.Cat("documents/outcome"); break;
			case qCurrentUserInfo: rResult.Cat("users/current"); break;
			case qGetIncomeDocList: rResult.Cat("documents/income"); break;
		}
		return rResult;
	}
	int    SLAPI MakeAuthRequest(InitBlock & rBlk, SString & rBuf)
	{
		rBuf.Z();
		int    ok = 1;
		json_t * p_json_req = 0;
		{
			p_json_req = new json_t(json_t::tOBJECT);
			p_json_req->Insert("client_id", json_new_string(rBlk.CliAccsKey));
			p_json_req->Insert("client_secret", json_new_string(rBlk.CliSecret));
			p_json_req->Insert("user_id", json_new_string(rBlk.CliIdent));
			p_json_req->Insert("auth_type", json_new_string("SIGNED_CODE"));
			THROW_SL(json_tree_to_string(p_json_req, rBuf));
		}
		CATCHZOK
		json_free_value(&p_json_req);
		return ok;
	}
	int    SLAPI MakeTokenRequest(InitBlock & rIb, const char * pAuthCode, SString & rBuf)
	{
		rBuf.Z();
		int    ok = 1;
		SString code_sign;
		json_t * p_json_req = 0;
		THROW(GetSign(rIb, pAuthCode, sstrlen(pAuthCode), code_sign));
		{
			p_json_req = new json_t(json_t::tOBJECT);
			p_json_req->Insert("code", json_new_string(pAuthCode));
			p_json_req->Insert("signature", json_new_string(code_sign));
			THROW_SL(json_tree_to_string(p_json_req, rBuf));
		}
		CATCHZOK
		json_free_value(&p_json_req);
		return ok;
	}
	class WinInternetHandleStack : private TSStack <HINTERNET> {
	public:
		WinInternetHandleStack()
		{
		}
		~WinInternetHandleStack()
		{
			Destroy();
		}
		HINTERNET Push(HINTERNET h)
		{
			if(h) {
				push(h);
			}
			return h;
		}
		void Destroy()
		{
			HINTERNET h = 0;
			while(pop(h)) {
				::InternetCloseHandle(h);
			}
		}
	};
	const CERT_CONTEXT * SLAPI GetClientSslCertificate(InitBlock & rIb)
	{
		uint  sys_err = 0;
		const CERT_CONTEXT * p_cert_context = NULL;
		HCERTSTORE h_store = CertOpenStore(CERT_STORE_PROV_SYSTEM, /*X509_ASN_ENCODING*/0, 0, /*CERT_SYSTEM_STORE_LOCAL_MACHINE*/CERT_SYSTEM_STORE_CURRENT_USER, L"MY");
		if(h_store) {
			SStringU cnu;
			SString temp_buf = rIb.Cn;
			//temp_buf.ReplaceStr("\"", " ", 0);
			//temp_buf.ReplaceStr("  ", " ", 0);
			temp_buf.Strip();
			cnu.CopyFromMb_OUTER(temp_buf, temp_buf.Len());

			//const CERT_CONTEXT * p_cert_next = CertFindCertificateInStore(h_store, X509_ASN_ENCODING|PKCS_7_ASN_ENCODING, 0, 
			CERT_INFO * p_cert_info = 0; // @debug
			wchar_t cert_text[256];
			for(const CERT_CONTEXT * p_cert_next = CertEnumCertificatesInStore(h_store, 0); p_cert_next; p_cert_next = CertEnumCertificatesInStore(h_store, p_cert_next)) {
				p_cert_info = p_cert_next->pCertInfo;
				DWORD para_type = 0;
				CertGetNameString(p_cert_next, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, &para_type, cert_text, SIZEOFARRAY(cert_text));
				if(cnu.IsEqual(cert_text)) {
					p_cert_context = CertDuplicateCertificateContext(p_cert_next);
					break;
				}
			}

			//p_cert_context = CertFindCertificateInStore(h_store, X509_ASN_ENCODING|PKCS_7_ASN_ENCODING, 
				//0, CERT_FIND_SUBJECT_STR_W, static_cast<const wchar_t *>(cnu)/*use appropriate subject name*/, NULL);
			CertCloseStore(h_store, CERT_CLOSE_STORE_CHECK_FLAG);
		}
		sys_err = ::GetLastError();
		return p_cert_context;
	}
	uint   SLAPI GetLastWinInternetResponse(SString & rMsgBuf)
	{
		rMsgBuf.Z();
		DWORD last_err = 0;
		TCHAR last_err_text[1024];
		DWORD last_err_text_len = SIZEOFARRAY(last_err_text);
		InternetGetLastResponseInfo(&last_err, last_err_text, &last_err_text_len);
		rMsgBuf.CopyUtf8FromUnicode(last_err_text, last_err_text_len, 1);
		return last_err;
	}
	SString & SLAPI MakeHeaderFields(const char * pToken, StrStrAssocArray * pHdrFlds, SString & rBuf)
	{
		StrStrAssocArray hdr_flds;
		SETIFZ(pHdrFlds, &hdr_flds);
		SHttpProtocol::SetHeaderField(*pHdrFlds, SHttpProtocol::hdrContentType, "application/json;charset=UTF-8");
		SHttpProtocol::SetHeaderField(*pHdrFlds, SHttpProtocol::hdrCacheControl, "no-cache");
		SHttpProtocol::SetHeaderField(*pHdrFlds, SHttpProtocol::hdrAcceptLang, "ru");
		if(!isempty(pToken)) {
			SString temp_buf;
			SHttpProtocol::SetHeaderField(*pHdrFlds, SHttpProtocol::hdrAuthorization, (temp_buf = "token").Space().Cat(pToken));
		}
		SHttpProtocol::PutHeaderFieldsIntoString(*pHdrFlds, rBuf);
		return rBuf;
	}
	uint   SLAPI ReadReply(HINTERNET hReq, SString & rBuf)
	{
		rBuf.Z();
		DWORD  read_bytes = 0;
		SBuffer ret_data_buf;
		do {
			uint8 buf[1024];
			InternetReadFile(hReq, buf, sizeof(buf), &read_bytes);
			if(read_bytes) {
				ret_data_buf.Write(buf, read_bytes);
			}
		} while(read_bytes > 0);
		const int avl_size = (int)ret_data_buf.GetAvailableSize();
		rBuf.CatN((const char *)ret_data_buf.GetBuf(), avl_size);
		return read_bytes;
	}
	int    SLAPI GetUserInfo2(InitBlock & rIb)
	{
		int    ok = -1;
		int    wininet_err = 0;
		int    win_err = 0;
		WinInternetHandleStack hstk;
		HINTERNET h_inet_sess = 0;
		HINTERNET h_connection = 0;
		HINTERNET h_req = 0;
		SString temp_buf;
		SString req_buf;
		InetUrl url(MakeTargetUrl(qCurrentUserInfo, rIb, temp_buf));
		{
			ulong access_type = /*INTERNET_OPEN_TYPE_PRECONFIG*/INTERNET_OPEN_TYPE_DIRECT;
			THROW(h_inet_sess = hstk.Push(InternetOpen(_T("Papyrus"), access_type, 0/*lpszProxy*/, 0/*lpszProxyBypass*/, 0/*dwFlags*/)));
		}
		{
			url.GetComponent(url.cHost, 0, temp_buf);
			int   port = url.GetDefProtocolPort(url.GetProtocol());
			THROW(h_connection = hstk.Push(InternetConnect(h_inet_sess, SUcSwitch(temp_buf), port, _T("")/*lpszUserName*/, _T("")/*lpszPassword*/, INTERNET_SERVICE_HTTP, 0, 0)));
		}
		{
			const TCHAR * p_types[] = { _T("text/*"), _T("application/json"), 0 };
			SString qbuf;
			url.GetComponent(url.cPath, 0, temp_buf);
			qbuf = temp_buf;
			url.GetComponent(url.cQuery, 0, temp_buf);
			if(temp_buf.NotEmptyS())
				qbuf.CatChar('?').Cat(temp_buf);
			THROW(hstk.Push(h_req = HttpOpenRequest(h_connection, _T("GET"), SUcSwitch(qbuf), _T("HTTP/1.1"), NULL, p_types, INTERNET_FLAG_KEEP_CONNECTION/*|INTERNET_FLAG_SECURE*/, 1)));
			//
			int  isor = 0;
			uint iresp = 0;
			{
				const CERT_CONTEXT * p_cert = GetClientSslCertificate(rIb);
				if(p_cert) {
					isor = InternetSetOption(h_req, INTERNET_OPTION_CLIENT_CERT_CONTEXT, (LPVOID)(p_cert), sizeof(*p_cert));
					if(!isor) {
						win_err = GetLastError();
						iresp = GetLastWinInternetResponse(temp_buf);
					}
				}
			}
			//
			MakeHeaderFields(rIb.Token, 0, temp_buf);
			if(HttpSendRequest(h_req, SUcSwitch(temp_buf), -1, const_cast<char *>(req_buf.cptr())/*optional data*/, req_buf.Len()/*optional data length*/)) {
				SString wi_msg;
				uint  wi_code = GetLastWinInternetResponse(wi_msg);
				ReadReply(h_req, temp_buf);
			}
			else {
				wininet_err = GetLastError();
			}
		}
		CATCHZOK
		return ok;
	}
	int    SLAPI GetIncomeDocList2(InitBlock & rIb)
	{
		int    ok = -1;
		int    wininet_err = 0;
		int    win_err = 0;
		json_t * p_json_req = 0;
		WinInternetHandleStack hstk;
		HINTERNET h_inet_sess = 0;
		HINTERNET h_connection = 0;
		HINTERNET h_req = 0;
		SString temp_buf;
		SString req_buf;
		InetUrl url(MakeTargetUrl(qGetIncomeDocList, rIb, temp_buf));
		{
			req_buf.Z();
			{
				/*
					{
						"filter": {
							"doc_status": "PROCESSED_DOCUMENT"
						},
						"start_from": 0,
						"count": 100
					}
				*/
				p_json_req = new json_t(json_t::tOBJECT);
				{
					json_t * p_json_filt = new json_t(json_t::tOBJECT);
					p_json_filt->Insert("doc_status", json_new_string("PROCESSED_DOCUMENT"));
					p_json_req->Insert("filter", p_json_filt);
				}
				p_json_req->Insert("start_from", json_new_number("0"));
				p_json_req->Insert("count", json_new_number("100"));
				THROW_SL(json_tree_to_string(p_json_req, req_buf));
			}
			json_free_value(&p_json_req);
		}
		{
			ulong access_type = /*INTERNET_OPEN_TYPE_PRECONFIG*/INTERNET_OPEN_TYPE_DIRECT;
			THROW(h_inet_sess = hstk.Push(InternetOpen(_T("Papyrus"), access_type, 0/*lpszProxy*/, 0/*lpszProxyBypass*/, 0/*dwFlags*/)));
		}
		{
			url.GetComponent(url.cHost, 0, temp_buf);
			int   port = url.GetDefProtocolPort(url.GetProtocol());
			THROW(h_connection = hstk.Push(InternetConnect(h_inet_sess, SUcSwitch(temp_buf), port, _T("")/*lpszUserName*/, _T("")/*lpszPassword*/, INTERNET_SERVICE_HTTP, 0, 0)));
		}
		{
			const TCHAR * p_types[] = { _T("text/*"), _T("application/json"), 0 };
			SString qbuf;
			url.GetComponent(url.cPath, 0, temp_buf);
			qbuf = temp_buf;
			url.GetComponent(url.cQuery, 0, temp_buf);
			if(temp_buf.NotEmptyS())
				qbuf.CatChar('?').Cat(temp_buf);
			THROW(hstk.Push(h_req = HttpOpenRequest(h_connection, _T("POST"), SUcSwitch(qbuf), _T("HTTP/1.1"), NULL, p_types, INTERNET_FLAG_KEEP_CONNECTION/*|INTERNET_FLAG_SECURE*/, 1)));
			//
			int  isor = 0;
			uint iresp = 0;
			{
				const CERT_CONTEXT * p_cert = GetClientSslCertificate(rIb);
				if(p_cert) {
					isor = InternetSetOption(h_req, INTERNET_OPTION_CLIENT_CERT_CONTEXT, (LPVOID)(p_cert), sizeof(*p_cert));
					if(!isor) {
						win_err = GetLastError();
						iresp = GetLastWinInternetResponse(temp_buf);
					}
				}
			}
			//
			MakeHeaderFields(rIb.Token, 0, temp_buf);
			if(HttpSendRequest(h_req, SUcSwitch(temp_buf), -1, const_cast<char *>(req_buf.cptr())/*optional data*/, req_buf.Len()/*optional data length*/)) {
				SString wi_msg;
				uint  wi_code = GetLastWinInternetResponse(wi_msg);
				ReadReply(h_req, temp_buf);
			}
			else {
				wininet_err = GetLastError();
			}
		}
		CATCHZOK
		json_free_value(&p_json_req);
		return ok;
	}
	int    SLAPI GetToken2(const char * pAuthCode, InitBlock & rIb)
	{
		int    ok = -1;
		int    wininet_err = 0;
		WinInternetHandleStack hstk;
		HINTERNET h_inet_sess = 0;
		HINTERNET h_connection = 0;
		HINTERNET h_req = 0;
		SString temp_buf;
		SString req_buf;
		InetUrl url(MakeTargetUrl(qToken, rIb, temp_buf));
		THROW(MakeTokenRequest(rIb, pAuthCode, req_buf));
		{
			ulong access_type = INTERNET_OPEN_TYPE_PRECONFIG;
			THROW(h_inet_sess = hstk.Push(InternetOpen(_T("Papyrus"), access_type, 0/*lpszProxy*/, 0/*lpszProxyBypass*/, 0/*dwFlags*/)));
		}
		{
			url.GetComponent(url.cHost, 0, temp_buf);
			int   port = url.GetDefProtocolPort(url.GetProtocol());
			THROW(h_connection = hstk.Push(InternetConnect(h_inet_sess, SUcSwitch(temp_buf), port, _T("")/*lpszUserName*/, _T("")/*lpszPassword*/, INTERNET_SERVICE_HTTP, 0, 0)));
		}
		{
			const TCHAR * p_types[] = { _T("text/*"), _T("application/json"), 0 };
			SString qbuf;
			url.GetComponent(url.cPath, 0, temp_buf);
			qbuf = temp_buf;
			url.GetComponent(url.cQuery, 0, temp_buf);
			if(temp_buf.NotEmptyS())
				qbuf.CatChar('?').Cat(temp_buf);
			THROW(hstk.Push(h_req = HttpOpenRequest(h_connection, _T("POST"), SUcSwitch(qbuf), _T("HTTP/1.1"), NULL, p_types, INTERNET_FLAG_KEEP_CONNECTION/*|INTERNET_FLAG_SECURE*/, 1)));
			MakeHeaderFields(0, 0, temp_buf);
			if(HttpSendRequest(h_req, SUcSwitch(temp_buf), -1, const_cast<char *>(req_buf.cptr())/*optional data*/, req_buf.Len()/*optional data length*/)) {
				SString wi_msg;
				uint  wi_code = GetLastWinInternetResponse(wi_msg);
				ReadReply(h_req, temp_buf);
				if(ReadJsonReplyForSingleItem(temp_buf, "token", rIb.Token) > 0) {
					ok = 1;
				}
			}
			else {
				wininet_err = GetLastError();
			}
		}
		CATCHZOK
		return ok;
	}
	int    SLAPI ReadJsonReplyForSingleItem(const char * pReply, const char * pTarget, SString & rResult)
	{
		rResult.Z();
		int    ok = -1;
		json_t * p_json_doc = 0;
		if(json_parse_document(&p_json_doc, pReply) == JSON_OK) {
			SString temp_buf;
			json_t * p_next = 0;
			for(json_t * p_cur = p_json_doc; rResult.Empty() && p_cur; p_cur = p_next) {
				p_next = p_cur->P_Next;
				switch(p_cur->Type) {
					case json_t::tOBJECT: p_next = p_cur->P_Child; break;
					case json_t::tSTRING:
						if(p_cur->P_Child) {
							if(sstreqi_ascii(p_cur->Text, pTarget)) {
								rResult = (temp_buf = p_cur->P_Child->Text).Unescape();
								ok = 1;
							}
						}
						break;
				}
			}
		}
		json_free_value(&p_json_doc);
		return ok;
	}
	int    SLAPI Connect2(InitBlock & rIb)
	{
		int    ok = -1;
		int    wininet_err = 0;
		WinInternetHandleStack hstk;
		HINTERNET h_inet_sess = 0;
		HINTERNET h_connection = 0;
		HINTERNET h_req = 0;
		SString temp_buf;
		SString result_code;
		SString req_buf;
		InetUrl url(MakeTargetUrl(qAuth, rIb, temp_buf));
		THROW(MakeAuthRequest(rIb, req_buf));
		{
			ulong access_type = INTERNET_OPEN_TYPE_PRECONFIG;
			THROW(h_inet_sess = hstk.Push(InternetOpen(_T("Papyrus"), access_type, 0/*lpszProxy*/, 0/*lpszProxyBypass*/, 0/*dwFlags*/)));
		}
		{
			url.GetComponent(url.cHost, 0, temp_buf);
			int   port = url.GetDefProtocolPort(url.GetProtocol());
			THROW(h_connection = hstk.Push(InternetConnect(h_inet_sess, SUcSwitch(temp_buf), port, _T("")/*lpszUserName*/, _T("")/*lpszPassword*/, INTERNET_SERVICE_HTTP, 0, 0)));
		}
		{
			const TCHAR * p_types[] = { _T("text/*"), _T("application/json"), 0 };
			SString qbuf;
			url.GetComponent(url.cPath, 0, temp_buf);
			qbuf = temp_buf;
			url.GetComponent(url.cQuery, 0, temp_buf);
			if(temp_buf.NotEmptyS())
				qbuf.CatChar('?').Cat(temp_buf);
			THROW(h_req = hstk.Push(HttpOpenRequest(h_connection, _T("POST"), SUcSwitch(qbuf), _T("HTTP/1.1"), NULL, p_types, INTERNET_FLAG_KEEP_CONNECTION/*|INTERNET_FLAG_SECURE*/, 1)));
			MakeHeaderFields(0, 0, temp_buf);
			if(HttpSendRequest(h_req, SUcSwitch(temp_buf), -1, const_cast<char *>(req_buf.cptr())/*optional data*/, req_buf.Len()/*optional data length*/)) {
				SString wi_msg;
				uint  wi_code = GetLastWinInternetResponse(wi_msg);
				ReadReply(h_req, temp_buf);
				if(ReadJsonReplyForSingleItem(temp_buf, "code", result_code) > 0) {
					ok = 1;
				}
			}
			else {
				wininet_err = GetLastError();
			}
		}
		if(ok > 0) {
			hstk.Destroy();
			THROW(GetToken2(result_code, rIb));
		}
		CATCHZOK
		return ok;
	}
	int    SLAPI GetIncomeDocList_(InitBlock & rIb)
	{
		int    ok = -1;
		json_t * p_json_req = 0;
		SString temp_buf;
		SString req_buf;
		SBuffer ack_buf;
		InetUrl url(MakeTargetUrl(qGetIncomeDocList, rIb, temp_buf));
		{
			req_buf.Z();
			{
				// { "filter": { "doc_status": "PROCESSED_DOCUMENT" }, "start_from": 0, "count": 100 }
				p_json_req = new json_t(json_t::tOBJECT);
				{
					json_t * p_json_filt = new json_t(json_t::tOBJECT);
					p_json_filt->Insert("doc_status", json_new_string("PROCESSED_DOCUMENT"));
					p_json_req->Insert("filter", p_json_filt);
				}
				p_json_req->Insert("start_from", json_new_number("0"));
				p_json_req->Insert("count", json_new_number("100"));
				THROW_SL(json_tree_to_string(p_json_req, req_buf));
			}
			json_free_value(&p_json_req);
		}
		{
			ScURL c;
			StrStrAssocArray hdr_flds;
			MakeHeaderFields(rIb.Token, &hdr_flds, temp_buf);
			{
				SFile wr_stream(ack_buf.Z(), SFile::mWrite);
				PPGetFilePath(PPPATH_BIN, "cacerts-mcs.pem", temp_buf);
				THROW_SL(c.SetupDefaultSslOptions(temp_buf, SSystem::sslTLS_v10, 0)); //CURLOPT_SSLVERSION значением CURL_SSLVERSION_TLSv1_0
				THROW_SL(c.HttpPost(url, /*ScURL::mfDontVerifySslPeer|*/ScURL::mfVerbose, &hdr_flds, req_buf, &wr_stream));
				{
					SBuffer * p_ack_buf = static_cast<SBuffer *>(wr_stream);
					if(p_ack_buf) {
						const int avl_size = (int)p_ack_buf->GetAvailableSize();
						temp_buf.Z().CatN((const char *)p_ack_buf->GetBuf(), avl_size);
					}
				}
			}
		}
		CATCHZOK
		json_free_value(&p_json_req);
		return ok;
	}
	int    SLAPI Connect(InitBlock & rIb)
	{
		int    ok = -1;
		SString temp_buf;
		SString url_buf;
		SString req_buf;
		SString result_code;
		SBuffer ack_buf;
		{
			InetUrl url(MakeTargetUrl(qAuth, rIb, url_buf));
			THROW(MakeAuthRequest(rIb, req_buf));
			{
				ScURL c;
				StrStrAssocArray hdr_flds;
				SHttpProtocol::SetHeaderField(hdr_flds, SHttpProtocol::hdrContentType, "application/json;charset=UTF-8");
				{
					SFile wr_stream(ack_buf.Z(), SFile::mWrite);
					//PPGetFilePath(PPPATH_BIN, "cacerts-mcs.pem", temp_buf);
					//THROW_SL(c.SetupDefaultSslOptions(temp_buf, SSystem::sslTLS_v10, 0)); //CURLOPT_SSLVERSION значением CURL_SSLVERSION_TLSv1_0
					THROW_SL(c.HttpPost(url, /*ScURL::mfDontVerifySslPeer|*/ScURL::mfVerbose, &hdr_flds, req_buf, &wr_stream));
					{
						SBuffer * p_ack_buf = static_cast<SBuffer *>(wr_stream);
						if(p_ack_buf) {
							const int avl_size = (int)p_ack_buf->GetAvailableSize();
							temp_buf.Z().CatN((const char *)p_ack_buf->GetBuf(), avl_size);
							if(ReadJsonReplyForSingleItem(temp_buf, "code", result_code) > 0)
								ok = 1;
						}
					}
				}
			}
		}
		if(ok > 0) {
			ok = -1;
			InetUrl url(MakeTargetUrl(qToken, rIb, url_buf));
			THROW(MakeTokenRequest(rIb, result_code, req_buf));
			{
				ScURL c;
				StrStrAssocArray hdr_flds;
				SHttpProtocol::SetHeaderField(hdr_flds, SHttpProtocol::hdrContentType, "application/json;charset=UTF-8");
				{
					SFile wr_stream(ack_buf.Z(), SFile::mWrite);
					//PPGetFilePath(PPPATH_BIN, "cacerts-mcs.pem", temp_buf);
					//THROW_SL(c.SetupDefaultSslOptions(temp_buf, SSystem::sslTLS_v10, 0)); //CURLOPT_SSLVERSION значением CURL_SSLVERSION_TLSv1_0
					THROW_SL(c.HttpPost(url, /*ScURL::mfDontVerifySslPeer|*/ScURL::mfVerbose, &hdr_flds, req_buf, &wr_stream));
					{
						SBuffer * p_ack_buf = static_cast<SBuffer *>(wr_stream);
						if(p_ack_buf) {
							const int avl_size = (int)p_ack_buf->GetAvailableSize();
							temp_buf.Z().CatN((const char *)p_ack_buf->GetBuf(), avl_size);
							if(ReadJsonReplyForSingleItem(temp_buf, "token", rIb.Token) > 0)
								ok = 1;
						}
					}
				}
			}
		}
		CATCHZOK
		return ok;
	}
private:
};

class ChZnPrcssr {
public:
	struct Param {
		Param() : GuaID(0)
		{
		}
		PPID   GuaID;
	};
	int SLAPI EditParam(Param * pParam)
	{
		class ChZnPrcssrParamDialog : public TDialog {
			DECL_DIALOG_DATA(ChZnPrcssr::Param);
		public:
			ChZnPrcssrParamDialog() : TDialog(DLG_CHZNIX)
			{
			}
			DECL_DIALOG_SETDTS()
			{
				RVALUEPTR(Data, pData);
				SetupPPObjCombo(this, CTLSEL_CHZNIX_GUA, PPOBJ_GLOBALUSERACC, Data.GuaID, OLW_CANINSERT, 0);
				return 1;
			}
			DECL_DIALOG_GETDTS()
			{
				int    ok = 1;
				getCtrlData(CTLSEL_CHZNIX_GUA, &Data.GuaID);
				ASSIGN_PTR(pData, Data);
				return ok;
			}
		};
		DIALOG_PROC_BODY(ChZnPrcssrParamDialog, pParam);
	}
//private:
	ChZnInterface::InitBlock Ib;
};

int SLAPI TestChZn()
{
	int    ok = 1;
	ChZnPrcssr prcssr;
	ChZnPrcssr::Param param;
	if(prcssr.EditParam(&param) > 0) {
		ChZnInterface ifc;
		THROW(ifc.SetupInitBlock(param.GuaID, prcssr.Ib));
		{
			//const CERT_CONTEXT * p_cert = ifc.GetClientSslCertificate(prcssr.Ib);
			if(ifc.Connect(prcssr.Ib) > 0) {
				ifc.GetIncomeDocList_(prcssr.Ib);
			}
			/*if(ifc.Connect2(prcssr.Ib) > 0) {
				//ifc.GetUserInfo2(prcssr.Ib);
				ifc.GetIncomeDocList2(prcssr.Ib);
			}*/
		}
	}
	CATCHZOKPPERR
	return ok;
}