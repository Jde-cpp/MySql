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
		DataValue operator[]( uint position )const override;
		const std::string GetString( uint position )const override;
		bool GetBit( uint position )const override;
		_int GetInt( uint position )const override;
		int32_t GetInt32( uint position )const override{ return static_cast<int32_t>( GetInt(position) ); }
		optional<_int> GetIntOpt( uint position )const override;
		uint GetUInt( uint position )const override;
		optional<uint> GetUIntOpt( uint position )const override;

		double GetDouble( uint position )const override;
		std::optional<double> GetDoubleOpt( uint position )const override;
		DBDateTime GetDateTime( uint position )const noexcept(false) override;
		//DBDateTime GetDateTime2( uint position )const override;
		std::optional<DBDateTime> GetDateTimeOpt( uint position )const override;

	private:
		const mysqlx::Row& _row;
	};
}