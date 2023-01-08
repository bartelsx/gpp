start GPP_TEST_RELEASE.exe -x 30 -y 30 -s 1

start GPP_TEST_RELEASE.exe -x 1000 -y 30 -s 22

start GPP_TEST_RELEASE.exe -x 30 -y 540 -s 33

start GPP_TEST_RELEASE.exe -x 1000 -y 540 -s 44
echo new ActiveXObject("WScript.Shell").AppActivate("GPP_TEST_RELEASE.exe"); > tmp.js
cscript //nologo tmp.js & del tmp.js