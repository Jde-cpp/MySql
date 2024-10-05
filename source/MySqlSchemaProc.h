#pragma once
#undef THROW
#include <jde/Exception.h>
#include <jde/db/metadata/Schema.h>
#include <jde/db/metadata/Table.h>
#include <jde/db/metadata/SchemaProc.h>

namespace Jde::DB{ struct IDataSource; }

namespace Jde::DB::MySql
{
	struct MySqlSchemaProc final : ISchemaProc
	{
		MySqlSchemaProc( sp<IDataSource> pDataSource ):
			ISchemaProc{ pDataSource }
		{}
		α LoadTables( sv catalog )noexcept(false)->flat_map<string,Table> override;
		α ToType( sv name )noexcept->EType override;
		α LoadIndexes( sv schema, sv tableName )noexcept(false)->vector<Index> override;
		α LoadForeignKeys( sv catalog )noexcept(false)->flat_map<string,ForeignKey> override;
		α LoadProcs( sv catalog={} )noexcept(false)->flat_map<string,Procedure> override;
	};
}