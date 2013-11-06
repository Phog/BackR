#include "TaskManager.h"

#include <sstream>

using StalkR::TaskManager;
using StalkR::Task;

TaskManager::TaskManager(const std::string& dbName,
			 const std::string& dbAddress,
			 const std::string& dbUser,
			 const std::string& dbPassword) throw(std::invalid_argument)
    : m_database(false)
{
    if (!m_database.connect(dbName.c_str(),
			    dbAddress.c_str(),
			    dbUser.c_str(),
			    dbPassword.c_str()))
    {
	std::stringstream errstream;
	errstream << "DB connection failed: " << m_database.error();
	throw std::invalid_argument(errstream.str());
    }
	
}

TaskManager::~TaskManager() throw()
{
    m_database.disconnect();
}

void TaskManager::fetchTasks() throw(std::runtime_error)
{
    mysqlpp::Query query = m_database.query("select * from tasks");
    query.storein(m_tasks);
}

void TaskManager::executeTasks() throw(std::runtime_error)
{
    for (size_t i = 0; i < m_tasks.size(); i++)
    {
	mysqlpp::Query query = m_database.query();

	Task inProgress   = m_tasks[i];
	inProgress.status = "In progress";
	query.update(m_tasks[i], inProgress);
	query.execute();

	executeTask(m_tasks[i]);

	query.reset();
	query << "delete from tasks where id=" << inProgress.id;
	query.execute();
    }
}

void TaskManager::executeTask(const Task&) throw(std::runtime_error)
{
}
