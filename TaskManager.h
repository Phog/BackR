#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "Database.h"

#include <vector>
#include <stdexcept>

namespace StalkR
{
    class FaceRecognizer;

    class TaskManager
    {
    public:
	TaskManager(const std::string& dbName,
		    const std::string& dbAddress,
		    const std::string& dbUser,
		    const std::string& dbPassword) throw(std::invalid_argument);
	~TaskManager() throw();

	void fetchTasks() throw(std::runtime_error);
	void executeTasks(FaceRecognizer *recognizer) throw(std::runtime_error);
	void clearTasks() throw() { m_tasks.clear(); }

    private:
	void executeTask(const Task& task, FaceRecognizer *recognizer) throw(std::runtime_error);

	// Make noncopyable, not implemented.
	TaskManager(const TaskManager&);
	TaskManager& operator=(const TaskManager&);

	mysqlpp::Connection m_database;
	std::vector<Task>   m_tasks;
    };
}

#endif // TASKMANAGER_H
