#include "TaskManager.h"
#include "FaceDetector.h"
#include "FaceTrainer.h"

#include <opencv2/contrib/contrib.hpp>

#include <sstream>
#include <cassert>

#define DO_QUOTE(X) #X
#define QUOTE(X) DO_QUOTE(X)

namespace 
{
    const std::string SERVR_ROOT           = QUOTE(WEB_ROOT);
    const std::string PUBLIC_ROOT          = SERVR_ROOT + "/public/";
    const std::string FACE_PREFIX          = "main/processed_images/";
    const std::string FACE_EXTENSION       = ".png";
    const std::string RECOGNIZER_PREFIX    = "main/recognizers/";
    const std::string RECOGNIZER_EXTENSION = ".xml";
    
    size_t makeFace(mysqlpp::Connection *database,
		    size_t friend_id, size_t image_id) throw(std::runtime_error)
    {
	mysqlpp::Transaction transaction(*database,
					 mysqlpp::Transaction::serializable,
					 mysqlpp::Transaction::this_transaction);
	    
	mysqlpp::Query query = database->query("insert into faces");
	query << " (friend_id, image_id) values (" 
	      << mysqlpp::quote << friend_id << ", "
	      << mysqlpp::quote << image_id << ")";

	query.execute();
	if (query.errnum())
	{
	    std::stringstream errstream;
	    errstream << "Couldn't add new face: " << query.error();
	    throw std::runtime_error(errstream.str());
	}
	
	size_t id = query.insert_id();
	transaction.commit();

	return id;
    }

    void preprocessImage(StalkR::Image image,
			 mysqlpp::Connection *database,
			 StalkR::FaceDetector *detector) throw(std::runtime_error)
    {
	const std::string inputPath = PUBLIC_ROOT + image.path.data;
	
	detector->recognize(inputPath);
	if (!detector->facesRemaining())
	    makeFace(database, image.friend_id, image.id);

	while (detector->facesRemaining())
	{
	    size_t id = makeFace(database, image.friend_id, image.id);
	    std::stringstream pathstream;
	    pathstream << FACE_PREFIX << id << FACE_EXTENSION;

	    mysqlpp::Query query = database->query("update faces set path=");
	    query << mysqlpp::quote << pathstream.str() << " where id="
		  << mysqlpp::quote << id;

	    query.execute();
	    if (query.errnum())
	    {
		std::stringstream errstream;
		errstream << "Couldn't update face path: " << query.error();
		throw std::runtime_error(errstream.str());
	    }

	    detector->outputFace(PUBLIC_ROOT + pathstream.str());
	}
    }

    void preprocessFaces(size_t user_id,
			 mysqlpp::Connection *database,
			 StalkR::FaceDetector *detector) throw(std::runtime_error)
    {
	mysqlpp::Query query = database->query("select i.*");   
	query << " from images i, friends f"
	      << " where i.friend_id=f.id and f.user_id=" << user_id
	      << " and not exists (select fr.* from faces fr"
	      << " where fr.image_id=i.id)";

	mysqlpp::StoreQueryResult result = query.store();
	if (!result)
	{
	    std::stringstream errstream;
	    errstream << "Couldn't fetch image metadata: " << query.error();
	    throw std::runtime_error(errstream.str());
	}
    
	for (size_t i = 0; i < result.num_rows(); i++)
	{
	    StalkR::Image image = result[i];
	    if (image.path.is_null)
		continue;

#ifndef NDEBUG
	    int rows   = result.num_rows();
	    int digits = 1;
	    while (rows > 0)
	    {
		digits++;
		rows /= 10;
	    }

	    std::cout << "Detecting faces [" << i + 1 << "/" << result.num_rows() << "]"
		      << std::string(digits, ' ') << "\r" << std::flush;
#endif
	    preprocessImage(image, database, detector);
	}
#ifndef NDEBUG
	std::cout << std::endl;
#endif
    }

    void loadFaces(size_t user_id, mysqlpp::Connection *database,
		   StalkR::FaceTrainer *trainer) throw(std::runtime_error)
    {
	mysqlpp::Query query = database->query("select * from friends");   
	query << " where user_id=" << user_id;

	mysqlpp::StoreQueryResult friends = query.store();
	if (!friends)
	{
	    std::stringstream errstream;
	    errstream << "Couldn't fetch friends: " << query.error();
	    throw std::runtime_error(errstream.str());
	}
    
	for (size_t i = 0; i < friends.num_rows(); i++)
	{
	    StalkR::Friend f = friends[i];

	    query.reset();
	    query << "select * from faces where friend_id=" << f.id;

	    mysqlpp::StoreQueryResult faces = query.store();
	    if (!faces)
	    {
		std::stringstream errstream;
		errstream << "Couldn't fetch faces: " << query.error();
		throw std::runtime_error(errstream.str());
	    }
	    
	    for (size_t j = 0; j < faces.num_rows(); j++)
	    {
		StalkR::Face face = faces[j];
		if (face.path.is_null)
		    continue;
		
		cv::Mat image = cv::imread(PUBLIC_ROOT + face.path.data,
					   CV_LOAD_IMAGE_GRAYSCALE);
		if (image.data)
		{
		    if (j == 0 && faces.num_rows() > 1)
			trainer->addValidationFace(image, f.id);
		    else
			trainer->addTrainingFace(image, f.id);
		}
	    }
	}
    }

    int countFriends(int user_id, mysqlpp::Connection *database) throw(std::runtime_error)
    {
	mysqlpp::Query query = database->query("select id from friends");   
	query << " where user_id=" << user_id;

	mysqlpp::StoreQueryResult friends = query.store();
	return !friends ? 0 : friends.num_rows();
    }
}

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

void TaskManager::executeTasks(FaceDetector *detector) throw(std::runtime_error)
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
	
	executeTask(m_tasks[i], detector);

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

void TaskManager::executeTask(const Task& t, FaceDetector *detector) throw(std::runtime_error)
{
    preprocessFaces(t.user_id, &m_database, detector);

    FaceTrainer trainer;
    trainer.loadNotFaces("./NotFaces");
    loadFaces(t.user_id, &m_database, &trainer);
    if (trainer.empty() || countFriends(t.user_id, &m_database) <= 1)
	return;

    cv::Ptr<cv::FaceRecognizer> recognizer = trainer.train();

    std::stringstream pathstream;
    pathstream << RECOGNIZER_PREFIX << t.user_id << RECOGNIZER_EXTENSION;
    recognizer->save(PUBLIC_ROOT + pathstream.str());

    mysqlpp::Query query = m_database.query("update users set recognizer=");
    query << mysqlpp::quote << pathstream.str()
	  << " where id=" << t.user_id;
    query.execute();
    if (query.errnum())
    {
	std::stringstream errstream;
	errstream << "Couldn't update recognizer path: " << query.error();
	throw std::runtime_error(errstream.str());
    }
}
