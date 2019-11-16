#pragma once
//#include "../../../framework/TypeDefs.h"
//#include "../../types/Schema.h"
#include "Exports.h"
#include "../../Framework/source/db/DataType.h"
#include "../../Framework/source/db/DataSource.h"

extern "C" JDE_MYSQL_VISIBILITY Jde::DB::IDataSource* GetDataSource(); 

namespace Jde::DB::MySql
{
	struct MySqlDataSource : public IDataSource
	{
		shared_ptr<mysqlx::Session> GetSession()noexcept(false);

		//std::variant Fetch( string_view sql, std::variant parameters )noexcept(false) override;
		//template<typename ...TColumns, typename ...TParameters>
		//void Select( string_view sql, std::function<void(TColumns...)> f, TParameters... );
		uint Execute( string_view sql )override;
		uint Execute( string_view sql, const std::vector<DataValue>& parameters, bool log )override;
		uint Execute( string_view sql, const std::vector<DataValue>& parameters, std::function<void(const IRow&)> f, bool log ) override;
		uint ExecuteProc( string_view sql, const std::vector<DataValue>& parameters, bool log )override;
		uint ExecuteProc( string_view sql, const std::vector<DataValue>& parameters, std::function<void(const IRow&)> f, bool log )override;
		void Select( string_view sql, std::function<void(const IRow&)> f, const std::vector<DataValue>& values, bool log )noexcept(false) override;
		void Select( string_view sql, std::function<void(const IRow&)> f )override;
	private:
		void Select( string_view sql, std::function<void(const IRow&)> f, const std::vector<DataValue>* pValues, bool log )noexcept(false);
		uint Execute2( string_view sql, bool log, const std::vector<DataValue>* pParameters=nullptr, std::function<void(const IRow&)>* pFunction=nullptr, bool isStoredProcedure=false );
	};

	/*
	template <std::size_t... Is, typename F, typename T>
	auto map_filter_tuple( F f, mysqlx::Row& row )
	{
		return std::make_tuple( f( std::get<Is>(t) )... );
	}

	
	template <typename T> 
	T read(std::istream& is)
	{
		T t; 
		is >> t; 
		return t;
	}

	template<class T, class... Args, std::size_t... Is> 
	T read2( std::istream& is, indices<Is...> )
	{
		T t; 
		is >> t; 
		return t;
	}

	template <typename... Args>
	std::tuple<Args...> parse( mysqlx::Row& row )
	{
		return std::make_tuple( read<Args>(is)... );
	}
	
	
	template<typename... TColumns, std::size_t... Is>
	std::tuple<TColumns...> extract( const mysqlx::Row& row, std::index_sequence<Is...> )
	{
		 return std::make_tuple( row, std::bind_argument<TColumns>::get_arg(obj, Is)... );
	}
*/
	template<class T>
	T extractx( const mysqlx::Value& value )
	{
		throw "not implemented";
	}
	template<>
	inline std::string extractx<string>( const mysqlx::Value& value )
	{
		//return value.get_string();
		return string("");
	}

	template<>
	inline int extractx<int>( const mysqlx::Value& value )
	{
		return 5;//value.get_sint();
	}

	template<typename ...TColumns, std::size_t... I>
	std::tuple<TColumns...> a2t_impl( const mysqlx::Row& row, std::index_sequence<I...> )
	{
		//return std::make_tuple( extractx<typename std::tuple_element<I, std::tuple<TColumns...> >::type>(row[I])... );
		//return std::make_tuple( 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 );
		return std::make_tuple<TColumns...>(string("0"), string("1"), 2, string("3"), 4, string("5"), 6, 7,8,9,10 );
	}

	template< typename ...TColumns, typename Indices = std::make_index_sequence<sizeof...(TColumns)>>
	std::tuple<TColumns...> a2t( const mysqlx::Row& row )
	{
		return a2t_impl<TColumns...>( row, Indices{} );
	}

/*	template<typename ...TColumns, typename ...TParameters>
	void MySqlDataSource::Select( string_view sql, std::function<void(TColumns...)> f, TParameters... params )
	{
		//constexpr auto ColumnCount = (sizeof(TColumns) + ...);
		auto pSession = GetSession();
		auto statement = pSession->sql( string(sql) );
		constexpr uint paramCount = sizeof...( params );
		GetDefaultLogger()->debug( "{}", paramCount );
		statement.bind( params... );
		try
		{
			auto result = statement.execute();//mysqlx::SqlResult
			std::list<mysqlx::Row> rows = result.fetchAll();
			for( mysqlx::Row& row : rows )
			{
				auto tuple = a2t<TColumns...>( row );
				// vector<DataValue> rowResults2 = { string("name"), string("cname"), 5, string(""), 0, string("none"), 5, 1,2,3,4 };
				// array<DataValue,ColumnCount> rowResult = { string("name"), string("cname"), 5, string(""), 0, string("none"), 5, 1,2,3,4 };
				// auto foo = std::make_tuple<TColumns...>(string("name"), string("cname"), 5, string(""), 0, string("none"), 5, 1,2,3,4 );
				std::apply( [f](auto &&... args) { f(args...); }, tuple );
			}
		}
		catch( ::mysqlx::Error e )
		{ 
			throw;
		}
	}*/
}