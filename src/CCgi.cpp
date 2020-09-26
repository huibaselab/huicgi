#include "CCgi.h"
#include "cgicc/CgiEnvironment.h"

#include <hprocess.h>
#include <hfname.h>
#include <hlog.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


using namespace cgicc;


CCgi::CCgi () {
    
    m_pTrans = nullptr;

}

CCgi::~CCgi (){

    if (m_pTrans != nullptr) {

        delete m_pTrans;

    }

}


void CCgi::LoadConfigure(  )
{   

    // Set CGI name;
    HSTR strName;
    if (HNOT_OK(HCProcess::GetProcName(strName))) {
        exit(-1);
    }

    HCFileName fname(strName);
    strName = fname.JustFileName ();
    m_conf.SetValue(CGI_NAME, strName);

    // Load config item in cgi_conf;
    m_conf.SetValue("conf", "../conf/cgi_conf");
    if (HNOT_OK(m_conf.LoadConfFile(m_conf.GetValue("conf")))) {
        exit(-1);
    }    

    // Setup log    
    strName = HSTR("../log/") + strName;
    m_conf.SetValue(CGI_LOG, strName);

    InitLog(strName.c_str());

    m_reqData.SetPassword(m_conf.GetValue("CGI_PS", "heshu1234567890heshuUTON"));

}

void CCgi::Run ( )
{
    try {

        LoadConfigure ();

        ERROR_INFO_MAP::Instance()->ReadErrorFromFile(m_conf.GetValue("error_file"));

        m_reqData.SetConf (&m_conf);

        FetchHttpRequest ();

        m_pTrans = MakeTransObj(&m_reqData);
        HASSERT_SS(m_pTrans != nullptr, 999999, SYS_FAILED, "new tran object failed");

        m_pTrans->StartWork();

    } catch (const CMiddleException& ex) {

		m_pTrans->userUnlock();
        SLOG_ERROR("throw a middle exception: [%s]", ex.what());
        ThrowWebError(ex);

    } catch (const HCException& ex) {

		m_pTrans->userUnlock();
        SLOG_ERROR("throw a huibase exception: [%s]", ex.what());
        ThrowWebError(ex);

    } catch (...) {

		m_pTrans->userUnlock();
        SLOG_ES("unkown error");
        cout << "Content-Type: text/plain \r\n";
        cout << "Access-Control-Allow-Origin: * \r\n";
        cout << "Access-Control-Allow-Method: POST,GET \r\n\r\n";
        cout << "{\"errno\":999999,\"err_msg\":\"Unknown Error\"}" << endl;

    }
}


HINT CCgi::GetCharset (cgicc::Cgicc * pCgi){

    const cgicc::CgiEnvironment & cgiEvn = pCgi->getEnvironment(  );

    if (!strcasecmp( cgiEvn.getRequestMethod ().c_str(), "post" ))
    {
        if (cgiEvn.getPostData().find("input_charset=GBK") !=
             string::npos)
        {
            return 1;
        }
        return 0;
    }
    else
    {
        if (cgiEvn.getQueryString ().find("input_charset=GBK") !=
             string::npos)
        {
            return 1;
        }
        return 0;
    }
}


HRET CCgi::FetchHttpRequest () {

    Cgicc formData;
    const_form_iterator iter;
    (void)GetCharset (&formData);

    for (iter = formData.getElements ().begin();
          iter != formData.getElements().end(); ++iter) {
        
        HSTR strName = iter->getName ();
        HSTR strValue = iter->getValue ();

        checkParaValid(strName);
        checkParaValid(strValue);
        SLOG_NORMAL("[%s]:[%s]", strName.c_str(), strValue.c_str());

        m_reqData.SetPara(strName, strValue);

    }

    const CgiEnvironment & env = formData.getEnvironment(  );
    
    m_reqData.SetWebData( env );

    /*LOG_NORMAL("method: %s", env.getRequestMethod().c_str());
    LOG_NORMAL("get_data: %s", env.getQueryString().c_str());
    LOG_NORMAL("data: %s", m_reqData.GetWebData().c_str());
    LOG_NORMAL("romote ip: %s", env.getRemoteAddr(  ).c_str() );
    LOG_NORMAL("userAgent: %s", env.getUserAgent().c_str());*/

    m_reqData.SetEnv("ClientIp", GetClientIp( env.getRemoteAddr(  ) ) );
    m_reqData.SetEnv("ClientAgent", env.getUserAgent(  ) );
    m_reqData.SetEnv("RequestMethod", env.getRequestMethod(  ) );
    m_reqData.SetEnv("CgiName", env.getScriptName(  ) );
    m_reqData.SetEnv("referer", env.getReferrer(  ) );
    m_reqData.SetEnv("ServerIp",
                      getenv( "SERVER_ADDR" ) ==
                      NULL ? "" : getenv( "SERVER_ADDR" ) );
    const vector < HTTPCookie > &cks = env.getCookieList(  );

    for (size_t i = 0; i < cks.size(  ); i++ )
    {
        m_reqData.SetCookie( cks[i].getName(  ), cks[i].getValue(  ) );
    }

    for (cgicc::const_file_iterator cfi = formData.getFiles().begin();
	 cfi != formData.getFiles().end(); ++cfi) {

	    m_reqData.SaveUploadFile(*cfi);
	
    }

    if (isEncryptCgi()) {
	    m_reqData.DePara();
    }

    HRETURN_OK;
}

char *CCgi::MakeMsgNo ()
{
    static char cMsgNo[32] = { 0 };
    char *pLocalIp = getenv( "SERVER_ADDR" );

    HASSERT_THROW(pLocalIp != nullptr, ILL_PT);

    in_addr_t lIp = inet_addr( pLocalIp );
    time_t t = time( NULL );
    pid_t iPid = getpid(  );

    snprintf( cMsgNo, sizeof( cMsgNo ) - 1, "510%08x%010u%05u", lIp, (unsigned)t, (unsigned)iPid );
    return cMsgNo;
}

string CCgi::GetClientIp( const string & strRemoteAddr )
{
   
    /*char *p = getenv( "HTTP_X_FORWARDED_FOR" );

    if ( p == NULL )
    {
        return strRemoteAddr;
    }
    else
    {
        CStrVector strVec;

        Tools::StrToVector( strVec, p, "," );
        return strVec[0];
	}*/
    return strRemoteAddr;
    
}

void CCgi::ThrowWebError (const CMiddleException& ex) {

    cout << "Content-Type: text/plain \r\n";
    cout << "Access-Control-Allow-Origin: * \r\n";
    cout << "Access-Control-Allow-Method: POST,GET \r\n\r\n";
    cout << "{\"errno\": \"" <<  ex.GetErrorNum() <<"\",\"err_msg\":\"" << ex.GetErrorMsg() << "\"}" << endl;

    exit(1);
    
}

void CCgi::ThrowWebError (const HCException& ex) {

    cout << "Content-Type: text/plain \r\n";
    cout << "Access-Control-Allow-Origin: * \r\n";
    cout << "Access-Control-Allow-Method: POST,GET \r\n\r\n";
    cout << "{\"errno\": \"" <<  ex.GetHErr() <<"\",\"err_msg\":\"" << ex.what() << "\"}" << endl;

    exit(1);
    
}

    
void CCgi::checkParaValid (HSTRR str) throw () {

    HASSERT_THROW_MSG(str.size(), "empty params", INVL_PARA);

    HCStr::Trim (str);

    HCStr::Removes (str, "'");
    HCStr::Removes (str, "\"");
    HCStr::Removes (str, "\r");
    //HCStr::Removes (str, "\n");

}


bool CCgi::isEncryptCgi () {

    HSTR strCgiName = m_conf.GetValue(CGI_NAME);
    HSTR strNoEncryptCgis = m_conf.GetValue("NO_ENCRYPT_CGI");

    HVSTRS binds;
    HCStr::Split(strNoEncryptCgis, "|", binds);

    if (std::find(binds.begin(), binds.end(), strCgiName) == binds.end()) {

        return true;

    }

    return false;

}