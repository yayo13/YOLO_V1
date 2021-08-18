#include "libDarknet.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "fstream"
#include "log.h"

bool lib_darknet::init(const char* cfg){
    AINFO << "loading config file from " << cfg << std::endl;
    cv::FileStorage fs(cfg, cv::FileStorage::READ);
    if(!fs.isOpened()){
        AERROR << "failed to load!" << std::endl;
        return false;
    }

    do{
        std::string mode_ = std::string(fs["RUN_MODE"]);
        if(mode_.compare("train") == 0) mode = MODE_TRAIN;
        else mode = MODE_DETECT; 

        threshold = fs["DETECT_THRESHOLD"];
        if(threshold <= 0 ) threshold = 0.2;

        nms_value = fs["NMS_VALUE"];
        nms_value = MAX(0, nms_value);

        std::string net_config  = std::string(fs["NET_CFG"]);
        std::string net_weights = std::string(fs["NET_WEIGHT"]);
        char ch_net_config[256]; char ch_net_weights[256];
        strcpy(ch_net_config, net_config.c_str());
        strcpy(ch_net_weights, net_weights.c_str());
        net = load_network(ch_net_config, ch_net_weights, 0);
        if(!net){
            AERROR << "failed to load network!" << std::endl;
            break;
        }
        set_batch_network(net, 1);

        std::string path_cls = std::string(fs["FILE_CLASS"]);
        if(!load_cls_info(path_cls.c_str())) break;

        std::string path_imgs = std::string(fs["FILE_IMAGES"]);
        load_images_from_file(path_imgs.c_str());
        
        fs.release();
        return true;
    }while(false);

    fs.release();
    return false;
}

bool lib_darknet::load_cls_info(const char* cls_def){
    class_name.clear();

    AINFO << "loading class info from file " << cls_def << std::endl;
    std::ifstream file(cls_def);
    if(!file.is_open()){
        AERROR << "failed to load!" << std::endl;
        return false;
    }
    
    std::string line;
    while(std::getline(file, line)){
        line = line.substr(0, line.rfind("\n"));
        line = line.substr(0, line.rfind("\r"));
        class_name.push_back(line);
    }

    layer l = net->layers[net->n-1];
    if(l.classes != class_name.size()){
        AERROR << "number of classes not match (" << class_name.size() << " VS )" << l.classes << std::endl;
        return false;
    }

    AINFO << class_name.size() << " classes loaded!" << std::endl;
    return true;
}

void lib_darknet::load_images_from_file(const char* list_file){
    image_list.clear();

    AINFO << "loading image files from file " << list_file << std::endl;
    std::ifstream file(list_file);
    if(!file.is_open()){
        AERROR << "failed to load!" << std::endl;
        return;
    }
    
    std::string line;
    while(std::getline(file, line)){
        line = line.substr(0, line.rfind("\n"));
        line = line.substr(0, line.rfind("\r"));
        image_list.push_back(line);
    }

    AINFO << image_list.size() << " images loaded!" << std::endl;
}

image lib_darknet::mat_to_image(const cv::Mat& cv_img){
    int height   = cv_img.rows;
    int width    = cv_img.cols;
    int channels = cv_img.channels();
    CV_Assert(channels == 3);

    image im = make_image(width, height, channels);
    for(int row=0; row<height; ++row){
        const cv::Vec3b* ptr = cv_img.ptr<cv::Vec3b>(row);
        for(int col=0; col<width; ++col){
            for(int c=0; c<channels; ++c)
                im.data[row*width + col + c*width*height] = int(ptr[col][c])/255.0;
        }
    }
    return im;
}

cv::Mat lib_darknet::letterbox_resize(const cv::Mat& cv_img, const cv::Size& dstsz, cv::Rect2f& rate_shift){
    cv::Size shrink_sz;
    rate_shift.x = dstsz.width/float(cv_img.cols);
    rate_shift.y = dstsz.height/float(cv_img.rows);
    if(rate_shift.x < rate_shift.y){
        shrink_sz.width  = dstsz.width;
        shrink_sz.height = int(cv_img.rows*rate_shift.x);
    }
    else{
        shrink_sz.height = dstsz.height;
        shrink_sz.width  = int(cv_img.cols*rate_shift.y);
    }
    rate_shift.x = float(cv_img.cols)/shrink_sz.width;
    rate_shift.y = float(cv_img.rows)/shrink_sz.height;

    cv::Mat resized;
    cv::resize(cv_img, resized, shrink_sz);
    rate_shift.height = int((dstsz.height-shrink_sz.height)/2);
    rate_shift.width  = int((dstsz.width-shrink_sz.width)/2);
    cv::copyMakeBorder(resized, resized, rate_shift.height, rate_shift.height, rate_shift.width, rate_shift.width, 
                       cv::BORDER_CONSTANT, cv::Scalar::all(127));

    rate_shift.height *= rate_shift.y;
    rate_shift.width  *= rate_shift.x;
    rate_shift.x      *= dstsz.width;
    rate_shift.y      *= dstsz.height;

    return resized;
}

void lib_darknet::detect(const cv::Mat& cv_img, std::vector<NET_RESULT>& res){
    res.clear();
    if(cv_img.empty()) return;

    cv::Mat cv_img_;
    cv::Rect2f rsz_rate(cv_img.cols,cv_img.rows,0,0);
#if 1
    cv::resize(cv_img, cv_img_, cv::Size(net->w, net->h));
#else
    // letterbox seems better
    cv_img_ = letterbox_resize(cv_img, cv::Size(net->w, net->h), rsz_rate);
#endif

    image im = mat_to_image(cv_img_);

    network_predict(net, im.data);

    int nboxes = 0;
    layer l = net->layers[net->n-1];
    int num_out = l.side*l.side*l.n;
    // AINFO << "l.side = " << l.side << ", l.n = " << l.n << ", num = " << num_out << std::endl;
    detection* dets = get_network_boxes(net, 1, 1, threshold, 0, 0, 0, &nboxes);
    // detection* dets = get_network_boxes(net, im.w, im.h, threshold, 0.5, 0, 1, &nboxes);
    // AINFO << "nboxs: " << nboxes << std::endl;

    // for yolov1 num_out>0
    // for yolov3 num_out=0
    if(num_out == 0) num_out = nboxes;

    if(nms_value) do_nms_sort(dets, num_out, l.classes, nms_value);
    generate_results(num_out, dets, rsz_rate, cv_img.size(), res);

    free_detections(dets, nboxes);
    free_image(im);
}

void lib_darknet::detect_batch(bool save_file){
    cv::Mat image;
    std::vector<NET_RESULT> res;
    for(int i=0; i<image_list.size(); ++i){
        AINFO << "[" << i+1 << "/" << image_list.size() << "]detecting " << image_list[i] << std::endl;
        image = cv::imread(image_list[i]);
        detect(image, res);
        
        std::string detect_info = "";
        for(int j=0; j<res.size(); ++j)
            detect_info += res[j].cls_name + "(" + std::to_string(res[j].confidence) + ") ";
        AINFO << detect_info << std::endl;

        cv::Mat drawed = draw_results(image, res);

        if(save_file){
            size_t pos = image_list[i].rfind(".");
            std::string path_save = image_list[i].substr(0, pos) + "_drawed";
            path_save += image_list[i].substr(pos);
            cv::imwrite(path_save, drawed);
            AINFO << "saved to " << path_save << std::endl;
        }

        cv::resize(drawed, drawed, cv::Size(), 0.25, 0.25);
        // cv::imshow("result", drawed);
        // cv::waitKey(10);
    }
}

void lib_darknet::generate_results(int lay_num, const detection* dets, const cv::Rect2f& rate_shift, 
                                  const cv::Size& imsz, std::vector<NET_RESULT>& res){
    res.clear();
    
    int classes = class_name.size();

    for(int i=0; i<lay_num; ++i){
        for(int j=0; j<classes; ++j){
            if(dets[i].prob[j] > threshold){
                NET_RESULT res_;
                res_.cls_name = class_name[j];
                res_.confidence = dets[i].prob[j];

                box b = dets[i].bbox;
                // AINFO << "box: " << b.x << " " << b.y << " " << b.w << " " << b.h << std::endl;
                cv::Rect rect(int((b.x-b.w/2.0)*rate_shift.x-rate_shift.width), int((b.y-b.h/2.0)*rate_shift.y-rate_shift.height),
                             int(b.w*rate_shift.x), int(b.h*rate_shift.y));
                rect.x = MAX(0, rect.x);
                rect.y = MAX(0, rect.y);
                rect.width  = MIN(imsz.width-rect.x, rect.width);
                rect.height = MIN(imsz.height-rect.y, rect.height);
                res_.pos = rect;

                res.push_back(res_);
            }
        }
    }
}

cv::Mat lib_darknet::draw_results(const cv::Mat image, const std::vector<NET_RESULT>& res){
    cv::Mat drawed = image.clone();
    int thickness_ = int(0.008*image.rows);
    for(int i=0; i<res.size(); ++i){
        cv::Point pt1 = res[i].pos.tl();
        cv::Point pt2 = res[i].pos.br();
        cv::rectangle(drawed, pt1, pt2, cv::Scalar(0,255,0), thickness_);

        int h_label = int(0.03*image.rows);
        pt1.x = MAX(0, pt1.x-thickness_/2);
        pt2.x = MIN(image.cols-1, pt2.x+thickness_/2);
        pt1.y = MAX(0, pt1.y-h_label);
        pt2.y = pt1.y + h_label - thickness_/2;
        cv::rectangle(drawed, pt1, pt2, cv::Scalar(125,125,125), -1); 
        cv::putText(drawed, res[i].cls_name, cv::Point(pt1.x, pt2.y), cv::FONT_HERSHEY_SIMPLEX, 
                    0.001*image.rows, cv::Scalar(0,0,0), 0.4*thickness_);
    }
    return drawed;
}