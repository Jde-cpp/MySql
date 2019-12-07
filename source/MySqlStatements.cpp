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
		os << "SELECT * FROM INFORMATION_SCHEMA.STATISTICS WHERE TABLE_SCHEMA = ?";
		if( addTable )
			os << " and TABLE_NAME=?";
		return os.str();
	}

}