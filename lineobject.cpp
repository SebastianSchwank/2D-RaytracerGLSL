#include "lineobject.h"

LineObject::LineObject(){
}

LineObject::LineObject(float x0, float y0, float x1, float y1, float emission, unsigned int color){
    FloatBits UintRgba;

    UintRgba.i_ = (unsigned int) (x0*4294967295.0);
    mData.append(UintRgba.i_);
    UintRgba.i_ = (unsigned int) (y0*4294967295.0);
    mData.append(UintRgba.i_);
    UintRgba.i_ = (unsigned int) (x1*4294967295.0);
    mData.append(UintRgba.i_);
    UintRgba.i_ = (unsigned int) (y1*4294967295.0);
    mData.append(UintRgba.i_);
    UintRgba.i_ = (unsigned int) (emission*4294967295.0);
    mData.append(UintRgba.i_);

    mData.append(color);
}

unsigned int LineObject::getAttributeAsPixel(int i){
    return mData[i];
}

int LineObject::getSize(){
    //Size of one line-object in "ints"/"floats"
    return 6;
}

//pack a 32bit float into 4 8bit, [0;1] clamped floats
QVector<unsigned char> LineObject::packFloat(float f){

    QVector<unsigned char> rgba;
    rgba.append(0);
    rgba.append(0);
    rgba.append(0);
    rgba.append(0);

    float F = abs(f);
    if(F == 0.0)
    {
        return  rgba;
    }

    float Sign = 0.0;
    if( -f > 0) Sign = 1.0;
    else Sign = -1.0;

    float Exponent = floor( log2(F));

    float Mantissa = F/ exp2(Exponent);
    //std::cout << "  sign: " << Sign << ", exponent: " << Exponent << ", mantissa: " << Mantissa << std::endl;
    //denormalized values if all exponent bits are zero
    if(Mantissa < 1.0)
        Exponent -= 1;

    Exponent +=  127;

    rgba[0] = Exponent;
    rgba[1] = 128.0 * Sign +  fmod(floor(Mantissa * float(128.0)),128.0);
    rgba[2] = floor( fmod(floor(Mantissa* exp2(float(23.0 - 8.0))), exp2(8.0)));
    rgba[3] = floor( exp2(23.0)* fmod(Mantissa, exp2(-15.0)));
    return rgba;
}
