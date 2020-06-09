@echo off

if [%1]==[] (
    echo Usage: %0 test_binaries_path
    exit /b
)

set BINPATH=%1

set UDP_SERVER_RESULT_FILE=%TEMP%\%RANDOM%_server_udp_test_log_%RANDOM%.log
set UDP_CLIENT_RESULT_FILE=%TEMP%\%RANDOM%_client_udp_test_log_%RANDOM%.log
set TCP_SERVER_RESULT_FILE=%TEMP%\%RANDOM%_server_tcp_test_log_%RANDOM%.log
set TCP_CLIENT_RESULT_FILE=%TEMP%\%RANDOM%_client_tcp_test_log_%RANDOM%.log

echo ===========================
echo Run UDP client/server tests
echo ===========================
timeout 1 > nul
@rem Start the client & server in the backgroup and log output to a file
start /B cmd /c %BINPATH%\server_udp.exe > %UDP_SERVER_RESULT_FILE% 2>&1
start /B cmd /c %BINPATH%\client_udp.exe > %UDP_CLIENT_RESULT_FILE% 2>&1
timeout 2 > nul

@rem Print the result to the console
echo - UDP_Client:
type %UDP_CLIENT_RESULT_FILE%
echo.
echo - UDP_Server:
type %UDP_SERVER_RESULT_FILE%

@rem Remove the log file
del %UDP_SERVER_RESULT_FILE%
del %UDP_CLIENT_RESULT_FILE%

echo.
echo ===========================
echo Run TCP client/server tests
echo ===========================
timeout 1 >nul
@rem Start the client & server in the backgroup and log output to a file
start /B cmd /c %BINPATH%\server_tcp.exe > %TCP_SERVER_RESULT_FILE% 2>&1
start /B cmd /c %BINPATH%\client_tcp.exe > %TCP_CLIENT_RESULT_FILE% 2>&1
timeout 2 >nul

@rem Print the result to the console
echo - TCP_Client:
type %TCP_CLIENT_RESULT_FILE%
echo.
echo - TCP_Server:
type %TCP_SERVER_RESULT_FILE%


@rem Remove the log file
del %TCP_SERVER_RESULT_FILE%
del %TCP_CLIENT_RESULT_FILE%