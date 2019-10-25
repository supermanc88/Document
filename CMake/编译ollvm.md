## 安装环境

CMake 

VS2019

bool bRet = false;
do 
{
    HCRYPTPROV Rnd;
    LPCSTR UserName = "MyKeyContainer";
    if (CryptAcquireContextA(&Rnd, UserName, NULL, PROV_RSA_FULL, 0))
    {

    }
    else
    {
        break;
    }

    if (CryptGenRandom(Rnd, 16, (BYTE*)key))
    {
        bRet = true;
    }
    else
    {
        break;
    }

    if (CryptReleaseContext(Rnd, 0))
    {

    }
    else
    {
        break;
    }
} while (false);
if (bRet)
{
    memset(ctr, 0, 16);

    // Once the seed is there, we compute the
    // AES128 key-schedule
    aes_compute_ks(ks, key);

    seeded = true;
}
else
{
    errs() << "Windows CryptGenRandom Error \n";
}


clang-cl.exe -I C:\Program Files (x86)\Windows Kits\8.1\Include\km -I C:\Program Files (x86)\Windows Kits\8.1\Include\shared -I C:\Program Files (x86)\Windows Kits\8.1\Include\um -I C:\Program Files (x86)\Windows Kits\8.1\Include\winrt -D _WIN64
_AMD64_
AMD64
DEPRECATE_DDK_FUNCTIONS=1
MSC_NOOPT
_WIN32_WINNT=$(WIN32_WINNT_VERSION)
WINVER=$(WINVER_VERSION)
WINNT=1
NTDDI_VERSION=$(NTDDI_VERSION)



clang-cl.exe --target=i686-w64-windows-msvc -c *.cpp -mllvm -sub -mllvm -bcf -mllvm -bcf_prob=100 -mllvm -fla -D _WIN64 -D _AMD64_ -D AMD64 -D DEPRECATE_DDK_FUNCTIONS=1 -D MSC_NOOPT -D _WIN32_WINNT=0x0601 -D WINVER=0x0601 -D WINNT=1 -D NTDDI_VERSION=0x06010000 -I "C:\Program Files (x86)\Windows Kits\8.1\Include\km" -I "C:\Program Files (x86)\Windows Kits\8.1\Include\km\crt" 

 -D _SIZE_T_DEFINED

link /ENTRY:"DriverEntry" /SUBSYSTEM:NATIVE",6.01" AntiDebug.obj

"C:\Program Files (x86)\Windows Kits\8.1\lib\win7\KM\x64\BufferOverflowK.lib" "C:\Program Files (x86)\Windows Kits\8.1\lib\win7\KM\x64\ntoskrnl.lib" "C:\Program Files (x86)\Windows Kits\8.1\lib\win7\KM\x64\hal.lib" "C:\Program Files (x86)\Windows Kits\8.1\lib\win7\KM\x64\wmilib.lib"


/OUT:"C:\Users\CHM\documents\visual studio 2013\Projects\KernelAntiDebug\x64\Win7Debug\KernelAntiDebug.sys" /MANIFEST:NO /PROFILE /Driver /PDB:"C:\Users\CHM\documents\visual studio 2013\Projects\KernelAntiDebug\x64\Win7Debug\KernelAntiDebug.pdb" "C:\Program Files (x86)\Windows Kits\8.1\lib\win7\KM\x64\BufferOverflowK.lib" "C:\Program Files (x86)\Windows Kits\8.1\lib\win7\KM\x64\ntoskrnl.lib" "C:\Program Files (x86)\Windows Kits\8.1\lib\win7\KM\x64\hal.lib" "C:\Program Files (x86)\Windows Kits\8.1\lib\win7\KM\x64\wmilib.lib" /RELEASE /VERSION:"6.3" /DEBUG /MACHINE:X64 /ENTRY:"GsDriverEntry" /WX /OPT:REF /INCREMENTAL:NO /PGD:"C:\Users\CHM\documents\visual studio 2013\Projects\KernelAntiDebug\x64\Win7Debug\KernelAntiDebug.pgd" /SUBSYSTEM:NATIVE",6.01" /OPT:ICF /ERRORREPORT:PROMPT /MERGE:"_TEXT=.text;_PAGE=PAGE" /NOLOGO /NODEFAULTLIB /SECTION:"INIT,d" 