## 安装环境

CMake 

VS2019

Mingw

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
