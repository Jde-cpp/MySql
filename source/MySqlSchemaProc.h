#pragma once
#include "../../Framework/source/db/types/Schema.h"
#include "../../Framework/source/db/types/Table.h"
#include "../../Framework/source/db/SchemaProc.h"

namespace Jde::DB
{
	struct IDataSource;
namespace MySql
{
	struct MySqlSchemaProc : public ISchemaProc
	{
		MapPtr<string,Types::Table> LoadTables( IDataSource& ds, string_view catalog )noexcept(false) override;
		DataType ToDataType( string_view name )noexcept override;
		//Types::Table LoadTable( IDataSource& ds, string_view catalog, string_view tableName )noexcept(false) override;
		//Types::Schema LoadSchema( IDataSource& ds, string_view catalog )noexcept(false) override;

		//static Types::Table ToTable( const mysqlx::Table& mysqlTable )noexcept;
	};
}}