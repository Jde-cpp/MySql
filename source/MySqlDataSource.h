#pragma once
#include "Exports.h"
#include "../../Framework/source/db/DataType.h"
#include "../../Framework/source/db/DataSource.h"

extern "C" JDE_MYSQL_VISIBILITY Jde::DB::IDataSource* GetDataSource();

namespace Jde::DB::MySql
{
	struct MySqlDataSource final: public IDataSource
	{
		α GetSession()noexcept(false)->shared_ptr<mysqlx::Session>;
		[[noreturn]] α SetAsynchronous()noexcept(false)->void override{ throw Exception{"Not Implemented"}; }

		α Execute( sv sql )noexcept(false)->uint override;
		α Execute( sv sql, const vector<DataValue>& parameters, bool log )noexcept(false)->uint override;
		α Execute( sv sql, const vector<DataValue>* pParameters, std::function<void(const IRow&)>* f, bool isStoredProc=false, bool log=true, SRCE )noexcept(false)->uint override;
		α ExecuteProc( sv sql, const vector<DataValue>& parameters, bool log )noexcept(false)->uint override;
		α ExecuteProc( sv sql, const vector<DataValue>& parameters, std::function<void(const IRow&)> f, bool log )noexcept(false)->uint override;
		α SelectCo( string&& sql, std::function<void(const IRow&)> f, const std::vector<DataValue>&& parameters, bool log )noexcept->up<IAwaitable> override;

		α SchemaProc()noexcept->sp<ISchemaProc> override;
	private:
		α Select( sv sql, std::function<void(const IRow&)> f, const vector<DataValue>* pValues, bool log, SRCE )noexcept(false)->uint override;
	};

	template<typename ...TColumns, std::size_t... I>
	std::tuple<TColumns...> a2t_impl( const mysqlx::Row& row, std::index_sequence<I...> )
	{
		return std::make_tuple<TColumns...>(string("0"), string("1"), 2, string("3"), 4, string("5"), 6, 7,8,9,10 );
	}

	template< typename ...TColumns, typename Indices = std::make_index_sequence<sizeof...(TColumns)>>
	std::tuple<TColumns...> a2t( const mysqlx::Row& row )
	{
		return a2t_impl<TColumns...>( row, Indices{} );
	}
}