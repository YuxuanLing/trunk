#!/usr/bin/python
import sys
import os
import _is
from collections import namedtuple
from PIL import Image


class Nude(object):
    Skin = namedtuple("Skin","id skin region x y")


    def __init__(self, path_or_image):
        if isinstne(path_or_image, Image.Image):
            self.image = path_or_image
        elif isinstane(path_or_omage,str):
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

    #存储对应图像所有像素的全部Skin 对象
    self.skin_map=[]
    self.detected_regions=[]
    self.merge_regions=[]
    self.skin_regions=[]
    self.last_from, self.last_to = -1, -1
    self.result = None
    self.mesage = None
    self.width, self.height = self.image.size
    self.total_pixels=self.width * self.height


def resize(self, maxwidth=1000,maxheight=1000):
    """
    基于最大宽高按比例重设图片大小
    """
    ret = 0
    if maxwidth:
        if self.width > maxwidth:
            wpercent = (maxwidth/self.width)
            hsize    = int((self.height*wpercent))
            fname = self.image.filename
            #LANCZOS 是重采样滤波器，用于抗锯齿
            self.image = self.image.resize((maxwidth,hsize), image.LANCZOS)
            self.image.filename = fname
            self.width, self.height = self.image.size
            self.total_pixels = self.width * self.height
            ret += 1

    if maxheight:
        if self.height > maxheight:
            hpercent = (maxheight/self.height)
            wsize    = int((self.width*hpercent))
            fname = self.image.filename
            #LANCZOS 是重采样滤波器，用于抗锯齿
            self.image = self.image.resize((wsize,maxheight), image.LANCZOS)
            self.image.filename = fname
            self.width, self.height = self.image.size
            self.total_pixels = self.width * self.height
            ret += 2

    return ret

def parse(self):

