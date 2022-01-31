#include <Tonic.h>

class MyFilter_ : public Tonic::Tonic_::Effect_
{
protected:
    void computeSynthesisBlock( const Tonic::Tonic_::SynthesisContext_ & context );
    float LP_Out = 0.0f;
    float HPA_Out = 0.0f, HPA_Prev = 0.0f;
    float HPB_Out = 0.0f, HPB_Prev = 0.0f;
};

inline void MyFilter_::computeSynthesisBlock( const Tonic::Tonic_::SynthesisContext_ & context )
{
    TonicFloat* fdata = &outputFrames_[0];
    TonicFloat* idata = &dryFrames_[0];
    for (unsigned int i=0; i<outputFrames_.size(); i++)
    {
        // Low filter
        float LP_in = *idata++;
        LP_Out = (LP_in - LP_Out) * 0.815686f;

        HPA_Out = HPA_Out * 0.996039f + LP_Out - HPA_Prev;
        HPA_Prev = LP_Out;

        HPB_Out = HPB_Out * 0.999835f + HPA_Out - HPB_Prev;
        HPB_Prev = HPA_Out;

        *fdata++ = HPB_Out;
    }
}

class MyFilter : public Tonic::TemplatedEffect<MyFilter, MyFilter_>
{
};