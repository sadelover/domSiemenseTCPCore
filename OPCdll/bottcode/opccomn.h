// **************************************************************************
// opccomn.h
//
// Description:
//	Defines OPC common data.  
//
//	File supplied by OPC Foundataion.
//
// DISCLAIMER:
//	This programming example is provided "AS IS".  As such Kepware, Inc.
//	makes no claims to the worthiness of the code and does not warranty
//	the code to be error free.  It is provided freely and can be used in
//	your own projects.  If you do find this code useful, place a little
//	marketing plug for Kepware in your code.  While we would love to help
//	every one who is trying to write a great OPC client application, the 
//	uniqueness of every project and the limited number of hours in a day 
//	simply prevents us from doing so.  If you really find yourself in a
//	bind, please contact Kepware's technical support.  We will not be able
//	to assist you with server related problems unless you are using KepServer
//	or KepServerEx.
// **************************************************************************

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.01.75 */
/* at Fri Jun 19 14:14:38 1998
*/
/* Compiler settings for opccomn.idl:
Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext
error checks: none
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __opccomn_h__
#define __opccomn_h__

#ifdef __cplusplus
extern "C"{
#endif 

	/* Forward Declarations */ 

#ifndef __IOPCShutdown_FWD_DEFINED__
#define __IOPCShutdown_FWD_DEFINED__
	typedef interface IOPCShutdown IOPCShutdown;
#endif 	/* __IOPCShutdown_FWD_DEFINED__ */


#ifndef __IOPCCommon_FWD_DEFINED__
#define __IOPCCommon_FWD_DEFINED__
	typedef interface IOPCCommon IOPCCommon;
#endif 	/* __IOPCCommon_FWD_DEFINED__ */

#ifndef __IOPCServerList_FWD_DEFINED__
#define __IOPCServerList_FWD_DEFINED__
	typedef interface IOPCServerList IOPCServerList;
#endif 	/* __IOPCServerList_FWD_DEFINED__ */


#ifndef __IOPCEnumGUID_FWD_DEFINED__
#define __IOPCEnumGUID_FWD_DEFINED__
	typedef interface IOPCEnumGUID IOPCEnumGUID;
#endif 	/* __IOPCEnumGUID_FWD_DEFINED__ */


#ifndef __IOPCServerList2_FWD_DEFINED__
#define __IOPCServerList2_FWD_DEFINED__
	typedef interface IOPCServerList2 IOPCServerList2;
#endif 	/* __IOPCServerList2_FWD_DEFINED__ */

#ifndef __IOPCCommon_FWD_DEFINED__
#define __IOPCCommon_FWD_DEFINED__
	typedef interface IOPCCommon IOPCCommon;
#endif 	/* __IOPCCommon_FWD_DEFINED__ */


#ifndef __IOPCShutdown_FWD_DEFINED__
#define __IOPCShutdown_FWD_DEFINED__
	typedef interface IOPCShutdown IOPCShutdown;
#endif 	/* __IOPCShutdown_FWD_DEFINED__ */


	/* header files for imported files */
#include "unknwn.h"

	void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
	void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IOPCShutdown_INTERFACE_DEFINED__
#define __IOPCShutdown_INTERFACE_DEFINED__

	/****************************************
	* Generated header for interface: IOPCShutdown
	* at Fri Jun 19 14:14:38 1998
	* using MIDL 3.01.75
	****************************************/
	/* [unique][uuid][object] */ 



	EXTERN_C const IID IID_IOPCShutdown;

#if defined(__cplusplus) && !defined(CINTERFACE)

	interface DECLSPEC_UUID("F31DFDE1-07B6-11d2-B2D8-0060083BA1FB")
IOPCShutdown : public IUnknown
	{
	public:
		virtual HRESULT STDMETHODCALLTYPE ShutdownRequest( 
			/* [string][in] */ LPCWSTR szReason) = 0;

	};

#else 	/* C style interface */

	typedef struct IOPCShutdownVtbl
	{
		BEGIN_INTERFACE

			HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
			IOPCShutdown __RPC_FAR * This,
			/* [in] */ REFIID riid,
			/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);

			ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
				IOPCShutdown __RPC_FAR * This);

			ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
				IOPCShutdown __RPC_FAR * This);

			HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShutdownRequest )( 
				IOPCShutdown __RPC_FAR * This,
				/* [string][in] */ LPCWSTR szReason);

		END_INTERFACE
	} IOPCShutdownVtbl;

	interface IOPCShutdown
	{
		CONST_VTBL struct IOPCShutdownVtbl __RPC_FAR *lpVtbl;
	};



#ifdef COBJMACROS


#define IOPCShutdown_QueryInterface(This,riid,ppvObject)	\
	(This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOPCShutdown_AddRef(This)	\
	(This)->lpVtbl -> AddRef(This)

#define IOPCShutdown_Release(This)	\
	(This)->lpVtbl -> Release(This)


#define IOPCShutdown_ShutdownRequest(This,szReason)	\
	(This)->lpVtbl -> ShutdownRequest(This,szReason)

#endif /* COBJMACROS */


#endif 	/* C style interface */



	HRESULT STDMETHODCALLTYPE IOPCShutdown_ShutdownRequest_Proxy( 
		IOPCShutdown __RPC_FAR * This,
		/* [string][in] */ LPCWSTR szReason);


	void __RPC_STUB IOPCShutdown_ShutdownRequest_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);



#endif 	/* __IOPCShutdown_INTERFACE_DEFINED__ */


#ifndef __IOPCCommon_INTERFACE_DEFINED__
#define __IOPCCommon_INTERFACE_DEFINED__

	/****************************************
	* Generated header for interface: IOPCCommon
	* at Fri Jun 19 14:14:38 1998
	* using MIDL 3.01.75
	****************************************/
	/* [unique][uuid][object] */ 



	EXTERN_C const IID IID_IOPCCommon;

#if defined(__cplusplus) && !defined(CINTERFACE)

	interface DECLSPEC_UUID("F31DFDE2-07B6-11d2-B2D8-0060083BA1FB")
IOPCCommon : public IUnknown
	{
	public:
		virtual HRESULT STDMETHODCALLTYPE SetLocaleID( 
			/* [in] */ LCID dwLcid) = 0;

		virtual HRESULT STDMETHODCALLTYPE GetLocaleID( 
			/* [out] */ LCID __RPC_FAR *pdwLcid) = 0;

		virtual HRESULT STDMETHODCALLTYPE QueryAvailableLocaleIDs( 
			/* [out] */ DWORD __RPC_FAR *pdwCount,
			/* [size_is][size_is][out] */ LCID __RPC_FAR *__RPC_FAR *pdwLcid) = 0;

		virtual HRESULT STDMETHODCALLTYPE GetErrorString( 
			/* [in] */ HRESULT dwError,
			/* [string][out] */ LPWSTR __RPC_FAR *ppString) = 0;

		virtual HRESULT STDMETHODCALLTYPE SetClientName( 
			/* [string][in] */ LPCWSTR szName) = 0;

	};

#else 	/* C style interface */

	typedef struct IOPCCommonVtbl
	{
		BEGIN_INTERFACE

			HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
			IOPCCommon __RPC_FAR * This,
			/* [in] */ REFIID riid,
			/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);

			ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
				IOPCCommon __RPC_FAR * This);

			ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
				IOPCCommon __RPC_FAR * This);

			HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetLocaleID )( 
				IOPCCommon __RPC_FAR * This,
				/* [in] */ LCID dwLcid);

			HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLocaleID )( 
				IOPCCommon __RPC_FAR * This,
				/* [out] */ LCID __RPC_FAR *pdwLcid);

			HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryAvailableLocaleIDs )( 
				IOPCCommon __RPC_FAR * This,
				/* [out] */ DWORD __RPC_FAR *pdwCount,
				/* [size_is][size_is][out] */ LCID __RPC_FAR *__RPC_FAR *pdwLcid);

			HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetErrorString )( 
				IOPCCommon __RPC_FAR * This,
				/* [in] */ HRESULT dwError,
				/* [string][out] */ LPWSTR __RPC_FAR *ppString);

			HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetClientName )( 
				IOPCCommon __RPC_FAR * This,
				/* [string][in] */ LPCWSTR szName);

		END_INTERFACE
	} IOPCCommonVtbl;

	interface IOPCCommon
	{
		CONST_VTBL struct IOPCCommonVtbl __RPC_FAR *lpVtbl;
	};



#ifdef COBJMACROS


#define IOPCCommon_QueryInterface(This,riid,ppvObject)	\
	(This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOPCCommon_AddRef(This)	\
	(This)->lpVtbl -> AddRef(This)

#define IOPCCommon_Release(This)	\
	(This)->lpVtbl -> Release(This)


#define IOPCCommon_SetLocaleID(This,dwLcid)	\
	(This)->lpVtbl -> SetLocaleID(This,dwLcid)

#define IOPCCommon_GetLocaleID(This,pdwLcid)	\
	(This)->lpVtbl -> GetLocaleID(This,pdwLcid)

#define IOPCCommon_QueryAvailableLocaleIDs(This,pdwCount,pdwLcid)	\
	(This)->lpVtbl -> QueryAvailableLocaleIDs(This,pdwCount,pdwLcid)

#define IOPCCommon_GetErrorString(This,dwError,ppString)	\
	(This)->lpVtbl -> GetErrorString(This,dwError,ppString)

#define IOPCCommon_SetClientName(This,szName)	\
	(This)->lpVtbl -> SetClientName(This,szName)

#endif /* COBJMACROS */


#endif 	/* C style interface */



	HRESULT STDMETHODCALLTYPE IOPCCommon_SetLocaleID_Proxy( 
		IOPCCommon __RPC_FAR * This,
		/* [in] */ LCID dwLcid);


	void __RPC_STUB IOPCCommon_SetLocaleID_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);


	HRESULT STDMETHODCALLTYPE IOPCCommon_GetLocaleID_Proxy( 
		IOPCCommon __RPC_FAR * This,
		/* [out] */ LCID __RPC_FAR *pdwLcid);


	void __RPC_STUB IOPCCommon_GetLocaleID_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);


	HRESULT STDMETHODCALLTYPE IOPCCommon_QueryAvailableLocaleIDs_Proxy( 
		IOPCCommon __RPC_FAR * This,
		/* [out] */ DWORD __RPC_FAR *pdwCount,
		/* [size_is][size_is][out] */ LCID __RPC_FAR *__RPC_FAR *pdwLcid);


	void __RPC_STUB IOPCCommon_QueryAvailableLocaleIDs_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);


	HRESULT STDMETHODCALLTYPE IOPCCommon_GetErrorString_Proxy( 
		IOPCCommon __RPC_FAR * This,
		/* [in] */ HRESULT dwError,
		/* [string][out] */ LPWSTR __RPC_FAR *ppString);


	void __RPC_STUB IOPCCommon_GetErrorString_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);


	HRESULT STDMETHODCALLTYPE IOPCCommon_SetClientName_Proxy( 
		IOPCCommon __RPC_FAR * This,
		/* [string][in] */ LPCWSTR szName);


	void __RPC_STUB IOPCCommon_SetClientName_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);



#endif 	/* __IOPCCommon_INTERFACE_DEFINED__ */


#ifndef __IOPCServerList_INTERFACE_DEFINED__
#define __IOPCServerList_INTERFACE_DEFINED__

	/* interface IOPCServerList */
	/* [unique][uuid][object] */ 


	EXTERN_C const IID IID_IOPCServerList;

#if defined(__cplusplus) && !defined(CINTERFACE)

	MIDL_INTERFACE("13486D50-4821-11D2-A494-3CB306C10000")
IOPCServerList : public IUnknown
	{
	public:
		virtual HRESULT STDMETHODCALLTYPE EnumClassesOfCategories( 
			/* [in] */ ULONG cImplemented,
			/* [size_is][in] */ CATID rgcatidImpl[  ],
			/* [in] */ ULONG cRequired,
			/* [size_is][in] */ CATID rgcatidReq[  ],
			/* [out] */ IEnumGUID **ppenumClsid) = 0;

		virtual HRESULT STDMETHODCALLTYPE GetClassDetails( 
			/* [in] */ REFCLSID clsid,
			/* [out] */ LPOLESTR *ppszProgID,
			/* [out] */ LPOLESTR *ppszUserType) = 0;

		virtual HRESULT STDMETHODCALLTYPE CLSIDFromProgID( 
			/* [in] */ LPCOLESTR szProgId,
			/* [out] */ LPCLSID clsid) = 0;

	};

#else 	/* C style interface */

	typedef struct IOPCServerListVtbl
	{
		BEGIN_INTERFACE

			HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
			IOPCServerList * This,
			/* [in] */ REFIID riid,
			/* [iid_is][out] */ void **ppvObject);

			ULONG ( STDMETHODCALLTYPE *AddRef )( 
				IOPCServerList * This);

			ULONG ( STDMETHODCALLTYPE *Release )( 
				IOPCServerList * This);

			HRESULT ( STDMETHODCALLTYPE *EnumClassesOfCategories )( 
				IOPCServerList * This,
				/* [in] */ ULONG cImplemented,
				/* [size_is][in] */ CATID rgcatidImpl[  ],
				/* [in] */ ULONG cRequired,
				/* [size_is][in] */ CATID rgcatidReq[  ],
				/* [out] */ IEnumGUID **ppenumClsid);

			HRESULT ( STDMETHODCALLTYPE *GetClassDetails )( 
				IOPCServerList * This,
				/* [in] */ REFCLSID clsid,
				/* [out] */ LPOLESTR *ppszProgID,
				/* [out] */ LPOLESTR *ppszUserType);

			HRESULT ( STDMETHODCALLTYPE *CLSIDFromProgID )( 
				IOPCServerList * This,
				/* [in] */ LPCOLESTR szProgId,
				/* [out] */ LPCLSID clsid);

		END_INTERFACE
	} IOPCServerListVtbl;

	interface IOPCServerList
	{
		CONST_VTBL struct IOPCServerListVtbl *lpVtbl;
	};



#ifdef COBJMACROS


#define IOPCServerList_QueryInterface(This,riid,ppvObject)	\
	(This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOPCServerList_AddRef(This)	\
	(This)->lpVtbl -> AddRef(This)

#define IOPCServerList_Release(This)	\
	(This)->lpVtbl -> Release(This)


#define IOPCServerList_EnumClassesOfCategories(This,cImplemented,rgcatidImpl,cRequired,rgcatidReq,ppenumClsid)	\
	(This)->lpVtbl -> EnumClassesOfCategories(This,cImplemented,rgcatidImpl,cRequired,rgcatidReq,ppenumClsid)

#define IOPCServerList_GetClassDetails(This,clsid,ppszProgID,ppszUserType)	\
	(This)->lpVtbl -> GetClassDetails(This,clsid,ppszProgID,ppszUserType)

#define IOPCServerList_CLSIDFromProgID(This,szProgId,clsid)	\
	(This)->lpVtbl -> CLSIDFromProgID(This,szProgId,clsid)

#endif /* COBJMACROS */


#endif 	/* C style interface */



	HRESULT STDMETHODCALLTYPE IOPCServerList_EnumClassesOfCategories_Proxy( 
		IOPCServerList * This,
		/* [in] */ ULONG cImplemented,
		/* [size_is][in] */ CATID rgcatidImpl[  ],
		/* [in] */ ULONG cRequired,
		/* [size_is][in] */ CATID rgcatidReq[  ],
		/* [out] */ IEnumGUID **ppenumClsid);


	void __RPC_STUB IOPCServerList_EnumClassesOfCategories_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);


	HRESULT STDMETHODCALLTYPE IOPCServerList_GetClassDetails_Proxy( 
		IOPCServerList * This,
		/* [in] */ REFCLSID clsid,
		/* [out] */ LPOLESTR *ppszProgID,
		/* [out] */ LPOLESTR *ppszUserType);


	void __RPC_STUB IOPCServerList_GetClassDetails_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);


	HRESULT STDMETHODCALLTYPE IOPCServerList_CLSIDFromProgID_Proxy( 
		IOPCServerList * This,
		/* [in] */ LPCOLESTR szProgId,
		/* [out] */ LPCLSID clsid);


	void __RPC_STUB IOPCServerList_CLSIDFromProgID_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);



#endif 	/* __IOPCServerList_INTERFACE_DEFINED__ */


#ifndef __IOPCEnumGUID_INTERFACE_DEFINED__
#define __IOPCEnumGUID_INTERFACE_DEFINED__

	/* interface IOPCEnumGUID */
	/* [unique][uuid][object] */ 

	typedef /* [unique] */ IOPCEnumGUID *LPOPCENUMGUID;


	EXTERN_C const IID IID_IOPCEnumGUID;

#if defined(__cplusplus) && !defined(CINTERFACE)

	MIDL_INTERFACE("55C382C8-21C7-4e88-96C1-BECFB1E3F483")
IOPCEnumGUID : public IUnknown
	{
	public:
		virtual HRESULT STDMETHODCALLTYPE Next( 
			/* [in] */ ULONG celt,
			/* [length_is][size_is][out] */ GUID *rgelt,
			/* [out] */ ULONG *pceltFetched) = 0;

		virtual HRESULT STDMETHODCALLTYPE Skip( 
			/* [in] */ ULONG celt) = 0;

		virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;

		virtual HRESULT STDMETHODCALLTYPE Clone( 
			/* [out] */ IOPCEnumGUID **ppenum) = 0;

	};

#else 	/* C style interface */

	typedef struct IOPCEnumGUIDVtbl
	{
		BEGIN_INTERFACE

			HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
			IOPCEnumGUID * This,
			/* [in] */ REFIID riid,
			/* [iid_is][out] */ void **ppvObject);

			ULONG ( STDMETHODCALLTYPE *AddRef )( 
				IOPCEnumGUID * This);

			ULONG ( STDMETHODCALLTYPE *Release )( 
				IOPCEnumGUID * This);

			HRESULT ( STDMETHODCALLTYPE *Next )( 
				IOPCEnumGUID * This,
				/* [in] */ ULONG celt,
				/* [length_is][size_is][out] */ GUID *rgelt,
				/* [out] */ ULONG *pceltFetched);

			HRESULT ( STDMETHODCALLTYPE *Skip )( 
				IOPCEnumGUID * This,
				/* [in] */ ULONG celt);

			HRESULT ( STDMETHODCALLTYPE *Reset )( 
				IOPCEnumGUID * This);

			HRESULT ( STDMETHODCALLTYPE *Clone )( 
				IOPCEnumGUID * This,
				/* [out] */ IOPCEnumGUID **ppenum);

		END_INTERFACE
	} IOPCEnumGUIDVtbl;

	interface IOPCEnumGUID
	{
		CONST_VTBL struct IOPCEnumGUIDVtbl *lpVtbl;
	};



#ifdef COBJMACROS


#define IOPCEnumGUID_QueryInterface(This,riid,ppvObject)	\
	(This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOPCEnumGUID_AddRef(This)	\
	(This)->lpVtbl -> AddRef(This)

#define IOPCEnumGUID_Release(This)	\
	(This)->lpVtbl -> Release(This)


#define IOPCEnumGUID_Next(This,celt,rgelt,pceltFetched)	\
	(This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IOPCEnumGUID_Skip(This,celt)	\
	(This)->lpVtbl -> Skip(This,celt)

#define IOPCEnumGUID_Reset(This)	\
	(This)->lpVtbl -> Reset(This)

#define IOPCEnumGUID_Clone(This,ppenum)	\
	(This)->lpVtbl -> Clone(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



	HRESULT STDMETHODCALLTYPE IOPCEnumGUID_Next_Proxy( 
		IOPCEnumGUID * This,
		/* [in] */ ULONG celt,
		/* [length_is][size_is][out] */ GUID *rgelt,
		/* [out] */ ULONG *pceltFetched);


	void __RPC_STUB IOPCEnumGUID_Next_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);


	HRESULT STDMETHODCALLTYPE IOPCEnumGUID_Skip_Proxy( 
		IOPCEnumGUID * This,
		/* [in] */ ULONG celt);


	void __RPC_STUB IOPCEnumGUID_Skip_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);


	HRESULT STDMETHODCALLTYPE IOPCEnumGUID_Reset_Proxy( 
		IOPCEnumGUID * This);


	void __RPC_STUB IOPCEnumGUID_Reset_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);


	HRESULT STDMETHODCALLTYPE IOPCEnumGUID_Clone_Proxy( 
		IOPCEnumGUID * This,
		/* [out] */ IOPCEnumGUID **ppenum);


	void __RPC_STUB IOPCEnumGUID_Clone_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);



#endif 	/* __IOPCEnumGUID_INTERFACE_DEFINED__ */


#ifndef __IOPCServerList2_INTERFACE_DEFINED__
#define __IOPCServerList2_INTERFACE_DEFINED__

	/* interface IOPCServerList2 */
	/* [unique][uuid][object] */ 


	EXTERN_C const IID IID_IOPCServerList2;

#if defined(__cplusplus) && !defined(CINTERFACE)

	MIDL_INTERFACE("9DD0B56C-AD9E-43ee-8305-487F3188BF7A")
IOPCServerList2 : public IUnknown
	{
	public:
		virtual HRESULT STDMETHODCALLTYPE EnumClassesOfCategories( 
			/* [in] */ ULONG cImplemented,
			/* [size_is][in] */ CATID rgcatidImpl[  ],
			/* [in] */ ULONG cRequired,
			/* [size_is][in] */ CATID rgcatidReq[  ],
			/* [out] */ IOPCEnumGUID **ppenumClsid) = 0;

		virtual HRESULT STDMETHODCALLTYPE GetClassDetails( 
			/* [in] */ REFCLSID clsid,
			/* [out] */ LPOLESTR *ppszProgID,
			/* [out] */ LPOLESTR *ppszUserType,
			/* [out] */ LPOLESTR *ppszVerIndProgID) = 0;

		virtual HRESULT STDMETHODCALLTYPE CLSIDFromProgID( 
			/* [in] */ LPCOLESTR szProgId,
			/* [out] */ LPCLSID clsid) = 0;

	};

#else 	/* C style interface */

	typedef struct IOPCServerList2Vtbl
	{
		BEGIN_INTERFACE

			HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
			IOPCServerList2 * This,
			/* [in] */ REFIID riid,
			/* [iid_is][out] */ void **ppvObject);

			ULONG ( STDMETHODCALLTYPE *AddRef )( 
				IOPCServerList2 * This);

			ULONG ( STDMETHODCALLTYPE *Release )( 
				IOPCServerList2 * This);

			HRESULT ( STDMETHODCALLTYPE *EnumClassesOfCategories )( 
				IOPCServerList2 * This,
				/* [in] */ ULONG cImplemented,
				/* [size_is][in] */ CATID rgcatidImpl[  ],
				/* [in] */ ULONG cRequired,
				/* [size_is][in] */ CATID rgcatidReq[  ],
				/* [out] */ IOPCEnumGUID **ppenumClsid);

			HRESULT ( STDMETHODCALLTYPE *GetClassDetails )( 
				IOPCServerList2 * This,
				/* [in] */ REFCLSID clsid,
				/* [out] */ LPOLESTR *ppszProgID,
				/* [out] */ LPOLESTR *ppszUserType,
				/* [out] */ LPOLESTR *ppszVerIndProgID);

			HRESULT ( STDMETHODCALLTYPE *CLSIDFromProgID )( 
				IOPCServerList2 * This,
				/* [in] */ LPCOLESTR szProgId,
				/* [out] */ LPCLSID clsid);

		END_INTERFACE
	} IOPCServerList2Vtbl;

	interface IOPCServerList2
	{
		CONST_VTBL struct IOPCServerList2Vtbl *lpVtbl;
	};



#ifdef COBJMACROS


#define IOPCServerList2_QueryInterface(This,riid,ppvObject)	\
	(This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOPCServerList2_AddRef(This)	\
	(This)->lpVtbl -> AddRef(This)

#define IOPCServerList2_Release(This)	\
	(This)->lpVtbl -> Release(This)


#define IOPCServerList2_EnumClassesOfCategories(This,cImplemented,rgcatidImpl,cRequired,rgcatidReq,ppenumClsid)	\
	(This)->lpVtbl -> EnumClassesOfCategories(This,cImplemented,rgcatidImpl,cRequired,rgcatidReq,ppenumClsid)

#define IOPCServerList2_GetClassDetails(This,clsid,ppszProgID,ppszUserType,ppszVerIndProgID)	\
	(This)->lpVtbl -> GetClassDetails(This,clsid,ppszProgID,ppszUserType,ppszVerIndProgID)

#define IOPCServerList2_CLSIDFromProgID(This,szProgId,clsid)	\
	(This)->lpVtbl -> CLSIDFromProgID(This,szProgId,clsid)

#endif /* COBJMACROS */


#endif 	/* C style interface */



	HRESULT STDMETHODCALLTYPE IOPCServerList2_EnumClassesOfCategories_Proxy( 
		IOPCServerList2 * This,
		/* [in] */ ULONG cImplemented,
		/* [size_is][in] */ CATID rgcatidImpl[  ],
		/* [in] */ ULONG cRequired,
		/* [size_is][in] */ CATID rgcatidReq[  ],
		/* [out] */ IOPCEnumGUID **ppenumClsid);


	void __RPC_STUB IOPCServerList2_EnumClassesOfCategories_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);


	HRESULT STDMETHODCALLTYPE IOPCServerList2_GetClassDetails_Proxy( 
		IOPCServerList2 * This,
		/* [in] */ REFCLSID clsid,
		/* [out] */ LPOLESTR *ppszProgID,
		/* [out] */ LPOLESTR *ppszUserType,
		/* [out] */ LPOLESTR *ppszVerIndProgID);


	void __RPC_STUB IOPCServerList2_GetClassDetails_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);


	HRESULT STDMETHODCALLTYPE IOPCServerList2_CLSIDFromProgID_Proxy( 
		IOPCServerList2 * This,
		/* [in] */ LPCOLESTR szProgId,
		/* [out] */ LPCLSID clsid);


	void __RPC_STUB IOPCServerList2_CLSIDFromProgID_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);



#endif 	/* __IOPCServerList2_INTERFACE_DEFINED__ */


#ifndef __OPCCOMN_LIBRARY_DEFINED__
#define __OPCCOMN_LIBRARY_DEFINED__

	/****************************************
	* Generated header for library: OPCCOMN
	* at Fri Jun 19 14:14:38 1998
	* using MIDL 3.01.75
	****************************************/
	/* [helpstring][version][uuid] */ 





	EXTERN_C const IID LIBID_OPCCOMN;
#endif /* __OPCCOMN_LIBRARY_DEFINED__ */

	/* Additional Prototypes for ALL interfaces */

	/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
