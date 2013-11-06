#ifndef DATABASE_H
#define DATABASE_H

#if !defined(EXPAND_MY_SSQLS_STATICS)
#   define MYSQLPP_SSQLS_NO_STATICS
#endif

#include <mysql++/mysql++.h>
#include <mysql++/ssqls.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
sql_create_3(tasks, 1, 3, 
	     mysqlpp::sql_int, id,
	     mysqlpp::sql_int, user_id,
	     mysqlpp::sql_varchar_null, status)
#pragma GCC diagnostic pop

namespace StalkR
{
    typedef tasks Task;
}

#endif //DATABASE_H
