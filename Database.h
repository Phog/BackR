#ifndef DATABASE_H
#define DATABASE_H

#if !defined(EXPAND_MY_SSQLS_STATICS)
#   define MYSQLPP_SSQLS_NO_STATICS
#endif

#include <mysql++/mysql++.h>
#include <mysql++/ssqls.h>

namespace Database
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
sql_create_3(tasks, 1, 3, 
	     mysqlpp::sql_int, id,
	     mysqlpp::sql_int, user_id,
	     mysqlpp::sql_varchar_null, status)

sql_create_3(images, 1, 3, 
	     mysqlpp::sql_int, id,
	     mysqlpp::sql_int, friend_id,
	     mysqlpp::sql_varchar_null, path)

sql_create_4(faces, 1, 4, 
	     mysqlpp::sql_int, id,
	     mysqlpp::sql_int, friend_id,
	     mysqlpp::sql_int_null, image_id,
	     mysqlpp::sql_varchar_null, path)

sql_create_6(friends, 1, 6, 
	     mysqlpp::sql_int, id,
	     mysqlpp::sql_int, user_id,
	     mysqlpp::sql_varchar_null, first_name,
	     mysqlpp::sql_varchar_null, last_name,
	     mysqlpp::sql_varchar, email,
	     mysqlpp::sql_varchar_null, phone)
#pragma GCC diagnostic pop
}

namespace StalkR
{
    typedef Database::tasks Task;
    typedef Database::images Image;
    typedef Database::faces Face;
    typedef Database::friends Friend;
}

#endif //DATABASE_H
