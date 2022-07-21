#pragma once
#include "Exports.h"
#include "../../Framework/source/db/DataType.h"
#include "../../Framework/source/db/DataSource.h"

extern "C" JDE_MYSQL_VISIBILITY Jde::DB::IDataSource* GetDataSource();

namespace Jde::DB::MySql
{
	struct MySqlDataSource final : IDataSource
	{
		α Execute( string sql, SL sl )ε->uint override;
		α Execute( string sql, const vector<object>& parameters, SL sl )ε->uint override;
		α Execute( string sql, const vector<object>* pParameters, RowΛ* f, bool isStoredProc=false, SRCE )ε->uint override;
		α ExecuteProc( string sql, const vector<object>& parameters, SL sl )ε->uint override;
		α ExecuteProc( string sql, const vector<object>& parameters, RowΛ f, SL sl )ε->uint override;
		α ExecuteProcCo( string sql, vector<object> p, SL sl )ι->up<IAwait> override;
		α ExecuteCo( string sql, vector<object> p, SRCE )ι->up<IAwait> override{ return ExecuteProcCo(move(sql), move(p), sl); }
		α SelectCo( ISelect* pAwait, string sql, vector<object>&& params, SL sl )ι->up<IAwait> override;
		α SchemaProc()ι->sp<ISchemaProc> override;

		α ExecuteNoLog( string sql, const vector<object>* pParameters, RowΛ* f, bool isStoredProc, SL sl )ε->uint override;
		α ExecuteProcNoLog( string sql, vec<object> parameters, SL sl )ε->uint override;
		α SelectNoLog( string sql, RowΛ f, const vector<object>* pValues, SL sl )ε->uint override;

	private:
		α Select( string sql, RowΛ f, const vector<object>* pValues, SRCE )ε->uint override;
	};
}