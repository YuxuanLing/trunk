#!/usr/bin/python
import sys
import os
import _is
from collections import namedtuple
from PIL import Image


class Nude(object):
    Skin = namedtuple("Skin","id skin region x y")


    def __init__(self, path_or_image):
        if isinstance(path_or_image, Image.Image):
            self.image = path_or_image
        elif isinstance(path_or_image,str):
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
        #
        if self.result is not None:
            return self

        pixels = self.image.load()

        for y in range(self.height):
            for x in range (self.width):
                r =  pixels[x,y][0]
                g =  pixels[x,y][1]
                b =  pixels[x,y][2]

                isSkin = True if self._classfiy_skin(r,g,b) else False
                _id = x + y*self.width + 1
                self.skin_map.append(self.Skin(_id, isSkin, None, x, y))
                if not isSkin:
                    continue

                check_indexes = [_id - 2,   #left
                                 _id - self.width - 2, #topleft
                                 _id - self.width - 1,     #top
                                 _id - self.width] #top right
                region = -1
                for index in check_indexes:
                    try:
                        self.skin_map[index]
                    except IndexError:
                        break

                    if self.skin_map[index].skin:
                        #如果相邻像素与当前像素的region都是有效的，且两者不同，
                        #且没有添加相同的合并任务
                        if (self.skin_map[index].region != None and
                                region != None and region != -1 and
                                self.skin_map[index].region != region and
                                self.last_from != region and
                                self.last_to != self.skin_map[index].region):
                            #添加这两个区域的合并任务
                            self._add_merge(region,self.skin_map[index].region)
                        region = self.skin_map[index].region

                    if region == -1:
                        #更改属性为新的区域号，注意元祖是不可变类型，不能直接改属性
                        _skin = self.skin_map[_id - 1]._replace(region=len(self.detected_regions))
                        self.skin_map[_id - 1]=_skin
                        #将此肤色像素所在区域创建为新区域
                        self.detected_regions.append([self.skin_map[_id]])
                    elif region != None:
                        #将此像素的区域号更改为与相邻的像素相同
                        _skin=self.skin_map[_idi - 1]._replace(region=region)
                        self.skin_map[_id - 1]=_skin
                        #向这个区域的像素列表中添加此像素
                        self.detected_regions[region].append(self.skin_map[_id])

        #完成所有区域的合并任务，合并整理后的区域存储到self.skin_regions
        self._merge(self.detected_regions, self.merge_regions)
        #分析皮肤区域，得到结果
        self._analyse_regions()
        return self

    # 基于像素的肤色检测技术
    def _classify_skin(self, r, g, b):
        # 根据RGB值判定
        rgb_classifier = r > 95 and \
                         g > 40 and g < 100 and \
                         b > 20 and \
                         max([r, g, b]) - min([r, g, b]) > 15 and \
                         abs(r - g) > 15 and \
                         r > g and \
                         r > b
        # 根据处理后的 RGB 值判定
        nr, ng, nb = self._to_normalized(r, g, b)
        norm_rgb_classifier = nr / ng > 1.185 and \
                              float(r * b) / ((r + g + b) ** 2) > 0.107 and \
                              float(r * g) / ((r + g + b) ** 2) > 0.112
        # HSV 颜色模式下的判定
        h, s, v = self._to_hsv(r, g, b)
        hsv_classifier = h > 0 and \
                         h < 35 and \
                         s > 0.23 and \
                         s < 0.68
        # YCbCr 颜色模式下的判定
        y, cb, cr = self._to_ycbcr(r, g,  b)
        ycbcr_classifier = 97.5 <= cb <= 142.5 and 134 <= cr <= 176
        # 效果不是很好，还需改公式
        return rgb_classifier   \
               or norm_rgb_classifier  \
               or hsv_classifier or    \
               ycbcr_classifier
        #return ycbcr_classifier


    def _to_normalized(self, r, g, b):
        if r == 0:
           r = 0.0001
        if g == 0:
           g = 0.0001
        if b == 0:
           b = 0.0001
        _sum = float(r + g + b)
        return [r / _sum, g / _sum, b / _sum]

    def _to_ycbcr(self, r, g, b):
        # 公式来源：
        # http://stackoverflow.com/questions/19459831/rgb-to-ycbcr-conversion-problems
        y = .299*r + .587*g + .114*b
        cb = 128 - 0.168736*r - 0.331364*g + 0.5*b
        cr = 128 + 0.5*r - 0.418688*g - 0.081312*b
        return y, cb, cr


    def _to_hsv(self, r, g, b):
        h = 0
        _sum = float(r + g + b)
        _max = float(max([r, g, b]))
        _min = float(min([r, g, b]))
        diff = float(_max - _min)
        if _sum == 0:
           _sum = 0.0001

        if _max == r:
           if diff == 0:
              h = sys.maxsize
           else:
              h = (g - b) / diff
        elif _max == g:
          h = 2 + ((g - r) / diff)
        else:
          h = 4 + ((r - g) / diff)

        h *= 60
        if h < 0:
           h += 360

        return [h, 1.0 - (3.0 * (_min / _sum)), (1.0 / 3.0) * _max]

    def _add_merge(self, _from, _to):
        self.last_from = _from
        self.last_to   = _to
        from_index = -1
        to_index = -1
        for index,region in enumerate(self.merge_regions):
            for r_index in region:
                if r_index == _from:
                   from_index = index
                if r_index == _to:
                   to_index = index
        if from_index == -1 and to_index == -1:
            self.merge_regions.append([_from,_to])  #创建新的区域号列表
            return
        #两个区域号都在的话合并这两个区域号，注意这里用extend合并列表
        if from_index != -1 and to_index != -1:
            if from_index != to_index:
                self.merge_regions[from_index].extend(self.merge_regions[to_index])
                del(self.merge_regions[to_index])
            return
        #其中一个在一个不在merge_regions中，则将不存在于merge_regions中的区域号
        #添加到存在的列表中 用append
        if from_index != -1 and to_index == -1:
            self.merge_regions[from_index].append(_to)
            return
        if from_index == -1 and to_index != -1:
            self.merge_regions[to_index].append(_from)
            return


    #合并该合并的皮肤区域
    def _merge(self, detected_regions, merge_regions):
        new_detected_regions =[]
        #将merge_regions 中的元素中的区域号代表皮肤的所有区域合并i
        #merge_regions中原来有多少个区域合并之后也有多少个区域，即便元素列表为空
        for index,region in enumerate(merge_regions):
            try:
                new_detected_regions[index]
            except IndexError:
                new_detected_regions.append([])
            for r_index in region:
                new_detected_regions[index].extend(detected_regions[r_index])
                detected_regions[r_index]=[]

        #添加剩下的其余皮肤区域到 new_detected_regions
        for region in detected_regions:
            if len(region) > 0:
                new_detected_regions.append(region)

        #清理
        self._clear_regions(new_detected_regions)



