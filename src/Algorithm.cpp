#include "Algorithm.h"

bool Algorithm::Init(const std::string& encPath, const std::string& decPath)
{
	try
	{
		SessionOptions.SetIntraOpNumThreads(std::thread::hardware_concurrency());
		SessionOptions.SetInterOpNumThreads(1);
		SessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
		SessionOptions.AppendExecutionProvider("DmlExecutionProvider");
		EncSession = std::make_unique<Ort::Session>(Environment, std::wstring(encPath.begin(), encPath.end()).c_str(), SessionOptions);
	}
	catch (Ort::Exception & e) {
		try 
		{
			SessionOptions.SetIntraOpNumThreads(std::thread::hardware_concurrency());
			SessionOptions.SetInterOpNumThreads(1);
			SessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
			EncSession = std::make_unique<Ort::Session>(Environment, std::wstring(encPath.begin(), encPath.end()).c_str(), SessionOptions);
		}
		catch (Ort::Exception & e)
		{ 
			return false;
		}
	}
	try 
	{
		DecSession = std::make_unique<Ort::Session>(Environment, std::wstring(decPath.begin(), decPath.end()).c_str(), SessionOptions);
	}
	catch (Ort::Exception&) {
		return false;
	}

	return true;
}

bool Algorithm::Run(const QImage & inputImg, QImage * outMask, const std::pair<int, int>& pt)
{
	if (!outMask)
		return false;

	QImage imgRGB = inputImg.convertToFormat(QImage::Format_RGB888);
	cv::Mat mat(imgRGB.height(), imgRGB.width(), CV_8UC3,
		const_cast<uchar*>(imgRGB.bits()),
		imgRGB.bytesPerLine());

	cv::Mat mask = Segment(mat, Point(pt.first, pt.second));

	if (mask.empty())
		return false;

	QImage out(mask.cols, mask.rows, QImage::Format_Grayscale8);
	for (int y = 0; y < mask.rows; ++y)
		memcpy(out.scanLine(y), mask.ptr(y), mask.cols);

	*outMask = out;

	return true;
}

cv::Mat Algorithm::Segment(const cv::Mat& image, Point point)
{
	int new_h, new_w, padTop, padLeft;
	auto inputTensor = Preprocess(image, new_h, new_w, padTop, padLeft);

	Ort::MemoryInfo mem = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
	std::array<int64_t, 4> imgShape = { 1,3,1024,1024 };
	Ort::Value imgValue = Ort::Value::CreateTensor<float>(
		mem, inputTensor.data(), inputTensor.size(), imgShape.data(), imgShape.size()
	);

	// Run encoder
	const char* encIn[] = { "input_image" };
	const char* encOut[] = { "image_embeddings" };
	auto encR = EncSession->Run(Ort::RunOptions{ nullptr }, encIn, &imgValue, 1, encOut, 1);
	Ort::Value embeddings = std::move(encR[0]);

	// Prepare points (for now support a single point)
	int numPoints = 1;
	std::array<float, 2> coords;
	std::array<float, 1> labels;

	// Scale + add padding
	coords[0] = point.x * (float(new_w) / image.cols) + padLeft;
	coords[1] = point.y * (float(new_h) / image.rows) + padTop;

	// Single positive point
	labels[0] = 1.0f;

	std::array<int64_t, 3> coordShape = { 1, numPoints, 2 };
	std::array<int64_t, 2> labelShape = { 1, numPoints };

	Ort::Value coordValue = Ort::Value::CreateTensor<float>(mem, coords.data(), coords.size(), coordShape.data(), coordShape.size());
	Ort::Value labelValue = Ort::Value::CreateTensor<float>(mem, labels.data(), labels.size(), labelShape.data(), labelShape.size());

	// Decoder
	const char* decIn[] = { "image_embeddings","point_coords","point_labels" };
	Ort::Value inputs[] = { std::move(embeddings), std::move(coordValue), std::move(labelValue) };
	const char* decOut[] = { "masks", "iou_predictions" };
	auto decR = DecSession->Run(Ort::RunOptions{ nullptr }, decIn, inputs, 3, decOut, 2);

	// Extract mask [1,1,256,256]
	float* data = decR[0].GetTensorMutableData<float>();
	cv::Mat mask256(256, 256, CV_32F, data);

	// Upscale to 1024×1024
	cv::Mat mask1024;
	cv::resize(mask256, mask1024, cv::Size(1024, 1024), 0, 0, cv::INTER_LINEAR);

	// Remove padding
	cv::Rect roi(padLeft, padTop, new_w, new_h);
	cv::Mat cropped = mask1024(roi);

	// Resize back to original size
	cv::Mat maskOrig;
	cv::resize(cropped, maskOrig, image.size(), 0, 0, cv::INTER_LINEAR);

	// Binarize
	cv::Mat maskBin;
	cv::threshold(maskOrig, maskBin, 0.0, 255.0, cv::THRESH_BINARY);
	maskBin.convertTo(maskBin, CV_8U);

	return maskBin;
}

cv::Mat Algorithm::CleanMask(const cv::Mat& mask) const
{
	cv::Mat bin;
	if (mask.type() != CV_8UC1) mask.convertTo(bin, CV_8UC1);
	else                      bin = mask;
	std::vector<std::vector<cv::Point>> ctr;
	cv::findContours(bin, ctr, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	if (ctr.empty()) return bin;
	double maxA = 0; int bi = 0;
	for (int i = 0; i < (int)ctr.size(); ++i) {
		double a = cv::contourArea(ctr[i]);
		if (a > maxA) { maxA = a; bi = i; }
	}
	cv::Mat clean = cv::Mat::zeros(bin.size(), CV_8UC1);
	cv::drawContours(clean, ctr, bi, cv::Scalar(255), cv::FILLED);
	return clean;
}

std::vector<float> Algorithm::Preprocess(const cv::Mat& image, int& resized_h, int& resized_w, int& padTop, int& padLeft)
{
	cv::Mat rgb;
	cv::cvtColor(image, rgb, cv::COLOR_BGR2RGB);
	int H = rgb.rows, W = rgb.cols;

	// Scale so longest side = 1024
	float scale = 1024.f / std::max(H, W);
	resized_h = int(H * scale);
	resized_w = int(W * scale);

	cv::Mat resized;
	cv::resize(rgb, resized, cv::Size(resized_w, resized_h));

	// Center pad to 1024x1024
	padTop = (1024 - resized_h) / 2;
	padLeft = (1024 - resized_w) / 2;
	cv::Mat padded(1024, 1024, CV_8UC3, cv::Scalar(0));
	resized.copyTo(padded(cv::Rect(padLeft, padTop, resized_w, resized_h)));

	// Normalize to float
	padded.convertTo(padded, CV_32F, 1.f / 255.f);
	static const std::array<float, 3> mean = { 0.485f, 0.456f, 0.406f };
	static const std::array<float, 3> std = { 0.229f, 0.224f, 0.225f };

	std::vector<cv::Mat> channels;
	cv::split(padded, channels);
	for (int c = 0; c < 3; ++c) {
		channels[c] = (channels[c] - mean[c]) / std[c];
	}
	cv::merge(channels, padded);

	// Convert HWC -> CHW
	std::vector<float> tensor(1 * 3 * 1024 * 1024);
	size_t idx = 0;
	for (int c = 0; c < 3; ++c) {
		for (int i = 0; i < 1024; ++i) {
			for (int j = 0; j < 1024; ++j) {
				tensor[idx++] = padded.at<cv::Vec3f>(i, j)[c];
			}
		}
	}
	return tensor;
}
