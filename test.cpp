#include "Database.h"
#include "FaceTrainer.h"

#include <opencv2/contrib/contrib.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "tinydir.h"

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <stdexcept>

typedef std::map<int, std::vector<cv::Mat> > FaceMap;
const std::string WEB_ROOT    = "/var/www/public/";
const std::string FACE_PREFIX = "main/processed_images/";

FaceMap buildFriendFaceMap(int id, mysqlpp::Connection *database) throw(std::runtime_error)
{
    mysqlpp::Query query = database->query("select * from friends");   
    query << " where user_id=" << id;

    mysqlpp::StoreQueryResult friends = query.store();
    if (!friends)
    {
	std::stringstream errstream;
	errstream << "Couldn't fetch friends: " << query.error();
	throw std::runtime_error(errstream.str());
    }
    
    FaceMap faceMap;
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
	
	std::vector<cv::Mat> faceList;
	for (size_t j = 0; j < faces.num_rows(); j++)
	{
	    StalkR::Face face = faces[j];
	    if (face.path.is_null)
		continue;
		
	    cv::Mat image = cv::imread(WEB_ROOT + face.path.data,
				       CV_LOAD_IMAGE_GRAYSCALE);
	    if (image.data)
		faceList.push_back(image);
	}

	faceMap[f.id] = faceList;
    }

    return faceMap;
}

FaceMap splitFaces(const FaceMap &allFaces,  double testBegin, double testEnd,
		   StalkR::FaceTrainer *trainer) throw(std::runtime_error)
{
    FaceMap testMap;
    for (FaceMap::const_iterator it = allFaces.cbegin(); it != allFaces.cend(); ++it)
    {
	int                         friendID = it->first;
	const std::vector<cv::Mat> &faces    = it->second;
	
	size_t testBeginIndex = testBegin * faces.size();
	size_t testEndIndex   = testEnd * faces.size();

	std::vector<cv::Mat> testFaces;
	bool addedValidationFace = false;
	for (size_t i = 0; i < faces.size(); ++i)
	{
	    if (i >= testBeginIndex && i < testEndIndex)
	    {
		testFaces.push_back(faces[i]);
		continue;
	    }

	    if (!addedValidationFace)
	    {
		trainer->addValidationFace(faces[i], friendID);
		addedValidationFace = true;
		continue;
	    }

	    trainer->addTrainingFace(faces[i], friendID);
	}

	testMap[friendID] = testFaces;
    }

    return testMap;
}

std::vector<cv::Mat> loadUnknownFaces(const std::string &path)
{
    std::vector<cv::Mat> unknownFaces;

    tinydir_dir dir;
    tinydir_open(&dir, path.c_str());
    while (dir.has_next)
    {
	tinydir_file file;
	tinydir_readfile(&dir, &file);

	if (file.is_dir)
	{
	    tinydir_next(&dir);
	    continue;
	}

	cv::Mat image = cv::imread(file.path, CV_LOAD_IMAGE_GRAYSCALE);
	if (!image.data)
	{
	    tinydir_next(&dir);
	    continue;
	}
	
	unknownFaces.push_back(image);
	tinydir_next(&dir);
    }
    tinydir_close(&dir);

    return unknownFaces;
}

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
	std::cout << "Usage: " << argv[0]
		  << " [database user] [database password] [user id]" << std::endl;
	return 1;
    }

    try
    {
	mysqlpp::Connection database("stalkr", "localhost", argv[1], argv[2]);
	if (!database.connected())
	    throw std::invalid_argument("Could not connect to database");

	int id;
	std::stringstream(argv[3]) >> id;
	FaceMap friendFaceMap = buildFriendFaceMap(id, &database);
	if (friendFaceMap.size() <= 1)
	    throw std::invalid_argument("Can't train classifier with less than 1 friend");

	std::vector<cv::Mat> unknownFaces = loadUnknownFaces("./UnknownFaces");
	std::cout << "\"Correct\", \"Misclassified\","
		     "\"True negatives\", \"False negatives\"" << std::endl;

	static const double TEST_FRACTION = 0.25;
	for (size_t i = 0; i < 4; ++i)
	{
	    StalkR::FaceTrainer trainer;
	    trainer.loadNotFaces("./NotFaces");

	    FaceMap testFaceMap = splitFaces(friendFaceMap, TEST_FRACTION * i,
					     TEST_FRACTION * (i + 1), &trainer);
	    testFaceMap[-1] = unknownFaces;

	    if (trainer.empty())
		throw std::invalid_argument("Not enough faces provided for classifier");

	    cv::Ptr<cv::FaceRecognizer> recognizer = trainer.train();
	    
	    size_t numCorrect     = 0;
	    size_t misidentified  = 0;
	    size_t trueNegatives  = 0;
	    size_t falseNegatives = 0;
	    
	    for (FaceMap::iterator it = testFaceMap.begin(); it != testFaceMap.end(); ++it)
	    {
		int realClass                     = it->first;
		const std::vector<cv::Mat> &faces = it->second;
		for (size_t j = 0; j < faces.size(); ++j)
		{
		    int predictedClass = recognizer->predict(faces[j]);
		    if (predictedClass == -1)
		    {
			if (realClass == -1)
			    trueNegatives++;
			else
			    falseNegatives++;
		    }
		    else if (predictedClass == realClass)
			numCorrect++;
		    else
			misidentified++;
		}
	    }

	    std::cout << "\"" << numCorrect << "\", " 
		      << "\"" << misidentified << "\", " 
		      << "\"" << trueNegatives << "\", " 
		      << "\"" << falseNegatives << "\"" << std::endl;
	}
    }
    catch(const std::exception &e)
    {
	// Unexpected exception, bail.
	std::cerr << e.what() << std::endl;
	return 1;
    }

    return 0;
}
