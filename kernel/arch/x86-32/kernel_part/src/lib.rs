#![no_std]

extern "C" {
    fn printf(fmt: *const u8, ...) -> i32;
}

pub fn add(left: u64, right: u64) -> u64 {
    left + right
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn it_works() {
        let result = add(2, 2);
        assert_eq!(result, 4);
    }
}

#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! {
    loop {}
}

#[no_mangle]
pub extern "C" fn rust_testfunction() {
    let message = b"Hallo von Rust\n\0";  // Null-terminierter String
    unsafe {
        printf(message.as_ptr());  // Aufruf der C-`printf`-Funktion
    }
}
