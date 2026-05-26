/*
Intel IPP Header - Minimal stub for compilation
*/

#ifndef __IPP_H__
#define __IPP_H__

#include <stdint.h>
#include <stdlib.h>

/* Basic types */
typedef int IppStatus;
typedef float Ipp32f;
typedef double Ipp64f;
typedef uint8_t Ipp8u;
typedef int8_t Ipp8s;
typedef int16_t Ipp16s;
typedef int32_t Ipp32s;
typedef uint16_t Ipp16u;
typedef uint32_t Ipp32u;
typedef int IppHintAlgorithm;

/* Complex type */
typedef struct { Ipp32f re; Ipp32f im; } Ipp32fc;

/* FFT/DCT Spec types - forward declare */
struct IppsDFTSpec_R_32f;
struct IppsDCTFwdSpec_32f;
typedef struct IppsDFTSpec_R_32f IppsDFTSpec_R_32f;
typedef struct IppsDCTFwdSpec_32f IppsDCTFwdSpec_32f;

/* Constants */
#define IPP_FFT_NODIV_BY_ANY 0
#define ippAlgHintAccurate ((IppHintAlgorithm)0)
#define ippAlgHintNone ((IppHintAlgorithm)0)
#define ippStsNoErr 0

/* Memory functions */
#define ippsMalloc_8u(sz) ((Ipp8u*)malloc(sz))
#define ippsFree(x) free(x)
#define ippFree(x) free(x)

/* DFT functions - stub that provides size values */
static inline IppStatus ippsDFTGetSize_C_32fc(int fftLen, int flag, IppHintAlgorithm hint,
    int* pSizeDFTSpec, int* pSizeDFTInitBuf, int* pSizeDFTWorkBuf) {
    if (pSizeDFTSpec) *pSizeDFTSpec = fftLen * fftLen * sizeof(float) * 4;
    if (pSizeDFTInitBuf) *pSizeDFTInitBuf = 4096;
    if (pSizeDFTWorkBuf) *pSizeDFTWorkBuf = fftLen * sizeof(float) * 2;
    return ippStsNoErr;
}

static inline IppStatus ippsDFTInit_R_32f(int fftLen, int flag, IppHintAlgorithm hint,
    IppsDFTSpec_R_32f* pDFTSpec, Ipp8u* pInitBuf) {
    return ippStsNoErr;
}

static inline IppStatus ippsDFTFwd_RToCCS_32f(const Ipp32f* pSrc, Ipp32f* pDst,
    const IppsDFTSpec_R_32f* pDFTSpec, Ipp8u* pWorkBuf) {
    return ippStsNoErr;
}

static inline IppStatus ippsDFTInv_CCSToR_32f(const Ipp32f* pSrc, Ipp32f* pDst,
    const IppsDFTSpec_R_32f* pDFTSpec, Ipp8u* pWorkBuf) {
    return ippStsNoErr;
}

/* DCT functions - stub that provides size values */
static inline IppStatus ippsDCTFwdGetSize_32f(int len, IppHintAlgorithm hint,
    int* pSizeDCTSpec, int* pSizeDCTInitBuf, int* pSizeDCTWorkBuf) {
    if (pSizeDCTSpec) *pSizeDCTSpec = len * len * sizeof(float) * 2;
    if (pSizeDCTInitBuf) *pSizeDCTInitBuf = 4096;
    if (pSizeDCTWorkBuf) *pSizeDCTWorkBuf = len * sizeof(float) * 2;
    return ippStsNoErr;
}

static inline IppStatus ippsDCTFwdInit_32f(IppsDCTFwdSpec_32f** ppDCTSpec, int len, IppHintAlgorithm hint,
    Ipp8u* pInitBuf, Ipp8u* pWorkBuf) {
    return ippStsNoErr;
}

static inline IppStatus ippsDCTFwd_32f_I(Ipp32f* pSrcDst, const IppsDCTFwdSpec_32f* pDCTSpec,
    Ipp8u* pWorkBuf) {
    return ippStsNoErr;
}

/* Init */
#define ippInit() 0

#endif
