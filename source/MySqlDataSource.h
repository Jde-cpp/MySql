#pragma once
#include "Exports.h"
#include "../../Framework/source/db/DataType.h"
#include "../../Framework/source/db/DataSource.h"

extern "C" JDE_MYSQL_VISIBILITY Jde::DB::IDataSource* GetDataSource();

namespace Jde::DB::MySql
{
	struct MySqlDataSource final: public IDataSource
	{
		α GetSession()noexcept(false)->sp<mysqlx::Session>;
		//[[noreturn]] α SetAsynchronous()noexcept(false)->void override{ throw Exception{"Not Implemented"}; }

		α Execute( sv sql, SRCE )noexcept(false)->uint override;
		α Execute( sv sql, const vector<DataValue>& parameters, bool log, SRCE )noexcept(false)->uint override;
		α Execute( sv sql, const vector<DataValue>* pParameters, std::function<void(const IRow&)>* f, bool isStoredProc=false, bool log=true, SRCE )noexcept(false)->uint override;
		α ExecuteProc( sv sql, const vector<DataValue>& parameters, bool log, SRCE )noexcept(false)->uint override;
		α ExecuteProc( sv sql, const vector<DataValue>& parameters, std::function<void(const IRow&)> f, bool log, SRCE )noexcept(false)->uint override;
		α SelectCo( string&& sql, std::function<void(const IRow&)> f, const std::vector<DataValue>&& parameters, bool log, SRCE )noexcept->up<IAwaitable> override;

		α SchemaProc()noexcept->sp<ISchemaProc> override;
	private:
		α Select( sv sql, std::function<void(const IRow&)> f, const vector<DataValue>* pValues, bool log, SRCE )noexcept(false)->uint override;
	};
}