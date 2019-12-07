#include "MySqlRow.h"
#include "../../Framework/source/db/DataType.h"
#include "../../Framework/source/db/DBException.h"
#define var const auto

namespace Jde::DB::MySql
{
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
	DataValue ToDataValue( const mysqlx::Value& value )
	{
		switch( value.getType() )
		{
			case mysqlx::common::Value::Type::VNULL:
				return DataValue{nullptr};
			case mysqlx::common::Value::Type::STRING:
				return DataValue{ value.get<string>() };
			case mysqlx::common::Value::Type::BOOL:
				return DataValue{ value.get<bool>() };  
			break;
			case mysqlx::common::Value::Type::INT64:
				return DataValue{ static_cast<_int>(value.get<_int>()) };
			break;
			case mysqlx::common::Value::Type::UINT64:
				return DataValue{ value.get<uint>() };  
			break;
			case mysqlx::common::Value::Type::DOUBLE:
				return DataValue{ optional<double>(value.get<double>()) };  
			break;
			default:
				THROW2( LogicException("{} dataValue not implemented", value.getType()) );
		}
		return DataValue{ nullptr };  
	}

	DataValue MySqlRow::operator[]( uint position )const
	{
		return ToDataValue( _row[static_cast<mysqlx::col_count_t>(position)] );
	}

	_int MySqlRow::GetInt( uint position )const
	{ 
		_int intValue;
		var& value = _row[static_cast<mysqlx::col_count_t>(position)];
		try{	intValue = value.get<_int>(); }
		catch( const ::mysqlx::Error& ) { THROW2( DBException("Could not convert position {} - {} to an int.", position, GetTypeName(value)) ); }
		return intValue;
	}
	optional<_int> MySqlRow::GetIntOpt( uint position )const
	{
		optional<_int> intValue;
		var& value = _row[static_cast<mysqlx::col_count_t>(position)];
		if( !value.isNull() )
		{
			try{	intValue = value.get<_int>(); }
			catch( const ::mysqlx::Error& ){ THROW2( DBException("Could not convert position {} - {} to an int.", position, GetTypeName(value)) ); }
		}
		return intValue;
	}

	uint MySqlRow::GetUInt( uint position )const
	{ 
		var& value = _row[static_cast<mysqlx::col_count_t>(position)];
		try
		{
			return value.get<uint>(); 
		}
		catch( const ::mysqlx::Error& )
		{ 
			THROW2( DBException("Could not convert position {} - {} to an uint.", position, GetTypeName(value)) );
		}
		return 0;//no-op
	}

	bool MySqlRow::GetBit( uint position )const
	{
		return GetInt( position )!=0;
	}

	optional<uint> MySqlRow::GetUIntOpt( uint position )const
	{
		return _row[static_cast<mysqlx::col_count_t>(position)].getType()==mysqlx::Value::Type::VNULL ? optional<uint>() : GetUInt( position ); 
	}

	const std::string MySqlRow::GetString( uint position )const
	{ 
		var& value = _row[static_cast<mysqlx::col_count_t>(position)];
		try
		{
			return value.getType()==mysqlx::Value::Type::VNULL ? string() : value.get<string>(); 
		}
		catch( const ::mysqlx::Error& )
		{ 
			THROW2( DBException("Could not convert position {} - {} to an string.", position, GetTypeName(value)) );
		}
		return std::string(); 
	}

	double MySqlRow::GetDouble( uint position )const
	{
		double doubleValue;
		var& value = _row[static_cast<mysqlx::col_count_t>(position)];
		try
		{
			doubleValue = value.get<double>();
		}
		catch( const ::mysqlx::Error& )
		{ 
			THROW2( DBException("Could not convert position {} - {} to an double.", position, GetTypeName(value)) );
		}
		return doubleValue; 
	}
	std::optional<double> MySqlRow::GetDoubleOpt( uint position )const
	{
		std::optional<double> doubleValue;
		var& value = _row[static_cast<mysqlx::col_count_t>(position)];
		return value.isNull() ? std::optional<double>() : std::optional<double>( GetDouble(position) );
	}
/*	DBDateTime GetDateTime2( uint position )const
	{

	}*/
	DBDateTime MySqlRow::GetDateTime( uint position )const noexcept(false)
	{
		DBDateTime dateTimeValue;
		var& value = _row[static_cast<mysqlx::col_count_t>(position)];
		try
		{
			if( value.getType()==::mysqlx::Value::DOUBLE )
			{
				var doubleValue = GetDouble( position );
				dateTimeValue = DBClock::from_time_t( (int)doubleValue );
				dateTimeValue+=std::chrono::microseconds( Math::URound((doubleValue-(uint)doubleValue)*1'000'000) );
			}
			else
				dateTimeValue = DBClock::from_time_t( GetInt(position) );
			
			//	THROW2( DBException("Expected DBType for datetime to be uint, got {}.", value.GetTypeName()) );
/*			if( value.getType()!=::mysqlx::Value::Type::STRING )
				THROW2( DBException("Expected DBType for datetime to be 6, got {}.", value.getType()) );

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
			var m2 = std::chrono::duration_cast<std::chrono::milliseconds>( time.GetTimePoint() - mysqlEpoch.GetTimePoint() ).count();
			
			auto byteSize = value.getRawBytes();
			if( byteSize.second!=6 )
				THROW2( DBException("Expected 6 bytes for datetime, got {}.", position, byteSize.second) );

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
			
			
			
			//dateTimeValue = mysqlEpoch.GetTimePoint() + std::chrono::milliseconds( milliseconds );
			Jde::DateTime myx( dateTimeValue );
			GetDefaultLogger()->debug( myx.ToIsoString() ); //2018-02-12 08:49:33
			*/
		}
		catch( const ::mysqlx::Error& )
		{ 
			THROW2( DBException("Could not convert position {} - {} to an date time.", position, GetTypeName(value)) );
		}
		return dateTimeValue;
	}
	
	std::optional<DBDateTime> MySqlRow::GetDateTimeOpt( uint position )const
	{
		std::optional<DBDateTime> dateTimeValue;
		var& value = GetIntOpt( position );
		if( value.has_value() )
			dateTimeValue = DBClock::from_time_t( value.value() );
		return dateTimeValue; 
	}	
}