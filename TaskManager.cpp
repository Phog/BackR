#include "TaskManager.h"
#include "FaceRecognizer.h"

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

void TaskManager::executeTasks(FaceRecognizer *recognizer) throw(std::runtime_error)
{
    mysqlpp::Query query = m_database.query();
    for (size_t i = 0; i < m_tasks.size(); i++)
    {
	Task inProgress   = m_tasks[i];
	inProgress.status = "In progress";

	query.reset();
	query.update(m_tasks[i], inProgress);
	query.execute();
	if (query.errnum())
	{
	    std::stringstream errstream;
	    errstream << "Couldn't update task status: " << query.error();
	    throw std::runtime_error(errstream.str());
	}
	
	executeTask(m_tasks[i], recognizer);

	query.reset();
	query << "delete from tasks where id=" << inProgress.id;
	query.execute();
	if (query.errnum())
	{
	    std::stringstream errstream;
	    errstream << "Couldn't delete completed task: " << query.error();
	    throw std::runtime_error(errstream.str());
	}
    }
}

void TaskManager::executeTask(const Task& t, FaceRecognizer *recognizer) throw(std::runtime_error)
{
    static const std::string WEB_ROOT       = "/var/www/public/";
    static const std::string FILE_EXTENSION = ".png";

    mysqlpp::Query query = m_database.query("select i.*");   
    query << " from images i, friends f"
	  << " where i.friend_id=f.id and f.user_id=" << t.user_id
	  << " and i.id not in (select image_id from faces)";


    mysqlpp::StoreQueryResult result = query.store();
    if (!result)
    {
	std::stringstream errstream;
	errstream << "Couldn't fetch image metadata: " << query.error();
	throw std::runtime_error(errstream.str());
    }
    
    for (size_t i = 0; i < result.num_rows(); i++)
    {
	Image image = result[i];
	if (image.path.is_null)
	    continue;

	const std::string inputPath    = WEB_ROOT + image.path.data;
	const std::string outputPrefix = "main/processed_images/";
	
	std::cout << "Recognizing face [" << i + 1 << "/"
		  << result.num_rows() << "]\r" << std::flush;

	recognizer->recognize(inputPath);
	while (recognizer->facesRemaining())
	{
	    mysqlpp::Transaction transaction(m_database,
					     mysqlpp::Transaction::serializable,
					     mysqlpp::Transaction::this_transaction);
	    
	    mysqlpp::Query query = m_database.query("insert into faces");
	    query << " (friend_id, image_id) values ("
		  << mysqlpp::quote << image.friend_id << ", "
		  << mysqlpp::quote << image.id << ")";

	    query.execute();
	    if (query.errnum())
	    {
		std::stringstream errstream;
		errstream << "Couldn't add new face: " << query.error();
		throw std::runtime_error(errstream.str());
	    }

	    size_t id = query.insert_id();
	    std::stringstream pathstream;
	    pathstream << outputPrefix << id << FILE_EXTENSION;

	    query.reset();
	    query << "update faces set path="
		  << mysqlpp::quote << pathstream.str()
		  << " where id=" << mysqlpp::quote << id;

	    query.execute();
	    if (query.errnum())
	    {
		std::stringstream errstream;
		errstream << "Couldn't update face path: " << query.error();
		throw std::runtime_error(errstream.str());
	    }

	    transaction.commit();
	    recognizer->outputFace(WEB_ROOT + pathstream.str());
	}
    }
}
