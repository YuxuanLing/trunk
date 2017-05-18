本目录主要是放一些ffmpeg和SDL学习用的code, base on (leixiaohua1020) http://blog.csdn.net/leixiaohua1020,斯人已逝留个纪念也好,已经全部重新编译通过vs2015,
请确保SDL的lib和dll编译的VS版本一致,如果不一致可以用相应的VS版本编译SDL进行库的替换

关于ffmpeg在window下的编译生成库函数的话有几点注意
1 一定要用官方的release的版本，不要用git上clone下来的，会有很多奇怪的编译问题
2 编译的时候需要安装msys或者cygwin, 通过相应VS的promot cmd 执行mays.bat 确保cl.exe 和 link.exe版本正确
3 如果出现重复定义这种错，在相应的.c上加入定义WIN32_LEAN_AND_MEAN试试看
4 $ ./configure --toolchain=msvc --arch=x86_64 --disable-yasm --enable-shared --enable-static
   ./configure --toolchain=msvc --arch=x86 --disable-yasm --enable-shared --enabe-static --enable-postproc --enable-avresample --enable-avdevice --enable-avcodec --enable-avformat --enable-swscale --enable-avfilter
   find ./ -name "*.lib" -exec cp {} g:/ \;  拷贝出来


由于我将所有的需要用的dll都放在一个目录下，但是VS搜索Dll的时候是搜的PATH下的路径，所以需要将dll目录路径加到PATH里去
可以定义一个LOCAL_DLLS变量然后将他加入到PATH，注意PATH的长度是有限制的，大概在2048个字符，如果太长了会被截断，用path命令看一下加进去的路径是否生效，重启下VS

关于指定DLL目录的方法，网上有如下介绍，但是第三种我试过对于VS2015好像不行

方法如下
在调试 Visual Studio 2008 程序时，经常有一些动态链接库（即 dll 文件）需要加载到工程里，这样才能依赖第三方库进行程序调试。 

这些动态链接库，往往都是测试版本或是开发中的版本，或者会有若干个版本；这个时候，如果直接把 dll 所在目录加到 PATH 里，则会有潜在冲突的危险；如果直接拷贝到 Visual Studio 的目录下，假如测试工程太多，每次有新版本的动态链接库更新时，你需要更新若干次，拷贝、粘贴苦不堪言。 

在开发过程中，究竟怎样来让 Visual Studio 链接这些 lib 及 dll 文件会比较好呢？ 

总体上来说，有几种方法可以改变 Visual Studio 的环境变量设置： 
直接添加到系统的 PATH 变量里： 

这个方法最简单，也最直接，但是坏处是会影响全局的 PATH 设置，尤其是你包含着大量测试用的 dll 时。 
在 Visual Studio 全局设置里，把 dll 所在目录添加到 PATH 里： 

通过 Visual Studio 菜单 ==> 工具 ==> 选项 ==> 项目和解决方案 ==> VC++目录，在下拉框里选择"可执行文件"，然后把 dll 所在路径添加进去。 
直接把所有 dll 拷贝到 Visual Studio 工程目录下，或是拷贝到生成可执行文件的文件夹（默认情况下是 Debug 或 Release 目录）下： 

这个方法也很简单，但是当你有若干个工程时，你每次更新 SDK 及其 dll 文件，你就要把所有的工程都更新，这个不符合文件唯一性的工程性准则。 
在调试程序时，让 Visual Studio 帮你切换当前工作目录到 dll 相应的目录下： 

在 Visual Studio ==> Project ==> Properties ==> Select Configuration ==> Configuration Properties ==> Debugging ==> Working directory 里填上 dll 所在目录，这样当在调试程序时，Visual Studio 会把当前工作目录切换到这个目录下，从而会自动读取本目录下的 dll 文件。 

这个方法的优点很明显，简单！副作用也很明显，在你切换了当前工作目录后，你可能会找不到程序的配置文件，在程序里写的诸如"./config.ini"全部都找不到了；另外，你要把所有的 dll 都放到这个工作目录里，否则一样会提示说找不到 xxx.dll 的问题。 
最后一个方法，也是我认为最好的一个方法，在 Visual Studio 工程属性里把一个目录临时添加到 PATH 环境变量里： 

MSDN 上也有类似的介绍：How to: Set Environment Variables for Projects，方法很简单，在 "工程属性" ==> "调试" ==> "环境"里，添加类似如下所示的内容： 
PATH=%PATH%;$(TargetDir)\DLLS 

这样就可以把 $(TargetDir)\DLLS 临时添加到该工程所属的系统 PATH 里。 

大家可以根据项目的实际情况，灵活选用以上方法。 
https://my.oschina.net/u/243648/blog/62847