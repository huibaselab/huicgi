#include "CTrans.h"

#include <huibase.h>
#include <hstr.h>

#include <libconfig.h++>
#include <htime.h>
#include <hlog.h>

#include <htime.h>
#include <hutil.h>

#include <serr.h>


using namespace HUIBASE;
using namespace HUIBASE::NOSQL;


CTrans::CTrans( CReqData * pReqData)
{
    m_pReqData = pReqData;
    HASSERT_THROW_MSG(m_pReqData != nullptr, "req_data is nullptr", ILL_PT);
}

CTrans::~CTrans() {

}


int CTrans::StartWork(  )
{

    CheckReqVersion();

    CheckIPLimit();

    initConnections();

    InitSession();

    initBusiness();

	// ���û���
	userLocked();

	int res = Commit();

	// ���û���
	userUnlock();

	return  res;
}


const ConnectionInfo& CTrans::GetRedisConn (HCSTRR strName) const{

    REDIS_CONN_INFOS::const_iterator fit = m_redisConns.find(strName);

    HASSERT_THROW_MSG(fit != m_redisConns.end(), "cann't find the redis connection info", POS_END);

    return fit->second;

}


HBOOL CTrans::CanIDo (HCSTRR strKey, HCSTRR strVal, HUINT exp) const throw () {

    const ConnectionInfo& ci = GetRedisConn("kkb_canido");

    CRedis redis(ci);
    redis.Init();

    HSTR canido_key = HCStr::Format("%s_%s", m_pReqData->GetConfValue(CGI_NAME).c_str(), strKey.c_str());

    IF_TRUE(redis.Setnx(canido_key, strVal)) {

        //HASSERT_CGI(redis.Expire(canido_key, exp), USER_CHECK_FAILED, ERR_STATUS, "username or password is invalid");
        redis.Expire(canido_key, exp);

        return HTRUE;

    }
  
    return HFALSE;
    
}


void CTrans::ThrowWebError( const string & sErrMsg ) const{

	//if (m_pReqData->GetPara("cl") == "pc") {
		cout << "Content-Type: text/plain \r\n";
		cout << "Access-Control-Allow-Origin: * \r\n";
		cout << "Access-Control-Allow-Method: POST,GET \r\n\r\n";
        //} else {
		//cout << "Content-Type: text/html \r\n\r\n";
        //}
    cout << "{\"errno\":9999,\"err_msg\":\"" << sErrMsg << "\"}" << endl;
    exit( 1 );

}

HRET CTrans::CallApi (HCSTRR strName, HCSTRR strApiName, const HCParam& inp, HCParam& outp) throw () {

    CLIENT_MAP::iterator fit = m_clients.find(strName);
    HASSERT_THROW_MSG(fit != m_clients.end(), "not this client", INDEX_OUT);

    CMidClient * p = fit->second;
    HASSERT_THROW_MSG(p != nullptr, "midclient is not init", ILL_PT);

    HCParam ps = inp;
    ps.setValue ("apiname", strApiName);

    HNOTOK_RETURN(p->CallApi (ps, outp));

    HASSERT_SS(HIS_TRUE(outp.hasKey("err_no")), 999999, ERR_STATUS, "api server return error");

    HRETURN_OK;
}


void CTrans::HandleError (const HCParam & res) throw () {

    HCSTRR err_no = res.GetValue("err_no");
    if (err_no != "0") {

        //LOG_ERROR("err-no:[%s]", err_no.c_str());
        HTHROW_MIDDLE_EX(err_no, ERR_STATUS, "call middle server failed");
		//int tem = HCStr::stoi(res.GetValue("err_no"));

        //HASSERT_CGI(tem != 1002, USER_CHECK_FAILED, ERR_STATUS, "username or password is invalid");
        //HASSERT_CGI(tem != 1101, KKB_HASBIND, ERR_STATUS, "kkb has be band");
        //HASSERT_CGI(tem != 1102, KKB_STATUS, ERR_STATUS, "kkb has be band");
        HASSERT_SS(false, 999999, ERR_STATUS, "system error");

	}

}

HSTR CTrans::GetPhone () {
    return m_pReqData->GetPara(SESSION_PHONE);
}

HSTR CTrans::GetUid (){
	return m_pReqData->GetPara(SESSION_UID);
}


HSTR CTrans::GetCoinPic (HCSTRR strPic) throw () {

    LOG_NORMAL("get pic: [%s]", strPic.c_str());
    HSTR pic;
    HSTR strKey = strPic;
    HCStr::Upper (strKey);
    strKey += "_pic";
    CRedis redis(GetRedisConn("comm"));
    redis.Get(strKey, pic);

    LOG_NORMAL("redis return: [%s][%s]", strKey.c_str(), pic.c_str());

    return pic;

}


void CTrans::initConnections () throw () {

    readConnection();

    connectServer();

}


void CTrans::readConnection () throw () {

    using namespace libconfig;

    Config cfg;

    try {

        cfg.readFile("../conf/server.cfg");

    } catch (const FileIOException& fiex) {

        SLOG_ERROR("I/O error while read file. msg: [%s]", fiex.what());

        return ;

    } catch (const ParseException& pex) {

        SLOG_ERROR("Parse config file failed at %s:%d--%s", pex.getFile(),
                  pex.getLine(), pex.getError());

        return ;

    }

    {

        const Setting& root = cfg.getRoot();

        const Setting& reds = root["reds"];
        for (int i = 0; i < reds.getLength(); ++i) {
            HSTR str;
            ConnectionInfo ci;
            reds[i].lookupValue("name", ci.strName);
            reds[i].lookupValue("ip", ci.strIp);
            reds[i].lookupValue("port", str);
            ci.nPort = HCStr::stoi(str);

            SLOG_NORMAL("redis-connection: name: [%s], ip: [%s], port: [%d]",
                       ci.strName.c_str(), ci.strIp.c_str(), ci.nPort);

            m_redisConns.insert(REDIS_CONN_INFOS::value_type(ci.strName, ci));
        }

        const Setting& cls = root["cls"];
        for (int i = 0; i < cls.getLength(); ++i) {
            HSTR strName, strIp, strPort, strTimeout;
            cls[i].lookupValue("name", strName);
            cls[i].lookupValue("ip", strIp);
            cls[i].lookupValue("port", strPort);
            cls[i].lookupValue("timeout", strTimeout);

            SLOG_NORMAL("client connection: name[%s], ip[%s], port[%s], timeout[%s]", strName.c_str(), strIp.c_str(), strPort.c_str(), strTimeout.c_str());

            CMidClient* client = new CMidClient(strIp, HCStr::stoi(strPort), HCStr::stoi(strTimeout));
            CHECK_NEWPOINT(client);

            m_clients.insert(CLIENT_MAP::value_type(strName, client));

        }

    }

}


void CTrans::CheckReqVersion() {

    HSTR str = m_pReqData->GetPara("version");
    HSTR cl = m_pReqData->GetPara("cl");
    HSTR agent = m_pReqData->GetEnv("ClientAgent");

    if (str.empty() || cl.empty() || str != "1.1")  {
	    SLOG_ES("version or useragent is empty");
	    ThrowWebError("invalid argument");
    }

    SLOG_NORMAL("Version: %s, client: %s, UserAgent: %s", str.c_str(), cl.c_str(), agent.c_str());

    if (cl != "pc" && cl != "app") {
        SLOG_ERROR("cl: [%s]", cl.c_str());
	    ThrowWebError("invalid argument");
    }

}


void CTrans::CheckIPLimit(  )
{
    SLOG_NORMAL("Get Request from %s", m_pReqData->GetEnv("ClientIp").c_str());

}


void CTrans::InitSession(  ) throw ()
{

    HSTR token = m_pReqData->GetPara( "token" );
    if (not token.empty() )
    {
        SLOG_NORMAL("input token: [%s]", token.c_str());

        HCParam ps, outps;
        ps.setValue ("token", token);

        if (m_pReqData->GetCgiBusinessType() != cb_kkb) {

            CallApi("userserver", "tokentouidapi", ps, outps);
            HandleError(outps);

            HSTR strUid = outps["uid"];
            HSTR strPhone = outps["phone"];

            SLOG_NORMAL("token: [%s], uid: [%s], phone:[%s]", token.c_str(), strUid.c_str(), strPhone.c_str());

            m_pReqData->SetPara(SESSION_UID, strUid);
            m_pReqData->SetPara(SESSION_PHONE, strPhone);

        } else {

            CallApi("kkb", "GetUser", ps, outps);
            HandleError(outps);

            HSTR strDBPhone = outps["phone"];
            m_pReqData->SetPara(SESSION_PHONE, strDBPhone);
            LOG_NORMAL("token[%s] phone[%s]", token.c_str(), strDBPhone.c_str());

        }

    }


}

void CTrans::checkBind() throw (){
    HSTR strUid, strPhone;

    if (m_pReqData->GetCgiBusinessType() == cb_kkb) {

        strPhone = m_pReqData->GetPara(SESSION_PHONE);
        if (strPhone.empty()) {
            return ;
        }

    } else {

        strUid = m_pReqData->GetPara(SESSION_UID);
        if (strUid.empty()) {
            return ;
        }

    }

    HSTR strCgiName = m_pReqData->GetConfValue(CGI_NAME);
    HSTR strNotBinds = m_pReqData->GetConfValue("NOT_BIND");
    SLOG_NORMAL("cgi name: [%s] no-bind-list: [%s]", strCgiName.c_str(), strNotBinds.c_str());

    HVSTRS binds;
    HCStr::Split(strNotBinds, "|", binds);

    if (std::find(binds.begin(), binds.end(), strCgiName) == binds.end()) {

        HINT con = 0, kkb = 0;
		if (strPhone.empty())
		{
			getBinds(strUid, kkb, con);
		}
		else
		{
		   getBinds(strPhone, kkb, con);
		}

        SLOG_NORMAL("kkb[%d], con[%d]", kkb, con);
        
        HASSERT_SS(con != 0 || kkb != 0, 002009, ERR_STATUS, "user don't bind");

    }

}


HSTR CTrans::getPicPath (HCSTRR strType) {

    const ConnectionInfo& ci = GetRedisConn("kkb_conf");
    CRedis redis(ci);
    redis.Init();

    HSTR ss = strType;
    HCStr::Upper(ss);

    HSTR res;
    HASSERT_SS(HIS_OK(redis.HGet("pic_path", ss, res)), 999999, QRY_ERR, "getPicPath qry redis failed");

    return res;


}


HSTR CTrans::getPrice (HCSTRR strType) {

    HSTR coin = strType;
    HCStr::Upper(coin);

    NoSqlConnectionInfo info;
    info.strIp = "172.18.253.68";
    info.nPort = 8101;

    CRedis redis(info);
    redis.Init();

    HSTR key = HCStr::Format("%s_price_day", coin.c_str());
    HSTR memres;
    redis.HGet(key, "price", memres);

    HFUN_END;
    
    return memres;
}


void CTrans::getTypes (HVSTRSR types) {

    types.clear();

    HSSTRS strs;

    NoSqlConnectionInfo ci = GetRedisConn("kkb_conf");

    CRedis redis(ci);
    redis.Init();

    HASSERT_SS(HIS_OK(redis.SMembers("kkb_types", strs)), 999999, QRY_ERR, "getTypes qry redis failed");
    std::copy(strs.begin(), strs.end(), std::back_inserter(types));

}


void CTrans::getBinds (HCSTRR strUid, HINT& kkb, HINT& con) throw () {
    const ConnectionInfo& ci = GetRedisConn("bindInfo");
    CRedis redis(ci);
    redis.Init();

    HSTR strKey = HCStr::Format("%s_bindkkb", strUid.c_str());
    kkb = redis.SCard(strKey);
        
    strKey = HCStr::Format("%s_bindcon", strUid.c_str());
    con = redis.SCard(strKey);

}


HSTR CTrans::getPowerInitUnit (HCSTRR strType) const {

    HSTR ss = strType;
    HCStr::Upper(ss);

    HVSTRS vs({"LKL", "LKT", "ETH", "ETC", "BYC", "XMR"});

    if (std::find(vs.begin(), vs.end(), ss) != vs.end()) {
        return "hash";
    }

    if (ss == "BURST" || ss == "STORJ") {
        return "G";
    }

    return "sol";

}

bool CTrans::checkParameter( HSTR Parameter )
{
	string key[14] = {"and","*","="," ","%0a","%","/","union","|","&","^" ,"#","/*","*/" };
	for (int i = 0; i < 14; i++)
	{
		if (Parameter.find(key[i]) != string::npos)
		{
			return false;
		}
	}
	return true;
}


void CTrans::initBusiness () {

    if (m_pReqData->GetCgiBusinessType() != cb_hsb) {

        checkBind();

    }

}

HSTR CTrans::getLockKey()
{
	HSTR user_lock_key;

	// ��ȡcgi����
	HSTR strCgiName = m_pReqData->GetConfValue(CGI_NAME);
	SLOG_NORMAL("strCgiName:%s", strCgiName.c_str());

	// �Ҽѱ�һ������������Ҫ���ᱸ�˺�
	if (strCgiName == "innerTransfer")
	{
		user_lock_key = HCStr::Format("%s_%s_lock", "13760232170", strCgiName.c_str());
	}
	else {
		// ��ȡ�ֻ���
		HSTR strPhone = m_pReqData->GetPara(SESSION_PHONE);
		// û���ֻ��ţ�����Ҫ���û���
		if (strPhone == "")
		{
			return "";
		}
		user_lock_key = HCStr::Format("%s_%s_lock", strPhone.c_str(), strCgiName.c_str());
	}
	SLOG_NORMAL("user_lock_key:%s", user_lock_key.c_str());
	return user_lock_key;
}

// ���û���
void CTrans::userLocked()
{
	HSTR user_lock_key = getLockKey();
	if (user_lock_key.empty())
	{
		return;
	}

	// ��ȡredis����
	const ConnectionInfo& ci = GetRedisConn("kkb_canido");
	CRedis redis(ci);
	redis.Init();

	IF_TRUE(redis.Setnx(user_lock_key, "0")) {
		redis.Expire(user_lock_key, LOCK_LIMIT_TIME);
	}
	else {
		HASSERT_SS(false, 005002, QRY_ERR, "user lock error");
	}

	SLOG_NORMAL("%s locked", user_lock_key.c_str());
}

// ���û���
void CTrans::userUnlock()
{
	HSTR user_lock_key = getLockKey();
	if (user_lock_key.empty())
	{
		return;
	}

	// ��ȡcgi����
	HSTR cgi_name = m_pReqData->GetConfValue(CGI_NAME);

	// ��ȡredis����
	const ConnectionInfo& ci = GetRedisConn("kkb_canido");

	CRedis redis(ci);
	redis.Init();

	redis.Del(user_lock_key);
	SLOG_NORMAL("delete lock:%s", user_lock_key.c_str());
}