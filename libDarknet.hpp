#ifndef LIB_DARKNET_HPP_
#define LIB_DARKNET_HPP_

#include "darknet.h"
#include "opencv2/core.hpp"

struct NET_RESULT{
    cv::Rect pos;
    float confidence;
    std::string cls_name;
};

enum MODE{
    MODE_TRAIN,
    MODE_DETECT
};

class lib_darknet{
public:
    bool init(const char* cfg="config.yml");
    void detect(const cv::Mat& image, std::vector<NET_RESULT>& res);
    void detect_batch(bool save_file=false);

private:
    MODE mode;
    float threshold;
    float nms_value;
    std::vector<std::string> class_name;
    std::vector<std::string> image_list;

    network* net;
private:
    bool load_cls_info(const char* cls_def);
    void load_images_from_file(const char* list_file);
    image mat_to_image(const cv::Mat& cv_img);
    void generate_results(int lay_num, const detection* dets, const cv::Size& imsz, std::vector<NET_RESULT>& res);
    cv::Mat draw_results(const cv::Mat image, const std::vector<NET_RESULT>& res);
};

#endif