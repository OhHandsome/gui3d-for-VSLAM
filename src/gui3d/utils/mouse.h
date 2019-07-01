#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

namespace gui3d {

inline cv::Mat convert(const cv::Mat& m) {
  cv::Mat gray;
  cv::convertScaleAbs(m, gray);
  //cv::equalizeHist(gray, gray);
  return gray;
}

inline void appendText(const std::string& message, cv::Mat& im) {
  int baseline = 0;
  cv::Size textSize = cv::getTextSize(message, cv::FONT_HERSHEY_PLAIN, 1, 1, &baseline);

  cv::Mat imText = cv::Mat(im.rows + textSize.height + 10, im.cols, im.type());
  im.copyTo(imText.rowRange(0, im.rows).colRange(0, im.cols));
  imText.rowRange(im.rows, imText.rows) = cv::Mat::zeros(textSize.height + 10, im.cols, im.type());
  cv::putText(imText, message, cv::Point(5, imText.rows - 5), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255), 1,
              8);
  im = imText;
}

inline void adjust_uv(const cv::Size& imsz, cv::Point& pt) {
  const int width = imsz.width;
  const int height = imsz.height;
  pt.x = pt.x < 0 ? 0 : pt.x;
  pt.x = pt.x >= width ? width - 1 : pt.x;

  pt.y = pt.y < 0 ? 0 : pt.y;
  pt.y = pt.y >= height ? height - 1 : pt.y;
}

inline bool im_shot(const cv::Mat& img, const cv::Point& pre_pt, const cv::Point& cur_pt, cv::Mat& img_box) {
  using namespace cv;
  int width = abs(pre_pt.x - cur_pt.x);
  int height = abs(pre_pt.y - cur_pt.y);
  if (width == 0 || height == 0) {
    LOGI("width == 0 || height == 0");
    return false;
  }
  img_box = img(cv::Rect(std::min(cur_pt.x, pre_pt.x), std::min(cur_pt.y, pre_pt.y), width, height));
  return true;
}

struct MouseCapture {

  explicit MouseCapture(
    const std::string& name,
    const cv::Mat& im)
      : cur_pt(-1, -1)
      , pre_pt(-1, -1)
      , click_pt(-1, -1) {
    this->name = name;
    org = im;
    org.copyTo(img);
  }

  cv::Rect roi;
  cv::Mat dst;
  cv::Point click_pt;
 private:
  cv::Mat org, img;
  std::string name;
  cv::Point pre_pt, cur_pt;
  friend void on_mouse(int event, int x, int y, int flags, void* ustc);
};

inline void on_mouse(int event, int x, int y, int flags, void* ustc) {
  using namespace cv;
  MouseCapture* pState = (MouseCapture*) (ustc);
  cv::Mat& org = pState->org;
  cv::Mat& img = pState->img;
  cv::Point& pre_pt = pState->pre_pt;
  cv::Point& cur_pt = pState->cur_pt;
  cv::Point& click_pt = pState->click_pt;
  std::string& window = pState->name;

  char str[16];
  if (event == CV_EVENT_LBUTTONDOWN)//左键按下，读取初始坐标，并在图像上该点处划圆
  {
    org.copyTo(img);  //将原始图片复制到img中
    sprintf(str, "(%d,%d)", x, y);
    pre_pt = cv::Point(x, y);
    click_pt = pre_pt;
    cv::putText(img, str, pre_pt, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0, 255), 1, 8);//在窗口上显示坐标
    cv::circle(img, pre_pt, 2, cv::Scalar(255, 0, 0, 0), CV_FILLED, CV_AA, 0);//划圆
    cv::imshow(window, img);
  }
  else if (event == CV_EVENT_MOUSEMOVE && !(flags & CV_EVENT_FLAG_LBUTTON))//左键没有按下的情况下鼠标移动的处理函数
  {
    cv::Mat tmp;
    img.copyTo(tmp);//将img复制到临时图像tmp上，用于显示实时坐标
    sprintf(str, "(%d,%d)", x, y);
    cur_pt = cv::Point(x, y);
    cv::putText(tmp, str, cur_pt, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0, 255));//只是实时显示鼠标移动的坐标
    cv::imshow(window, tmp);
  }
  else if (event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON))//左键按下时，鼠标移动，则在图像上划矩形
  {
    cv::Mat tmp;
    img.copyTo(tmp);
    sprintf(str, "(%d,%d)", x, y);
    cur_pt = cv::Point(x, y);
    cv::putText(tmp, str, cur_pt, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0, 255));
    cv::rectangle(tmp, pre_pt, cur_pt, cv::Scalar(0, 255, 0, 0), 1, 8, 0);    //在临时图像上实时显示鼠标拖动时形成的矩形
    cv::imshow(window, tmp);
  }
  else if (event == CV_EVENT_LBUTTONUP) //左键松开，将在图像上划矩形
  {
    org.copyTo(img);
    sprintf(str, "(%d,%d)", x, y);
    cur_pt = cv::Point(x, y);
    cv::putText(img, str, cur_pt, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0, 255));
    cv::circle(img, pre_pt, 2, cv::Scalar(255, 0, 0, 0), CV_FILLED, CV_AA, 0);

    adjust_uv(img.size(), pre_pt);
    adjust_uv(img.size(), cur_pt);
    cv::rectangle(img, pre_pt, cur_pt, cv::Scalar(0, 255, 0, 0), 1, 8, 0);//根据初始点和结束点，将矩形画到img上
    cv::imshow(window, img);

    cv::Mat& dst = pState->dst;
    if (im_shot(org, pre_pt, cur_pt, dst)) {
      cv::Rect& roi = pState->roi;
      roi = cv::Rect(pre_pt, cur_pt);

      cv::namedWindow("dst");
      cv::imshow("dst", dst);
      cv::waitKey(10);
    }
  }
}

} // namespace gui3d

