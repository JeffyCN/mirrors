/*  Example

Json: sc450ai_CRK4F4209_styleTstP0
  CIS setting:  // Describe the CIS configuration information that is paired with the IQ parameters.	
    1) 20fps setting
  IQ style:     // Describe the style for IQ parameters.
    1) Tst
  isp:          // Describe the style for IQ parameters.
    1)btnr log domain
  other:        // Describe other matters needing attention.
    1)
*/
  
Json: sc450ai_CRK4F4209_styleDahP0
  CIS setting:	
    1) 序列采用多帧叠加技术blc，保证sensor输出blc稳定性。commit id：6315067bcb31692c9af3900e8bc26c1e09fb6de4 
	   media: i2c: sc450ai update register for BLC trigger, add trigger times to 8 frame 
  IQ style:
    1) Dah
  isp:
    1) btnr log domain
  other:
    1) blc、ob calibration data is oyyf module    

