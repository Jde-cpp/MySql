#pragma once
#include "Exports.h"
#include "../../Framework/source/db/DataType.h"
#include "../../Framework/source/db/DataSource.h"

extern "C" JDE_MYSQL_VISIBILITY Jde::DB::IDataSource* GetDataSource();

namespace Jde::DB::MySql
{
	struct MySqlDataSource final : IDataSource
	{
		//[[noreturn]] α SetAsynchronous()noexcept(false)->void override{ throw Exception{"Not Implemented"}; }

		α Execute( string sql, SL sl )noexcept(false)->uint override;
		α Execute( string sql, const vector<object>& parameters, SL sl )noexcept(false)->uint override;
		α Execute( string sql, const vector<object>* pParameters, RowΛ* f, bool isStoredProc=false, SRCE )noexcept(false)->uint override;
		α ExecuteProc( string sql, const vector<object>& parameters, SL sl )noexcept(false)->uint override;
		α ExecuteProc( string sql, const vector<object>& parameters, RowΛ f, SL sl )noexcept(false)->uint override;
		α ExecuteProcCo( string&& sql, const vector<object>&& parameters, SL sl )noexcept->up<IAwaitable> override;
		α SelectCo( ISelect* pAwait, string sql, vector<object>&& params, SL sl )noexcept->up<IAwaitable> override;
		α SchemaProc()noexcept->sp<ISchemaProc> override;

		α ExecuteNoLog( string sql, const vector<object>* pParameters, RowΛ* f, bool isStoredProc, SL sl )noexcept(false)->uint override;
		α ExecuteProcNoLog( string sql, vec<object> parameters, SL sl )noexcept(false)->uint override;
		α SelectNoLog( string sql, RowΛ f, const vector<object>* pValues, SL sl )noexcept(false)->uint override;

	private:
		α Select( string sql, RowΛ f, const vector<object>* pValues, SRCE )noexcept(false)->uint override;
		//α Session()noexcept(false)->mysqlx::Session;

	};
}