#ifndef FACETRAINER_H
#define FACETRAINER_H

#include <opencv2/core/core.hpp>
#include <opencv2/contrib/contrib.hpp>

#include <vector>

namespace StalkR
{
    class FaceTrainer
    {
    public:
	cv::Ptr<cv::FaceRecognizer> train();
	void loadNotFaces(const std::string &path);

	void addTrainingFace(const cv::Mat& mat, int c)
	{
	    m_trainingFaces.push_back(mat);
	    m_trainingClasses.push_back(c);
	}

	void addValidationFace(const cv::Mat& mat, int c)
	{
	    m_validationFaces.push_back(mat);
	    m_validationClasses.push_back(c);
	}

	bool empty() const { return m_trainingFaces.empty(); }

    private:
	std::vector<cv::Mat> m_trainingFaces;
	std::vector<cv::Mat> m_validationFaces;
	std::vector<cv::Mat> m_notFaces;
	std::vector<int>     m_trainingClasses;
	std::vector<int>     m_validationClasses;
    };
}

#endif
