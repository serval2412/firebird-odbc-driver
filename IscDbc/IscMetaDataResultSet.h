/*
 *  
 *     The contents of this file are subject to the Initial 
 *     Developer's Public License Version 1.0 (the "License"); 
 *     you may not use this file except in compliance with the 
 *     License. You may obtain a copy of the License at 
 *     http://www.ibphoenix.com/idpl.html. 
 *
 *     Software distributed under the License is distributed on 
 *     an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either 
 *     express or implied.  See the License for the specific 
 *     language governing rights and limitations under the License.
 *
 *
 *  The Original Code was created by James A. Starkey for IBPhoenix.
 *
 *  Copyright (c) 1999, 2000, 2001 James A. Starkey
 *  All Rights Reserved.
 */

// IscMetaDataResultSet.h: interface for the IscMetaDataResultSet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_ISCMETADATARESULTSET_H_)
#define _ISCMETADATARESULTSET_H_

#include "IscResultSet.h"
#include "JString.h"	// Added by ClassView

class IscDatabaseMetaData;
class IscPreparedStatement;
class IscResultSet;

class IscMetaDataResultSet : public IscResultSet  
{
public:
	JString expandPattern (const char *prefix, const char *string, const char *pattern);
	bool isWildcarded (const char *pattern);
	void trimBlanks (int id);
	virtual void prepareStatement (const char *sql);
	IscMetaDataResultSet(IscDatabaseMetaData *meta);
	virtual ~IscMetaDataResultSet();

	IscDatabaseMetaData		*metaData;
	IscPreparedStatement	*statement;
};

#endif // !defined(_ISCMETADATARESULTSET_H_)
