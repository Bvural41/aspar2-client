#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sched.h>
#include <sys/stat.h>

#include "granny.h"
#include "granny_types.h"
#include "granny_memory.h"
#include "granny_file_operations.h"
#include "granny_file_reader.h"
#include "granny_file_writer.h"
#include "granny_system_clock.h"
#include "granny_threads.h"
#include "granny_accelerated_deformers.h"
#include "granny_bone_operations.h"
#include "granny_matrix_operations.h"
#include "granny_log.h"
#include "granny_assert.h"
#include "rrAtomics.h"

// This should always be the last header included
#include "granny_cpp_settings.h"
#include "granny_version.h"

CompileAssert(ProductMajorVersion  == 2);
CompileAssert(ProductMinorVersion  == 9);
CompileAssert(ProductBuildNumber   == 12);

USING_GRANNY_NAMESPACE;

// 1. Memory allocation
void *GRANNY PlatformAllocate(uintaddrx Size)
{
    return malloc(Size);
}

void GRANNY PlatformDeallocate(void *Memory)
{
    if (Memory)
        free(Memory);
}

// 2. File operations
void GRANNY DeleteFile(char const *FileName)
{
    if (FileName)
        remove(FileName);
}

int32x GRANNY GetFileSize(char const *FileName)
{
    if (!FileName) return 0;
    FILE *f = fopen(FileName, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fclose(f);
    return (int32x)sz;
}

char const *GRANNY GetTemporaryDirectory(void)
{
#if defined(__APPLE__)
    static char s_tmp[512] = {0};
    if (s_tmp[0] == '\0') {
        const char *tmpEnv = getenv("TMPDIR");
        if (tmpEnv && strlen(tmpEnv) > 0) {
            snprintf(s_tmp, sizeof(s_tmp), "%s", tmpEnv);
        } else {
            snprintf(s_tmp, sizeof(s_tmp), "/tmp/");
        }
    }
    return s_tmp;
#else
    return "/data/local/tmp/";
#endif
}

static int32x ANSISeek(FILE *ANSIFileHandle, int32x Offset, int32x MoveMethod)
{
    if (fseek(ANSIFileHandle, (long)Offset, MoveMethod))
    {
        // seek error
    }
    return (int32x)ftell(ANSIFileHandle);
}

// 3. System clock
void GRANNY GetSystemSeconds(system_clock* Result)
{
    if (Result)
    {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        Result->Data[0] = (uint32)ts.tv_sec;
        Result->Data[1] = (uint32)ts.tv_nsec;
        Result->Data[2] = 0;
        Result->Data[3] = 0;
    }
}

real32 GRANNY GetSecondsElapsed(system_clock const &StartClock, system_clock const &EndClock)
{
    double s = (double)EndClock.Data[0] - (double)StartClock.Data[0];
    double ns = (double)EndClock.Data[1] - (double)StartClock.Data[1];
    return (real32)(s + ns * 1e-9);
}

void GRANNY RequeryTimerFrequency()
{
}

// 4. Threading
void GRANNY ThreadYieldToAny()
{
    sched_yield();
}

// 5. File reader
struct android_file_reader
{
    file_reader Base;
    FILE* Handle;
};

static CALLBACK_FN(void) CloseAndroidFileReader(file_reader* Reader)
{
    if (Reader)
    {
        android_file_reader* AR = (android_file_reader*)Reader;
        if (AR->Handle)
            fclose(AR->Handle);
        free(AR);
    }
}

static CALLBACK_FN(int32x) ReadAtMostAndroid(file_reader* Reader, int32x FilePosition, int32x UInt8Count, void* Buffer)
{
    if (!Reader || !Buffer) return 0;
    android_file_reader* AR = (android_file_reader*)Reader;
    if (!AR->Handle) return 0;
    if (fseek(AR->Handle, FilePosition, SEEK_SET) != 0) return 0;
    return (int32x)fread(Buffer, 1, UInt8Count, AR->Handle);
}

static CALLBACK_FN(bool) GetAndroidReaderSize(file_reader* Reader, int32x* NumBytes)
{
    if (!Reader || !NumBytes) return false;
    android_file_reader* AR = (android_file_reader*)Reader;
    if (!AR->Handle) return false;
    long cur = ftell(AR->Handle);
    fseek(AR->Handle, 0, SEEK_END);
    *NumBytes = (int32x)ftell(AR->Handle);
    fseek(AR->Handle, cur, SEEK_SET);
    return true;
}

CALLBACK_FN(file_reader*) GRANNY CreatePlatformFileReaderInternal(char const *FileNameToOpen)
{
    if (!FileNameToOpen) return NULL;
    FILE* f = fopen(FileNameToOpen, "rb");
    if (!f) return NULL;
    android_file_reader* Reader = (android_file_reader*)malloc(sizeof(android_file_reader));
    InitializeFileReader(CloseAndroidFileReader, ReadAtMostAndroid, GetAndroidReaderSize, Reader->Base);
    Reader->Handle = f;
    return &Reader->Base;
}

open_file_reader_callback *GRANNY OpenFileReaderCallback = CreatePlatformFileReaderInternal;

// 5b. File writer
struct android_file_writer
{
    file_writer Base;
    FILE* Handle;
};

static CALLBACK_FN(void) AndroidDeleteFileWriter(file_writer* Writer)
{
    if (Writer)
    {
        android_file_writer* AW = (android_file_writer*)Writer;
        if (AW->Handle)
            fclose(AW->Handle);
        free(AW);
    }
}

static CALLBACK_FN(int32x) AndroidSeekWriter(file_writer* Writer, int32x OffsetInBytes, int32x SeekType)
{
    if (!Writer) return 0;
    android_file_writer* AW = (android_file_writer*)Writer;
    if (!AW->Handle) return 0;
    fseek(AW->Handle, OffsetInBytes, SeekType);
    return (int32x)ftell(AW->Handle);
}

static CALLBACK_FN(bool) AndroidWrite(file_writer* Writer, int32x UInt8Count, void const* WritePointer)
{
    if (!Writer || !WritePointer) return false;
    android_file_writer* AW = (android_file_writer*)Writer;
    if (!AW->Handle) return false;
    return fwrite(WritePointer, 1, UInt8Count, AW->Handle) == (size_t)UInt8Count;
}

static CALLBACK_FN(void) AndroidBeginWriterCRC(file_writer* Writer)
{
}

static CALLBACK_FN(uint32) AndroidEndWriterCRC(file_writer* Writer)
{
    return 0;
}

CALLBACK_FN(file_writer*) GRANNY CreatePlatformFileWriterInternal(char const *FileNameToOpen, bool EraseExisting)
{
    if (!FileNameToOpen) return NULL;
    FILE* f = fopen(FileNameToOpen, EraseExisting ? "wb" : "r+b");
    if (!f && !EraseExisting)
    {
        f = fopen(FileNameToOpen, "ab");
    }
    if (!f) return NULL;
    android_file_writer* Writer = (android_file_writer*)malloc(sizeof(android_file_writer));
    InitializeFileWriter(AndroidDeleteFileWriter,
                         AndroidSeekWriter,
                         AndroidWrite,
                         AndroidBeginWriterCRC,
                         AndroidEndWriterCRC,
                         Writer->Base);
    Writer->Handle = f;
    return &Writer->Base;
}

open_file_writer_callback *GRANNY OpenFileWriterCallback = CreatePlatformFileWriterInternal;

// 6. Accelerated deformers stub
void GRANNY AddAcceleratedDeformers(void)
{
}

// 7. Optimized bone operations dispatch (generic C fallback)
BEGIN_GRANNY_NAMESPACE;

OPTIMIZED_DISPATCH(BuildIdentityWorldPoseOnly) = BuildIdentityWorldPoseOnly_Generic;
OPTIMIZED_DISPATCH(BuildPositionWorldPoseOnly) = BuildPositionWorldPoseOnly_Generic;
OPTIMIZED_DISPATCH(BuildPositionOrientationWorldPoseOnly) = BuildPositionOrientationWorldPoseOnly_Generic;
OPTIMIZED_DISPATCH(BuildFullWorldPoseOnly) = BuildFullWorldPoseOnly_Generic;

OPTIMIZED_DISPATCH(BuildSingleCompositeFromWorldPose) = BuildSingleCompositeFromWorldPose_Generic;
OPTIMIZED_DISPATCH(BuildSingleCompositeFromWorldPoseTranspose) = BuildSingleCompositeFromWorldPoseTranspose_Generic;

// 8. Optimized matrix operations dispatch (generic C fallback)
OPTIMIZED_DISPATCH(ColumnMatrixMultiply4x3Impl)          = ColumnMatrixMultiply4x3Impl_Generic;
OPTIMIZED_DISPATCH(ColumnMatrixMultiply4x3TransposeImpl) = ColumnMatrixMultiply4x3TransposeImpl_Generic;
OPTIMIZED_DISPATCH(ColumnMatrixMultiply4x4Impl)          = ColumnMatrixMultiply4x4Impl_Generic;

bool DisplayAssertion(char const * const Expression,
                      char const * const File,
                      int32x const LineNumber,
                      char const * const Function,
                      bool* IgnoreAssertion,
                      void* UserData)
{
    return false;
}

END_GRANNY_NAMESPACE;

// 9. GCC/Clang Atomics for rrAtomics
extern "C" {

void rrAtomicMemoryBarrierFull(void)
{
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
}

U32 rrAtomicLoadAcquire32(U32 const volatile * ptr)
{
    return __atomic_load_n((U32*)ptr, __ATOMIC_ACQUIRE);
}

U64 rrAtomicLoadAcquire64(U64 const volatile * ptr)
{
    return __atomic_load_n((U64*)ptr, __ATOMIC_ACQUIRE);
}

void rrAtomicStoreRelease32(U32 volatile * ptr, U32 val)
{
    __atomic_store_n((U32*)ptr, val, __ATOMIC_RELEASE);
}

void rrAtomicStoreRelease64(U64 volatile * ptr, U64 val)
{
    __atomic_store_n((U64*)ptr, val, __ATOMIC_RELEASE);
}

U32 rrAtomicCmpXchg32(U32 volatile * pDestVal, U32 newVal, U32 compareVal)
{
    U32 expected = compareVal;
    __atomic_compare_exchange_n((U32*)pDestVal, &expected, newVal, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return expected;
}

U64 rrAtomicCmpXchg64(U64 volatile * pDestVal, U64 newVal, U64 compareVal)
{
    U64 expected = compareVal;
    __atomic_compare_exchange_n((U64*)pDestVal, &expected, newVal, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return expected;
}

rrbool rrAtomicCAS32(U32 volatile * pDestVal, U32 * pOldVal, U32 newVal)
{
    return __atomic_compare_exchange_n((U32*)pDestVal, pOldVal, newVal, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

rrbool rrAtomicCAS64(U64 volatile * pDestVal, U64 * pOldVal, U64 newVal)
{
    return __atomic_compare_exchange_n((U64*)pDestVal, pOldVal, newVal, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

U32 rrAtomicExchange32(U32 volatile * pDestVal, U32 newVal)
{
    return __atomic_exchange_n((U32*)pDestVal, newVal, __ATOMIC_SEQ_CST);
}

U64 rrAtomicExchange64(U64 volatile * pDestVal, U64 newVal)
{
    return __atomic_exchange_n((U64*)pDestVal, newVal, __ATOMIC_SEQ_CST);
}

U32 rrAtomicAddExchange32(U32 volatile * pDestVal, S32 incVal)
{
    return __atomic_fetch_add((U32*)pDestVal, incVal, __ATOMIC_SEQ_CST);
}

U64 rrAtomicAddExchange64(U64 volatile * pDestVal, S64 incVal)
{
    return __atomic_fetch_add((U64*)pDestVal, incVal, __ATOMIC_SEQ_CST);
}

} // extern "C"
