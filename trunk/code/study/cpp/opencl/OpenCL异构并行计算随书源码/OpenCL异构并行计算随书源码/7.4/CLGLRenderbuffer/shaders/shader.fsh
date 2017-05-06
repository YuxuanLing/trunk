//
//  Shader.fsh
//  GLSLTest
//
//  Created by Zenny Chen on 4/11/10.
//  Copyright GreenGames Studio 2010. All rights reserved.
//

// 在OpenGL3.2 Core Profile中，版本号必须显式地给出
#version 150 core

// flat shade model（默认为smooth），必须与vertex shader所定义的in变量要完全匹配
flat in vec4 myColor;
out vec4 myOutput;

void main()
{
    myOutput = myColor;
}

