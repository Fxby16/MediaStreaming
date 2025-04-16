# MediaStreaming
Stream media from Telegram.

## Dependencies
- [civetweb](https://github.com/civetweb/civetweb.git)
- [tdjson](https://github.com/tdlib/td.git)

## Build
### Server
`cd Server`  
`premake5 BUILD_TYPE`  
For example, `premake5 gmake2` for Linux or `premake5 vs2019` for Windows.
Then run `make` or open the generated project in your IDE and build it.

## Run
### Server
`cd Server`  
`bin/Release/MediaStreamingServer` or `bin/Debug/MediaStreamingServer` for Linux.