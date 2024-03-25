use core::ffi::{c_char, c_uchar, CStr};

#[repr(u32)]
pub enum AsmResult {
    Ok = 0,
    Failed = 1,
    NullAssembly = 2,
    NullBinary = 3,
    NullBinaryLen = 4,
}

#[no_mangle]
pub extern "C" fn assemble_str_to_binary(
    assembly: *const c_char,
    binary: *mut *const c_uchar,
    binary_len: *mut usize,
) -> AsmResult {
    let source = unsafe {
        if assembly.is_null() {
            return AsmResult::NullAssembly;
        }
        CStr::from_ptr(assembly).to_str().unwrap()
    };
    let result = customasm::assemble_str_to_binary(source);
    let vec = result.0;
    let report = result.1;

    if report.has_errors() {
        let mut fileserver = customasm::util::FileServerMock::new();
        fileserver.add("str", source);
        report.print_all(&mut std::io::stderr(), &fileserver, true)
    }

    match vec {
        Some(mut vec) => {
            vec.shrink_to_fit();
            let length = vec.len();
            let ptr = vec.as_ptr();
            unsafe {
                if binary.is_null() {
                    return AsmResult::NullBinary;
                }
                if binary_len.is_null() {
                    return AsmResult::NullBinaryLen;
                }
                *binary = ptr as *const c_uchar;
                *binary_len = length as usize;
            }
            std::mem::forget(vec);
            AsmResult::Ok
        }
        None => AsmResult::Failed,
    }
}

#[no_mangle]
pub extern "C" fn free_binary(binary: *const c_uchar, binary_len: usize) -> AsmResult {
    // will be dropped after this call
    unsafe {
        if binary.is_null() {
            return AsmResult::NullBinary;
        }
        let ptr = binary as *mut u8;
        Vec::<u8>::from_raw_parts(ptr, binary_len, binary_len)
    };

    AsmResult::Ok
}
