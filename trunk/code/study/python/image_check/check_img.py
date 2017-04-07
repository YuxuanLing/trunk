#!/usr/bin/python
import sys
import os
import _is
from collections import namedtuple
from PIL import Image


class Nude(object):
    Skin = namedtuple("Skin","id skin region x y")


    def __init__(self, path_or_image):
        if isinstnce(path_or_image, Image.Image):
            self.image = path_or_image
        elif isinstance(path_or_omage,str):
            self.image = Image.open(path_or_image)


    #获取图片所有颜色通道，RBG HEV 还是YUV
    bands = self.image.getbands()
    #判断是否为单通道，如果是则转换为RGB
    if len(bands) == 1:
        new_img = Image.new("RGB",self.image.size)
        #拷贝到新图，PIL自动颜色通道转换
        new_img.paste(self.image)
        f = self.image.filename
        #替换self.image
        self.image = new_img
        self.image.filename = f


