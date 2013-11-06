#include "FaceRecognizer.h"
#include <cassert>

using StalkR::FaceRecognizer;

FaceRecognizer::FaceRecognizer(const std::string &classifierPath) throw(std::invalid_argument)
    : m_cascade(new cv::CascadeClassifier(classifierPath))
{
    if (m_cascade->empty())
	throw std::invalid_argument("Couldn't load classifier from: " + classifierPath);
}

void FaceRecognizer::recognize(const std::string &inputPath) throw(std::runtime_error)
{
    m_faces.clear();
    m_image = cv::imread(inputPath, CV_LOAD_IMAGE_GRAYSCALE);
    if (!m_image.data)
	throw std::invalid_argument("Couldn't load image from: " + inputPath);


    static const double scaleFactor   = 1.1;
    static const int    minNeighbours = 10;
    m_cascade->detectMultiScale(m_image, m_faces, scaleFactor, minNeighbours, 0);
}

void FaceRecognizer::outputFace(const std::string &outputPath) throw(std::runtime_error)
{
    assert(facesRemaining());
    cv::Mat face = m_image(m_faces.back());
    m_faces.pop_back();

    cv::Mat resized;
    cv::resize(face, resized, cv::Size(256, 256));
    cv::imwrite(outputPath, resized);
}
