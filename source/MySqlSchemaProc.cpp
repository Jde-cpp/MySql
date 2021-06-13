#include "MySqlSchemaProc.h"
#include "MySqlDataSource.h"
#include "MySqlStatements.h"
#include "../../Framework/source/db/Row.h"
#include "../../Framework/source/db/Syntax.h"
#define var const auto

namespace Jde::DB::MySql
{
	MapPtr<string,Table> MySqlSchemaProc::LoadTables( sv catalog )noexcept(false)
	{
		string catalogLocal;
		if( catalog.empty() )
			catalog = catalogLocal = _pDataSource->Catalog( DB::MySqlSyntax{}.CatalogSelect() );
		auto pTables = make_shared<map<string,Table>>();
		//std::function<void(const string& name, const string& COLUMN_NAME, int ordinalPosition, const string& dflt, int isNullable, const string& type, int maxLength, int isIdentity, int isId, int NumericPrecision, int NumericScale)>
		auto result2 = [&]( sv tableName, sv name, _int ordinalPosition, sv dflt, string isNullable, sv type, optional<_int> maxLength, _int isIdentity, _int isId, optional<_int> numericPrecision, optional<_int> numericScale )
		{
			auto& table = pTables->emplace( tableName, Table{catalog,tableName} ).first->second;
			table.Columns.resize( ordinalPosition );

			table.Columns[ordinalPosition-1] = Column{ name, (uint)ordinalPosition, dflt, isNullable!="NO", ToDataType(type), maxLength.value_or(0), isIdentity!=0, isId!=0, numericPrecision, numericScale };
		};
		auto result = [&]( const IRow& row )
		{
			result2( row.GetString(0), row.GetString(1), row.GetInt(2), row.GetString(3), row.GetString(4), row.GetString(5), row.GetIntOpt(6), row.GetInt(7), row.GetInt(8), row.GetIntOpt(9), row.GetIntOpt(10) );
		};
		var sql = Sql::ColumnSql( false );
		//var sql = "select * from INFORMATION_SCHEMA.TABLES where catalog=:catalog";
		//DBG( sql );
		//Jde::DB::MySql::MySqlDataSource& dataSource = dynamic_cast<Jde::DB::MySql::MySqlDataSource&>( ds );
		//std::vector<DataValue> parameters{ "market"sv };
		_pDataSource->Select( sql, result, {catalog} );
		//dataSource.Select( sql, result, {{"catalog","market"}} );
		return pTables;
	}

	vector<Index> MySqlSchemaProc::LoadIndexes( sv catalog, sv tableName )noexcept(false)
	{
		string catalogLocal;
		if( catalog.empty() )
			catalog = catalogLocal = _pDataSource->Catalog( DB::MySqlSyntax{}.CatalogSelect() );

		vector<Index> indexes;
		//std::function<void(const string& indexName, const string& tableName, bool unique, const string& columnName, bool primaryKey)>
		auto result = [&]( const IRow& row )
		{
			uint i=0;
			var tableName = row.GetString(i++); var indexName = row.GetString(i++); var columnName = row.GetString(i++); var unique = row.GetBit(i++)==0;

			//var ordinal = row.GetUInt(i++); var dflt = row.GetString(i++);  //var primaryKey = row.GetBit(i);
			vector<SchemaName>* pColumns;
			auto pExisting = std::find_if( indexes.begin(), indexes.end(), [&](auto index){ return index.Name==indexName && index.TableName==tableName; } );
			if( pExisting==indexes.end() )
			{
				bool clustered = false;//Boolean.Parse( row["CLUSTERED"].ToString() );
				bool primaryKey = indexName=="PRIMARY";//Boolean.Parse( row["PRIMARY_KEY"].ToString() );
				//var pTable = tables.find( tableName );
				//if( pTable==tables.end() )
				//	THROW2( LogicException("Could not find table '{}' for index '{}'.", tableName, indexName) );
				pColumns = &indexes.emplace_back( indexName, tableName, primaryKey, nullptr, unique, clustered ).Columns;
			}
			else
				pColumns = &pExisting->Columns;
			pColumns->push_back( columnName );
		};

		std::vector<DataValue> values{catalog};
		if( tableName.size() )
			values.push_back( tableName );
		_pDataSource->Select( Sql::IndexSql(tableName.size()), result, values, true );

		return indexes;
	}

	flat_map<string,Procedure> MySqlSchemaProc::LoadProcs( sv catalog )noexcept(false)
	{
		string catalogLocal;
		if( catalog.empty() )
			catalog = catalogLocal = _pDataSource->Catalog( DB::MySqlSyntax{}.CatalogSelect() );
		//std::vector<DataValue> params;
		//if( catalog.size() )
		//	params.emplace_back( catalog );
		flat_map<string,Procedure> values;
		auto fnctn = [&values]( const IRow& row )
		{
			string name = row.GetString(0);
			values.try_emplace( name, Procedure{name} );
		};
		_pDataSource->Select( Sql::ProcSql(true), fnctn, {catalog}, true );
		return values;
	}

	DataType MySqlSchemaProc::ToDataType( sv typeName )noexcept
	{
		DataType type{ DataType::None };
		if(typeName=="datetime")
			type=DataType::DateTime;
		else if( typeName=="smalldatetime" )
			type=DataType::SmallDateTime;
		else if(typeName=="float")
			type=DataType::Float;
		else if(typeName=="real")
			type=DataType::SmallFloat;
		else if( typeName=="int" || typeName=="int(11)" )
			type = DataType::Int;
		else if( typeName=="int(10) unsigned" || typeName=="int unsigned" )
			type = DataType::UInt;
		else if( typeName=="bigint(21) unsigned" || typeName=="bigint(20) unsigned" )
			type = DataType::ULong;
		else if( Str::StartsWith(typeName, "bigint") )
			type=DataType::Long;
		else if( typeName=="nvarchar" )
			type=DataType::VarWChar;
		else if(typeName=="nchar")
			type=DataType::WChar;
		else if( Str::StartsWith(typeName, "smallint"))
			type=DataType::Int16;
		else if(typeName=="tinyint")
			type=DataType::Int8;
		else if( typeName=="tinyint unsigned" )
			type=DataType::UInt8;
		else if( typeName=="uniqueidentifier" )
			type=DataType::Guid;
		else if(typeName=="varbinary")
			type=DataType::VarBinary;
		else if( Str::StartsWithInsensitive(typeName, "varchar") )
			type=DataType::VarChar;
		else if(typeName=="ntext")
			type=DataType::NText;
		else if(typeName=="text")
			type=DataType::Text;
		else if( typeName.starts_with("char") )
			type=DataType::Char;
		else if(typeName=="image")
			type=DataType::Image;
		else if( Str::StartsWith(typeName, "bit") )
			type=DataType::Bit;
		else if( Str::StartsWith(typeName, "binary") )
			type=DataType::Binary;
		else if( Str::StartsWith(typeName, "decimal") )
			type=DataType::Decimal;
		else if(typeName=="numeric")
			type=DataType::Numeric;
		else if(typeName=="money")
			type=DataType::Money;
		else
			GetDefaultLogger()->warn( "Unknown datatype({}).  need to implement, no big deal if not our table.", typeName );
		return type;
	}

	flat_map<string,ForeignKey> MySqlSchemaProc::LoadForeignKeys( sv catalog )noexcept(false)
	{
		string catalogLocal;
		if( catalog.empty() )
			catalog = catalogLocal = _pDataSource->Catalog( DB::MySqlSyntax{}.CatalogSelect() );
		flat_map<string,ForeignKey> fks;
		auto result = [&]( const IRow& row )
		{
			uint i=0;
			var name = row.GetString(i++); var fkTable = row.GetString(i++); var column = row.GetString(i++); var pkTable = row.GetString(i++); //var pkColumn = row.GetString(i++); var ordinal = row.GetUInt(i);
			auto pExisting = fks.find( name );
			if( pExisting==fks.end() )
				fks.emplace( name, ForeignKey{name, fkTable, {column}, pkTable} );
			else
				pExisting->second.Columns.push_back( column );
		};
		_pDataSource->Select( Sql::ForeignKeySql(catalog.size()), result, {catalog}, true );
		return fks;
	}
/*	Schema LoadSchema( IDataSource& ds, sv catalog )noexcept(false) override;
	{

	}

	Table MySqlSchemaProc::ToTable( const mysqlx::Table& mysqlTable )noexcept
	{

		//return Table( mysqlTable.);
	}
*/
}