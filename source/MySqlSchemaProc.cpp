#include "MySqlSchemaProc.h"
#include "MySqlDataSource.h"
#include "MySqlStatements.h"
#include "../../Framework/source/db/Row.h"
#include "../../Framework/source/db/Syntax.h"
#define var const auto

namespace Jde::DB::MySql{
	static sp<LogTag> _logTag = Logging::Tag( "dbDriver" );

	α MySqlSchemaProc::LoadTables( sv catalog )noexcept(false)->flat_map<string,Table>{
		string catalogLocal;
		if( catalog.empty() )
			catalog = catalogLocal = _pDataSource->Catalog( string{DB::MySqlSyntax{}.CatalogSelect()} );
		auto tables = flat_map<string,Table>();
		auto result2 = [&]( sv tableName, sv name, _int ordinalPosition, sv dflt, string isNullable, sv type, optional<_int> maxLength, _int isIdentity, _int isId, optional<_int> numericPrecision, optional<_int> numericScale ){
			auto& table = tables.emplace( tableName, Table{catalog,tableName} ).first->second;
			table.Columns.resize( ordinalPosition );

			table.Columns[ordinalPosition-1] = Column{ name, (uint)ordinalPosition, dflt, isNullable!="NO", ToType(type), maxLength.value_or(0), isIdentity!=0, isId!=0, numericPrecision, numericScale };
		};
		auto result = [&]( const IRow& row ){
			result2( row.GetString(0), row.GetString(1), row.GetInt(2), row.GetString(3), row.GetString(4), row.GetString(5), row.GetIntOpt(6), row.GetInt(7), row.GetInt(8), row.GetIntOpt(9), row.GetIntOpt(10) );
		};
		var sql = Sql::ColumnSql( false );
		_pDataSource->Select( sql, result, {catalog} );
		return tables;
	}

	α MySqlSchemaProc::LoadIndexes( sv catalog, sv tableName )noexcept(false)->vector<Index>{
		string catalogLocal;
		if( catalog.empty() )
			catalog = catalogLocal = _pDataSource->Catalog( string{DB::MySqlSyntax{}.CatalogSelect()} );

		vector<Index> indexes;
		auto result = [&]( const IRow& row ){
			uint i=0;
			var tableName = row.GetString(i++); var indexName = row.GetString(i++); var columnName = row.GetString(i++); var unique = row.GetBit(i++)==0;

			//var ordinal = row.GetUInt(i++); var dflt = row.GetString(i++);  //var primaryKey = row.GetBit(i);
			vector<SchemaName>* pColumns;
			auto pExisting = std::find_if( indexes.begin(), indexes.end(), [&](auto index){ return index.Name==indexName && index.TableName==tableName; } );
			if( pExisting==indexes.end() ){
				bool clustered = false;//Boolean.Parse( row["CLUSTERED"].ToString() );
				bool primaryKey = indexName=="PRIMARY";//Boolean.Parse( row["PRIMARY_KEY"].ToString() );
				pColumns = &indexes.emplace_back( indexName, tableName, primaryKey, nullptr, unique, clustered ).Columns;
			}
			else
				pColumns = &pExisting->Columns;
			pColumns->push_back( columnName );
		};

		vector<object> values{catalog};
		if( tableName.size() )
			values.push_back( tableName );
		_pDataSource->Select( Sql::IndexSql(tableName.size()), result, values );

		return indexes;
	}

	α MySqlSchemaProc::LoadProcs( sv catalog )noexcept(false)->flat_map<string,Procedure>{
		string catalogLocal;
		if( catalog.empty() )
			catalog = catalogLocal = _pDataSource->Catalog( string{DB::MySqlSyntax{}.CatalogSelect()} );
		flat_map<string,Procedure> values;
		auto fnctn = [&values]( const IRow& row ){
			string name = row.GetString(0);
			values.try_emplace( name, Procedure{name} );
		};
		_pDataSource->Select( Sql::ProcSql(true), fnctn, {catalog} );
		return values;
	}

	α MySqlSchemaProc::ToType( sv typeName )noexcept->EType{
		auto type{ EType::None };
		if(typeName=="datetime")
			type = EType::DateTime;
		else if( typeName=="smalldatetime" )
			type = EType::SmallDateTime;
		else if(typeName=="float")
			type = EType::Float;
		else if(typeName=="real")
			type = EType::SmallFloat;
		else if( typeName=="int" || typeName=="int(11)" )
			type = EType::Int;
		else if( typeName=="int(10) unsigned" || typeName=="int unsigned" )
			type = EType::UInt;
		else if( typeName=="bigint(21) unsigned" || typeName=="bigint(20) unsigned" )
			type = EType::ULong;
		else if( Str::StartsWith(typeName, "bigint") )
			type = EType::Long;
		else if( typeName=="nvarchar" )
			type = EType::VarWChar;
		else if(typeName=="nchar")
			type = EType::WChar;
		else if( Str::StartsWith(typeName, "smallint"))
			type = EType::Int16;
		else if(typeName=="tinyint")
			type = EType::Int8;
		else if( typeName=="tinyint unsigned" )
			type = EType::UInt8;
		else if( typeName=="uniqueidentifier" )
			type = EType::Guid;
		else if(typeName=="varbinary")
			type = EType::VarBinary;
		else if( Str::StartsWithInsensitive(typeName, "varchar") )
			type = EType::VarChar;
		else if(typeName=="ntext")
			type = EType::NText;
		else if(typeName=="text")
			type = EType::Text;
		else if( typeName.starts_with("char") )
			type = EType::Char;
		else if(typeName=="image")
			type = EType::Image;
		else if( Str::StartsWith(typeName, "bit") )
			type = EType::Bit;
		else if( Str::StartsWith(typeName, "binary") )
			type = EType::Binary;
		else if( Str::StartsWith(typeName, "decimal") )
			type = EType::Decimal;
		else if(typeName=="numeric")
			type = EType::Numeric;
		else if(typeName=="money")
			type = EType::Money;
		else
			WARN( "Unknown datatype({}).  need to implement, ok if not our table."sv, typeName );
		return type;
	}

	α MySqlSchemaProc::LoadForeignKeys( sv catalog )noexcept(false)->flat_map<string,ForeignKey>{
		string catalogLocal;
		if( catalog.empty() )
			catalog = catalogLocal = _pDataSource->Catalog( string{DB::MySqlSyntax{}.CatalogSelect()} );
		flat_map<string,ForeignKey> fks;
		auto result = [&]( const IRow& row ){
			uint i=0;
			var name = row.GetString(i++); var fkTable = row.GetString(i++); var column = row.GetString(i++); var pkTable = row.GetString(i++); //var pkColumn = row.GetString(i++); var ordinal = row.GetUInt(i);
			auto pExisting = fks.find( name );
			if( pExisting==fks.end() )
				fks.emplace( name, ForeignKey{name, fkTable, {column}, pkTable} );
			else
				pExisting->second.Columns.push_back( column );
		};
		_pDataSource->Select( Sql::ForeignKeySql(catalog.size()), result, {catalog} );
		return fks;
	}
}