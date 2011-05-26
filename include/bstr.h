#ifndef BSTR_H
#define BSTR_H

// http://www.codeproject.com/KB/bugs/bug_in__bstr_t_amp__varia.aspx

//------------------------//

// Convert char * to BSTR //

//------------------------//

inline BSTR ConvertStringToBSTR(const char* pSrc)
{
    if(!pSrc) return NULL;

    DWORD cwch;

    BSTR wsOut(NULL);

    if(cwch = ::MultiByteToWideChar(CP_ACP, 0, pSrc, 
         -1, NULL, 0))//get size minus NULL terminator

    {
                cwch--;
            wsOut = ::SysAllocStringLen(NULL, cwch);

        if(wsOut)
        {
            if(!::MultiByteToWideChar(CP_ACP, 
                     0, pSrc, -1, wsOut, cwch))
            {
                if(ERROR_INSUFFICIENT_BUFFER == ::GetLastError())
                    return wsOut;
                ::SysFreeString(wsOut);//must clean up

                wsOut = NULL;
            }
        }

    };

    return wsOut;
};

//------------------------//

// Convert BSTR to char * //

//------------------------//

inline char* ConvertBSTRToString(BSTR pSrc)
{
    if(!pSrc) return NULL;

    //convert even embeded NULL

    DWORD cb,cwch = ::SysStringLen(pSrc);

    char *szOut = NULL;

    if(cb = ::WideCharToMultiByte(CP_ACP, 0, 
               pSrc, cwch + 1, NULL, 0, 0, 0))
    {
        szOut = new char[cb];
        if(szOut)
        {
            szOut[cb - 1]  = '\0';

            if(!::WideCharToMultiByte(CP_ACP, 0, 
                pSrc, cwch + 1, szOut, cb, 0, 0))
            {
                delete []szOut;//clean up if failed;

                szOut = NULL;
            }
        }
    }

    return szOut;
};

#endif