#ifndef pix2image
#define pix2image

#include <vector>

//#include <iomainip>
#include <stdexcept>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <string>


//#include <photonfocus_camera.h>

namespace POLPro
{
    std::vector<cv::Mat> raw2mat(const cv::Mat& origin);

    std::vector<cv::Mat> compute_stokes(const cv::Mat& origin);
    std::vector<cv::Mat> compute_stokes(
        const std::vector<cv::Mat>& angles_img);

    std::vector<cv::Mat> compute_polar_params(const cv::Mat& origin);
    std::vector<cv::Mat> compute_polar_params(
        const std::vector<cv::Mat>& origin);

    void imshow(std::vector<cv::Mat> img, bool as_hsv, bool as_stokes);
    int minmax(cv::Mat img, std::string s);
}
#endif //pix2image.h
