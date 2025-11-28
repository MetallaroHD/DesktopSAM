#pragma once
#include <opencv2/opencv.hpp>
#include <onnxruntime/onnxruntime_cxx_api.h>
#include <qimage.h>
#include <vector>
#include <string>

struct Point {
    float x;
    float y;
public:
    Point(float ix, float iy)
    {
        x = ix;
        y = iy;
    }
};

class Algorithm 
{
public:
    Algorithm() = default;
    bool Init(const std::string& encPath, const std::string& decPath);
    bool Run(const QImage & img, QImage* outMask, const std::pair<int, int> & pt);

private:
    Ort::Env Environment{ ORT_LOGGING_LEVEL_WARNING, "samcpp" };
    Ort::SessionOptions SessionOptions = Ort::SessionOptions();
    std::unique_ptr<Ort::Session> EncSession { nullptr };
    std::unique_ptr<Ort::Session> DecSession { nullptr };
    std::vector<float> Mask;

private:
    std::vector<float> Preprocess(const cv::Mat& image, int& resized_h, int& resized_w, int& padTop, int& padLeft);
    cv::Mat Segment(const cv::Mat& image, Point point);
    cv::Mat CleanMask(const cv::Mat& mask) const;
};