#ifndef OEAUTILS_H
#define OEAUTILS_H

#include <OeaStatus.h>

const char* oeaStatusToStr (oeaStatus status)
{
    switch (status)
    {
    case OEA_STATUS_NOMEM: 
        return "ENTITLE_STATUS_NOMEM";
    case OEA_STATUS_BAD_PARAM: 
        return "ENTITLE_STATUS_BAD_PARAM";
    case OEA_STATUS_BAD_DATA: 
        return "ENTITLE_STATUS_BAD_DATA";
    case OEA_STATUS_HTTP_URL_ERROR: 
        return "ENTITLE_STATUS_URL_ERROR";
    case OEA_STATUS_OS_LOGIN_ID_UNAVAILABLE: 
        return "ENTITLE_STATUS_OS_LOGIN_ID_UNAVAILABLE";
    case OEA_STATUS_ALREADY_ENTITLED: 
        return "ENTITLE_STATUS_ALREADY_ENTITLED";
    case OEA_STATUS_CAC_LIMIT_EXCEEDED: 
        return "ENTITLE_STATUS_ALREADY_ENTITLED";
    case OEA_STATUS_OEP_LISTENER_CREATION_FAILURE: 
        return "ENTITLE_STATUS_CAC_LIMIT_EXCEEDED";
    case OEA_HTTP_ERRHOST: 
        return "ENTITLE_HTTP_ERRHOST";
    case OEA_HTTP_ERRSOCK: 
        return "ENTITLE_HTTP_ERRSOCK";
    case OEA_HTTP_ERRCONN: 
        return "ENTITLE_HTTP_ERRCONN";
    case OEA_HTTP_ERRWRHD: 
        return "ENTITLE_HTTP_ERRWRHD";
    case OEA_HTTP_ERRWRDT: 
        return "ENTITLE_HTTP_ERRWRDT";
    case OEA_HTTP_ERRRDHD: 
        return "ENTITLE_HTTP_ERRRDHD";
    case OEA_HTTP_ERRPAHD: 
        return "ENTITLE_HTTP_ERRAHD";
    case OEA_HTTP_ERRNULL: 
        return "ENTITLE_HTTP_ERRNULL";
    case OEA_HTTP_ERRNOLG: 
        return "ENTITLE_HTTP_ERRNOLG";
    case OEA_HTTP_ERRMEM: 
        return "ENTITLE_HTTP_ERRMEM";
    case OEA_HTTP_ERRRDDT: 
        return "ENTITLE_HTTP_ERRDDT";
    case OEA_HTTP_ERRURLH: 
        return "ENTITLE_HTTP_ERRURLH";
    case OEA_HTTP_ERRURLP: 
        return "ENTITLE_HTTP_ERRURLP";
    case OEA_HTTP_BAD_QUERY: 
        return "ENTITLE_HTTP_BAD_QUERY";
    case OEA_HTTP_FORBIDDEN: 
        return "ENTITLE_HTTP_FORBIDDEN";
    case OEA_HTTP_TIMEOUT: 
        return "ENTITLE_HTTP_TIMEOUT";
    case OEA_HTTP_SERVER_ERR: 
        return "ENTITLE_HTTP_SERVER_ERR";
    case OEA_HTTP_NO_IMPL: 
        return "ENTITLE_HTTP_NO_IMPL";
    case OEA_HTTP_OVERLOAD: 
        return "ENTITLE_HTTP_OVERLOAD";
    case 9029:
        return "ENTITLE_NO_USER";
    case 9030:
        return "ENTITLE_NO_SERVERS_SPECIFIED";
    case 9031:
        return "ENTITLE_SITE_NOT_FOUND";
    default: 
        return "";
    }
}
#endif
