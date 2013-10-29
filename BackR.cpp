#include <mysql++/mysql++.h>
#include <iostream>

int main(int, char**)
{
    // Connect to the sample database.
    mysqlpp::Connection conn(false);
    if (!conn.connect("StalkR", "localhost", "BackR", ""))
    {
	std::cerr << "DB connection failed: " << conn.error() << std::endl;
	return 1;
    }

    // Retrieve the sample stock table set up by resetdb
    mysqlpp::Query query = conn.query("select * from Tasks");
    mysqlpp::StoreQueryResult res = query.store();
	
    if (!res)
    {
	std::cerr << "Failed to get Task table: " << query.error() << std::endl;
	return 1;
    }

    for (size_t i = 0; i < res.num_rows(); ++i)
    {
	// std::cout << res[i]["column_name"] << std::endl;
    }

    std::cout << "Hello, BackR!" << std::endl;

    return 0;
}
