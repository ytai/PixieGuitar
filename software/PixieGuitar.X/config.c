#include <xc.h>

_FICD(ICS_PGD3 & JTAGEN_OFF)
_FPOR(ALTI2C1_OFF & ALTI2C2_ON & WDTWIN_WIN25)
_FWDT(WDTPOST_PS32768 & WDTPRE_PR128 & WINDIS_ON & FWDTEN_OFF)
_FOSC(POSCMD_NONE & OSCIOFNC_ON & IOL1WAY_OFF & FCKSM_CSECMD)
_FOSCSEL(FNOSC_FRC & PWMLOCK_OFF & IESO_OFF)
_FGS(GWRP_OFF & GCP_OFF)

