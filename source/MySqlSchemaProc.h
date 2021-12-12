#pragma once
#undef THROW
#include <jde/Exception.h>
#include "../../Framework/source/db/types/Schema.h"
#include "../../Framework/source/db/types/Table.h"
#include "../../Framework/source/db/SchemaProc.h"

namespace Jde::DB{ struct IDataSource; }

namespace Jde::DB::MySql
{
	struct MySqlSchemaProc final : ISchemaProc
	{
		MySqlSchemaProc( sp<IDataSource> pDataSource ):
			ISchemaProc{ pDataSource }
		{}
		α LoadTables( sv catalog )noexcept(false)->up<flat_map<string,Table>> override;
		α ToType( sv name )noexcept->EType override;
		α LoadIndexes( sv schema, sv tableName )noexcept(false)->vector<Index> override;
		α LoadForeignKeys( sv catalog )noexcept(false)->flat_map<string,ForeignKey> override;
		α LoadProcs( sv catalog={} )noexcept(false)->flat_map<string,Procedure> override;
	};
}