extern crate winapi;

use std::fs;
use std::mem;
use std::process;
use std::ptr;
use winapi::ctypes::c_long;
use winapi::shared::ntdef::CHAR;
use winapi::shared::ws2def::{ADDRESS_FAMILY, AF_UNIX};
use winapi::um::winsock2::{
    accept, bind, closesocket, ioctlsocket, listen, send, shutdown, socket, WSACleanup,
    WSAGetLastError, WSAStartup, INVALID_SOCKET, SOCKET_ERROR, SOCK_STREAM, SOMAXCONN, WSADATA,
};

const BUFFER: &str = "af_unix from Windows to Windows!";
const SOCKET_PATH: &str = "server.sock";
const SIO_AF_UNIX_GETPEERPID: c_long = 0x5800_0100;

#[repr(C)]
#[allow(non_camel_case_types)]
pub struct sockaddr_un {
    pub sun_family: ADDRESS_FAMILY,
    pub sun_path: [CHAR; 108],
}

fn main() {
    unsafe {
        let mut data: WSADATA = mem::zeroed();
        let ret = WSAStartup(0x202, &mut data);
        if ret != 0 {
            println!("socket error: {}\n", ret);
            process::exit(ret);
        }

        let sock = socket(AF_UNIX, SOCK_STREAM, 0);
        if sock == INVALID_SOCKET {
            let error = WSAGetLastError();
            println!("socket error: {}", error);
            process::exit(error);
        }

        let mut server_addr = sockaddr_un {
            sun_family: AF_UNIX as u16,
            sun_path: mem::zeroed(),
        };
        for (dst, src) in server_addr.sun_path.iter_mut().zip(SOCKET_PATH.chars()) {
            *dst = src as i8
        }
        let len = mem::size_of::<sockaddr_un>();

        if bind(sock, &server_addr as *const _ as *const _, len as _) == SOCKET_ERROR {
            let error = WSAGetLastError();
            println!("bind error: {}", error);
            process::exit(error);
        }

        if listen(sock, SOMAXCONN) == SOCKET_ERROR {
            let error = WSAGetLastError();
            println!("listen error: {}", error);
            process::exit(error);
        }

        println!("accepting connections on: '{}'", SOCKET_PATH);

        let client = accept(sock, ptr::null_mut(), ptr::null_mut());
        if client == INVALID_SOCKET {
            let error = WSAGetLastError();
            println!("accept error: {}", error);
            process::exit(error);
        }

        println!("accepted a connection");

        println!("relaying {} bytes: '{}'", BUFFER.len() - 1, BUFFER);

        let buf: *const i8 = &BUFFER.as_bytes()[0] as *const u8 as *const _;

        if send(client, buf, (BUFFER.len() - 1) as i32, 0) == SOCKET_ERROR {
            let error = WSAGetLastError();
            println!("send error: {}", error);
            process::exit(error);
        }

        let mut pid = 0u32;
        if ioctlsocket(client, SIO_AF_UNIX_GETPEERPID, &mut pid as *mut u32) == SOCKET_ERROR {
            let error = WSAGetLastError();
            println!("ioctlsocket error: {}", error);
            process::exit(error);
        }

        println!("peer PID: {}", pid);

        if shutdown(client, 0) == SOCKET_ERROR {
            let error = WSAGetLastError();
            println!("shutdown error: {}", error);
            process::exit(error);
        }

        let _ = fs::remove_file(SOCKET_PATH); // analogous to 'unlink'
        closesocket(sock);
        WSACleanup();
    }
}
