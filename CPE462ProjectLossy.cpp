#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <cmath>

using namespace std;
using namespace cv;

//Function to perform fftshift-like operation on a complex matrix
void fftshift(Mat& mat) {
    int cx = mat.cols / 2;
    int cy = mat.rows / 2;

    Mat m0(mat(Rect(0, 0, cx, cy)));   // Top-Left
    Mat m1(mat(Rect(cx, 0, cx, cy)));  // Top-Right
    Mat m2(mat(Rect(0, cy, cx, cy)));  // Bottom-Left
    Mat m3(mat(Rect(cx, cy, cx, cy))); // Bottom-Right
    Mat temp;

    m0.copyTo(temp);
    m3.copyTo(m0);
    temp.copyTo(m3);

    m1.copyTo(temp);
    m2.copyTo(m1);
    temp.copyTo(m2);
}

void compressChannelDFT(Mat& channel, double keepFraction, bool calibrate) {
    //Convert to float
    Mat data;
    channel.convertTo(data, CV_32F);

    if (calibrate == false) {
        data -= 128.0f;
    }

    //Optimal DFT Size padding
    int m = getOptimalDFTSize(data.rows);
    int n = getOptimalDFTSize(data.cols);
    Mat padded;
    copyMakeBorder(data, padded, 0, m - data.rows, 0, n - data.cols, BORDER_CONSTANT, Scalar::all(0));

    //Complex Matrix
    Mat planes[] = { padded, Mat::zeros(padded.size(), CV_32F) };
    Mat freq;
    merge(planes, 2, freq);

    //Forward DFT
    dft(freq, freq);

    //Shift zero-frequency to center
    fftshift(freq);

    int rows = freq.rows;
    int cols = freq.cols;
    int crow = rows / 2;
    int ccol = cols / 2;

    //Mask coordinates
    int r = static_cast<int>(keepFraction * min(crow, ccol));
    Mat compressedFreq = Mat::zeros(freq.size(), freq.type());

    Rect centralRect(ccol - r, crow - r, 2 * r, 2 * r);

    freq(centralRect).copyTo(compressedFreq(centralRect));

    //Inverse shift
    fftshift(compressedFreq);

    //Inverse DFT
    Mat reconstructed;
    idft(compressedFreq, reconstructed, DFT_REAL_OUTPUT | DFT_SCALE);

    //Restore to original size 
    reconstructed = reconstructed(Rect(0, 0, channel.cols, channel.rows));

    //Normalize before converting, check if Y or Cr/Cb and only normalize Y
    if (calibrate == true) {
        normalize(reconstructed, reconstructed, 0, 255, NORM_MINMAX);
    }
    else {
        reconstructed += 128.0f;
    }
    
    reconstructed.convertTo(channel, CV_8U);
}

void compressImageDFT(const string& inputPath, const string& outputPath, double keepFraction)
{
    //Read image
    Mat input = imread(inputPath, IMREAD_COLOR);
    if (input.empty()){
        cout << "Error: Could not load image from " << inputPath << endl;
        return;
    }

    //Convert YCrCb
    Mat ycrcb;
    cvtColor(input, ycrcb, COLOR_BGR2YCrCb);

    //Isolate color channels
    vector<Mat> channels;
    split(ycrcb, channels);

    //Separate channel compression & split keepFraction values to adjust compression of each channel
    double yFraction = keepFraction;
    double cFraction = keepFraction * 0.4;

    compressChannelDFT(channels[0], yFraction, true); //Higher Y compression
    compressChannelDFT(channels[1], cFraction, false); //Lower Cr, Cb compression
    compressChannelDFT(channels[2], cFraction, false);

    //Recombine color channels & Create output image to send back
    merge(channels, ycrcb);
    Mat outputImage;
    cvtColor(ycrcb, outputImage, COLOR_YCrCb2BGR);

    //Save result & reduce file size
    vector<int> qualityParameter = { IMWRITE_JPEG_QUALITY, static_cast<int>(100 * keepFraction) };
    if (!imwrite(outputPath, outputImage, qualityParameter)) {
        cout << "Error: Could not save image to " << outputPath << endl;
    }
    else {
        cout << "Compressed image saved to " << outputPath << endl;
    }
}

int main(int argc, char** argv)
{
    string inputPath;
    string outputPath = "CompressedResult.jpg";
    double keepFraction;

    cout << "Choose your image: " << endl;
    cin >> inputPath;
    cout << "Enter between 0.01 (uneligible) to 1 (no compression)" << endl;
    cin >> keepFraction;

    compressImageDFT(inputPath, outputPath, keepFraction);
    return 0;
}