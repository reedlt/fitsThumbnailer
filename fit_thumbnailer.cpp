#include <opencv2/opencv.hpp>
#include <fitsio.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <cmath>

void autoStretch(cv::Mat& image, double scale_factor = 0.03) {
    // Normalize the image to range [0, 1]
    double minVal, maxVal;
    cv::minMaxLoc(image, &minVal, &maxVal);
    image = (image - minVal) / (maxVal - minVal);

    // Apply asinh stretch
    image.forEach<cv::Vec3f>([&](cv::Vec3f& pixel, const int*) {
        for(int i=0; i<3; i++){
          pixel[i] = std::asinh(pixel[i] / scale_factor) / std::asinh(1.0 / scale_factor);
        }
    });
}

void create_thumbnail(const std::string& fits_file_path, const std::string& output_path, cv::Size thumbnail_size) {
    fitsfile* fptr;
    int status = 0;
    int bitpix, naxis;
    long naxes[3] = {1, 1, 1};

    // Open FITS file
    if (fits_open_file(&fptr, fits_file_path.c_str(), READONLY, &status)) {
        fits_report_error(stderr, status);
        return;
    }
    fits_get_img_param(fptr, 3, &bitpix, &naxis, naxes, &status);

    // Read image data
    long fpixel[3] = {1, 1, 1};
    std::vector<float> image_data(naxes[0] * naxes[1] * naxes[2]);
    if (fits_read_pix(fptr, TFLOAT, fpixel, image_data.size(), nullptr, image_data.data(), nullptr, &status)) {
        fits_report_error(stderr, status);
        fits_close_file(fptr, &status);
        return;
    }
    fits_close_file(fptr, &status);

    // Convert to OpenCV Mat
    cv::Mat img;
    if (naxis == 3) {
        cv::Mat r = cv::Mat(naxes[1], naxes[0], CV_32FC1, image_data.data());
        cv::Mat g = cv::Mat(naxes[1], naxes[0], CV_32FC1, &image_data.data()[naxes[1]*naxes[0]]);
        cv::Mat b = cv::Mat(naxes[1], naxes[0], CV_32FC1, &image_data.data()[naxes[1]*naxes[0]*2]);
        std::vector<cv::Mat> channels = {b,g,r};
        cv::merge(channels, img);
    } else if (naxis == 2) {
        img = cv::Mat(naxes[1], naxes[0], CV_32FC1, image_data.data());
        img.convertTo(img, CV_16U);
        cv::cvtColor(img, img, cv::COLOR_BayerBG2BGR);
        img.convertTo(img, CV_32F);
        //cv::log(img.() , img);  // sqrt(img^2 + 1)
        //cv::normalize(img, img, 0, 255, cv::NORM_MINMAX, CV_8UC3);
        //float gamma = 1.2;
        //cv::Mat lookUpTable(1, 256, CV_8U);
        //for (int i = 0; i < 256; ++i) {
        //  lookUpTable.at<uchar>(i) = cv::saturate_cast<uchar>(pow(i / 255.0, 1.0 / gamma) * 255.0);
        //}
        //cv::LUT(img, lookUpTable, img);
        autoStretch(img);
    }

    // Normalize and convert to 8-bit
    double minVal, maxVal;
    cv::minMaxLoc(img, &minVal, &maxVal);
    img.convertTo(img, CV_8U, 255.0 / (maxVal - minVal), -minVal * 255.0 / (maxVal - minVal));

    // Resize to thumbnail size
    int original_width = img.cols;
    int original_height = img.rows;

    // Calculate aspect ratios
    float aspect_ratio = static_cast<float>(original_width) / original_height;
    float target_aspect_ratio = static_cast<float>(thumbnail_size.width) / thumbnail_size.height;

    int new_width, new_height;
    if (aspect_ratio > target_aspect_ratio) {
        // Width is the limiting factor
        new_width = thumbnail_size.width;
        new_height = static_cast<int>(thumbnail_size.width / aspect_ratio);
    } else {
        // Height is the limiting factor
        new_height = thumbnail_size.height;
        new_width = static_cast<int>(thumbnail_size.height * aspect_ratio);
    }

    // Resize to new dimensions
    cv::Mat thumbnail;
    cv::resize(img, thumbnail, cv::Size(new_width, new_height), 0, 0, cv::INTER_AREA);
    // Write thumbnail to file
    cv::imwrite(output_path, thumbnail);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file> [thumbnail_size]\n";
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_file = argv[2];
    cv::Size thumbnail_size(128, 128);
    if (argc > 3) {
        int size_arg = std::stoi(argv[3]);
        thumbnail_size = cv::Size(size_arg, size_arg);
    }

    create_thumbnail(input_file, output_file, thumbnail_size);
    return 0;
}

