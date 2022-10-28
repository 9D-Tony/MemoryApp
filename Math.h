/*
MIT License

Copyright (c) 2022 Tony D

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#ifndef _MATH_H
#define _MATH_H

#define Min(X, Y) (((X) < (Y)) ? (X) : (Y))

static Vector2 SetPos(real32 x, real32 y)
{
    Vector2 resultPos = {x,y};
    return resultPos;
}

static Color GetRandomColor()
{
    uint32 randomNum = rand()/255;
    uint32 randomNum1 = rand()/255;
    uint32 randomNum2 = rand()/255;
    Color blockColor = {(uint8)randomNum,(uint8)randomNum1,(uint8)randomNum2, 255};
    
    
    return blockColor;
}

static inline uint32 numDigits(const uint32 n) 
{
    if (n < 10) return 1;
    return 1 + numDigits(n / 10);
}

static inline Vector2 SetTextureAtCenter(Rectangle base,Texture2D centered, real32 scale = 1.0f)
{
    Vector2 texturePos = {base.x + base.width / 2 - (centered.width * scale) / 2 ,base.y + (base.height / 2) - (centered.height * scale) / 2};
    
    return texturePos;
}

static real32 ShinkToFitBounds(Texture2D texture, Rectangle rect)
{
    //width height
    real32 scaleFactorX = rect.width / texture.width;
    real32 scaleFactorY = rect.height / texture.height;
    float minimumNewSizeRatio = Min(scaleFactorX, scaleFactorY);
    
    return minimumNewSizeRatio;
}


static Vector2 GetRectCenter(Rectangle rectangle)
{
    Vector2 result= {};
    result.x = rectangle.x + rectangle.width / 2;
    result.y = rectangle.y + rectangle.height / 2;
    return result; 
}

#endif //_MATH_H
