/*
    This file is a part of the AID (Another Image Debugger) project.

    Copyright (C) 2013  Olinski Krzysztof E.

    This program is free software: you can redistribute it 
    and/or modify it under the terms of the GNU General Public License 
    as published by the Free Software Foundation, either version 3 of the License, 
    or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
    or FITNESS FOR A PARTICULAR PURPOSE. 
    See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with this program.  
    If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

#include "./inc/aidcon.h"

using namespace std;

int main()
{
    using namespace aidcon;

    int imageSize = 256;
    void* imageBuffer;

    int x,y;
    CTransim imTrans;
    //----------------------------------------------------------------------------------------------
    //Simple 256x256 RGB 8-bit unsigned
    cout << "Sending the image 1...";
    imageBuffer = malloc(imageSize*imageSize*3);
    for(x=0;x<imageSize;x++)
        for(y=0;y<imageSize;y++){
            ((char*)imageBuffer)[3*(y*imageSize + x)   ] = x%imageSize;
            ((char*)imageBuffer)[3*(y*imageSize + x)+1 ] = y%imageSize;
            ((char*)imageBuffer)[3*(y*imageSize + x)+2 ] = (x+y)%imageSize;
        }
    imTrans.sendImg(imageSize, imageSize, "R8G8B8", imageSize*imageSize*3, imageBuffer, "My image 1", "Simple 256x256 RGB 8-bit unsigned.");
    free(imageBuffer);
    cout << "   DONE\n";
    //----------------------------------------------------------------------------------------------
    //Simple 1024x1024 RGBA float
    cout << "Sending the image 2...";
    imageSize = 1024;
    imageBuffer = malloc(imageSize*imageSize*16);
    for(x=0;x<imageSize;x++)
        for(y=0;y<imageSize;y++){
            ((float*)imageBuffer)[4*(y*imageSize + x)   ] = (float)x/imageSize;
            ((float*)imageBuffer)[4*(y*imageSize + x)+1 ] = (float)y/imageSize;
            ((float*)imageBuffer)[4*(y*imageSize + x)+2 ] = (float)(x+y)/(2*imageSize);
            ((float*)imageBuffer)[4*(y*imageSize + x)+3 ] = (float)(0.5f*x)/imageSize+0.5f;
        }
    imTrans.sendImg(imageSize, imageSize, "f R32 G32 B32 A32", imageSize*imageSize*16, imageBuffer, "My image 2", "Simple 1024x1024 RGBA float");
    free(imageBuffer);
    cout << "   DONE\n";
    //----------------------------------------------------------------------------------------------
    //2048x2048 RGBA float with manual scaling
    cout << "Sending the image 3...";
    imageSize = 512;
    imageBuffer = malloc(imageSize*imageSize*16);
    for(x=0;x<imageSize;x++)
        for(y=0;y<imageSize;y++){
            ((float*)imageBuffer)[4*(y*imageSize + x)   ] = (float)x/imageSize*2;
            ((float*)imageBuffer)[4*(y*imageSize + x)+1 ] = (float)x/imageSize/2;
            ((float*)imageBuffer)[4*(y*imageSize + x)+2 ] = (float)(x+y)/(2*imageSize)/4;
            ((float*)imageBuffer)[4*(y*imageSize + x)+3 ] = (float)(0.5f*x)/imageSize+0.5f;
        }
        
    imTrans.setBias(0, 0.0f);
    imTrans.setGain(0, 0.5f);
    imTrans.setBias(1, 0.0f);
    imTrans.setGain(1, 0.5f);
    imTrans.setBias(2, 0.1f);
    imTrans.setGain(2, 4.0f);
        

    imTrans.sendImg(imageSize, imageSize, "f G10 G22 R30 R2 B10 B22 A32", imageSize*imageSize*16, imageBuffer, "My image 3", "2048x2048 RGBA float with manual scaling");
    free(imageBuffer);
    cout << "   DONE\n";
    //----------------------------------------------------------------------------------------------
    //2048x2048 RGBA float/uint/float/int with auto scaling
    cout << "Sending the image 4...";
    imageSize = 2048;
    imageBuffer = malloc(imageSize*imageSize*16);
    for(x=0;x<imageSize;x++)
        for(y=0;y<imageSize;y++){
            ((float*)imageBuffer)[4*(y*imageSize + x)   ] = (float)x/imageSize*2;
            ((unsigned int*)imageBuffer)[4*(y*imageSize + x)+1 ] = (unsigned int)y;
            ((float*)imageBuffer)[4*(y*imageSize + x)+2 ] = (float)(x+y)/(2*imageSize)/4;
            ((int*)imageBuffer)[4*(y*imageSize + x)+3 ] = (int)(0.5*x);
        }
        
    imTrans.setFlags(FLAG_AUTO_GAIN_BIAS);
    imTrans.sendImg(imageSize, imageSize, "fR32uG32fA32iB32", imageSize*imageSize*16, imageBuffer, "My image 4", "2048x2048 RGBA float/uint/float/int with auto scaling");
    free(imageBuffer);
    cout << "   DONE\n";
    return 0;
}
