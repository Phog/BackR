#include "FaceTrainer.h"

#include <opencv2/highgui/highgui.hpp>
#include "tinydir.h"
#include <iostream>

using StalkR::FaceTrainer;

cv::Ptr<cv::FaceRecognizer> FaceTrainer::train()
{
    static const double NOT_FACE_SCORE       = -10.0;
    static const double WRONG_CLASS_SCORE    = -1.0;
    static const double NOT_RECOGNIZED_SCORE = 0.0;
    static const double CORRECT_CLASS_SCORE  = 1.0;
    static const double DEFAULT_THRESHOLD    = 50.0;

    double bestThreshold = DEFAULT_THRESHOLD;
    double bestScore     = -10000.0;
    cv::Ptr<cv::FaceRecognizer> recognizer;
    recognizer = cv::createFisherFaceRecognizer(0, DEFAULT_THRESHOLD);
    recognizer->train(m_trainingFaces, m_trainingClasses);

    static const double THRESHOLD_MAX  = 3000.0;
    static const double THRESHOLD_STEP = 100.0;
    for (double threshold = DEFAULT_THRESHOLD; threshold < THRESHOLD_MAX; threshold += THRESHOLD_STEP)
    {
	double score = 0;
	recognizer->set("threshold", threshold);
	for (size_t i = 0; i < m_notFaces.size(); i++)
	{
	    if (recognizer->predict(m_notFaces[i]) != -1)
		score += NOT_FACE_SCORE;
	}

	for (size_t i = 0; i < m_validationFaces.size(); i++)
	{
	    int c = recognizer->predict(m_validationFaces[i]);
	    if (c == -1)
		score += NOT_RECOGNIZED_SCORE;
	    else if (c == m_validationClasses[i])
		score += CORRECT_CLASS_SCORE;
	    else
		score += WRONG_CLASS_SCORE;
	}

	if (score > bestScore)
	{
	    bestScore     = score;
	    bestThreshold = threshold;
	}
    }

    for (size_t i = 0; i < m_validationFaces.size(); ++i)
    {
	m_trainingFaces.push_back(m_validationFaces[i]);
	m_trainingClasses.push_back(m_validationClasses[i]);
    }

    recognizer->train(m_trainingFaces, m_trainingClasses);
    recognizer->set("threshold", bestThreshold);
    return recognizer;
}

void FaceTrainer::loadNotFaces(const std::string &path)
{
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
	
	m_notFaces.push_back(image);
	tinydir_next(&dir);
    }
    tinydir_close(&dir);
}
