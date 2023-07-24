// IOCPBuffer.cpp: implementation of the CIOCPBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IocpBuffer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIOCPBuffer::CIOCPBuffer()
{
Init(); // Never called if the class is allocated with VirtualAlloc.. 	
}

void CIOCPBuffer::Init()
{
	m_nUsed=0;
	m_Operation=-1;
	m_pPos=NULL;
	ZeroMemory(&m_ol, sizeof(OVERLAPPED));
	ZeroMemory(&m_Buffer,sizeof(m_Buffer));
}


CIOCPBuffer::~CIOCPBuffer()
{
	// Never called if the class is allocated with VirtualAlloc.. 
	m_nUsed=0;
	//FreeBuffer();
}



/*
* Returns the pointer to the Buffer.  
*
*/
PBYTE CIOCPBuffer::GetBuffer()
{
	return (PBYTE)m_Buffer;
}

PBYTE CIOCPBuffer::GetPayLoadBuffer()
{
	return m_nUsed>MINIMUM_PACKAGE_SIZE ? GetBuffer()+MINIMUM_PACKAGE_SIZE : NULL;	
}

/*
* Adds a singel BYTE to the data. 
*
*/

BOOL CIOCPBuffer::AddData(BYTE data)
{
	return AddData(&data, 1);
}

BOOL CIOCPBuffer::AddData(UINT data)
{
return AddData(reinterpret_cast<const BYTE*>(&data), sizeof(UINT));
}


BOOL CIOCPBuffer::AddData(unsigned short data)
{
return AddData(reinterpret_cast<const BYTE*>(&data), sizeof(unsigned short));
}


/*
* Adds a stream of char to the buffer. 
*
*/
BOOL CIOCPBuffer::AddData(const char *const pData, UINT nSize)
{ 
	return AddData(reinterpret_cast<const BYTE*>(pData),nSize);
}

/*
* Adds a stream of BYTES to the buffer. 
*
*/

BOOL CIOCPBuffer::AddData(const BYTE *const pData, UINT nSize)
{
	if ( nSize > MAXIMUM_PACKAGE_SIZE-m_nUsed )
		return FALSE;
	else
	{
		memcpy(m_Buffer + m_nUsed, pData, nSize);
		m_nUsed += nSize;
		return TRUE;
	}
}


// cheks if the Buffer is valid. 
BOOL CIOCPBuffer::IsValid()
{
return TRUE;
}

UINT CIOCPBuffer::GetUsed()
{
return m_nUsed;
}

// Used to indicate that we did have  a successfull Receive 
UINT CIOCPBuffer::Use(UINT nSize)
{
m_nUsed+=nSize;
return m_nUsed;
}

// Empty A used structure. 
void CIOCPBuffer::EmptyUsed()
{
m_nUsed=0;
}

// Sets the current Operation. 
void CIOCPBuffer::SetOperation(int op)
{
ZeroMemory(&m_ol, sizeof(OVERLAPPED));
m_Operation=op;
}


// Setup the buffer for a ZeroByteRead. 
void CIOCPBuffer::SetupZeroByteRead()
{
 m_wsabuf.buf =(char*)m_Buffer;
 m_wsabuf.len = 0; 
}

// Setup Setup the buffer for a Read. 
void CIOCPBuffer::SetupRead()
{
if (m_nUsed == 0)
   {
      m_wsabuf.buf = reinterpret_cast<char*>(m_Buffer);
      m_wsabuf.len = MAXIMUM_PACKAGE_SIZE; 
   }
   else // We have received some of the data but not all .. 
   {
      m_wsabuf.buf = reinterpret_cast<char*>(m_Buffer) + m_nUsed;
      m_wsabuf.len = MAXIMUM_PACKAGE_SIZE - m_nUsed; 
   }
}

// Setup the buffer for a Write
void CIOCPBuffer::SetupWrite()
{
   m_wsabuf.buf = reinterpret_cast<char*>(m_Buffer);
   m_wsabuf.len = m_nUsed;
}

// Returns the WSABUF used with WsaRead and WsaWrite. 
WSABUF * CIOCPBuffer::GetWSABuffer()
{
return  const_cast<WSABUF*>(&m_wsabuf);
}

//Get the Operation 
int CIOCPBuffer::GetOperation()
{
return m_Operation;
}

// Saves a position into the buffer (e.g. in a CList) 
void CIOCPBuffer::SetPosition(POSITION pos)
{
m_pPos=pos;
}

// Returns the Position of a buffer. 
POSITION CIOCPBuffer::GetPosition()
{
return m_pPos;
}


// removes nSize byte from the Buffer. 
BOOL CIOCPBuffer::Flush(UINT nBytesToRemove)
{
if ((nBytesToRemove > MAXIMUM_PACKAGE_SIZE) || (nBytesToRemove > m_nUsed) ) 
{
TRACE("ERROR BOOL CIOCPBuffer::Flush(UINT nBytesToRemove)");
	return FALSE;
}
m_nUsed-=nBytesToRemove;
memmove(m_Buffer, m_Buffer + nBytesToRemove, m_nUsed);
return TRUE;
}



// Sets the sequence number of the buffer..
void CIOCPBuffer::SetSequenceNumber(int nr)
{
m_iSequenceNumber=nr;
}


// Gets the sequence number of the buffer. 
int CIOCPBuffer::GetSequenceNumber()
{
return m_iSequenceNumber;
}


/*
 * CreatePackage(CString stxt)
 * Creates one package in the buffe. (Se below) 
 *
 * [SizeHeader|...String..\0]
 */

BOOL CIOCPBuffer::CreatePackage(CStringA stxt)
{
	UINT nBufLen = stxt.GetLength();	
	if(nBufLen<MAXIMUM_PACKAGE_SIZE-MINIMUM_PACKAGE_SIZE&&nBufLen>0)
	{
		// Perpare Package. 
		// Empty the Buffer..
		EmptyUsed();
		// Add one to the size header for the null termination byte. 
		nBufLen++;
		// Add The Header
		AddData(nBufLen);
		// Add the string. 
		int length=stxt.GetLength();
		AddData((PBYTE) stxt.GetBuffer(length),length);


		// Null Teriminate (for Strings) 
		BYTE nullTerm='\0';
		AddData(nullTerm);
		return TRUE;
	}
	return FALSE;
}


/*
 * CreatePackage(BYTE Type,CString stxt)
 * Creates one package in the buffe. (Se below) 
 *
 * [SizeHeader|Type|...String..\0]
 */
BOOL CIOCPBuffer::CreatePackage(BYTE Type,CStringA stxt)
{
	UINT nBufLen = stxt.GetLength();	
	if(nBufLen<(MAXIMUM_PACKAGE_SIZE-MINIMUM_PACKAGE_SIZE-1)&&nBufLen>0)
	{
		// Perpare Package. 
		// Empty the Buffer..
		EmptyUsed();
		
		// Add one to the size header for the null termination byte.  
		nBufLen++;

		// Add one to the size header for the Type byte. .  
		nBufLen++;

		// Add The Header
		AddData(nBufLen);
		
		// Add the Type. 
		AddData(Type);
		
		// Add the string. 
		int length=stxt.GetLength();
		AddData((PBYTE) stxt.GetBuffer(length),length);


		// Null Teriminate (for Strings) 
		BYTE nullTerm='\0';
		AddData(nullTerm);
		return TRUE;
	}
	return FALSE;
}

/*
 * CreatePackage(BYTE Type,CString stxt)
 * Creates one package in the buffe. (Se below) 
 *
 * [SizeHeader|Type|...String..\0]
 */
BOOL CIOCPBuffer::CreatePackage(BYTE Type)
{
		// Perpare Package. 
		// Empty the Buffer..
		EmptyUsed();
		// Add one to the size header for the Type byte. .  
		UINT nBufLen=1;
		// Add The Header
		AddData(nBufLen);
		// Add the Type. 
		AddData(Type);
		return TRUE;
}



/*
 * CreatePackage(BYTE Type,CString stxt)
 * Creates one package in the buffe. (Se below) 
 *
 * [SizeHeader|Type|...String..\0]
 */

BOOL CIOCPBuffer::CreatePackage(BYTE Type,UINT nData, CStringA stxt)
{
	UINT nBufLen = stxt.GetLength();	
	if(nBufLen<(MAXIMUM_PACKAGE_SIZE-MINIMUM_PACKAGE_SIZE-1-sizeof(UINT))&&nBufLen>0)
	{
		// Perpare Package. 
		// Empty the Buffer..
		EmptyUsed();
		
		// Add one to the size header for the null termination byte.  
		nBufLen++;

		// Add one to the size header for the Type byte. .  
		nBufLen++;

		// Add The bytes for the nSize data. 
		nBufLen+=sizeof(UINT);

		// Add The Header
		AddData(nBufLen);
		
		// Add the Type. 
		AddData(Type);
		
		//Add the size data
		AddData(nData);

		// Add the string. 
		int length=stxt.GetLength();
		AddData((PBYTE) stxt.GetBuffer(length),length);

		// Null Teriminate (for Strings) 
		BYTE nullTerm='\0';
		AddData(nullTerm);
		return TRUE;
	}
	return FALSE;
}

/*
 * CreatePackage(BYTE Type, BYTE key, CString stxt)
 * Creates one package in the buffe. (Se below) 
 *
 * [SizeHeader|Type|Key|...String..\0]
 */
BOOL CIOCPBuffer::CreatePackage(BYTE Type, BYTE key, CStringA stxt)
{
	UINT nBufLen = stxt.GetLength();	
	if(nBufLen<(MAXIMUM_PACKAGE_SIZE-MINIMUM_PACKAGE_SIZE-2))
	{
		// Perpare Package. 
		// Empty the Buffer..
		EmptyUsed();
		
		// Add one to the size header for the null termination byte.  
		nBufLen++;

		// Add one to the size header for the Type byte. .  
		nBufLen++;
		
		// Add one to the size header for the key. .  
		nBufLen++;

		// Add The Header
		AddData(nBufLen);
		
		// Add the Type. 
		AddData(Type);
		
		//Add the key data
		AddData(key);
	

		// Add the string. 
		int length=stxt.GetLength();
		if(length>0)
		AddData((PBYTE) stxt.GetBuffer(length),length);

		// Null Teriminate (for Strings) 
		BYTE nullTerm='\0';
		AddData(nullTerm);
		return TRUE;
	}
	return FALSE;
}


/*
 * CreatePackage(BYTE Type,BYTE key1, BYTE key2, BYTE key3,CString stxt)
 * Creates one package in the buffe. (Se below) 
 *
 * [SizeHeader|Type|Key1|key2|key3|...String..\0]
 */
BOOL CIOCPBuffer::CreatePackage(BYTE Type, BYTE key1, BYTE key2, BYTE key3, CStringA stxt)
{
	UINT nBufLen = stxt.GetLength();	
	if(nBufLen<(MAXIMUM_PACKAGE_SIZE-MINIMUM_PACKAGE_SIZE-4))
	{
		// Perpare Package. 
		// Empty the Buffer..
		EmptyUsed();
		
		// Add one to the size header for the null termination byte.  
		nBufLen++;

		// Add one to the size header for the Type byte. .  
		nBufLen++;
		
		// Add four to the size header for the Keys. .  
		nBufLen+=3;

		
		// Add The Header
		AddData(nBufLen);
		
		// Add the Type. 
		AddData(Type);
		
		//Add the keys data
		AddData(key1);
		AddData(key2);
		AddData(key3);

		// Add the string. 
		int length=stxt.GetLength();
		if(length>0)
		AddData((PBYTE) stxt.GetBuffer(length),length);

		// Null Teriminate (for Strings) 
		BYTE nullTerm='\0';
		AddData(nullTerm);
		return TRUE;
	}
	return FALSE;
}


/*
 * CreatePackage(BYTE Type, UINT iFilesize,UINT iBufferSize, const BYTE *const pData)
 * Creates one package in the buffe. (Se below) 
 *
 * [SizeHeader|Type|iFilesize|..Buffer data...]
 */

BOOL CIOCPBuffer::CreatePackage(BYTE Type, UINT iFilesize,UINT iBufferSize, const BYTE *const pData)
{
	UINT nBufLen=iBufferSize;
	if(iBufferSize<(MAXIMUM_PACKAGE_SIZE-MINIMUM_PACKAGE_SIZE-1-sizeof(UINT)))
	{
		// Perpare Package. 
		// Empty the Buffer..
		EmptyUsed();
		
		// Add one to the size header for the Type byte. .  
		nBufLen++;

		// Add The bytes for the iFilesize data. 
		nBufLen+=sizeof(UINT);

		// Add The Header
		AddData(nBufLen);
		
		// Add the Type. 
		AddData(Type);
		
		//Add the size data
		AddData(iFilesize);

		// Add the Buffer. 
		if(iBufferSize>0)
		AddData(pData,iBufferSize);
	return TRUE;
	}
	return FALSE;
}




/*
 * Returns the PackageSize
 */
UINT CIOCPBuffer::GetPackageSize()
{
	UINT nSize=0;
	if(m_nUsed>=MINIMUM_PACKAGE_SIZE)
	{

		memmove(&nSize,GetBuffer(),MINIMUM_PACKAGE_SIZE);
	}
	return nSize;
}

// Gets the package Type return 255 if error. 
BYTE CIOCPBuffer::GetPackageType()
{
	BYTE Type= 255;
	UINT nSize=GetPackageSize();
	if ( nSize>=1 && nSize <=MAXIMUM_PAYLOAD_SIZE) 
	{
	 memmove(&Type,GetBuffer()+MINIMUM_PACKAGE_SIZE,1);
	} 
	return Type;
}


// Reads the info from a package created with CreatePackage(CString stxt);
BOOL CIOCPBuffer::GetPackageInfo(CStringA &stxt)
{
	int nSize=GetPackageSize();
	if ( nSize>0 && nSize <= MAXIMUM_PAYLOAD_SIZE)
	{
		PBYTE pData=GetBuffer()+MINIMUM_PACKAGE_SIZE;
		// Assumes that we already have a null termination. 
		stxt=pData;
		return TRUE;		
	}
	return FALSE;
}

// Reads the info from a package created with CreatePackage(BYTE Type,CString stxt);
BOOL CIOCPBuffer::GetPackageInfo(BYTE& Type, CStringA &stxt)
{
	int nSize=GetPackageSize();
	if ( nSize>=1 && nSize<=MAXIMUM_PAYLOAD_SIZE )
	{
		memmove(&Type,GetBuffer()+MINIMUM_PACKAGE_SIZE,1);
		
		PBYTE pData=GetBuffer()+MINIMUM_PACKAGE_SIZE+1;
		// Assumes that we already have a null termination. 
		stxt=pData;
		return TRUE;		
	}
	return FALSE;
}

// Reads the info from a package created with CreatePackage(BYTE Type,CString stxt);
BOOL CIOCPBuffer::GetPackageInfo(BYTE& Type,UINT& nData, CString& stxt)
{
	int nSize=GetPackageSize();
	if ( nSize>= 1+sizeof(UINT) && nSize<=MAXIMUM_PAYLOAD_SIZE)
	{
		// Get the Type
		memmove(&Type,GetBuffer()+MINIMUM_PACKAGE_SIZE,1);
		// Get the nSize 
		memmove(&nData,GetBuffer()+MINIMUM_PACKAGE_SIZE+1,sizeof(UINT));
		// Get The text
		PBYTE pData=GetBuffer()+MINIMUM_PACKAGE_SIZE+1+sizeof(UINT);
		// Assumes that we already have a null termination. 
		stxt=pData;
		return TRUE;		
	}
	return FALSE;
}



BOOL CIOCPBuffer::GetPackageInfo(BYTE &Type, BYTE &key1, BYTE &key2, BYTE &key3, CStringA &stxt)
{
	int nSize=GetPackageSize();
	if ( nSize>=1 && nSize<=MAXIMUM_PAYLOAD_SIZE )
	{
		// Get the Type
		memmove(&Type,GetBuffer()+MINIMUM_PACKAGE_SIZE,1);
		// Get the keys
		memmove(&key1,GetBuffer()+MINIMUM_PACKAGE_SIZE+1,1);
		memmove(&key2,GetBuffer()+MINIMUM_PACKAGE_SIZE+2,1);
		memmove(&key3,GetBuffer()+MINIMUM_PACKAGE_SIZE+3,1);
		// Get The text
		PBYTE pData=GetBuffer()+MINIMUM_PACKAGE_SIZE+4;
		// Assumes that we already have a null termination. 
		stxt=pData;
		return TRUE;	
	}
	return FALSE;
}


BOOL CIOCPBuffer::GetPackageInfo(BYTE &Type, BYTE &key, CStringA &stxt)
{
	int nSize=GetPackageSize();
	if ( nSize>=1 && nSize<=MAXIMUM_PAYLOAD_SIZE )
	{
		// Get the Type
		memmove(&Type,GetBuffer()+MINIMUM_PACKAGE_SIZE,1);
		// Get the keys
		memmove(&key,GetBuffer()+MINIMUM_PACKAGE_SIZE+1,1);
		// Get The text
		PBYTE pData=GetBuffer()+MINIMUM_PACKAGE_SIZE+2;
		// Assumes that we already have a null termination. 
		stxt=pData;
		return TRUE;	
	}
	return FALSE;
}


void CIOCPBuffer::DUMP()
{
for(unsigned int i=0;i<GetUsed();i++)
	{
	CString txt;
	BYTE b=*(GetBuffer()+i);
	txt.Format(_T("%d:%s;%d"),i,GetBuffer()+i,b);
	TRACE("%s\r\n",txt);
	}
}


