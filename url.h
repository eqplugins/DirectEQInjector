// url.h
#pragma once

#include <string>
using namespace std;

class CURL
{
private:
       // The default browser
    string m_strBrowser;
public:
    void Open(LPCTSTR lpszURL, bool bNewWindow = true)
    {
        if (bNewWindow)
            ::ShellExecute(NULL, NULL, GetBrowser(), lpszURL, NULL, SW_SHOWNORMAL);
        else
            ::ShellExecute(NULL, NULL, lpszURL, NULL, NULL, SW_SHOWNORMAL);
    }

    LPCTSTR GetBrowser(void)
    {
        // Do we have the default browser yet?
        if (m_strBrowser.empty())
        {
            // Get the default browser from HKCR\http\shell\open\command
            HKEY hKey = NULL;
            // Open the registry
            if (::RegOpenKeyEx(HKEY_CLASSES_ROOT, "http\\shell\\open\\command", 0,
                    KEY_READ, &hKey) == ERROR_SUCCESS)
            {
                // Data size
                DWORD cbData = 0;
                // Get the default value
                if (::RegQueryValueEx(hKey, NULL, NULL, NULL, NULL, &cbData) == ERROR_SUCCESS && cbData > 0)
                {
                    // Allocate a suitable buffer
                    TCHAR* psz = new TCHAR [cbData];
                    // Success?
                    if (psz != NULL)
                    {
                        if (::RegQueryValueEx(hKey, NULL, NULL,
                            NULL, (LPBYTE)psz, &cbData) ==
                            ERROR_SUCCESS)
                        {
                            // Success!
                            m_strBrowser = psz;
                        }
                        delete [] psz;
                    }
                }
                ::RegCloseKey(hKey);
            }
            // Do we have the browser?
            if( m_strBrowser.size() > 0 )
            {
                // Strip the full path from the string
                string::size_type IndexStart= m_strBrowser.find('"');
                string::size_type IndexEnd= m_strBrowser.rfind('"');
                // Do we have either quote?
                // If so, then the path contains spaces
                if (IndexStart != string::npos && IndexEnd != string::npos)
                {
                    // Are they the same?
                    if (IndexStart != IndexEnd)
                    {            
                        // Get the full path
                        m_strBrowser = m_strBrowser.substr(IndexStart + 1, IndexEnd - IndexStart - 1);
                    }
                }
                else
                {
                    // We may have a pathname with spaces but
                    // no quotes (Netscape), e.g.:
                    //   C:\PROGRAM FILES\NETSCAPE\COMMUNICATOR\PROGRAM\NETSCAPE.EXE -h "%1"
                    // Look for the last backslash
                    string::size_type Index_LastBackSlash = m_strBrowser.rfind('\\');
                    // Success?
                    if (Index_LastBackSlash != string::npos)
                    {
                        // Look for the next space after the final
                        // backslash
                        string::size_type Index_Space = m_strBrowser.find(' ', Index_LastBackSlash);
                        // Do we have a space?
                        if (Index_Space != string::npos)
                            m_strBrowser = m_strBrowser.substr(0, Index_Space);                
                    }
                }
            }
        }
        // Done
        return m_strBrowser.c_str();
    }
}; 
