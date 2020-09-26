#ifndef __CTRANS_H__
#define __CTRANS_H__

#include "CReqData.h"
#include <huibase.h>
#include <hredis.h>
#include <midclient.h>

#define SESSION_PHONE "SESSION_PHONE"
#define SESSION_UID "SESSION_UID"

#define CGI_NAME "CGI_NAME"
#define CGI_LOG "CGI_LOG"

using namespace HUIBASE;
using namespace HUIBASE::NOSQL;


#define BIT_RET_FACTORY 100000000

typedef NoSqlConnectionInfo ConnectionInfo;
typedef map<HSTR, ConnectionInfo> REDIS_CONN_INFOS;
typedef std::map<HSTR, CMidClient*> CLIENT_MAP;


class CReqData;

class CTrans
{
public:
	static const HINT LOCK_LIMIT_TIME = 5;

    CTrans(CReqData* pReqData);

    virtual ~ CTrans();

    virtual int StartWork();

    const ConnectionInfo& GetRedisConn (HCSTRR strName) const;

    HBOOL CanIDo (HCSTRR strKey, HCSTRR strVal, HUINT exp = 0) const throw ();

    void ThrowWebError( const string & sErrMsg ) const;

    HRET CallApi (HCSTRR strName, HCSTRR strApiName, const HCParam& inp, HCParam& outp) throw ();

    void HandleError (const HCParam & res) throw ();

    HSTR GetPhone ();

	HSTR GetUid ();

    HSTR GetCoinPic (HCSTRR strPic) throw ();

	HSTR getLockKey();

	// ���û���
	void userLocked();

	// ���û���
	void userUnlock();

protected:
    virtual int Commit() = 0;

    virtual void CheckReqVersion ();

    virtual void CheckIPLimit();
    
    void initConnections () throw ();

    virtual void readConnection () throw ();

    virtual void connectServer () throw () { }

    virtual void InitSession() throw ();

    void checkBind() throw ();

    HSTR getPicPath (HCSTRR strType);

    HSTR getPrice (HCSTRR strType);

    void getTypes (HVSTRSR types);

    void getBinds (HCSTRR strUid, HINT& kkb, HINT& con) throw ();

    HSTR getPowerInitUnit (HCSTRR strType) const;
	
	bool checkParameter( HSTR Parameter );
	
private:
    void initBusiness ();

 protected:
    CReqData* m_pReqData;

    REDIS_CONN_INFOS m_redisConns;

    mutable CLIENT_MAP m_clients;
};


#endif
