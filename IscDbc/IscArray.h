//////////////////////////////////////////////////////////////////////
//  
// IscArray.h: interface for the IscArray class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_IscArray_H_)
#define _IscArray_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "BinaryBlob.h"
#include "Connection.h"

class IscConnection;
class IscStatement;
class Value;

struct SIscArrayData
{
	void*			arrBufData;
	int				arrBufDataSize;
	int				arrCountElement;
	int				arrSizeElement;
	int				arrTypeElement;
};

class IscArray : public BinaryBlob
{
public:

	void attach(SIscArrayData * arr, bool bClear = false);
	void detach(SIscArrayData * arr);
	void removeBufData();
	void getBytesFromArray();
	void fetchArrayToString();
	void writeArray(Value * value);

	virtual void getBytes(long pos, long length, void * address);
	virtual int length();
	virtual int getSegment (int offset, int length, void *address);
	virtual int	getLength();

	IscArray(SIscArrayData * ptArr);
	IscArray(IscConnection	*parentConnection,XSQLVAR *var);
	virtual ~IscArray();

	IscConnection	*connection;
	ISC_QUAD		arrayId;
	bool			clear;
	bool			fetched;
	bool			fetchedBinary;
	ISC_ARRAY_DESC	arrDesc;
	void*			arrBufData;
	int				arrBufDataSize;
	int				arrCountElement;
	int				arrSizeElement;
	int				arrTypeElement;
};

#endif // !defined(_IscArray_H_)
