#pragma once
//#include "../../../framework/TypeDefs.h"
//#include "../../types/Schema.h"
//#include "../../DataType.h"
#include "../../Framework/source/db/DataSource.h"
#include "../../Framework/source/db/Row.h"


namespace Jde::DB::MySql
{
	struct MySqlRow : public IRow
	{
		MySqlRow( const mysqlx::Row& row );
		α operator[]( uint position )const noexcept(false)->object override;
		α GetCIString( uint position, SRCE )const noexcept(false)->CIString override;
		α GetString( uint position, SRCE )const noexcept(false)->std::string override;
		α GetBit( uint position, SRCE )const noexcept(false)->bool override;
		α GetInt( uint position, SRCE )const noexcept(false)->_int override;
		α GetInt32( uint position, SRCE )const noexcept(false)->int32_t override{ return static_cast<int32_t>( GetInt(position) ); }
		α GetIntOpt( uint position, SRCE )const noexcept(false)->optional<_int> override;
		α GetUInt( uint position, SRCE )const noexcept(false)->uint override;
		α GetUIntOpt( uint position, SRCE )const noexcept(false)->optional<uint> override;

		α GetDouble( uint position, SRCE )const noexcept(false)->double override;
		α GetDoubleOpt( uint position, SRCE )const noexcept(false)->std::optional<double> override;
		α GetTimePoint( uint position, SRCE )const noexcept(false)->DBTimePoint override;
		α GetTimePointOpt( uint position, SRCE )const noexcept(false)->std::optional<DBTimePoint> override;

	private:
		const mysqlx::Row& _row;
	};
}