#include "MySqlDataSource.h"
#include "MySqlRow.h"
#include "../../Framework/source/db/DBException.h"
#include "MySqlSchemaProc.h"
#include "../../Framework/source/db/Database.h"

#define var const auto

Jde::DB::IDataSource* GetDataSource()
{
	return new Jde::DB::MySql::MySqlDataSource();
}

namespace Jde::DB::MySql
{
	using mysqlx::SessionOption;
	shared_ptr<mysqlx::Session> MySqlDataSource::GetSession()noexcept(false)
	{
		try
		{
			auto pShared = new mysqlx::Session( ConnectionString() );
			return shared_ptr<mysqlx::Session>( pShared );
		}
		catch( const mysqlx::Error& e )
		{
			THROW2( "Could not create mysql session - {}", e.what() );
		}
	}

	mysqlx::Value ToMySqlValue( const DataValue& dataValue )
	{
		switch( (EDataValue)dataValue.index() )
		{
			case EDataValue::Null:
				return mysqlx::Value();
			case EDataValue::String:
				return mysqlx::Value( std::get<string>(dataValue) );
			case EDataValue::StringView:
				return mysqlx::Value( string(std::get<sv>(dataValue)) );
			case EDataValue::StringPtr:
				return mysqlx::Value( string(*std::get<sp<string>>(dataValue)) );
			case EDataValue::Bool:
				return mysqlx::Value( std::get<bool>(dataValue) );
			case EDataValue::Int:
				return mysqlx::Value( std::get<int>(dataValue) );
			case EDataValue::Int64:
				return mysqlx::Value( std::get<_int>(dataValue) );
			case EDataValue::Uint:
				return mysqlx::Value( std::get<uint>(dataValue) );
			case EDataValue::Decimal2:
				return mysqlx::Value( (double)std::get<Jde::Decimal2>(dataValue) );
			case EDataValue::Double:
				return mysqlx::Value( std::get<double>(dataValue) );
			case EDataValue::DoubleOptional:
			{
				var value = std::get<optional<double>>( dataValue );
				return value.has_value() ? mysqlx::Value( value.value() ) : mysqlx::Value();
			}
			case EDataValue::DateOptional:
			{
				var value = std::get<optional<DBDateTime>>( dataValue );
				if( !value.has_value() )
					return mysqlx::Value();
				else
				{
					const Jde::DateTime date{ value.value() };
					auto stringValue = date.ToIsoString();
					stringValue = Str::Replace( stringValue, "T", " " );
					stringValue = Str::Replace( stringValue, "Z", "" );
					uint nanos = value.value().time_since_epoch().count();
					constexpr uint NanosPerSecond = std::chrono::nanoseconds(1s).count();
					const double remainingNanos = nanos%NanosPerSecond;
				 	double fraction = remainingNanos/NanosPerSecond;
					var rounded = std::round( fraction*1000000 );
					var fraction2 = rounded/1000000;
					ostringstream os;
					os << std::setprecision(6) << std::fixed << fraction2;
					auto fractionString = os.str().substr(1);

					string value2 = fmt::format( "{}{}", stringValue, fractionString );//:.6
					if( value2.find('e')!=string::npos )
						ERR( "{} has {} for {} returning {}, nanos={}"sv, value2, fractionString, fraction, value2, value.value().time_since_epoch().count() );//2019-12-13 20:43:04.305e-06 has .305e-06 for 9.30500e-06 returning 2019-12-13 20:43:04.305e-06, nanos=1576269784000009305
					return mysqlx::Value( value2 );
				}
			}
		}
		THROWX( LogicException("dataValue index {} not implemented", dataValue.index()) );
		return mysqlx::Value( "compiler remove warning noop" );
	}

	uint MySqlDataSource::Select( sv sql, std::function<void(const IRow&)> f, const vector<DataValue>* pValues, bool log )noexcept(false)
	{
		auto pSession = GetSession();
		auto statement = pSession->sql( string(sql) );
		if( pValues )
		{
			for( var& value : *pValues )
				statement.bind( ToMySqlValue( value ) );
		}
		uint count = 0;
		try
		{
			auto result = statement.execute();
			if( log )
				DBLOG( sql, pValues );
			std::list<mysqlx::Row> rows = result.fetchAll();
			count = rows.size();
			for( mysqlx::Row& row : rows )
				f( MySqlRow(row) );
			return count;
		}
		catch( const ::mysqlx::Error& e )
		{
			THROWX( DBException(e, sql, pValues) );
		}
	}
	uint MySqlDataSource::Execute( sv sql )
	{
		return Execute( sql, nullptr, nullptr );
	}
	uint MySqlDataSource::Execute( sv sql, const vector<DataValue>& parameters, bool log )noexcept(false)
	{
		return Execute( sql, &parameters, nullptr, false, log );
	}

	uint MySqlDataSource::ExecuteProc( sv sql, const vector<DataValue>& parameters, bool log )
	{
		return Execute( sql, &parameters, nullptr, true, log );
	}
	uint MySqlDataSource::ExecuteProc( sv sql, const vector<DataValue>& parameters, function<void(const IRow&)> f, bool log )
	{
		return Execute( sql, &parameters, &f, true, log );
	}

//https://dev.mysql.com/doc/refman/8.0/en/c-api-prepared-call-statements.html
	uint MySqlDataSource::Execute( sv sql, const vector<DataValue>* pParameters, std::function<void(const IRow&)>* pFunction, bool isStoredProc, bool log )noexcept(false)
	{
		auto pSession = GetSession();
		string fullSql = isStoredProc ? fmt::format( "call {}", sql ) : string( sql );
		auto statement = pSession->sql( fullSql );
		if( pParameters )
		{
			for( var& parameter : *pParameters )
				statement.bind( ToMySqlValue(parameter) );
		}
		try
		{
			auto result = statement.execute();
			if( pFunction )
			{
				auto rows = result.fetchAll();
				for( const mysqlx::Row& row : rows )
					(*pFunction)( MySqlRow(row) );
			}
		}
		catch( const ::mysqlx::Error& e )
		{
			THROWX( DBException(e, fullSql, pParameters) );
		}
		return 1;//wari
	}

	sp<ISchemaProc> MySqlDataSource::SchemaProc()noexcept
	{
		sp<IDataSource> p = shared_from_this();
		return make_shared<MySqlSchemaProc>( p );
	}

	AsyncAwaitable MySqlDataSource::SelectCo( string&& sql, std::function<void(const IRow&)> f, const std::vector<DataValue>&& parameters, bool log )noexcept
	{
		return AsyncAwaitable{ [ql=move(sql),params=move(parameters),log,f,this]()
		{
			return make_shared<uint>( Select(ql, f, &params, log) );
		}};
	}
}