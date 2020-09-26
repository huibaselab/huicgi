

#include "CReqData.h"

#include <des.h>
#include <md5.h>
#include <urlencode.h>
#include <base64.h>
#include <hutil.h>

#include <hlog.h>

using namespace HUIBASE::CRYPTO;

void CReqData::SetPara( const string & paraName, const string & paraValue )
{
    m_formData[paraName] = paraValue;
}

HSTR CReqData::GetPara(HCSTRR paraName) {

    if ( m_formData.find( paraName ) != m_formData.end(  ) )
    {
        return m_formData[paraName];
    }
    return "";

}


void CReqData::SetCookie( const string & paraName, const string & paraValue )
{
    m_cookieData[paraName] = paraValue;
}

void CReqData::SetEnv( const string & paraName, const string & paraValue )
{
    m_envData[paraName] = paraValue;
}

string CReqData::GetEnv( const string & paraName )
{
    if ( m_envData.find( paraName ) != m_envData.end(  ) )
    {
        return m_envData[paraName];
    }
    return "";
}

void CReqData::SetWebData( const cgicc::CgiEnvironment & cgiEnv )
{
    
    if(!strcasecmp( cgiEnv.getRequestMethod(  ).c_str(  ), "post" )){
	    m_strWebData = cgiEnv.getPostData(  );
    } else {
        // get
        m_strWebData = cgiEnv.getQueryString();
    }
}


HSTR CReqData::GetConfValue (HCSTR key) throw () {

    HASSERT_THROW(m_pConf != nullptr, ILL_PT);
    
    return m_pConf->GetValue (key);
    
}

void CReqData::SaveUploadFile(const cgicc::FormFile& uploadFile) {

    m_uploadFiles.push_back(uploadFile);

}

void CReqData::GetUploadFiles(std::vector<cgicc::FormFile>& files) {

    files = m_uploadFiles;

}



void CReqData::DePara() throw (){

    HASSERT_THROW_MSG(HIS_TRUE(m_formData.hasKey(HS_CGI_PARA)), "not parameter 'para'", INVL_PARA);

    HASSERT_THROW_MSG(HIS_TRUE(m_formData.hasKey(HS_CGI_PARA_MD5)), "not parameter 'parasign'", INVL_PARA);

    HCSTRR strPara = m_formData.GetValue(HS_CGI_PARA);

	SLOG_NORMAL("strPara length: [%d]", strPara.length());

    //HASSERT_THROW_MSG(strPara.length() > HS_CGI_PARA_LENGTH, "parameter 'para' is invalid", INVL_PARA);

    HCSTRR strSign = m_formData.GetValue(HS_CGI_PARA_MD5);

	SLOG_NORMAL("strSign length: [%d]", strSign.length());

    //HASSERT_THROW_MSG(strSign.length() > HS_CGI_PARA_MD5_LENGTH, "parameter 'parasign' is invalid", INVL_PARA);

    HSTR strUnUrl =  UrlDecode(strPara);

    if (HIS_FALSE(IsBase64Str(strUnUrl))) {
        strUnUrl = strPara;
    }

    HMEM base_mem, des_mem, src_mem;
    hstr_uvs(strUnUrl, base_mem);

    HASSERT_THROW_MSG(HIS_OK(CBase64::Decode(base_mem, des_mem)), "base64 decode failed", INVL_RES);

	SLOG_NORMAL("Des3EcbDecrypt password:%s", m_strPassword.c_str());
    HASSERT_THROW_MSG(HIS_OK(Des3EcbDecrypt(des_mem, src_mem, m_strPassword)), "des decrypt failed", SSL_ERR);
    HSTR strSrc = huvs_str(src_mem);
    //HASSERT_THROW_MSG(strSrc.length() < 16, "params is wrong", INVL_PARA);
    SLOG_NORMAL("after des: [%s]", strSrc.c_str());

    HSTR strMd5 = Md5(strSrc);

    HASSERT_THROW_MSG(strMd5 == strSign, "md5 check failed", INVL_PARA);

    m_formData.SetParam(strSrc, "&", "=");

}


CGI_BTYPE CReqData::GetCgiBusinessType () const {

    HSTR btype = m_pConf->GetValue("cgi_btype", "hsb");

    HASSERT_THROW_MSG(btype == "hsb" or btype == "hp" or btype == "kkb", "config error", SRC_FAIL);

    if (btype == "hsb") {
        return cb_hsb;
    }

    if (btype == "kkb") {
        return cb_kkb;
    }

    if (btype == "hp") {
        return cb_hp;
    }

    return cb_end;
}
