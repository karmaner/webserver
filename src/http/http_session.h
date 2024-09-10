#ifndef __SRC_HEEP_HTTP_SESSION_H__
#define __SRC_HTTP_HTTP_SESSION_H__

#include "src/basic/socket_stream.h"
#include "http.h"

namespace webserver {
namespace http {

class HttpSession : public SocketStream {
public:
    typedef std::shared_ptr<HttpSession> ptr;
    HttpSession(Socket::ptr sock, bool owner = true);
    HttpRequest::ptr recvRequest();
    int sendResponse(HttpResponse::ptr rsp);
};

}
}

#endif