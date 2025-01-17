@echo=off

set test=p:\work\ssleay\ms
set opath=%PATH%
PATH=%1;%PATH%

rem run this from inside the bin directory

echo destest
destest
if errorlevel 1 goto done

echo ideatest
ideatest
if errorlevel 1 goto done

echo bftest
bftest
if errorlevel 1 goto done

echo shatest
shatest
if errorlevel 1 goto done

echo sha1test
sha1test
if errorlevel 1 goto done

echo md5test
md5test
if errorlevel 1 goto done

echo md2test
md2test
if errorlevel 1 goto done

echo mdc2test
mdc2test
if errorlevel 1 goto done

echo rc2test
rc2test
if errorlevel 1 goto done

echo rc4test
rc4test
if errorlevel 1 goto done

echo randtest
randtest
if errorlevel 1 goto done

echo dhtest
dhtest
if errorlevel 1 goto done

echo exptest
exptest
if errorlevel 1 goto done

echo dsatest
dsatest
if errorlevel 1 goto done

echo testenc
call %test%\testenc ssleay
if errorlevel 1 goto done

echo testpem
call %test%\testpem ssleay
if errorlevel 1 goto done

echo verify
copy ..\certs\*.pem cert.tmp >nul
ssleay verify -CAfile cert.tmp ..\certs\*.pem

echo testss
call %test%\testss ssleay
if errorlevel 1 goto done

echo test sslv2
ssltest -ssl2
if errorlevel 1 goto done

echo test sslv2 with server authentication
ssltest -ssl2 -server_auth -CAfile cert.tmp
if errorlevel 1 goto done

echo test sslv2 with client authentication 
ssltest -ssl2 -client_auth -CAfile cert.tmp
if errorlevel 1 goto done

echo test sslv2 with both client and server authentication
ssltest -ssl2 -server_auth -client_auth -CAfile cert.tmp
if errorlevel 1 goto done

echo test sslv3
ssltest -ssl3
if errorlevel 1 goto done

echo test sslv3 with server authentication
ssltest -ssl3 -server_auth -CAfile cert.tmp
if errorlevel 1 goto done

echo test sslv3 with client authentication 
ssltest -ssl3 -client_auth -CAfile cert.tmp
if errorlevel 1 goto done

echo test sslv3 with both client and server authentication
ssltest -ssl3 -server_auth -client_auth -CAfile cert.tmp
if errorlevel 1 goto done

echo test sslv2/sslv3
ssltest
if errorlevel 1 goto done

echo test sslv2/sslv3 with server authentication
ssltest -server_auth -CAfile cert.tmp
if errorlevel 1 goto done

echo test sslv2/sslv3 with client authentication 
ssltest -client_auth -CAfile cert.tmp
if errorlevel 1 goto done

echo test sslv2/sslv3 with both client and server authentication
ssltest -server_auth -client_auth -CAfile cert.tmp
if errorlevel 1 goto done


del cert.tmp

echo passed all tests
goto end
:done
echo problems.....
:end
PATH=%opath%
