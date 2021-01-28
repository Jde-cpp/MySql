#include "MySqlStatements.h"

namespace Jde::DB::MySql::Sql
{
	string ColumnSql( bool addTable )noexcept
	{
		std::ostringstream os;
		os << "select	t.TABLE_NAME, COLUMN_NAME, ORDINAL_POSITION, COLUMN_DEFAULT, IS_NULLABLE, COLUMN_TYPE, CHARACTER_MAXIMUM_LENGTH, EXTRA='auto_increment' is_identity, 0 is_id, NUMERIC_PRECISION, NUMERIC_SCALE" << std::endl
			<< "from		INFORMATION_SCHEMA.TABLES t" << std::endl
			<< "\tinner join INFORMATION_SCHEMA.COLUMNS c on t.TABLE_CATALOG=c.TABLE_CATALOG and t.TABLE_SCHEMA=c.TABLE_SCHEMA and t.TABLE_NAME=c.TABLE_NAME" << std::endl
			<< "\twhere t.TABLE_SCHEMA=?" << std::endl
			<< "\tand\tt.TABLE_TYPE='BASE TABLE'" << std::endl;
		if( addTable )
			os << "\tand t.TABLE_NAME=:table_name" << std::endl;
		os << "order by t.TABLE_NAME, ORDINAL_POSITION" << std::endl;
		return os.str();
	}

	string IndexSql( bool addTable )noexcept
	{
		std::ostringstream os;
		os << "SELECT TABLE_NAME, INDEX_NAME, COLUMN_NAME, NON_UNIQUE, SEQ_IN_INDEX FROM INFORMATION_SCHEMA.STATISTICS WHERE TABLE_SCHEMA = ?";
		if( addTable )
			os << " and TABLE_NAME=?";
		os << " order by TABLE_NAME, INDEX_NAME, SEQ_IN_INDEX";
		return os.str();
	}

	string ForeignKeySql( bool addSchema )noexcept
	{
		std::ostringstream os;
		os << "select	fk.CONSTRAINT_NAME name, fk.TABLE_NAME foreign_table, fk.COLUMN_NAME fk, pk.TABLE_NAME primary_table, pk.COLUMN_NAME pk, pk.ORDINAL_POSITION ordinal" << endl
			<< "from		INFORMATION_SCHEMA.REFERENTIAL_CONSTRAINTS con" << endl
			<< "	join INFORMATION_SCHEMA.KEY_COLUMN_USAGE fk on con.CONSTRAINT_NAME=fk.CONSTRAINT_NAME" << endl
			<< "	join INFORMATION_SCHEMA.KEY_COLUMN_USAGE pk on pk.CONSTRAINT_NAME COLLATE utf8_general_ci=con.UNIQUE_CONSTRAINT_NAME and pk.ORDINAL_POSITION=fk.ORDINAL_POSITION and pk.TABLE_NAME=con.REFERENCED_TABLE_NAME" << endl;
		if( addSchema )
			os << "where pk.TABLE_SCHEMA=?" << endl;

		os << "order by name, ordinal";
		return os.str();
	}
	string ProcSql( bool addSchema )noexcept
	{
		std::ostringstream os{ "select SPECIFIC_NAME from INFORMATION_SCHEMA.ROUTINES", std::ios::ate };
		if( addSchema )
			os << " where ROUTINE_SCHEMA=?";
		return os.str();
	}
}