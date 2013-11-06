#ifndef FACEDETECTOR_H
#define FACEDETECTOR_H

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#include <vector>
#include <string>
#include <stdexcept>

namespace StalkR
{
    class FaceDetector
    {
    public:
	FaceDetector(const std::string &classifierPath) throw(std::invalid_argument);

	void   recognize(const std::string &inputPath) throw(std::runtime_error);
	void   outputFace(const std::string &outputPath) throw(std::runtime_error);
	size_t facesRemaining() throw() { return m_faces.size(); }

    private:
	// Make noncopyable.
	FaceDetector(const FaceDetector&);
	FaceDetector& operator=(const FaceDetector&);

	cv::Ptr<cv::CascadeClassifier> m_cascade;
	std::vector<cv::Rect>          m_faces;
	cv::Mat                        m_image;
    };
}

#endif // FACEDETECTOR_H
