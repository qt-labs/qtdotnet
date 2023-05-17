/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#pragma once

#ifdef __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wconversion"
#endif
#include <QtGlobal>
#ifdef __GNUC__
#   pragma GCC diagnostic pop
#endif

#ifdef Q_OS_WINDOWS
#   define char_t char16_t
#   define STR(qstr) reinterpret_cast<const char_t*>(qstr.utf16())
#   define QSTR(str) QString::fromUtf16(str)
#else
#   define char_t char
#   define STR(qstr) reinterpret_cast<const char_t*>(qstr.toUtf8().constData())
#   define QSTR(str) QString(str)
#endif
#define FN_STR(t_fn) QString(#t_fn).chopped(3)
#define GET_FN(lib,t_fn) (t_fn)lib.resolve(FN_STR(t_fn).toLocal8Bit())


/*
    adapted from:
        https://github.com/dotnet/runtime/blob/main/src/native/corehost/error_codes.h
*/
enum StatusCode
{
    // Success
    Success = 0,
    Success_HostAlreadyInitialized = 0x00000001,
    Success_DifferentRuntimeProperties = 0x00000002,

    // Failure
    InvalidArgFailure = 0x80008081,
    CoreHostLibLoadFailure = 0x80008082,
    CoreHostLibMissingFailure = 0x80008083,
    CoreHostEntryPointFailure = 0x80008084,
    CoreHostCurHostFindFailure = 0x80008085,
    // unused = 0x80008086,
    CoreClrResolveFailure = 0x80008087,
    CoreClrBindFailure = 0x80008088,
    CoreClrInitFailure = 0x80008089,
    CoreClrExeFailure = 0x8000808a,
    ResolverInitFailure = 0x8000808b,
    ResolverResolveFailure = 0x8000808c,
    LibHostCurExeFindFailure = 0x8000808d,
    LibHostInitFailure = 0x8000808e,
    // unused = 0x8000808f,
    LibHostExecModeFailure = 0x80008090,
    LibHostSdkFindFailure = 0x80008091,
    LibHostInvalidArgs = 0x80008092,
    InvalidConfigFile = 0x80008093,
    AppArgNotRunnable = 0x80008094,
    AppHostExeNotBoundFailure = 0x80008095,
    FrameworkMissingFailure = 0x80008096,
    HostApiFailed = 0x80008097,
    HostApiBufferTooSmall = 0x80008098,
    LibHostUnknownCommand = 0x80008099,
    LibHostAppRootFindFailure = 0x8000809a,
    SdkResolverResolveFailure = 0x8000809b,
    FrameworkCompatFailure = 0x8000809c,
    FrameworkCompatRetry = 0x8000809d,
    AppHostExeNotBundle= 0x8000809e,
    BundleExtractionFailure = 0x8000809f,
    BundleExtractionIOError = 0x800080a0,
    LibHostDuplicateProperty = 0x800080a1,
    HostApiUnsupportedVersion = 0x800080a2,
    HostInvalidState = 0x800080a3,
    HostPropertyNotFound = 0x800080a4,
    CoreHostIncompatibleConfig = 0x800080a5,
    HostApiUnsupportedScenario = 0x800080a6,
    HostFeatureDisabled = 0x800080a7
};

#define HOSTFN_FAILED(status) ((static_cast<qint32>(static_cast<StatusCode>(status))) < 0)


/*
    adapted from:
        https://github.com/dotnet/runtime/blob/main/src/native/corehost/nethost/nethost.h
*/
#ifdef Q_OS_WINDOWS
#   define NETHOST_CALLTYPE __stdcall
#else
#   define NETHOST_CALLTYPE
#endif

struct get_hostfxr_parameters
{
    size_t size;
    const char_t *assembly_path;
    const char_t *dotnet_root;
};

using get_hostfxr_path_fn = quint32(NETHOST_CALLTYPE *)(
    char_t *buffer,
    size_t *buffer_size,
    const get_hostfxr_parameters *parameters);


/*
    adapted from:
        https://github.com/dotnet/runtime/blob/main/src/native/corehost/hostfxr.h
*/
#ifdef Q_OS_WINDOWS
#   define HOSTFXR_CALLTYPE __cdecl
#else
#   define HOSTFXR_CALLTYPE
#endif

enum hostfxr_delegate_type
{
    hdt_com_activation,
    hdt_load_in_memory_assembly,
    hdt_winrt_activation,
    hdt_com_register,
    hdt_com_unregister,
    hdt_load_assembly_and_get_function_pointer,
    hdt_get_function_pointer
};

using hostfxr_error_writer_fn = void(HOSTFXR_CALLTYPE *)(const char_t *message);
//
using hostfxr_set_error_writer_fn = hostfxr_error_writer_fn(HOSTFXR_CALLTYPE *)(
    hostfxr_error_writer_fn error_writer);

using hostfxr_handle = void*;
struct hostfxr_initialize_parameters
{
    size_t size;
    const char_t *host_path;
    const char_t *dotnet_root;
};

using hostfxr_initialize_for_runtime_config_fn = quint32(HOSTFXR_CALLTYPE *)(
    const char_t *runtime_config_path,
    const hostfxr_initialize_parameters *parameters,
    /*out*/ hostfxr_handle *host_context_handle);

using hostfxr_get_runtime_property_value_fn = quint32(HOSTFXR_CALLTYPE *)(
    hostfxr_handle host_context_handle,
    const char_t *name,
    /*out*/ const char_t **value);

using hostfxr_set_runtime_property_value_fn = quint32(HOSTFXR_CALLTYPE *)(
    hostfxr_handle host_context_handle,
    const char_t *name,
    const char_t *value);

using hostfxr_get_runtime_properties_fn = quint32(HOSTFXR_CALLTYPE *)(
    hostfxr_handle host_context_handle,
    /*inout*/ size_t *count,
    /*out*/ const char_t **keys,
    /*out*/ const char_t **values);

using hostfxr_get_runtime_delegate_fn = quint32(HOSTFXR_CALLTYPE *)(
    hostfxr_handle host_context_handle,
    hostfxr_delegate_type type,
    /*out*/ void **delegate);

using hostfxr_close_fn = quint32(HOSTFXR_CALLTYPE *)(hostfxr_handle host_context_handle);


/*
    adapted from:
        https://github.com/dotnet/runtime/blob/main/src/native/corehost/coreclr_delegates.h
*/
#ifdef Q_OS_WINDOWS
#   define CORECLR_DELEGATE_CALLTYPE __stdcall
#else
#   define CORECLR_DELEGATE_CALLTYPE
#endif

using component_entry_point_fn = quint32(CORECLR_DELEGATE_CALLTYPE *)(
    void *arg, qint32 arg_size_in_bytes);

using load_assembly_and_get_function_pointer_fn = quint32(CORECLR_DELEGATE_CALLTYPE *)(
    const char_t *assembly_path ,
    const char_t *type_name ,
    const char_t *method_name ,
    const char_t *delegate_type_name,
    void *reserved,
    /*out*/ void **delegate );

using get_function_pointer_fn = quint32(CORECLR_DELEGATE_CALLTYPE *)(
    const char_t *type_name,
    const char_t *method_name,
    const char_t *delegate_type_name,
    void *load_context,
    void *reserved,
    /*out*/ void **delegate);


/*
    adapted from:
     https://learn.microsoft.com/en-us/dotnet/api/system.runtime.interopservices.unmanagedtype
*/
enum class UnmanagedType
{
    Void = 0,
    ObjectRef = -1,
    Bool = 2,
    I1 = 3,
    U1 = 4,
    I2 = 5,
    U2 = 6,
    I4 = 7,
    U4 = 8,
    I8 = 9,
    U8 = 10,
    R4 = 11,
    R8 = 12,
    Currency = 15,
    BStr = 19,
    LPStr = 20,
    LPWStr = 21,
    LPTStr = 22,
    ByValTStr = 23,
    IUnknown = 25,
    IDispatch = 26,
    Struct = 27,
    Interface = 28,
    SafeArray = 29,
    ByValArray = 30,
    SysInt = 31,
    SysUInt = 32,
    VBByRefStr = 34,
    AnsiBStr = 35,
    TBStr = 36,
    VariantBool = 37,
    FunctionPtr = 38,
    AsAny = 40,
    LPArray = 42,
    LPStruct = 43,
    CustomMarshaler = 44,
    Error = 45,
    IInspectable = 46,
    HString = 47,
    LPUTF8Str = 48
};
