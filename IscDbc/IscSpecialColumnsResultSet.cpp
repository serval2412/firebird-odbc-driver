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
 *
 *
 *	2002-11-24	IscSpecialColumnsResultSet.cpp
 *				Contributed by C. G. Alvarez
 *				Improve handling of NUMERIC and DECIMAL fields
 *
 *
 */

// IscSpecialColumnsResultSet.cpp: implementation of the IscSpecialColumnsResultSet class.
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include "IscDbc.h"
#include "IscConnection.h"
#include "IscDatabaseMetaData.h"
#include "IscResultSet.h"
#include "IscPreparedStatement.h"
#include "IscBlob.h"
#include "IscSqlType.h"

#include "IscSpecialColumnsResultSet.h"

#define TYPE_NAME  4


namespace IscDbcLibrary {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
IscSpecialColumnsResultSet::IscSpecialColumnsResultSet(IscDatabaseMetaData *metaData)
		: IscMetaDataResultSet(metaData)

{

}

void IscSpecialColumnsResultSet::specialColumns (const char * catalog, const char * schema, const char * table, int scope, int nullable)
{
	JString sql = 
		"select distinct f.rdb$field_type as scope,\n"				// 1 
				"\tcast (rfr.rdb$field_name as varchar(31)) as column_name, \n"	// 2
				"\tf.rdb$field_type as data_type,\n"				// 3
				"\tcast (rfr.rdb$field_name as varchar(31)) as type_name,\n"	// 4
				"\t0 as column_size,\n"								// 5
				"\t0 as buffer_length,\n"							// 6
				"\t0 as decimal_digits,\n"							// 7
				"\tf.rdb$field_type as pseudo_column,\n"			// 8
				"\trel.rdb$constraint_type,\n"						// 9
				"\ti.rdb$index_id,\n"								//10
				"\tf.rdb$field_length as column_length,\n"			//11
				"\tf.rdb$field_scale as column_digits,\n"			//12
				"\tf.rdb$field_precision as column_precision\n,"	//13
				"\tf.rdb$field_sub_type\n"							//14
		"from rdb$fields f\n"
			"\tjoin rdb$relation_fields rfr\n" 
				"\t\ton rfr.rdb$field_source = f.rdb$field_name\n"
			"\tjoin rdb$indices i\n"
				"\t\ton rfr.rdb$relation_name = i.rdb$relation_name\n"
			"\tjoin rdb$index_segments s\n"
				"\t\ton rfr.rdb$field_name = s.rdb$field_name\n"
				"\t\tand i.rdb$index_name = s.rdb$index_name\n"
			"\tleft outer join rdb$relation_constraints rel\n"
				"\t\ton rel.rdb$constraint_type = 'PRIMARY KEY'\n"
				"\t\tand rel.rdb$index_name = i.rdb$index_name\n"
		"where i.rdb$unique_flag = 1\n";

	if ( !metaData->allTablesAreSelectable() )
		sql += metaData->existsAccess("\t\tand ", "rfr", 0, "\n");

	if(table && *table)
		sql += expandPattern ("\t\tand ","rfr.rdb$relation_name", table);

	sql += " order by rel.rdb$constraint_type, rdb$index_name, rdb$field_position";

	prepareStatement (sql);
	numberColumns = 8;
	index_id = -1;
}

bool IscSpecialColumnsResultSet::next ()
{
	if (!IscResultSet::next())
		return false;

	sqlda->updateShort (1,2);	//scope is always transaction for us

	int	idx_id = sqlda->getShort (10);
	if (index_id == -1) 
		index_id = idx_id;
	else if (idx_id != index_id)
		return false;

	//translate to the SQL type information
	int blrType = sqlda->getShort (3);	// field type
	int subType = sqlda->getShort (14);
	int length = sqlda->getShort (11);
	int scale = sqlda->getShort (12);
	int precision = sqlda->getShort (13);

	int dialect = statement->connection->getDatabaseDialect();
	IscSqlType sqlType (blrType, subType, length, length, dialect, precision, scale);

	char *type, t[50];
	type = t;
	sprintf (type, "%s", sqlType.typeName);

	sqlda->updateShort (3, sqlType.type);
	sqlda->updateVarying (4, type);

	setCharLen (5, 6, sqlType);

	scale = sqlda->getShort(12)*-1;
	sqlda->updateInt (7,scale);

	sqlda->updateShort (8,1);

	adjustResults (sqlType);

	return true;
}

void IscSpecialColumnsResultSet::setCharLen (int charLenInd, 
								      int fldLenInd, 
									  IscSqlType sqlType)
{
	int fldLen = sqlda->getInt (fldLenInd);
	int charLen = sqlda->getInt (charLenInd);

	if ( sqlda->isNull(charLenInd) )
		charLen = fldLen;

	if (sqlType.type != JDBC_VARCHAR &&
		sqlType.type != JDBC_CHAR)
	{
		charLen = sqlType.length;
		fldLen  = sqlType.bufferLength;
	}
	else
		charLen = fldLen = sqlType.length;
	
	sqlda->updateInt (fldLenInd, fldLen);

	if (!charLen)
		sqlda->setNull (charLenInd);
	else
		sqlda->updateInt (charLenInd, charLen);
}

void IscSpecialColumnsResultSet::adjustResults (IscSqlType sqlType)
{
	// decimal digits have no meaning for some columns
	// radix - doesn't mean much for some colums either
	switch (sqlType.type)
	{
	case JDBC_CHAR:
	case JDBC_VARCHAR:
	case JDBC_LONGVARCHAR:
	case JDBC_LONGVARBINARY:
	case JDBC_DATE:
	case JDBC_SQL_DATE:
		sqlda->setNull (7);
		break;
	case JDBC_TIME:
	case JDBC_SQL_TIME:
	case JDBC_TIMESTAMP:
	case JDBC_SQL_TIMESTAMP:
		sqlda->updateShort (7, -ISC_TIME_SECONDS_PRECISION_SCALE);
	}	
}

}; // end namespace IscDbcLibrary
