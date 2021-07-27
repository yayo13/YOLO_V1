import os
from os import listdir, getcwd
from os.path import join
import random

if __name__ == '__main__':
    source_folder='/Users/mayuan/Desktop/20210714-佛山群创仓库AGV/'
    dest='/Users/mayuan/Desktop/20210714-佛山群创仓库AGV/train.txt'
    dest2='/Users/mayuan/Desktop/20210714-佛山群创仓库AGV/val.txt'
    file_list=os.listdir(source_folder)
    train_file=open(dest,'a')
    val_file=open(dest2,'a')

    file_name_list = []
    for file_obj in file_list:
        file_path=os.path.join(source_folder,file_obj)
        file_name,file_extend=os.path.splitext(file_obj)
        if file_extend != '.xml':
            continue
        file_name_list.append(file_name)
    
    random.shuffle(file_name_list)

    train_num = int(len(file_name_list)*0.8)
    val_num   = len(file_name_list)-train_num

    for i in range(train_num):
        train_file.write(file_name_list[i]+'\n')
    for i in range(val_num):
        val_file.write(file_name_list[train_num+i]+'\n')
train_file.close()
val_file.close()