#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
#define PSTRF(fsh_pointer) (reinterpret_cast<PGM_P>(fsh_pointer))
