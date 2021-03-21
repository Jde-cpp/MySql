#include "MySqlDataSource.h"
#include "MySqlRow.h"
#include "../../Framework/source/db/DBException.h"
#include "MySqlSchemaProc.h"
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
			auto pShared = new mysqlx::Session( ConnectionString );
			return shared_ptr<mysqlx::Session>( pShared );
		}
		catch( const mysqlx::Error& e )
		{
			THROW2( Exception("Could not create mysql session - {}", e.what()) );
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
				return mysqlx::Value( string(std::get<string_view>(dataValue)) );
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
					stringValue = StringUtilities::Replace( stringValue, "T", " " );
					stringValue = StringUtilities::Replace( stringValue, "Z", "" );
					//double fraction = (double)date.Nanos()/Chrono::TimeSpan::NanosPerSecond;
					//auto fractionString = fmt::format( "{:g}", fraction ).substr( 1 );//:.6
					uint nanos = value.value().time_since_epoch().count();
					constexpr uint NanosPerSecond = std::chrono::nanoseconds(1s).count();
					const double remainingNanos = nanos%NanosPerSecond;
				 	double fraction = remainingNanos/NanosPerSecond;
					var rounded = std::round( fraction*1000000 );
					var fraction2 = rounded/1000000;
					ostringstream os;
					os << std::setprecision(6) << std::fixed << fraction2;
					//os << fraction2;
					//DBG0( os.str() );
					auto fractionString = os.str().substr(1);//fraction<.000001 ? ".0" : fmt::format( "{:g}", fraction ).substr( 1 );//:.6

					string value2 = fmt::format( "{}{}", stringValue, fractionString );//:.6
					if( value2.find('e')!=string::npos )
						ERR( "{} has {} for {} returning {}, nanos={}"sv, value2, fractionString, fraction, value2, value.value().time_since_epoch().count() );//2019-12-13 20:43:04.305e-06 has .305e-06 for 9.30500e-06 returning 2019-12-13 20:43:04.305e-06, nanos=1576269784000009305
					return mysqlx::Value( value2 );
				}
					// ? mysqlx::Value( std::chrono::duration_cast<std::chrono::microseconds>( value.value()-Chrono::Epoch() ).count() )
					// : mysqlx::Value();//FROM_UNIXTIME(1366790400)
			}
		}
		THROW2( LogicException("dataValue index {} not implemented", dataValue.index()) );
		return mysqlx::Value( "compiler remove warning noop" );
	}
	bool MySqlDataSource::TrySelect( string_view sql, std::function<void(const IRow&)> f )noexcept
	{
		return Try( [&]{Select( sql, f);} );
	}
	void MySqlDataSource::Select( string_view sql, std::function<void(const IRow&)> f )
	{
		Select( sql, f, nullptr, false );
	}
	void MySqlDataSource::Select( string_view sql, std::function<void(const IRow&)> f, const vector<DataValue>& values, bool log )noexcept(false)
	{
		Select( sql, f, &values, log );
	}
	void MySqlDataSource::Select( string_view sql, std::function<void(const IRow&)> f, const vector<DataValue>* pValues, bool log )noexcept(false)
	{
		auto pSession = GetSession();
		auto statement = pSession->sql( string(sql) );
		if( pValues )
		{
			for( var& value : *pValues )
				statement.bind( ToMySqlValue( value ) );
		}
		try
		{
			auto result = statement.execute();
			if( log )
				DBLOG( sql, pValues );
			std::list<mysqlx::Row> rows = result.fetchAll();
			for( mysqlx::Row& row : rows )
				f( MySqlRow(row) );
		}
		catch( const ::mysqlx::Error& e )
		{
			THROW2( DBException(e, sql, pValues) );
		}
	}
	uint MySqlDataSource::Execute( string_view sql )
	{
		return Execute2( sql, true );
	}
	optional<uint> MySqlDataSource::TryExecute( string_view sql )noexcept
	{
		optional<uint> result;
		try
		{
			result = Execute2( sql, true );
		}
		catch( const Exception& e )
		{
			e.Log();
		}
		return result;
	}
	optional<uint> MySqlDataSource::TryExecute( string_view sql, const vector<DataValue>& parameters, bool log )noexcept
	{
		optional<uint> result;
		try
		{
			result = Execute( sql, parameters, log );
		}
		catch( const Exception& e )
		{
			e.Log();
		}
		return result;
	}
	uint MySqlDataSource::Execute( string_view sql, const vector<DataValue>& parameters, bool log )noexcept(false)
	{
		return Execute2( sql, log, &parameters, nullptr );
	}
	uint MySqlDataSource::Execute( string_view sql, const vector<DataValue>& parameters, std::function<void(const IRow&)> f, bool log )
	{
		return Execute2( sql, log, &parameters, &f );
	}
	optional<uint> MySqlDataSource::TryExecuteProc( string_view sql, const vector<DataValue>& parameters, bool log )noexcept
	{
		optional<uint> result;
		try
		{
			result = ExecuteProc( sql, parameters, log );
		}
		catch( const Exception& e )
		{
			e.Log();
		}
		return result;
	}
	uint MySqlDataSource::ExecuteProc( string_view sql, const vector<DataValue>& parameters, bool log )
	{
		return Execute2( sql, log, &parameters, nullptr, true );
	}
	uint MySqlDataSource::ExecuteProc( string_view sql, const vector<DataValue>& parameters, function<void(const IRow&)> f, bool log )
	{
		return Execute2( sql, log, &parameters, &f, true );
	}

	uint MySqlDataSource::Scaler( string_view sql, const vector<DataValue>& parameters )noexcept(false)
	{
		uint count = 0;
		function<void(const IRow&)> fnctn = [&count](const IRow& row){ row >> count; };
		Execute2( sql, true, &parameters, &fnctn, false );
		return count;
	}
	optional<uint> MySqlDataSource::ScalerOptional( string_view sql, const vector<DataValue>& parameters )noexcept(false)
	{
		optional<uint> value;
		function<void(const IRow&)> f = [&value](var& row){ value = row.GetUIntOpt(0); };
		Execute2( sql, true, &parameters, &f, false );
		return value;
	}
//https://dev.mysql.com/doc/refman/8.0/en/c-api-prepared-call-statements.html
	uint MySqlDataSource::Execute2( string_view sql, bool log, const vector<DataValue>* pParameters, function<void(const IRow&)>* pFunction, bool isStoredProcedure )noexcept(false)
	{
		auto pSession = GetSession();
		string fullSql = isStoredProcedure ? fmt::format( "call {}", sql ) : string( sql );
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
				list<mysqlx::Row> rows = result.fetchAll();
				for( mysqlx::Row& row : rows )
					(*pFunction)( MySqlRow(row) );
			}
			return 1;
		}
		catch( const ::mysqlx::Error& e )
		{
			THROW2( DBException(e, fullSql, pParameters) );
		}
		return 1;//wari
	}

	sp<ISchemaProc> MySqlDataSource::SchemaProc()noexcept
	{
		sp<IDataSource> p = shared_from_this();
		return make_shared<MySqlSchemaProc>( p );
	}

	string MySqlDataSource::Catalog()noexcept
	{
		string db;
		auto fnctn = [&db]( auto& row ){ row >> db; };
		Select( "select database()", fnctn, {}, false );
		return db;
	}

/*	variant MySqlDataSource::Fetch( string_view sql, variant parameters )noexcept(false)
	{
		mysqlx::Session* pSession = GetSession();
		auto statement = pSession->sql( sql );
		statement.bind( params );
		mysqlx::SqlResult result = statement.execute();
		RowList rows = result.fetchAll();
		for( Row& row : rows )
		{
			tuple<TColumns...> tuple;
			//f(  )
			//mysqlx
		}
	}*/
}