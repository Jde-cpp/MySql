#include "MySqlSchemaProc.h"
#include "MySqlDataSource.h"
#include "MySqlStatements.h"
#include "../../Framework/source/db/Row.h"
#define var const auto

namespace Jde::DB::MySql
{
	MapPtr<string,Types::Table> MySqlSchemaProc::LoadTables( IDataSource& ds, string_view catalog )noexcept(false)
	{
		auto pTables = make_shared<map<string,Types::Table>>();
		//std::function<void(const string& name, const string& COLUMN_NAME, int ordinalPosition, const string& dflt, int isNullable, const string& type, int maxLength, int isIdentity, int isId, int NumericPrecision, int NumericScale)>
		auto result2 = [&]( const string& name, const string& COLUMN_NAME, _int ordinalPosition, const string& dflt, string isNullable, const string& type, optional<_int> maxLength, _int isIdentity, _int isId, optional<_int> numericPrecision, optional<_int> numericScale )
		{
			auto& table = pTables->emplace( name, Types::Table{catalog,name} ).first->second;
			table.Columns.resize( ordinalPosition );

			table.Columns[ordinalPosition-1] = make_shared<Types::Column>( name, ordinalPosition, dflt, isNullable!="NO", ToDataType(type), maxLength, isIdentity, isId, numericPrecision, numericScale );
		};
		auto result = [&]( const IRow& row )
		{
			result2( row.GetString(0), row.GetString(1), row.GetInt(2), row.GetString(3), row.GetString(4), row.GetString(5), row.GetIntOpt(6), row.GetInt(7), row.GetInt(8), row.GetIntOpt(9), row.GetIntOpt(10) );
		};
		var sql = Sql::ColumnSql( false );  
		//var sql = "select * from INFORMATION_SCHEMA.TABLES where catalog=:catalog";
		GetDefaultLogger()->debug( sql ); 
		//Jde::DB::MySql::MySqlDataSource& dataSource = dynamic_cast<Jde::DB::MySql::MySqlDataSource&>( ds );
		std::vector<DataValue> parameters{ "market"sv };
		ds.Select( sql, result, parameters );
		//dataSource.Select( sql, result, {{"catalog","market"}} );
		return pTables;
	}

/*	forward_list<Types::Index> LoadIndexes( IDataSource ds, map<string,Types::TablePtr_> tables, string_view schema )
	{
		auto indexes = forward_list<Types::Index>();
		std::function<void(const string& indexName, const string& tableName, bool unique, const string& columnName, bool primaryKey)> result = [&]( const string& indexName, const string& tableName, bool unique, const string& columnName, bool primaryKey )
		{
			auto pExisting = std::find_if( indexes.begin(), indexes.end(), [&indexName, &tableName](auto index){ index.Name==indexName && index.TableName==tableName; } );
			if( pExisting==indexes.end() )
			{
				bool clustered = false;//Boolean.Parse( row["CLUSTERED"].ToString() );
				bool primaryKey = indexName=="PRIMARY";//Boolean.Parse( row["PRIMARY_KEY"].ToString() );
				var pTable = tables.find( tableName );
				if( pTable==tables.end() )
					THROW2( LogicException("Could not find table '{}' for index '{}'.", tableName, indexName) );
				indexes.emplace_front( pTable->second, indexName, clustered, unique, primaryKey );
			}

			auto pColumn = pExisting->TablePtr->FindColumn( columnName );
			if( pColumn==nullptr )
				THROW2( LogicException("Could not find column '{}' for table '{}' for index '{}'.", columnName, tableName, indexName) );
			if( primaryKey )
				pColumn->SrrgtKey = DB::Types::SurrogateKey();
			pExisting->Columns.push_back( pColumn );	
		};

		Jde::DB::MySql::MySqlDataSource& dataSource = dynamic_cast<Jde::DB::MySql::MySqlDataSource&>( ds );
		dataSource.Select( Sql::IndexSql(false), result, schema );

		return indexes;
	}
*/

	DataType MySqlSchemaProc::ToDataType( string_view typeName )noexcept
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
		else if( typeName=="int(10) unsigned" )
			type = DataType::UInt;
		else if( typeName=="bigint(21) unsigned" || typeName=="bigint(20) unsigned" )
			type = DataType::ULong;
		else if( StringUtilities::StartsWith(typeName, "bigint") )
			type=DataType::Long;
		else if( typeName=="nvarchar" )
			type=DataType::VarWChar;
		else if(typeName=="nchar")
			type=DataType::WChar;
		else if(StringUtilities::StartsWith(typeName, "smallint"))
			type=DataType::Int16;
		else if(typeName=="tinyint")
			type=DataType::Int8;
		else if(typeName=="uniqueidentifier")
			type=DataType::Guid;
		else if(typeName=="varbinary")
			type=DataType::VarBinary;
		else if( StringUtilities::StartsWithInsensitive(typeName, "varchar") )
			type=DataType::VarChar;
		else if(typeName=="ntext")
			type=DataType::NText;
		else if(typeName=="text")
			type=DataType::Text;
		else if(typeName=="char")
			type=DataType::Char;
		else if(typeName=="image")
			type=DataType::Image;
		else if(StringUtilities::StartsWith(typeName, "bit") )
			type=DataType::Bit;
		else if( StringUtilities::StartsWith(typeName, "binary") )
			type=DataType::Binary;
		else if(StringUtilities::StartsWith(typeName, "decimal") )
			type=DataType::Decimal;
		else if(typeName=="numeric")
			type=DataType::Numeric;
		else if(typeName=="money")
			type=DataType::Money;
		else
			GetDefaultLogger()->warn( "Unknown datatype({}).  need to implement, no big deal if not our table.", typeName );
		return type;
	}
/*	Types::Schema LoadSchema( IDataSource& ds, string_view catalog )noexcept(false) override;
	{

	}

	Types::Table MySqlSchemaProc::ToTable( const mysqlx::Table& mysqlTable )noexcept
	{
		
		//return Types::Table( mysqlTable.);
	}
*/
}