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

Json: sc200ai_CRK2F3537-V2_styleTstP0
  CIS setting: 
	1) 20fps
  IQ style:
    1) Tst
  isp:
    1)btnr log domain
  other:
    1) CRK2F3537-V2 模组硬件存在问题，更换后的模组PCB丝印号：CRK2F3537_v11_20240906lxF
    
Json: sc450ai_CRK4F4209_styleDahP0
  CIS setting:	
    1) 序列采用多帧叠加技术blc，保证sensor输出blc稳定性。commit id：6315067bcb31692c9af3900e8bc26c1e09fb6de4 
	   media: i2c: sc450ai update register for BLC trigger, add trigger times to 8 frame 
  IQ style:
    1) Dah
  isp:
    1) btnr log domain
  other:

Json: sc4336p_CRK4F4168-V1_styleTpkP0
  CIS setting:	
  IQ style:
    1) Tpk
  isp:
    1) btnr log domain
  other:
  
Json: sc850sl_Carpo-CSP-MIPI-0833LJ_YT10152-8MP-IR0670_styleRKP0
  CIS setting:	
    1) 内核驱动的提交点为bd2c93fdc560 media: i2c: ps5458: rectify power up sequence
	2) rtt 驱动的提交点为de592b9a0d   bsp: rockchip: camera: ps5458 remove context switch regs  
  IQ style:
    1) tuned on RK_EVB1, without camera for contrast
  isp:
    1) 4k resolution
    2) btnr log domain
  other:

