//
//  MyGLView.m
//  OpenGLShaderBasic
//
//  Created by Zenny Chen on 10/4/10.
//  Copyright 2010 GreenGames Studio. All rights reserved.
//

#import "MyGLView.h"

#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED

// 这里必须注意！<gl3.h>头文件必须被包含并取代<gl.h>，否则VAO接口会调用不正常，从而无法正确显示图形！
#import <OpenGL/gl3.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

@interface MyGLView()
{
@private
    
    GLuint mProgram;
    GLuint mFBO, mRBO;
    GLuint mVAO, mVBOVertices, mVBOColors;
    GLint mSamplerLocation;
    GLuint mVAOForDraw, mVBOVertForDraw, mVBOTextureCoords;
    GLuint mTexName;
    
    GLsizei mRenderWidth, mRenderHeight;
    
    NSInteger mTag;
}

@end


@implementation MyGLView


static GLuint CompileShader(GLenum type, const char *filename)
{
	FILE *fp = fopen(filename, "r");
    if(fp == NULL)
    {
        printf("File %s cannot be opened!", filename);
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    const size_t length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    GLchar *souceBuffer = malloc(length);
    fread(souceBuffer, 1, length, fp);
    fclose(fp);
    
    const GLchar *source = souceBuffer;
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, (GLint[]){ (int)length });
    glCompileShader(shader);
    free(souceBuffer);
	
	GLint logLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = malloc(logLength);
        glGetShaderInfoLog(shader, logLength, &logLength, log);
        printf("Shader compile log:\n%s\n", log);
        free(log);
    }
    
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if(status == 0)
	{
		glDeleteShader(shader);
		return 0;
	}
	
	return shader;
}

static bool LinkProgram(GLuint prog)
{
	glLinkProgram(prog);
    
	GLint logLength;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        printf("Program link log:\n%s\n", log);
        free(log);
    }
    
    GLint status;
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (status == 0)
		return false;
	
	return true;
}

static bool ValidateProgram(GLuint prog)
{
	GLint logLength, status;
	
	glValidateProgram(prog);
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        printf("Program validate log:\n%s\n", log);
        free(log);
    }
    
    glGetProgramiv(prog, GL_VALIDATE_STATUS, &status);
    if (status == 0)
		return false;
	
	return true;
}

- (BOOL)loadShaders:(NSString*)shaderName
{
    // 创建着色器程序对象
    mProgram = glCreateProgram();
    
    // 在做程序连接之前绑定顶点着色器中属性的位置
    glBindAttribLocation(mProgram, 0, "inPos");
    
    [shaderName isEqualToString:@"shader"]? glBindAttribLocation(mProgram, 1, "inColor") : glBindAttribLocation(mProgram, 1, "inTexCoords");
	
    // 创建并编译顶点着色器
	NSString *vertShaderPathname = [[NSBundle mainBundle] pathForResource:shaderName ofType:@"vsh"];
	GLuint vertShader = CompileShader(GL_VERTEX_SHADER, [vertShaderPathname UTF8String]);
    if(vertShader == 0)
	{
		NSLog(@"Failed to compile vertex shader");
		return FALSE;
	}
	
    // 创建并编译片段着色器
	NSString *fragShaderPathname = [[NSBundle mainBundle] pathForResource:shaderName ofType:@"fsh"];
    GLuint fragShader = CompileShader(GL_FRAGMENT_SHADER, [fragShaderPathname UTF8String]);
	if(fragShader == 0)
	{
		NSLog(@"Failed to compile fragment shader");
		return FALSE;
	}
    
    // 将顶点着色器添加到程序中
    glAttachShader(mProgram, vertShader);
    
    // 将片段着色器添加到程序中
    glAttachShader(mProgram, fragShader);
    
    // 连接程序
	if (!LinkProgram(mProgram))
	{
		NSLog(@"Failed to link program: %d", mProgram);
		return FALSE;
	}
    
    // 如果当前着色器名为draw，则获取片段着色器中采样器uniform变量的位置
    if([shaderName isEqualToString:@"draw"])
        mSamplerLocation = glGetUniformLocation(mProgram, "texSampler");
    
    // 这里顶点着色器对象以及片段着色器对象已经没用了，将它们释放
    if(vertShader != 0)
		glDeleteShader(vertShader);
    if(fragShader != 0)
		glDeleteShader(fragShader);
	
    // 校验程序
	return ValidateProgram(mProgram);
}

- (id)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];
    
    const NSOpenGLPixelFormatAttribute attrs[] =
	{
        NSOpenGLPFADoubleBuffer,    // 可选地，可以使用双缓冲
		NSOpenGLPFAOpenGLProfile,   // 必须使用这个属性以指定我们将使用OpenGL Core Profile
		NSOpenGLProfileVersion3_2Core,  // 指定使用OpenGL3.2 Core Profile
        
        // 这里使用多重采样反走样处理
        NSOpenGLPFAMultisample,
        NSOpenGLPFASampleBuffers, (NSOpenGLPixelFormatAttribute)1,
        NSOpenGLPFASamples, (NSOpenGLPixelFormatAttribute)4,    // 采用4个样本对应一个像素
        
        // end
		0
	};

	NSOpenGLPixelFormat *pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
	
	if (pf == nil)
	{
		NSLog(@"No OpenGL pixel format");
        return nil;
	}
    
    NSOpenGLContext* context = [[NSOpenGLContext alloc] initWithFormat:pf shareContext:nil];
    
    [self setPixelFormat:pf];
    [pf release];
    
    [self setOpenGLContext:context];
    [context release];
    
    return self;
}

- (void)dealloc
{
    NSLog(@"MyGLView deallocated!");
    
    [super dealloc];
}

- (void)destroyBuffers
{
    // 清除帧缓存对象
    if(mFBO != 0)
    {
        glDeleteFramebuffers(1, &mFBO);
        mFBO = 0;
    }
    
    // 清除渲染缓存对象
    if(mRBO != 0)
    {
        glDeleteFramebuffers(1, &mRBO);
        mRBO = 0;
    }
    
    // 释放程序对象
    if(mProgram != 0)
    {
        glDeleteProgram(mProgram);
        mProgram = 0;
    }
    
    // 释放VAO对象
    if(mVAO != 0)
    {
        glDeleteVertexArrays(1, &mVAO);
        mVAO = 0;
    }
    if(mVAOForDraw != 0)
    {
        glDeleteVertexArrays(1, &mVAOForDraw);
        mVAOForDraw = 0;
    }
    
    // 释放顶点与颜色VBO
    if(mVBOVertices != 0)
    {
        glDeleteBuffers(1, &mVBOVertices);
        mVBOVertices = 0;
    }
    if(mVBOColors != 0)
    {
        glDeleteBuffers(1, &mVBOColors);
        mVBOColors = 0;
    }
    if(mVBOVertForDraw != 0)
    {
        glDeleteBuffers(1, &mVBOVertForDraw);
        mVBOVertForDraw = 0;
    }
    if(mVBOTextureCoords != 0)
    {
        glDeleteBuffers(1, &mVBOTextureCoords);
        mVBOTextureCoords = 0;
    }
    
    // 释放纹理对象
    if(mTexName != 0)
    {
        glDeleteTextures(1, &mTexName);
        mTexName = 0;
    }
    
    // 清除上下文
    [[self openGLContext] clearDrawable];
    
    [self clearGLContext];
}

- (void)setTag:(NSInteger)tag
{
    mTag = tag;
}

- (NSInteger)tag
{
    return mTag;
}

- (void)prepareOpenGL
{
	[[self openGLContext] makeCurrentContext];
    
    mRenderWidth = self.frame.size.width;
    mRenderHeight = self.frame.size.height;
	
	// 用垂直刷新率来同步缓存交换
	GLint swapInt = 1;
	[[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
    
    // 创建FBO
    glGenFramebuffers(1, &mFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    
    // 创建RBO
    glGenRenderbuffers(1, &mRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, mRBO);
    
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, mRenderWidth, mRenderHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mRBO);
    
    // 测试帧缓存完成
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Failed to make complete framebuffer object %x\n", glCheckFramebufferStatus(GL_FRAMEBUFFER));
        return;
    }
    
    // 在OpenGL3.2 Core Profile中，必须使用VAO（顶点数组对象）
    glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);
    
    // 这里要绘制一个圆形，因此需要362个顶点,
    // 每个顶点分配4个分量（分别为x, y, z坐标与w）
    // 最后扩充到512个顶点，以优化OpenCL的数据处理
    const size_t dataLength = 512 * 4 * sizeof(GLfloat);
    
    // 设置顶点VBO
    glGenBuffers(1, &mVBOVertices);
    glBindBuffer(GL_ARRAY_BUFFER, mVBOVertices);
    // 初始化顶点VBO，这里仅分配空间，而不传递任何数据
    glBufferData(GL_ARRAY_BUFFER, dataLength, NULL, GL_STATIC_DRAW);
    
    GLenum errCode =  glGetError();
    if(errCode != GL_NO_ERROR)
        puts("Buffer data vertices error!");
    
    // 将顶点VBO绑定到属性0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);
    
    // 设置颜色VBO
    glGenBuffers(1, &mVBOColors);
    glBindBuffer(GL_ARRAY_BUFFER, mVBOColors);
    // 初始化颜色VBO，这里仅分配空间，不传递任何数据
    glBufferData(GL_ARRAY_BUFFER, dataLength, NULL, GL_STATIC_DRAW);
    // 将颜色VBO绑定到属性1
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);
    
    // 解绑
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // 加载着色器并构建OpenGL程序
    if(![self loadShaders:@"shader"])
        return;
    
    glUseProgram(mProgram);
    
    glViewport(0, 0, mRenderWidth, mRenderHeight);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
}

- (void)doOpenCLComputing
{
    /** 做OpenCL初始化 */
    cl_platform_id oclPlatform = NULL;
    cl_device_id oclDevice = NULL;
    cl_context context = NULL;       // 要被创建的OpenCL上下文对象
    cl_command_queue commandQueue = NULL;
    cl_program oclProgram = NULL;
    cl_kernel kernel = NULL;
    cl_mem  memObjVertices = NULL;  // 要被创建的与GL共享的顶点缓存对象
    cl_mem  memObjColors = NULL;    // 要被创建的与GL共享的颜色缓存对象
    
#ifdef __APPLE__
    
    CGLContextObj cgl_context = CGLGetCurrentContext();
    CGLShareGroupObj sharegroup = CGLGetShareGroup(cgl_context);
    gcl_gl_set_sharegroup(sharegroup);

#endif
    
    do
    {
        // 获得当前OpenCL平台
        cl_int status = clGetPlatformIDs(1, &oclPlatform, NULL);
        if(status != CL_SUCCESS)
        {
            NSLog(@"OpenCL platform get failed!");
            break;
        }
        // 获得当前GPU设备。严格地来说，此GPU设备也应该是OpenGL所使用的设备
        status = clGetDeviceIDs(oclPlatform, CL_DEVICE_TYPE_GPU, 1, &oclDevice, NULL);
        if(status != CL_SUCCESS)
        {
            NSLog(@"OpenCL GPU cannot be found!");
            break;
        }
        
        // 设置用于创建OpenCL上下文的属性列表
        cl_context_properties properties[] = {
            
#ifdef WIN32
            CL_GL_CONTEXT_KHR , (cl_context_properties)wglGetCurrentContext(),
            CL_WGL_HDC_KHR , (cl_context_properties)wglGetCurrentDC(),
#endif
            
#ifdef __linux__
            CL_GL_CONTEXT_KHR , (cl_context_properties)glXGetCurrentContext(),
            CL_GLX_DISPLAY_KHR , (cl_context_properties)glXGetCurrentDisplay(),
#endif
            
#ifdef __APPLE__
            CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)sharegroup,
#endif
            
            0
        };
        
        // 创建OpenCL上下文
        context = clCreateContext(properties, 1, &oclDevice, NULL, NULL, NULL);
        
        // 创建命令队列
        commandQueue = clCreateCommandQueue(context, oclDevice, 0, NULL);
        
        // 编译内核程序
        NSString *kernelPath = [[NSBundle mainBundle] pathForResource:@"compute" ofType:@"ocl"];
        
        const char *aSource = [[NSString stringWithContentsOfFile:kernelPath encoding:NSUTF8StringEncoding error:nil] UTF8String];
        size_t kernelLength = strlen(aSource);
        oclProgram = clCreateProgramWithSource(context, 1, &aSource, &kernelLength, NULL);
        if(oclProgram == NULL)
        {
            NSLog(@"OpenCL program create failed!");
            break;
        }
        
        // 构建程序
        status = clBuildProgram(oclProgram, 1, &oclDevice, NULL, NULL, NULL);
        if(status != CL_SUCCESS)
        {
            NSLog(@"OpenCL kernel build failed!");
            break;
        }
        
        // 创建与GL顶点缓存对象共享的存储器对象
        memObjVertices = clCreateFromGLBuffer(context, CL_MEM_WRITE_ONLY, mVBOVertices, &status);
        
        // 创建与GL颜色缓存对象共享的存储器对象
        memObjColors = clCreateFromGLBuffer(context, CL_MEM_WRITE_ONLY, mVBOColors, &status);
        
        // 创建内核对象
        kernel = clCreateKernel(oclProgram, "GenerateRoundVertices", NULL);
        
        // 设置内核参数
        status |= clSetKernelArg(kernel, 0, sizeof(memObjVertices), &memObjVertices);
        status |= clSetKernelArg(kernel, 1, sizeof(memObjColors), &memObjColors);
        if(status != CL_SUCCESS)
        {
            NSLog(@"Kernel parameters pass failed!");
            break;
        }
        
        // 这里我们总共使用512个工作项
        // 由于一共要处理362个顶点，每个顶点对应到一个工作项，
        // 而362向上对应的能满足2的N次幂整数就是512
        size_t global_work_size[1] = { 512 };

        size_t groupSize;
        clGetDeviceInfo(oclDevice, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(groupSize), &groupSize, NULL);
        size_t local_work_size[1] = { groupSize };
        
        // 运行内核程序
        status |= clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);
        
        // 这里直接用clFinish进行同步，确保顶点坐标以及相应的颜色值全都设置好
        clFinish(commandQueue);
        
        if(status != CL_SUCCESS)
        {
            NSLog(@"OpenCL kernel run error!");
        }
    }
    while(NO);
    
    // 释放OpenCL各种对象
    if(memObjVertices != NULL)
        clReleaseMemObject(memObjVertices);
    if(memObjColors != NULL)
        clReleaseMemObject(memObjColors);
    
    if(kernel != NULL)
        clReleaseKernel(kernel);
    
    if(oclProgram != NULL)
        clReleaseProgram(oclProgram);
    
    if(commandQueue != NULL)
        clReleaseCommandQueue(commandQueue);
    
    if(context != NULL)
        clReleaseContext(context);
}

static GLfloat sVertexCoords[] = {
    
    // 左上顶点
    -1.0f, 1.0f, 0.0f, 1.0f,
    
    // 左下顶点
    -1.0f, -1.0f, 0.0f, 1.0f,
    
    // 右上顶点
    1.0f, 1.0f, 0.0f, 1.0f,
    
    // 右下顶点
    1.0f, -1.0f, 0.0f, 1.0f
};

static GLfloat sTextureCoords[] = {
    // 左上顶点
    0.0f, 1.0f,
    
    // 左下顶点
    0.0f, 0.0f,
    
    // 右上顶点
    1.0f, 1.0f,
    
    // 右下顶点
    1.0f, 0.0f
};

-(void)prepareForDraw
{
    glGenVertexArrays(1, &mVAOForDraw);
    glBindVertexArray(mVAOForDraw);
    
    // 设置顶点VBO
    glGenBuffers(1, &mVBOVertForDraw);
    glBindBuffer(GL_ARRAY_BUFFER, mVBOVertForDraw);
    // 将顶点坐标数据拷贝到mVBOVertices对象的缓存中
    glBufferData(GL_ARRAY_BUFFER, sizeof(sVertexCoords), sVertexCoords, GL_STATIC_DRAW);
    
    // 将顶点VBO绑定到属性0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);
    
    // 设置纹理坐标VBO
    glGenBuffers(1, &mVBOTextureCoords);
    glBindBuffer(GL_ARRAY_BUFFER, mVBOTextureCoords);
    // 将sTextureCoords中的数据拷贝到mVBOTextureCoords对象的缓存中
    glBufferData(GL_ARRAY_BUFFER, sizeof(sTextureCoords), sTextureCoords, GL_STATIC_DRAW);
    
    // 将纹理坐标VBO绑定到属性1
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);
    
    // 解绑
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // 设置纹理
    glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &mTexName);
    glBindTexture(GL_TEXTURE_2D, mTexName);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    // 这里只分配纹理大小及属性，而不对它进行图像数据的拷贝
    // 纹理的实际图像数据将在OpenCL内核程序中给出
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mRenderWidth, mRenderHeight, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
    
    if(mProgram != 0)
    {
        glDeleteProgram(mProgram);
        mProgram = 0;
    }
    
    // 加载着色器并构建OpenGL程序
    if(![self loadShaders:@"draw"])
        return;
    
    glUseProgram(mProgram);
    
    // 将采样器对象映射到GL_TEXTURE0，使得它对0号纹理单元进行采样
    glUniform1i(mSamplerLocation, 0);
    
    glViewport(0, 0, mRenderWidth, mRenderHeight);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

- (void)generateTextureWithCLMemFromRBO
{
    /** 做OpenCL初始化 */
    cl_platform_id oclPlatform = NULL;
    cl_device_id oclDevice = NULL;
    cl_context context = NULL;       // 要被创建的OpenCL上下文对象
    cl_command_queue commandQueue = NULL;
    cl_program oclProgram = NULL;
    cl_kernel kernel = NULL;
    cl_mem  dstMem = NULL;          // 从GL的RBO创建用于输出的CL存储器对象
    cl_mem  srcMem = NULL;          // 从GL的RBO创建用于输入的CL存储器对象
    cl_sampler sampler = NULL;      // 访问图像对象的采样器
    
#ifdef __APPLE__
    
    CGLContextObj cgl_context = CGLGetCurrentContext();
    CGLShareGroupObj sharegroup = CGLGetShareGroup(cgl_context);
    gcl_gl_set_sharegroup(sharegroup);
    
#endif
    
    do
    {
        // 获得当前OpenCL平台
        cl_int status = clGetPlatformIDs(1, &oclPlatform, NULL);
        if(status != CL_SUCCESS)
        {
            NSLog(@"OpenCL platform get failed!");
            break;
        }
        // 获得当前GPU设备。严格地来说，此GPU设备也应该是OpenGL所使用的设备
        status = clGetDeviceIDs(oclPlatform, CL_DEVICE_TYPE_GPU, 1, &oclDevice, NULL);
        if(status != CL_SUCCESS)
        {
            NSLog(@"OpenCL GPU cannot be found!");
            break;
        }
        
        // 设置用于创建OpenCL上下文的属性列表
        cl_context_properties properties[] = {
            
#ifdef WIN32
            CL_GL_CONTEXT_KHR , (cl_context_properties)wglGetCurrentContext(),
            CL_WGL_HDC_KHR , (cl_context_properties)wglGetCurrentDC(),
#endif
            
#ifdef __linux__
            CL_GL_CONTEXT_KHR , (cl_context_properties)glXGetCurrentContext(),
            CL_GLX_DISPLAY_KHR , (cl_context_properties)glXGetCurrentDisplay(),
#endif
            
#ifdef __APPLE__
            CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)sharegroup,
#endif
            
            0
        };
        
        // 创建OpenCL上下文
        context = clCreateContext(properties, 1, &oclDevice, NULL, NULL, NULL);
        
        // 创建命令队列
        commandQueue = clCreateCommandQueue(context, oclDevice, 0, NULL);
        
        // 编译内核程序
        NSString *kernelPath = [[NSBundle mainBundle] pathForResource:@"compute" ofType:@"ocl"];
        
        const char *aSource = [[NSString stringWithContentsOfFile:kernelPath encoding:NSUTF8StringEncoding error:nil] UTF8String];
        size_t kernelLength = strlen(aSource);
        oclProgram = clCreateProgramWithSource(context, 1, &aSource, &kernelLength, NULL);
        if(oclProgram == NULL)
        {
            NSLog(@"OpenCL program create failed!");
            break;
        }
        
        // 构建程序
        status = clBuildProgram(oclProgram, 1, &oclDevice, NULL, NULL, NULL);
        if(status != CL_SUCCESS)
        {
            NSLog(@"OpenCL kernel build failed!");
            break;
        }
        
        // 创建与GL纹理对象共享的CL图像存储器对象，用于纹理输出
        dstMem = clCreateFromGLTexture(context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, mTexName, NULL);
        
        // 从GL渲染缓存创建与CL共享的存储器对象，作为输入源
        srcMem = clCreateFromGLRenderbuffer(context, CL_MEM_READ_ONLY, mRBO, NULL);
        
        // 创建采样器对象
        sampler = clCreateSampler(context, CL_FALSE, CL_ADDRESS_CLAMP_TO_EDGE, CL_FILTER_LINEAR, NULL);
        
        // 创建内核对象
        kernel = clCreateKernel(oclProgram, "ImageProcessing", NULL);
        
        // 设置内核参数
        status = clSetKernelArg(kernel, 0, sizeof(dstMem), &dstMem);
        status |= clSetKernelArg(kernel, 1, sizeof(srcMem), &srcMem);
        status |= clSetKernelArg(kernel, 2, sizeof(cl_sampler), (void*)&sampler);
        if(status != CL_SUCCESS)
        {
            NSLog(@"Kernel parameters pass failed!");
            break;
        }
        
        // 用于跟踪获得GL对象的事件
        cl_event acquireEvt = NULL;

        // 获得从GL对象所创建的CL对象，等待GL绘制完成
        clEnqueueAcquireGLObjects(commandQueue, 2, (cl_mem[]){dstMem, srcMem}, 0, NULL, &acquireEvt);
        
        // 这里我们总共使用mImageWidth * mImageHeight个工作项，
        // 每个工作项来处理一个像素
        size_t global_work_size[] = { mRenderWidth, mRenderHeight };
        
        // 然后设置一个工作组中的工作项个数。
        // 要注意，x维度与y维度两个数相乘不能大于工作组中最多可容纳的工作项的个数
        size_t local_work_size[] = { 16, 16 };
        
        // 运行内核程序
        status |= clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL, global_work_size, local_work_size, 1, &acquireEvt, NULL);
        
        // 这里直接用clFinish进行同步，确保顶点坐标以及相应的颜色值全都设置好
        clFinish(commandQueue);
        
        // 调用过clEnqueueAcquireGLObjects之后必须调用此函数，
        // 并且在后续使用相关OpenGL对象之前完成调用
        clEnqueueReleaseGLObjects(commandQueue, 2, (cl_mem[]){dstMem, srcMem}, 0, NULL, NULL);
        
        clReleaseEvent(acquireEvt);
        
        if(status != CL_SUCCESS)
        {
            NSLog(@"OpenCL kernel run error!");
        }
    }
    while(NO);
    
    // 释放OpenCL各种对象
    if(dstMem != NULL)
        clReleaseMemObject(dstMem);
    if(srcMem != NULL)
        clReleaseMemObject(srcMem);
    
    if(sampler != NULL)
        clReleaseSampler(sampler);
    
    if(kernel != NULL)
        clReleaseKernel(kernel);
    
    if(oclProgram != NULL)
        clReleaseProgram(oclProgram);
    
    if(commandQueue != NULL)
        clReleaseCommandQueue(commandQueue);
    
    if(context != NULL)
        clReleaseContext(context);
}

- (void)drawRect:(NSRect)dirtyRect
{
    // 绑定设置好的帧缓存对象与渲染缓存对象
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, mRBO);
    
    // 准备在绑定的渲染缓存上绘制彩色的圆
    [self doOpenCLComputing];
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 362);
    
    // 这里将使用acquire接口进行同步，而不直接使用glFinish()
    //glFinish();
    
    // 准备将渲染缓存中的图像变为纹理，然后交给OpenCL做进一步的处理
    [self prepareForDraw];
    
    [self generateTextureWithCLMemFromRBO];
    
    // 使用默认帧缓存与渲染缓存，准备绘制
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glFlush();
    
    [[self openGLContext] flushBuffer];
}

@end

