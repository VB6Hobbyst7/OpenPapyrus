/* soapUhttMainBindingProxy.h
   Generated by gSOAP 2.8.4 from d:\Papyrus\Src\Tools\Uhttsoap\universe_htt.h

Copyright(C) 2000-2011, Robert van Engelen, Genivia Inc. All Rights Reserved.
The generated code is released under one of the following licenses:
1) GPL or 2) Genivia's license for commercial use.
This program is released under the GPL with the additional exemption that
compiling, linking, and/or using OpenSSL is allowed.
*/

#ifndef soapUhttMainBindingProxy_H
#define soapUhttMainBindingProxy_H
#include "soapH.h"
class UhttMainBinding
{   public:
	/// Runtime engine context allocated in constructor
	struct soap *soap;
	/// Endpoint URL of service 'UhttMainBinding' (change as needed)
	const char *endpoint;
	/// Constructor allocates soap engine context, sets default endpoint URL, and sets namespace mapping table
	UhttMainBinding()
	{ soap = soap_new(); endpoint = "http://localhost/Universe-HTT/soap/universe_htt.php"; if (soap && !soap->namespaces) { static const struct Namespace namespaces[] = 
{
	{"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/", "http://www.w3.org/*/soap-envelope", NULL},
	{"SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/", "http://www.w3.org/*/soap-encoding", NULL},
	{"xsi", "http://www.w3.org/2001/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance", NULL},
	{"xsd", "http://www.w3.org/2001/XMLSchema", "http://www.w3.org/*/XMLSchema", NULL},
	{"ns1", "http://localhost/Universe-HTT/soap/", NULL, NULL},
	{NULL, NULL, NULL, NULL}
};
	soap->namespaces = namespaces; } };
	/// Destructor frees deserialized data and soap engine context
	virtual ~UhttMainBinding() { if (soap) { soap_destroy(soap); soap_end(soap); soap_free(soap); } };
	/// Invoke 'SelectObject' of service 'UhttMainBinding' and return error code (or SOAP_OK)
	virtual int ns1__SelectObject(ns1__UhttObjSelectCriteria *ObjSelectCriteria, struct ns1__SelectObjectResponse &_param_1) { return soap ? soap_call_ns1__SelectObject(soap, endpoint, NULL, ObjSelectCriteria, _param_1) : SOAP_EOM; };
	/// Invoke 'Get_World' of service 'UhttMainBinding' and return error code (or SOAP_OK)
	virtual int ns1__Get_USCOREWorld(int ID, struct ns1__Get_USCOREWorldResponse &_param_2) { return soap ? soap_call_ns1__Get_USCOREWorld(soap, endpoint, NULL, ID, _param_2) : SOAP_EOM; };
	/// Invoke 'Get_Currency' of service 'UhttMainBinding' and return error code (or SOAP_OK)
	virtual int ns1__Get_USCORECurrency(int ID, struct ns1__Get_USCORECurrencyResponse &_param_3) { return soap ? soap_call_ns1__Get_USCORECurrency(soap, endpoint, NULL, ID, _param_3) : SOAP_EOM; };
	/// Invoke 'Get_PersonKind' of service 'UhttMainBinding' and return error code (or SOAP_OK)
	virtual int ns1__Get_USCOREPersonKind(int ID, struct ns1__Get_USCOREPersonKindResponse &_param_4) { return soap ? soap_call_ns1__Get_USCOREPersonKind(soap, endpoint, NULL, ID, _param_4) : SOAP_EOM; };
	/// Invoke 'Get_PersonCategory' of service 'UhttMainBinding' and return error code (or SOAP_OK)
	virtual int ns1__Get_USCOREPersonCategory(int ID, struct ns1__Get_USCOREPersonCategoryResponse &_param_5) { return soap ? soap_call_ns1__Get_USCOREPersonCategory(soap, endpoint, NULL, ID, _param_5) : SOAP_EOM; };
	/// Invoke 'Get_RegisterType' of service 'UhttMainBinding' and return error code (or SOAP_OK)
	virtual int ns1__Get_USCORERegisterType(int ID, struct ns1__Get_USCORERegisterTypeResponse &_param_6) { return soap ? soap_call_ns1__Get_USCORERegisterType(soap, endpoint, NULL, ID, _param_6) : SOAP_EOM; };
	/// Invoke 'Get_Person' of service 'UhttMainBinding' and return error code (or SOAP_OK)
	virtual int ns1__Get_USCOREPerson(int ID, struct ns1__Get_USCOREPersonResponse &_param_7) { return soap ? soap_call_ns1__Get_USCOREPerson(soap, endpoint, NULL, ID, _param_7) : SOAP_EOM; };
	/// Invoke 'Get_GoodsGroup' of service 'UhttMainBinding' and return error code (or SOAP_OK)
	virtual int ns1__Get_USCOREGoodsGroup(int ID, struct ns1__Get_USCOREGoodsGroupResponse &_param_8) { return soap ? soap_call_ns1__Get_USCOREGoodsGroup(soap, endpoint, NULL, ID, _param_8) : SOAP_EOM; };
	/// Invoke 'Get_Brand' of service 'UhttMainBinding' and return error code (or SOAP_OK)
	virtual int ns1__Get_USCOREBrand(int ID, struct ns1__Get_USCOREBrandResponse &_param_9) { return soap ? soap_call_ns1__Get_USCOREBrand(soap, endpoint, NULL, ID, _param_9) : SOAP_EOM; };
	/// Invoke 'Get_Goods' of service 'UhttMainBinding' and return error code (or SOAP_OK)
	virtual int ns1__Get_USCOREGoods(int ID, struct ns1__Get_USCOREGoodsResponse &_param_10) { return soap ? soap_call_ns1__Get_USCOREGoods(soap, endpoint, NULL, ID, _param_10) : SOAP_EOM; };
	/// Invoke 'Get_Calendar' of service 'UhttMainBinding' and return error code (or SOAP_OK)
	virtual int ns1__Get_USCORECalendar(int ID, struct ns1__Get_USCORECalendarResponse &_param_11) { return soap ? soap_call_ns1__Get_USCORECalendar(soap, endpoint, NULL, ID, _param_11) : SOAP_EOM; };
	/// Invoke 'SetGoodsCode' of service 'UhttMainBinding' and return error code (or SOAP_OK)
	virtual int ns1__SetGoodsCode(ns1__UhttArCode *ArCode, struct ns1__SetGoodsCodeResponse &_param_12) { return soap ? soap_call_ns1__SetGoodsCode(soap, endpoint, NULL, ArCode, _param_12) : SOAP_EOM; };
	/// Invoke 'GetGoodsCode' of service 'UhttMainBinding' and return error code (or SOAP_OK)
	virtual int ns1__GetGoodsCode(ns1__UhttArCode *ArCode, struct ns1__GetGoodsCodeResponse &_param_13) { return soap ? soap_call_ns1__GetGoodsCode(soap, endpoint, NULL, ArCode, _param_13) : SOAP_EOM; };
	/// Invoke 'SetGoodsPrice' of service 'UhttMainBinding' and return error code (or SOAP_OK)
	virtual int ns1__SetGoodsPrice(ns1__UhttQuot *Quot, struct ns1__SetGoodsPriceResponse &_param_14) { return soap ? soap_call_ns1__SetGoodsPrice(soap, endpoint, NULL, Quot, _param_14) : SOAP_EOM; };
	/// Invoke 'GetGoodsPrice' of service 'UhttMainBinding' and return error code (or SOAP_OK)
	virtual int ns1__GetGoodsPrice(ns1__UhttQuot *Quot, struct ns1__GetGoodsPriceResponse &_param_15) { return soap ? soap_call_ns1__GetGoodsPrice(soap, endpoint, NULL, Quot, _param_15) : SOAP_EOM; };
	/// Invoke 'GetSpoiledSeries' of service 'UhttMainBinding' and return error code (or SOAP_OK)
	virtual int ns1__GetSpoiledSeries(int ID, struct ns1__GetSpoiledSeriesResponse &_param_16) { return soap ? soap_call_ns1__GetSpoiledSeries(soap, endpoint, NULL, ID, _param_16) : SOAP_EOM; };
	/// Invoke 'Login' of service 'UhttMainBinding' and return error code (or SOAP_OK)
	virtual int ns1__Login(char *Login, char *Password, struct ns1__LoginResponse &_param_17) { return soap ? soap_call_ns1__Login(soap, endpoint, NULL, Login, Password, _param_17) : SOAP_EOM; };
	/// Invoke 'Logout' of service 'UhttMainBinding' and return error code (or SOAP_OK)
	virtual int ns1__Logout(int ID, struct ns1__LogoutResponse &_param_18) { return soap ? soap_call_ns1__Logout(soap, endpoint, NULL, ID, _param_18) : SOAP_EOM; };
};
#endif
