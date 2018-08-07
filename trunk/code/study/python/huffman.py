#coding:utf-8
import sys
import six
import pdb 
##reload(sys) 
##sys.setdefaultencoding("utf-8")

class HuffNode(object):
    def get_weight(self):
        raise NotImplementedError(
            "The Abstract Node Class doesn't define 'get_weight'"
            )
    
    def is_leaf(self):
        raise NotImplementedError(
            "The Abstract Node Class doesn't define 'is_leaf'"
            )


class LeafNode(object):
    value = None
    weight = None
    def __init__(self,val=0,freq=0):
        super(LeafNode,self).__init__()
        self.value = val
        self.weight = freq
    def is_leaf(self):
        return True
    def get_weight(self):
        return self.weight
    def get_value(self):
        return self.value

class InterNode(HuffNode):
    weight = None
    left_child = None
    right_child = None
    def __init__(self,l_child=None, r_child=None):
        super(InterNode,self).__init__()
        self.weight = l_child.get_weight() + r_child.get_weight()
        self.left_child = l_child
        self.right_child = r_child
    def is_leaf(self):
        return False
    def get_weight(self):
        return self.weight
    def get_left(self):
        return self.left_child
    def get_right(self):
        return self.right_child
    

class HuffTree(object):
    root = None
    def __init__(self,flag,val=0,freq=0,l_tree=None,r_tree=None):
        super(HuffTree,self).__init__()
        if flag == 0:
            self.root = LeafNode(val,freq)
        else:
            self.root = InterNode(l_tree.get_root(),r_tree.get_root())
    def get_root(self):
        return self.root
    def get_weight(self):
        return self.get_root().get_weight()
    def traverse_huffman_tree(self,root,code,char_freq):
        """
        递归遍历
        """
        if root.is_leaf():
            char_freq[root.get_value()] = code
            print("it = %d/%c and freq = %d code = %s")%(root.get_value(),
                   chr(root.get_value()), root.get_weight(), code)
            return None
        else:
            self.traverse_huffman_tree(root.get_left(), code + '0', char_freq)
            self.traverse_huffman_tree(root.get_right(),code + '1', char_freq)

def buildHuffmanTree(list_hufftrees):
    print list_hufftrees[0].get_weight()
    while len(list_hufftrees) > 1:
        list_hufftrees.sort(key=lambda x:x.get_weight())
        tmp1 = list_hufftrees[0]            
        tmp2 = list_hufftrees[1]
        list_hufftrees = list_hufftrees[2:]
        new_hufftree = HuffTree(1, 0, 0, tmp1, tmp2)
        list_hufftrees.append(new_hufftree)

    return list_hufftrees[0]

if __name__ == '__main__':
    # 获取用户的输入
    if len(sys.argv) != 2:
        print "please input inputfilename "
        exit(0)
    else:
        infile = sys.argv[1]

    #1. 以二进制的方式打开文件 
    f = open(infile,'rb')
    filedata = f.read()
    # 获取文件的字节总数
    filesize = f.tell()

    # 2. 统计 byte的取值［0-255］ 的每个值出现的频率
    # 保存在字典 char_freq中
    char_freq = {}
    for x in range(filesize):
        tem = six.byte2int(filedata[x])
        if tem in char_freq.keys():
            char_freq[tem] = char_freq[tem] + 1
        else:
            char_freq[tem] = 1

    # 输出统计结果
    for tem in char_freq.keys():
        print tem,' : ',char_freq[tem]


    # 3. 开始构造原始的huffman编码树 数组，用于构造Huffman编码树
    list_hufftrees = []
    for x in char_freq.keys():
        # 使用 HuffTree的构造函数 定义一棵只包含一个叶节点的Huffman树
        tem = HuffTree(0, x, char_freq[x], None, None)
        # 将其添加到数组 list_hufftrees 当中
        list_hufftrees.append(tem)
    print list_hufftrees[0].get_weight()
    #print dir(list_hufftrees[0])
     # 5. 构造huffman编码树，并且获取到每个字符对应的 编码并且打印出来
    tem = buildHuffmanTree(list_hufftrees)
    tem.traverse_huffman_tree(tem.get_root(),'',char_freq)