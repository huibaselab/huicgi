

#ifndef __CREQ_DATA_H__
#define __CREQ_DATA_H__



#include <cgicc/FormFile.h>
#include <cgicc/CgiEnvironment.h>
#include <limits>

#include <huibase.h>
#include <hdict.h>
#include <hconf.h>

#include <cgicc/FormFile.h>
#include <cgicc/CgiEnvironment.h>

using namespace HUIBASE;

#define HS_CGI_PARA "para"
#define HS_CGI_PARA_MD5 "parasign"

#define HS_CGI_PARA_LENGTH 32
#define HS_CGI_PARA_MD5_LENGTH 32

enum CGI_BTYPE {
    cb_kkb,
    cb_hp,
    cb_hsb,
    cb_end
};

class CReqData
{
public:
    CReqData(  )
    {
    }


    ~CReqData(  )
    {
    }


    string GetEnv( const string & paraName );

    void SetPara( const string & paraName, const string & paraValue );

    HSTR GetPara (HCSTRR paraName);

    void SetCookie( const string & paraName, const string & paraValue );

    void SetEnv( const string & paraName, const string & paraValue );

    void SetWebData( const cgicc::CgiEnvironment & cgiEnv );

    HCSTRR GetWebData () const { return m_strWebData; }

    void SetConf (HCMapConf * pConf) { m_pConf = pConf; }

    HSTR GetConfValue (HCSTR key) throw ();

    void SaveUploadFile (const cgicc::FormFile& uploadFile);

    void GetUploadFiles (std::vector<cgicc::FormFile>& files);

    void DePara() throw ();

    void SetPassword(HCSTRR strPassword) { m_strPassword = strPassword; }

    CGI_BTYPE GetCgiBusinessType () const;
    
private:

    HCParam m_formData;
    HCParam m_envData;
    
    HCParam m_cookieData;
    
    string m_strWebData;

    HCMapConf *m_pConf = nullptr;

    std::vector<cgicc::FormFile> m_uploadFiles;

    HSTR m_strPassword;
    
};


 
#endif








