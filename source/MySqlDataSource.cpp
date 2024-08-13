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

namespace Jde::DB::MySql{
	static sp<LogTag> _logTag = Logging::Tag( "dbDriver" );

	using mysqlx::SessionOption;
	α Session( str cs, SRCE )ε->mysqlx::Session{
		try{
			return mysqlx::Session{ cs };
		}
		catch( std::exception& e ){
			Exception e2{ SRCE_CUR, move(e), "Could not create mysql session" };
			e2.Push( sl );
			e2.SetLevel( ELogLevel::Critical );
			e2.Throw();
		}
	}

	α ToMySqlValue( const object& dataValue )ε->mysqlx::Value{
		switch( (EObject)dataValue.index() ){
			case EObject::Null:
				return mysqlx::Value();
			case EObject::String:
				return mysqlx::Value( std::get<string>(dataValue) );
			case EObject::StringView:
				return mysqlx::Value( string(std::get<sv>(dataValue)) );
			case EObject::StringPtr:
				return mysqlx::Value( string(*std::get<sp<string>>(dataValue)) );
			case EObject::Bool:
				return mysqlx::Value( std::get<bool>(dataValue) );
			case EObject::Int8:
				return mysqlx::Value( std::get<int8_t>(dataValue) );
			case EObject::Int32:
				return mysqlx::Value( std::get<int>(dataValue) );
			case EObject::Int64:
				return mysqlx::Value( std::get<_int>(dataValue) );
			case EObject::UInt32:
				return mysqlx::Value( std::get<uint32_t>(dataValue) );
			case EObject::UInt64:
				return mysqlx::Value( std::get<uint>(dataValue) );
			case EObject::Double:
				return mysqlx::Value( std::get<double>(dataValue) );
			case EObject::Time:{
				var value = std::get<DBTimePoint>( dataValue );
				const Jde::DateTime date{ value };
				auto stringValue = date.ToIsoString();
				stringValue = Str::Replace( stringValue, "T", " " );
				stringValue = Str::Replace( stringValue, "Z", "" );
				uint nanos = value.time_since_epoch().count();
				constexpr uint NanosPerSecond = std::chrono::nanoseconds(1s).count();
				const double remainingNanos = nanos%NanosPerSecond;
				double fraction = remainingNanos/NanosPerSecond;
				var rounded = std::round( fraction*1000000 );
				var fraction2 = rounded/1000000;
				ostringstream os;
				os << std::setprecision(6) << std::fixed << fraction2;
				auto fractionString = os.str().substr(1);

				string value2 = Jde::format( "{}{}", stringValue, fractionString );//:.6
				if( value2.find('e')!=string::npos )
					ERR( "{} has {} for {} returning {}, nanos={}"sv, value2, fractionString, fraction, value2, value.time_since_epoch().count() );//2019-12-13 20:43:04.305e-06 has .305e-06 for 9.30500e-06 returning 2019-12-13 20:43:04.305e-06, nanos=1576269784000009305
				return mysqlx::Value( value2 );
			}
		}
		throw Exception{ SRCE_CUR, ELogLevel::Debug, "{} dataValue not implemented", dataValue.index() };
		return mysqlx::Value( "compiler remove warning noop" );
	}

//https://dev.mysql.com/doc/refman/8.0/en/c-api-prepared-call-statements.html
	α Execute( str cs, string&& sql, const vector<object>* pParameters, const RowΛ* pFunction, bool proc, SL sl, bool log=true )ε->uint{
		var fullSql = proc ? Jde::format( "call {}", move(sql) ) : move( sql );
		if( log )
			DBLOG( fullSql, pParameters );
		mysqlx::Session session = Session( cs );
		auto statement = session.sql( fullSql );
		if( pParameters ){
			for( var& parameter : *pParameters )
				statement.bind( ToMySqlValue(parameter) );
		}
		mysqlx::SqlResult result;
		try{
			result = statement.execute();
			if( pFunction && result.hasData() ){
				auto rows = result.fetchAll();
				for( const mysqlx::Row& row : rows )
					(*pFunction)( MySqlRow(row) );
			}
		}
		catch( ::mysqlx::Error& e ){
			throw DBException{ fullSql, pParameters, e.what(), sl };
		}
		try{
			return proc || result.hasData() ? 0 : result.getAffectedItemsCount();
		}
		catch( ::mysqlx::Error& e ){//Only available after end of query execute
			return 0;
		}
	}
	α Select( str cs, str sql, RowΛ f, const vector<object>* pValues, SL sl, bool log=true )ε->uint{
		mysqlx::Session session = Session( cs );
		auto statement = session.sql( sql );
		if( pValues ){
			for( var& value : *pValues )
				statement.bind( ToMySqlValue(value) );
		}
		try{
			if( log )
				DBLOG( sql, pValues );
			auto result = statement.execute();
			std::list<mysqlx::Row> rows = result.fetchAll();//
			for( mysqlx::Row& row : rows )
				f( MySqlRow(row) );
			return rows.size();
		}
		catch( ::mysqlx::Error& e ){
			throw DBException{ sql, pValues, e.what(), sl };
		}
	}

	α MySqlDataSource::Execute( string sql, SL sl )ε->uint{
		return Execute( move(sql), nullptr, nullptr, false, sl );
	}
	α MySqlDataSource::Execute( string sql, const vector<object>& parameters, SL sl )ε->uint{
		return Execute( move(sql), &parameters, nullptr, false, sl );
	}
	α MySqlDataSource::Execute( string sql, const vector<object>* pParams, const RowΛ* f, bool proc, SL sl )ε->uint{
		return MySql::Execute( CS(), move(sql), pParams, f, proc, sl );
	}

	α MySqlDataSource::ExecuteProc( string sql, const vector<object>& parameters, SL sl )ε->uint{
		return MySql::Execute( CS(), move(sql), &parameters, nullptr, true, sl );
	}
	α MySqlDataSource::ExecuteProc( string sql, const vector<object>& parameters, RowΛ f, SL sl )ε->uint{
		return MySql::Execute( CS(), move(sql), &parameters, &f, true, sl );
	}

	α MySqlDataSource::SchemaProc()ι->sp<ISchemaProc>{
		sp<IDataSource> p = shared_from_this();
		return ms<MySqlSchemaProc>( p );
	}
	α MySqlDataSource::Select( string sql, RowΛ f, const vector<object>* pValues, SL sl )ε->uint{
		return MySql::Select( CS(), move(sql), f, pValues, sl );
	}

	α MySqlDataSource::SelectCo( ISelect* pAwait, string sql_, vector<object>&& params_, SL sl )ι->up<IAwait>{
		return mu<PoolAwait>( [pAwait, sql{move(sql_)}, params=move(params_), sl, this]()ε{
			auto rowΛ = [pAwait]( const IRow& r )ε{ pAwait->OnRow(r); };
			Select( sql, rowΛ, &params, sl );
		});
	}
	α MySqlDataSource::ExecuteCo( string sql_, const vector<object> p, bool proc_, SL sl )ι->up<IAwait>{
		return mu<TPoolAwait<uint>>( [sql=move(sql_), params=move(p), sl, proc=proc_, this]()ε{
			return mu<uint>( Execute(move(sql), &params, nullptr, proc, sl) );
		});
	}
	α MySqlDataSource::ExecuteCo( string sql_, vector<object> p, RowΛ f, bool proc_, SL sl )ε->up<IAwait>{
		return mu<TPoolAwait<uint>>( [sql=move(sql_), params=move(p), sl, proc=proc_, func=f, this]()ε{
			return mu<uint>( Execute(move(sql), &params, &func, proc, sl) );
		}, "ExecuteCo", sl );
	}
	α MySqlDataSource::ExecuteNoLog( string sql, const std::vector<object>* pParameters, RowΛ* f, bool proc, SL sl )ε->uint{
		return MySql::Execute( CS(), move(sql), pParameters, f, proc, sl, false );
	}
	α MySqlDataSource::ExecuteProcNoLog( string sql, const std::vector<object>& parameters, SL sl )ε->uint{
		return MySql::Execute( CS(), move(sql), &parameters, nullptr, true, sl, false );
	}
	α MySqlDataSource::SelectNoLog( string sql, RowΛ f, const std::vector<object>* pValues, SL sl )ε->uint{
		return MySql::Select( CS(), move(sql), f, pValues, sl, false );
	}
}