#pragma once


namespace Jde::DB::MySql::Sql
{
	string ColumnSql( bool addTable=false )noexcept;
	string IndexSql( bool addTable=false )noexcept;
}