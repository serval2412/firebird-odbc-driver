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
 *     2002-08-12	File Created by Carlos G. Alvarez
 *
 *     The Original Code was created by James A. Starkey for IBPhoenix.
 *
 *     Copyright (c) 1999, 2000, 2001 James A. Starkey
 *     All Rights Reserved.
 */

// IscTablePrivilegesResultSet.cpp: implementation of the IscTablePrivilegesResultSet class.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include "IscDbc.h"
#include "IscTablePrivilegesResultSet.h"
#include "IscConnection.h"
#include "IscDatabaseMetaData.h"
#include "IscResultSet.h"
#include "IscPreparedStatement.h"
#include "IscBlob.h"

#define TABLE_TYPE    4


namespace IscDbcLibrary {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IscTablePrivilegesResultSet::IscTablePrivilegesResultSet(IscDatabaseMetaData *metaData)
        : IscMetaDataResultSet(metaData)
{
}

void IscTablePrivilegesResultSet::getTablePrivileges(const char * catalog, const char * schemaPattern, const char * tableNamePattern)
{
	JString sql = "select cast (NULL as varchar(7)) as table_cat,"						//1
				          "cast (NULL as varchar(7)) as table_schem,"					//2
						  "cast (tbl.rdb$relation_name as varchar(31)) as table_name,"	//3
						  "cast (priv.rdb$grantor as varchar(31)) as grantor,"			//4
						  "cast (priv.rdb$user as varchar(31)) as grantee,"				//5
						  "cast (priv.rdb$privilege as varchar(11)) as privilege,"		//6
						  "cast ('YES' as varchar(3)) as isgrantable, "					//7
						  "priv.rdb$grant_option as GRANT_OPTION "						//8
                          "from rdb$relations tbl, rdb$user_privileges priv\n"
                          "where tbl.rdb$relation_name = priv.rdb$relation_name\n";

	if ( !metaData->allTablesAreSelectable() )
	{
		char buf[128];
		sprintf (buf, "and priv.rdb$object_type = 0\n"
					  "and priv.rdb$user = '%s' and priv.rdb$user_type = %d\n",
						metaData->getUserAccess(),metaData->getUserType());
		sql +=	buf;
	}

    if (tableNamePattern && *tableNamePattern)
        sql += expandPattern (" and ","tbl.rdb$relation_name", tableNamePattern);

    sql += " order by tbl.rdb$relation_name, priv.rdb$privilege, priv.rdb$user";

    prepareStatement (sql);
    numberColumns = 7;
}

bool IscTablePrivilegesResultSet::next()
{
    if (!IscResultSet::next())
        return false;

	int len1, len2;
	const char *grantor = sqlda->getVarying(4, len1);
	const char *grantee = sqlda->getVarying(5, len2);
	if( len1 == len2 && !strncmp(grantor,grantee,len1) )
		sqlda->updateVarying (4, "_SYSTEM");

	const char *privilege = sqlda->getVarying(6, len1);

    switch ( *privilege )
    {
        case 'S':
            sqlda->updateVarying( 6, "SELECT" );
            break;

        case 'I':
            sqlda->updateVarying( 6, "INSERT" );
            break;

        case 'U':
            sqlda->updateVarying( 6, "UPDATE" );
            break;

        case 'D':
            sqlda->updateVarying( 6, "DELETE" );
            break;

        case 'R':
            sqlda->updateVarying( 6, "REFERENCES" );
            break;
    }

	int isGrantable = sqlda->getShort(8);

	if ( !isGrantable )
		sqlda->updateVarying( 7, "NO" );

    return true;
}

}; // end namespace IscDbcLibrary
