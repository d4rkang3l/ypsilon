/*
    Ypsilon Scheme System
    Copyright (c) 2004-2009 Y.FUJITA / LittleWing Company Limited.
    See license.txt for terms and conditions of use
*/

#include "core.h"
#include "vm.h"
#include "ffi.h"
#include "file.h"
#include "heap.h"
#include "subr.h"
#include "arith.h"
#include "violation.h"

// load-shared-object
scm_obj_t
subr_load_shared_object(VM* vm, int argc, scm_obj_t argv[])
{
    if (argc == 1) {
        if (STRINGP(argv[0])) {
            scm_string_t string = (scm_string_t)argv[0];
            void* hdl = load_shared_object(string);
            if (hdl) return uintptr_to_integer(vm->m_heap, (uintptr_t)hdl);
            invalid_argument_violation(vm, "load-shared-object", last_shared_object_error(), NULL, -1, argc, argv);
            return scm_undef;
        }
        wrong_type_argument_violation(vm, "load-shared-object", 0, "string", argv[0], argc, argv);
        return scm_undef;
    }
    wrong_number_of_arguments_violation(vm, "load-shared-object", 1, 1, argc, argv);
    return scm_undef;
}

// lookup-shared-object
scm_obj_t
subr_lookup_shared_object(VM* vm, int argc, scm_obj_t argv[])
{
    if (argc == 2) {
        void* hdl;
        if (exact_positive_integer_pred(argv[0])) {
            if (exact_integer_to_uintptr(argv[0], (uintptr_t*)&hdl) == false) {
                invalid_argument_violation(vm, "lookup-shared-object", "value out of bound,", argv[0], 0, argc, argv);
                return scm_undef;
            }
        } else {
            wrong_type_argument_violation(vm, "lookup-shared-object", 0, "shared object handle", argv[0], argc, argv);
            return scm_undef;
        }
        if (STRINGP(argv[1]) || SYMBOLP(argv[1])) {
            uintptr_t adrs = (uintptr_t)lookup_shared_object(hdl, argv[1]);
            if (adrs == 0) return scm_false;
            return uintptr_to_integer(vm->m_heap, adrs);
        }
        wrong_type_argument_violation(vm, "lookup-shared-object", 1, "string or symbol", argv[1], argc, argv);
        return scm_undef;
    }
    wrong_number_of_arguments_violation(vm, "lookup-shared-object", 2, 2, argc, argv);
    return scm_undef;
}

// shared-object-errno
scm_obj_t
subr_shared_object_errno(VM* vm, int argc, scm_obj_t argv[])
{
    if (argc == 0) return int_to_integer(vm->m_heap, vm->m_shared_object_errno);
    if (argc == 1) {
        if (exact_integer_pred(argv[0])) {
            int val;
            if (exact_integer_to_int(argv[0], &val)) {
                errno = val;
                vm->m_shared_object_errno = val;
                return scm_unspecified;
            }
            invalid_argument_violation(vm, "shared-object-errno", "value out of range,", argv[0], 0, argc, argv);
            return scm_undef;
        }
        wrong_type_argument_violation(vm, "shared-object-errno", 0, "exact integer", argv[0], argc, argv);
        return scm_undef;
    }
    wrong_number_of_arguments_violation(vm, "shared-object-errno", 0, 1, argc, argv);
    return scm_undef;
}

// shared-object-win32-last-error
scm_obj_t
subr_shared_object_win32_lasterror(VM* vm, int argc, scm_obj_t argv[])
{
#if _MSC_VER
    if (argc == 0) return int_to_integer(vm->m_heap, vm->m_shared_object_win32_lasterror);
    if (argc == 1) {
        if (exact_integer_pred(argv[0])) {
            if (n_positive_pred(argv[0])) {
                uint32_t val;
                if (exact_integer_to_uint32(argv[0], &val)) {
                    SetLastError(val);
                    vm->m_shared_object_win32_lasterror = val;
                    return scm_unspecified;
                }
            } else {
                int32_t val;
                if (exact_integer_to_int32(argv[0], &val)) {
                    SetLastError(val);
                    vm->m_shared_object_win32_lasterror = val;
                    return scm_unspecified;
                }
            }
            invalid_argument_violation(vm, "shared-object-win32-lasterror", "value out of range,", argv[0], 0, argc, argv);
            return scm_undef;
        }
        wrong_type_argument_violation(vm, "shared-object-win32-lasterror", 0, "exact integer", argv[0], argc, argv);
        return scm_undef;
    }
    wrong_number_of_arguments_violation(vm, "shared-object-win32-lasterror", 0, 1, argc, argv);
    return scm_undef;
#else
    raise_error(vm, "shared-object-win32-last-error", "operating system does not support this feature", 0, argc, argv);
    return scm_undef;
#endif
}

// win32-error->string
scm_obj_t
subr_win32_error_string(VM* vm, int argc, scm_obj_t argv[])
{
#if _MSC_VER
    if (argc == 1) {
        if (exact_integer_pred(argv[0])) {
            uint32_t val;
            if (exact_integer_to_uint32(argv[0], &val)) {
                char* message;
                FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,
                        val,
                        MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
                        (LPSTR)&message,
                        0,
                        NULL);
                int tail = strlen(message);
                while (--tail >= 0) {
                    if (message[tail] == '\r' || message[tail] == '\n') {
                        message[tail] = 0;
                        continue;
                    }
                    break;
                }
                scm_string_t obj = make_string(vm->m_heap, message);
                LocalFree(message);
                return obj;
            }
            invalid_argument_violation(vm, "win32-error->string", "value out of range,", argv[0], 0, argc, argv);
            return scm_undef;
        }
        wrong_type_argument_violation(vm, "win32-error->string", 0, "exact integer", argv[0], argc, argv);
        return scm_undef;
    }
    wrong_number_of_arguments_violation(vm, "win32-error->string", 1, 1, argc, argv);
    return scm_undef;
#else
    raise_error(vm, "win32-error->string", "operating system does not support this feature", 0, argc, argv);
    return scm_undef;
#endif
}

// make-callback-trampoline
scm_obj_t
subr_make_callback_trampoline(VM* vm, int argc, scm_obj_t argv[])
{
    if (argc == 3) {
        if (exact_non_negative_integer_pred(argv[0])) {
            if (exact_non_negative_integer_pred(argv[1])) {
                if (CLOSUREP(argv[2])) {
                    return make_callback(vm, FIXNUM(argv[0]), FIXNUM(argv[1]), (scm_closure_t)argv[2]);
                }
                wrong_type_argument_violation(vm, "make-callback-trampoline", 2, "closure", argv[2], argc, argv);
                return scm_undef;
            }
            wrong_type_argument_violation(vm, "make-callback-trampoline", 1, "exact non-negative integer", argv[1], argc, argv);
            return scm_undef;
        }
        wrong_type_argument_violation(vm, "make-callback-trampoline", 0, "exact non-negative integer", argv[0], argc, argv);
        return scm_undef;
    }
    wrong_number_of_arguments_violation(vm, "make-callback-trampoline", 3, 3, argc, argv);
    return scm_undef;
}

#define FFI_RETURN_TYPE_VOID        0x0000
#define FFI_RETURN_TYPE_BOOL        0x0001
#define FFI_RETURN_TYPE_SHORT       0x0002
#define FFI_RETURN_TYPE_INT         0x0003
#define FFI_RETURN_TYPE_INTPTR      0x0004
#define FFI_RETURN_TYPE_USHORT      0x0005
#define FFI_RETURN_TYPE_UINT        0x0006
#define FFI_RETURN_TYPE_UINTPTR     0x0007
#define FFI_RETURN_TYPE_FLOAT       0x0008
#define FFI_RETURN_TYPE_DOUBLE      0x0009
#define FFI_RETURN_TYPE_STRING      0x000a
#define FFI_RETURN_TYPE_SIZE_T      0x000b
#define FFI_RETURN_TYPE_INT8_T      0x000c
#define FFI_RETURN_TYPE_UINT8_T     0x000d
#define FFI_RETURN_TYPE_INT16_T     0x000e
#define FFI_RETURN_TYPE_UINT16_T    0x000f
#define FFI_RETURN_TYPE_INT32_T     0x0010
#define FFI_RETURN_TYPE_UINT32_T    0x0011
#define FFI_RETURN_TYPE_INT64_T     0x0012
#define FFI_RETURN_TYPE_UINT64_T    0x0013
#define FFI_RETURN_TYPE_MASK        0x00ff

#define FFI_CALL_TYPE_STDCALL       0x0100
#define FFI_CALL_TYPE_MASK          0xff00

class synchronize_errno {
    VM* m_vm;
public:
    synchronize_errno(VM* vm) {
        m_vm = vm;
        errno = m_vm->m_shared_object_errno;
#if _MSC_VER
        SetLastError(m_vm->m_shared_object_win32_lasterror);
#endif
    }
    ~synchronize_errno() {
        m_vm->m_shared_object_errno = errno;
#if _MSC_VER
        m_vm->m_shared_object_win32_lasterror = GetLastError();
#endif
    }
};

inline intptr_t
call_cdecl_intptr(VM* vm, void* func, c_stack_frame_t& stack)
{
    synchronize_errno sync(vm);
#if ARCH_IA32
    return c_func_stub_intptr(func, stack.count(), stack.frame());
#elif ARCH_X64
    return c_func_stub_intptr_x64(func, stack.count(), stack.sse_use(), stack.frame());
#else
    fatal("%s:%u ffi not supported on this build", __FILE__, __LINE__);
#endif
}

inline int64_t
call_cdecl_int64(VM* vm, void* func, c_stack_frame_t& stack)
{
    synchronize_errno sync(vm);
#if ARCH_IA32
    return c_func_stub_int64(func, stack.count(), stack.frame());
#elif ARCH_X64
    return (int64_t)c_func_stub_intptr_x64(func, stack.count(), stack.sse_use(), stack.frame());
#else
    fatal("%s:%u ffi not supported on this build", __FILE__, __LINE__);
#endif
}


inline float
call_cdecl_float(VM* vm, void* func, c_stack_frame_t& stack)
{
    synchronize_errno sync(vm);
#if ARCH_IA32
    return c_func_stub_float(func, stack.count(), stack.frame());
#elif ARCH_X64
    return c_func_stub_float_x64(func, stack.count(), stack.sse_use(), stack.frame());
#else
    fatal("%s:%u ffi not supported on this build", __FILE__, __LINE__);
#endif
}

inline double
call_cdecl_double(VM* vm, void* func, c_stack_frame_t& stack)
{
    synchronize_errno sync(vm);
#if ARCH_IA32
    return c_func_stub_double(func, stack.count(), stack.frame());
#elif ARCH_X64
    return c_func_stub_double_x64(func, stack.count(), stack.sse_use(), stack.frame());
#else
    fatal("%s:%u ffi not supported on this build", __FILE__, __LINE__);
#endif
}

inline intptr_t call_stdcall_intptr(VM* vm, void* func, c_stack_frame_t& stack)
{
#if _MSC_VER && ARCH_IA32
    synchronize_errno sync(vm);
    return stdcall_func_stub_intptr(func, stack.count(), stack.frame());
#endif
    assert(false);
}

inline int64_t call_stdcall_int64(VM* vm, void* func, c_stack_frame_t& stack)
{
#if _MSC_VER && ARCH_IA32
    synchronize_errno sync(vm);
    return stdcall_func_stub_int64(func, stack.count(), stack.frame());
#endif
    assert(false);
}

inline float call_stdcall_float(VM* vm, void* func, c_stack_frame_t& stack)
{
#if _MSC_VER && ARCH_IA32
    synchronize_errno sync(vm);
    return stdcall_func_stub_float(func, stack.count(), stack.frame());
#endif
    assert(false);
}

inline double call_stdcall_double(VM* vm, void* func, c_stack_frame_t& stack)
{
#if _MSC_VER && ARCH_IA32
    synchronize_errno sync(vm);
    return stdcall_func_stub_double(func, stack.count(), stack.frame());
#endif
    assert(false);
}

// call-shared-object
scm_obj_t
subr_call_shared_object(VM* vm, int argc, scm_obj_t argv[])
{
    if (argc >= 1) {
        if (!FIXNUMP(argv[0])) {
            wrong_type_argument_violation(vm, "call-shared-object", 0, "fixnum", argv[0], argc, argv);
            return scm_undef;
        }
        int type = FIXNUM(argv[0]);
        void *func = NULL;
        if (exact_positive_integer_pred(argv[1])) {
            if (exact_integer_to_uintptr(argv[1], (uintptr_t*)&func) == false) {
                invalid_argument_violation(vm, "call-shared-object", "value out of bound,", argv[1], 1, argc, argv);
                return scm_undef;
            }
        } else {
            wrong_type_argument_violation(vm, "call-shared-object", 1, "c function address", argv[1], argc, argv);
            return scm_undef;
        }
        const char* who;
        if (SYMBOLP(argv[2])) {
            who = ((scm_symbol_t)argv[2])->name;
        } else {
            wrong_type_argument_violation(vm, "call-shared-object", 2, "symbol", argv[2], argc, argv);
            return scm_undef;
        }
        const char* signature;
        if (STRINGP(argv[3])) {
            signature = ((scm_string_t)argv[3])->name;
        } else {
            wrong_type_argument_violation(vm, "call-shared-object", 3, "string", argv[3], argc, argv);
            return scm_undef;
        }
        if (argc - 4 <= FFI_MAX_ARGC) {
            c_stack_frame_t stack(vm);
            for (int i = 4; i < argc; i++) {
                const char* err = stack.push(argv[i], signature[0]);
                if (err) {
                    wrong_type_argument_violation(vm, who, i, err, argv[i], argc, argv);
                    return scm_undef;
                }
                signature++;
            }
#if _MSC_VER && ARCH_IA32
            const bool use_stdcall = ((type & FFI_CALL_TYPE_MASK) == FFI_CALL_TYPE_STDCALL);
#else
            const bool use_stdcall = false;
#endif
            switch (type & FFI_RETURN_TYPE_MASK) {
                case FFI_RETURN_TYPE_VOID: {
                    if (use_stdcall) call_stdcall_intptr(vm, func, stack);
                    else call_cdecl_intptr(vm, func, stack);
                    return scm_unspecified;
                }
                case FFI_RETURN_TYPE_BOOL: {
                    intptr_t retval;
                    if (use_stdcall) retval = call_stdcall_intptr(vm, func, stack);
                    else retval = call_cdecl_intptr(vm, func, stack);
                    return retval ? scm_true : scm_false;
                }
                case FFI_RETURN_TYPE_SHORT: {
                    short retval;
                    if (use_stdcall) retval = (short)call_stdcall_intptr(vm, func, stack);
                    else retval = (short)call_cdecl_intptr(vm, func, stack);
                    return int_to_integer(vm->m_heap, retval);
                }
                case FFI_RETURN_TYPE_INT: {
                    int retval;
                    if (use_stdcall) retval = (int)call_stdcall_intptr(vm, func, stack);
                    else retval = (int)call_cdecl_intptr(vm, func, stack);
                    return int_to_integer(vm->m_heap, retval);
                }
                case FFI_RETURN_TYPE_INTPTR: {
                    intptr_t retval;
                    if (use_stdcall) retval = call_stdcall_intptr(vm, func, stack);
                    else retval = call_cdecl_intptr(vm, func, stack);
                    return intptr_to_integer(vm->m_heap, retval);
                }
                case FFI_RETURN_TYPE_USHORT: {
                    unsigned short retval;
                    if (use_stdcall) retval = (unsigned short)call_stdcall_intptr(vm, func, stack);
                    else retval = (unsigned short)call_cdecl_intptr(vm, func, stack);
                    return uint_to_integer(vm->m_heap, retval);
                }
                case FFI_RETURN_TYPE_UINT: {
                    unsigned int retval;
                    if (use_stdcall) retval = (unsigned int)call_stdcall_intptr(vm, func, stack);
                    else retval = (unsigned int)call_cdecl_intptr(vm, func, stack);
                    return uint_to_integer(vm->m_heap, retval);
                }
                case FFI_RETURN_TYPE_UINTPTR: {
                    uintptr_t retval;
                    if (use_stdcall) retval = (uintptr_t)call_stdcall_intptr(vm, func, stack);
                    else retval = (uintptr_t)call_cdecl_intptr(vm, func, stack);
                    return uintptr_to_integer(vm->m_heap, retval);
                }
                case FFI_RETURN_TYPE_FLOAT: {
                    float retval;
                    if (use_stdcall) retval = (uintptr_t)call_stdcall_float(vm, func, stack);
                    else retval = call_cdecl_float(vm, func, stack);
                    return make_flonum(vm->m_heap, retval);
                }
                case FFI_RETURN_TYPE_DOUBLE: {
                    double retval;
                    if (use_stdcall) retval = (uintptr_t)call_stdcall_double(vm, func, stack);
                    else retval = call_cdecl_double(vm, func, stack);
                    return make_flonum(vm->m_heap, retval);
                }
                case FFI_RETURN_TYPE_STRING: {
                    char* p;
                    if (use_stdcall) p = (char*)call_stdcall_intptr(vm, func, stack);
                    else p = (char*)call_cdecl_intptr(vm, func, stack);
                    if (p == NULL) return MAKEFIXNUM(0);
                    return make_string(vm->m_heap, p);
                }
                case FFI_RETURN_TYPE_SIZE_T: {
                    if (sizeof(size_t) == sizeof(int)) {
                        unsigned int retval;
                        if (use_stdcall) retval = (unsigned int)call_stdcall_intptr(vm, func, stack);
                        else retval = (unsigned int)call_cdecl_intptr(vm, func, stack);
                        return uint_to_integer(vm->m_heap, retval);
                    }
                    uintptr_t retval;
                    if (use_stdcall) retval = (uintptr_t)call_stdcall_intptr(vm, func, stack);
                    else retval = (uintptr_t)call_cdecl_intptr(vm, func, stack);
                    return uintptr_to_integer(vm->m_heap, retval);
                }
                case FFI_RETURN_TYPE_INT8_T: {
                    int8_t retval;
                    if (use_stdcall) retval = (int8_t)call_stdcall_intptr(vm, func, stack);
                    else retval = (int8_t)call_cdecl_intptr(vm, func, stack);
                    return int_to_integer(vm->m_heap, retval);
                }
                case FFI_RETURN_TYPE_UINT8_T: {
                    uint8_t retval;
                    if (use_stdcall) retval = (uint8_t)call_stdcall_intptr(vm, func, stack);
                    else retval = (uint8_t)call_cdecl_intptr(vm, func, stack);
                    return uint_to_integer(vm->m_heap, retval);
                }
                case FFI_RETURN_TYPE_INT16_T: {
                    int16_t retval;
                    if (use_stdcall) retval = (int16_t)call_stdcall_intptr(vm, func, stack);
                    else retval = (int16_t)call_cdecl_intptr(vm, func, stack);
                    return int_to_integer(vm->m_heap, retval);
                }
                case FFI_RETURN_TYPE_UINT16_T: {
                    uint16_t retval;
                    if (use_stdcall) retval = (uint16_t)call_stdcall_intptr(vm, func, stack);
                    else retval = (uint16_t)call_cdecl_intptr(vm, func, stack);
                    return uint_to_integer(vm->m_heap, retval);
                }
                case FFI_RETURN_TYPE_INT32_T: {
                    int32_t retval;
                    if (use_stdcall) retval = (int32_t)call_stdcall_intptr(vm, func, stack);
                    else retval = (int32_t)call_cdecl_intptr(vm, func, stack);
                    return int_to_integer(vm->m_heap, retval);
                }
                case FFI_RETURN_TYPE_UINT32_T: {
                    uint32_t retval;
                    if (use_stdcall) retval = (uint32_t)call_stdcall_intptr(vm, func, stack);
                    else retval = (uint32_t)call_cdecl_intptr(vm, func, stack);
                    return uint_to_integer(vm->m_heap, retval);
                }
                case FFI_RETURN_TYPE_INT64_T: {
                    int64_t retval;
                    if (use_stdcall) retval = (int64_t)call_stdcall_int64(vm, func, stack);
                    else retval = (int64_t)call_cdecl_int64(vm, func, stack);
                    return int64_to_integer(vm->m_heap, retval);
                }
                case FFI_RETURN_TYPE_UINT64_T: {
                    uint64_t retval;
                    if (use_stdcall) retval = (uint64_t)call_stdcall_int64(vm, func, stack);
                    else retval = (uint64_t)call_cdecl_int64(vm, func, stack);
                    return uint64_to_integer(vm->m_heap, retval);
                }
            }
            invalid_argument_violation(vm, "call-shared-object", "invalid c function return type", argv[0], 0, argc, argv);
            return scm_undef;
        }
        invalid_argument_violation(vm, "call-shared-object", "too many arguments,", MAKEFIXNUM(argc), -1, argc, argv);
        return scm_undef;
    }
    wrong_number_of_arguments_violation(vm, "call-shared-object", 2, -1, argc, argv);
    return scm_undef;
}

void init_subr_ffi(object_heap_t* heap)
{
    #define DEFSUBR(SYM, FUNC)  heap->intern_system_subr(SYM, FUNC)

    DEFSUBR("load-shared-object", subr_load_shared_object);
    DEFSUBR("lookup-shared-object", subr_lookup_shared_object);
    DEFSUBR("call-shared-object", subr_call_shared_object);
    DEFSUBR("make-callback-trampoline", subr_make_callback_trampoline);
    DEFSUBR("shared-object-errno", subr_shared_object_errno);
    DEFSUBR("shared-object-win32-lasterror", subr_shared_object_win32_lasterror);
    DEFSUBR("win32-error->string", subr_win32_error_string);
}
