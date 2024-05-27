#include "MySqlRow.h"
#include "../../Framework/source/db/DataType.h"
#include "../../Framework/source/db/DBException.h"
#include "../../Framework/source/math/MathUtilities.h"
#define var const auto

namespace Jde::DB::MySql
{
	using namespace std::chrono;

	string GetTypeName( mysqlx::Value mySqlValue )
	{
		string value;
		switch( mySqlValue.getType() )
		{
		case mysqlx::Value::Type::VNULL:
			value = "null";
			break;
		case mysqlx::Value::Type::UINT64:
			value = "uint";
			break;
		case mysqlx::Value::Type::INT64:
			value = "int";
			break;
		case mysqlx::Value::Type::FLOAT:
			value = "float";
			break;
		case mysqlx::Value::Type::DOUBLE:
			value = "double";
			break;
		case mysqlx::Value::Type::BOOL:
			value = "bool";
			break;
		case mysqlx::Value::Type::STRING:
			value = "string";
			break;
		case mysqlx::Value::Type::DOCUMENT:
			value = "document";
			break;
		case mysqlx::Value::Type::RAW:
			value = "raw";
			break;
		case mysqlx::Value::Type::ARRAY:
			value = "array";
			break;
		default:
			value = "unknown";
			break;
		};
		return value;
	}
	MySqlRow::MySqlRow( const mysqlx::Row& row ):
		_row{ row }
	{}
	object ToObject( const mysqlx::Value& value, SL sl )noexcept(false)
	{
		var type = value.getType();
		object v{ nullptr };
		if( type==mysqlx::Value::Type::VNULL )
			v = object{nullptr};
		else if( type==mysqlx::Value::Type::STRING )
			v = object{ value.get<string>() };
		else if( type==mysqlx::Value::Type::BOOL )
			v = object{ value.get<bool>() };
		else if( type==mysqlx::Value::Type::INT64 )
			v = object{ static_cast<_int>(value.get<_int>()) };
		else if( type==mysqlx::Value::Type::UINT64 )
			v = object{ value.get<uint>() };
		else if( type==mysqlx::Value::Type::DOUBLE )
			v = object{ value.get<double>() };
		else
			throw Exception{ sl, ELogLevel::Debug, "{} object not implemented", value.getType() };
		return v;
	}

	object MySqlRow::operator[]( uint position )const
	{
		return ToObject( _row[static_cast<mysqlx::col_count_t>(position)], SRCE_CUR );
	}
#define CATCH(type) catch( ::mysqlx::Error& e ){ throw Exception{ sl, move(e), "Could not convert position {} - {} to an ##type.", position, GetTypeName(value) }; }
	_int MySqlRow::GetInt( uint position, SL sl )Ε{
		_int intValue;
		var& value = _row[static_cast<mysqlx::col_count_t>(position)];
		try{	intValue = value.get<_int>(); } CATCH( int )
		return intValue;
	}
	optional<_int> MySqlRow::GetIntOpt( uint position, SL sl )Ε{
		optional<_int> intValue;
		var& value = _row[static_cast<mysqlx::col_count_t>(position)];
		if( !value.isNull() ){
			try{	intValue = value.get<_int>(); } CATCH( optional<int> )
		}
		return intValue;
	}

	uint MySqlRow::GetUInt( uint position, SL sl )Ε{
		var& value = _row[static_cast<mysqlx::col_count_t>(position)];
		try{ return value.get<uint>(); } CATCH( uint )
		return 0;//no-op
	}

	bool MySqlRow::GetBit( uint position, SL sl )Ε{return GetInt( position )!=0;}

	optional<uint> MySqlRow::GetUIntOpt( uint position, SL sl )const
	{
		return _row[static_cast<mysqlx::col_count_t>(position)].getType()==mysqlx::Value::Type::VNULL ? optional<uint>() : GetUInt( position );
	}

	string MySqlRow::GetString( uint position, SL sl )const
	{
		var& value = _row[static_cast<mysqlx::col_count_t>(position)];
		try{ return value.getType()==mysqlx::Value::Type::VNULL ? string() : value.get<string>(); } CATCH( string )
		return string();
	}
	CIString MySqlRow::GetCIString( uint position, SL sl )const
	{
		var& value = _row[static_cast<mysqlx::col_count_t>(position)];
		try
		{
			return value.getType()==mysqlx::Value::Type::VNULL ? CIString{} : CIString{ value.get<string>() };
		}
		CATCH( CIString );
		//return string();
	}
	double MySqlRow::GetDouble( uint position, SL sl )const
	{
		double doubleValue;
		var& value = _row[static_cast<mysqlx::col_count_t>(position)];
		try
		{
			doubleValue = value.get<double>();
		}
		CATCH( double );
		return doubleValue;
	}
	optional<double> MySqlRow::GetDoubleOpt( uint position, SL sl )const
	{
		var& value = _row[static_cast<mysqlx::col_count_t>(position)];
		return value.isNull() ? optional<double>() : optional<double>( GetDouble(position) );
	}

	DBTimePoint MySqlRow::GetTimePoint( uint position, SL sl )Ε{
		DBTimePoint y;
		var& value = _row[static_cast<mysqlx::col_count_t>(position)];
		try
		{
			if( value.getType()==::mysqlx::Value::DOUBLE )
			{
				var doubleValue = GetDouble( position );
				y = DBClock::from_time_t( (int)doubleValue );
				y+=microseconds( Round((doubleValue-(uint)doubleValue)*1'000'000) );
			}
			else
				y = DBClock::from_time_t( GetInt(position) );

			//	THROWX( DBException("Expected DBType for datetime to be uint, got {}.", value.GetTypeName()) );
/*			if( value.getType()!=::mysqlx::Value::Type::STRING )
				THROWX( DBException("Expected DBType for datetime to be 6, got {}.", value.getType()) );

			var stringValue = value.get<string>();
			var year = stoi( stringValue.substr(0, 4) );
			var month = stoi( stringValue.substr(5, 2) );
			var day = stoi( stringValue.substr(8, 2) );
			var hour = stoi( stringValue.substr(11, 2) );
			var minute = stoi( stringValue.substr(14, 2) );
			var second = stoi( stringValue.substr(17, 2) );
			dateTimeValue = Jde::DateTime( year, month, day, hour, minute, second ).GetTimePoint();
			*/
/*			const Jde::DateTime mysqlEpoch( 1980, 1, 1 );
			const Jde::DateTime time( 2018, 2, 12, 8, 49, 33 );
			var m2 = chrono::duration_cast<chrono::milliseconds>( time.GetTimePoint() - mysqlEpoch.GetTimePoint() ).count();

			auto byteSize = value.getRawBytes();
			if( byteSize.second!=6 )
				THROWX( DBException("Expected 6 bytes for datetime, got {}.", position, byteSize.second) );

			var bytes = byteSize.first;
			_int bytes2 = ((_int)bytes[0] << 40) | ((_int)bytes[1] << 32) | ((_int)bytes[2] << 24) | (bytes[3] << 16) | (bytes[4] << 8) | bytes[5];
			_int bytes3 = ((_int)bytes[5] << 40) | ((_int)bytes[4] << 32) | ((_int)bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];
			auto expectedYear = 2018*13+2;//0 0110 0110 0111 1100
			auto expectedMonth = 2;
			auto expectedDay = 12;
			auto expectedHour = 8;
			auto expectedMinute = 49;
			auto expectedSecond = 33;

			//17 bits = year*13+month
			uint32_t partA = bytes[0] << 16 | bytes[1] << 8 | bytes[2];//1-24
			partA <<= 9; //clear signed bit, always 1.
			partA >>= 16; //24+1-8=17
			var year = partA/13;
			var month = partA%13;
			uint8_t day = bytes[2];
			day <<= 2; day >>= 3; //clear 17,18, 24
			uint16_t hour = bytes[2] << 8 | bytes[3]; //17-32
			hour <<= 7; hour >>= 12; //clear 17-24, 28-32,
			uint16_t minute = bytes[3] << 8 | bytes[4]; //24-40
			minute <<= 4; minute >>= 10; //clear 17-23, 28-32,
			uint16_t second = bytes[4]; //40-48
			second <<= 2; second >>= 2; //clear 40,41
			uint8_t millisecond = bytes[5];
			const Jde::DateTime time2( year, month, day, hour, minute, second );



			//dateTimeValue = mysqlEpoch.GetTimePoint() + chrono::milliseconds( milliseconds );
			Jde::DateTime myx( dateTimeValue );
			GetDefaultLogger()->debug( myx.ToIsoString() ); //2018-02-12 08:49:33
			*/
		}
		CATCH( TimePoint );

		return y;
	}

	optional<DBTimePoint> MySqlRow::GetTimePointOpt( uint position, SL sl )const
	{
		optional<DBTimePoint> y;
		var& value = GetIntOpt( position );
		if( value.has_value() )
			y = DBClock::from_time_t( value.value() );
		return y;
	}
}