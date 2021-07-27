#include "libDarknet.hpp"
#include "log.h"

int main(int argc, char* argv[]){
    init_log(argc, argv, "darknet_lib");

    lib_darknet darknet;
    if(!darknet.init()) return 0;
    darknet.detect_batch(true);

    return 0;
}