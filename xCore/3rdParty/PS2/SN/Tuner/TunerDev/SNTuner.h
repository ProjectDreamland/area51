#ifndef __SNTUNER_H_INC__
#define __SNTUNER_H_INC__

#ifdef __cplusplus
extern "C" {
#endif

int snIsTunerRunning(void);
void snStartMarker(unsigned int uID, const char *pText);
void snStopMarker(unsigned int uID);

#ifdef __cplusplus
}
#endif

#endif //#ifndef __SNTUNER_H_INC__