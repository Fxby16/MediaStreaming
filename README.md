# MediaStreaming
Stream media from Telegram.

## Dependencies

### Required Libraries

- civetweb: https://github.com/civetweb/civetweb.git
- tdjson (TDLib): https://github.com/tdlib/td.git
- libcurl
- OpenSSL
- zlib
- MySQL Connector/C 6.1 (Windows only)

### Windows

On Windows, some libraries must be installed using vcpkg. 

`vcpkg install curl:x64-windows openssl:x64-windows zlib:x64-windows civetweb:x64-windows`  
`vcpkg integrate install`

TDLib, MySQL Connector, and nlohmann/json are already included in the Dependencies folder. You may build them yourself and replace the files if you prefer.

### Linux

On Linux, all libraries (tdjson, civetweb, libcurl, openssl, zlib, mysqlclient) must be preinstalled and available system-wide.

## Build

### Server

`cd Server`  
`premake5 BUILD_TYPE`

For example:

`premake5 gmake2`      # for Linux  
`premake5 vs2022`      # for Windows

Then run `make` (Linux) or open the generated solution in Visual Studio (Windows) and build the project.

## Run

### Server

`cd Server`  
`bin/Release/MediaStreamingServer`  
or  
`bin/Debug/MediaStreamingServer`
