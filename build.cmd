setlocal EnableExtensions EnableDelayedExpansion

set build_config=Debug
if /i "%1" == "release" set build_config=Release

rem C server
msbuild server\c\server.vcxproj /p:Configuration=%build_config%;Platform=x64
if %errorlevel% neq 0 exit /b %errorlevel%

rem Rust server
cargo build --manifest-path server/rust/Cargo.toml
if %errorlevel% neq 0 exit /b %errorlevel%

rem C client
msbuild client\c\client.vcxproj /p:Configuration=%build_config%;Platform=x64
if %errorlevel% neq 0 exit /b %errorlevel%

rem .NET Core client
dotnet publish client\dotnet\client.csproj -c %build_config% -f netcoreapp2.1 -r win10-x64
if %errorlevel% neq 0 exit /b %errorlevel%