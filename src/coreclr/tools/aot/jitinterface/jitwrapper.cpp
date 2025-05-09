// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>

#include <minipal/cpufeatures.h>

#include "dllexport.h"
#include "jitinterface_generated.h"

DLL_EXPORT int JitCompileMethod(
    CorInfoExceptionClass **ppException,
    ICorJitCompiler * pJit,
    void * thisHandle,
    void ** callbacks,
    CORINFO_METHOD_INFO* methodInfo,
    unsigned flags,
    uint8_t** entryAddress,
    uint32_t* nativeSizeOfCode)
{
    *ppException = nullptr;

    GUID versionId;
    pJit->getVersionIdentifier(&versionId);
    if (memcmp(&versionId, &JITEEVersionIdentifier, sizeof(GUID)) != 0)
    {
        // JIT and the compiler disagree on how the interface looks like.
        return 1;
    }

    try
    {
        JitInterfaceWrapper jitInterfaceWrapper(thisHandle, callbacks);
        return pJit->compileMethod(&jitInterfaceWrapper, methodInfo, flags, entryAddress, nativeSizeOfCode);
    }
    catch (CorInfoExceptionClass *pException)
    {
        *ppException = pException;
    }

    return 1;
}

DLL_EXPORT void JitSetOs(ICorJitCompiler* pJit, CORINFO_OS os)
{
    pJit->setTargetOS(os);
}

DLL_EXPORT void JitProcessShutdownWork(ICorJitCompiler * pJit)
{
    return pJit->ProcessShutdownWork(nullptr);
}

DLL_EXPORT int JitGetProcessorFeatures()
{
#ifndef CROSS_COMPILE
    return minipal_getcpufeatures();
#else
    return 0;
#endif
}
