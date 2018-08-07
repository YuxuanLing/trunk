//
//  Shader.vsh
//  GLSLTest
//
//  Created by Zenny Chen on 4/11/10.
//  Copyright GreenGames Studio 2010. All rights reserved.
//

// 在OpenGL3.2 Core Profile中，版本号必须显式地给出
#version 150 core

in vec4 inPos;
in vec4 inColor;

// flat shade model（默认为smooth）
flat out vec4 myColor;

/** 模型视图变换矩阵 *
 * [ 1  0  0  0
     0  1  0  0
     0  0  1  0
     x  y  z  1
 * ]
*/

/** 正交投影变换矩阵 *
 * [ 2/(r-l)       0             0             0
     0             2/(t-b)       0             0
     0             0             -2/(f-n)      0
     -(r+l)/(r-l)  -(t+b)/(t-b)  -(f+n)/(f-n)  1
 * ]
*/

void main()
{
    // glTranslate(0.0, 0.0, -1.0, 1.0)
    mat4 translateMatrix = mat4(1.0, 0.0, 0.0, 0.0,    // column 0
                                0.0, 1.0, 0.0, 0.0,    // column 1
                                0.0, 0.0, 1.0, -1.0,   // column 2
                                0.0, 0.0, 0.0, 1.0      // column 3
                                );
    
    // glOrtho(-1.0, 1.0, -1.0, 1.0, 1.0, 3.0)
    mat4 projectionMatrix = mat4(1.0, 0.0, 0.0, 0.0,    // column 0
                                 0.0, 1.0, 0.0, 0.0,    // column 1
                                 0.0, 0.0, -1.0, -2.0,  // column 2
                                 0.0, 0.0, 0.0, 1.0     // colimn 3
                                 );
    
    gl_Position = inPos * (translateMatrix * projectionMatrix);
    
    myColor = inColor;
}

