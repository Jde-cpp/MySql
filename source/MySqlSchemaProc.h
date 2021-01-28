#pragma once
#include "../../Framework/source/db/types/Schema.h"
#include "../../Framework/source/db/types/Table.h"
#include "../../Framework/source/db/SchemaProc.h"

namespace Jde::DB
{
	struct IDataSource;
namespace MySql
{
	struct MySqlSchemaProc final : public ISchemaProc
	{
		MySqlSchemaProc( sp<IDataSource> pDataSource ):
			ISchemaProc{ pDataSource }
		{}
		MapPtr<string,Table> LoadTables( string_view catalog )noexcept(false) override;
		DataType ToDataType( string_view name )noexcept override;
		vector<Index> LoadIndexes( sv schema, sv tableName )noexcept(false) override;
		flat_map<string,ForeignKey> LoadForeignKeys( sv catalog )noexcept(false) override;
		flat_map<string,Procedure> LoadProcs( string_view catalog={} )noexcept(false) override;
		//Types::Table LoadTable( IDataSource& ds, string_view catalog, string_view tableName )noexcept(false) override;
		//Types::Schema LoadSchema( IDataSource& ds, string_view catalog )noexcept(false) override;

		//static Types::Table ToTable( const mysqlx::Table& mysqlTable )noexcept;
	};
}}