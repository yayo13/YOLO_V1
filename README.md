- 原版darknet，用于训练和生成第三方调用的库  

# 训练

1. 创建如下目录树  
|--VOCdevkit  
|--|--VOC2019  
|--|--|--Annotations  
|--|--|--ImageSets  
|--|--|--JPEGImages  
|--|--|--labels  

2. 将所有待训练的图片放到JPEGImages下，将labelImg输出的所有标注结果（xml）放到Annotations下  

3. 使用scripts/xml2txt.py生成文件列表2019_train.txt和2019_val.txt，将其放到ImageSets/Main下  

4. 使用scripts/voc_label.py根据上一步生成的文件列表，解析对应的xml，生成标注参数文件，自动保存在labels下  

5. 修改examples/yolo.c  
5.1 首先修改char *voc_names[]内容为自定义的类别  
5.2 然后修改char *train_images为第4步生成的train.txt路径，修改char *backup_directory为模型保存路径  

6. 第5步没有改变类别数，仅修改某类的名称，也可以修改类别数（待验证）  
参考https://blog.csdn.net/sinat_30071459/article/details/53100791和https://docs.khadas.com/zh-cn/vim3/HowToTrainYolo.html  

7. python train.sh开始训练  

# 库调用
make会生成libdarknet.so，将其连同include/darknet.h拷贝到其他工程可实现调用