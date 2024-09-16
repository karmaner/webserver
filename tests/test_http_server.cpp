#include "src/http/http_server.h"
#include "src/basic/log.h"

static webserver::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();

#define XX(...) #__VA_ARGS__


webserver::IOManager::ptr woker;
void run() {
    g_logger->setLevel(webserver::LogLevel::INFO);
    webserver::http::HttpServer::ptr server(new webserver::http::HttpServer(true));
    webserver::Address::ptr addr = webserver::Address::LookupAnyIPAddress("0.0.0.0:8020");
    while(!server->bind(addr)) {
        sleep(2);
    }
    auto sd = server->getServletDispatch();
    sd->addServlet("/webserver/xx", [](webserver::http::HttpRequest::ptr req
                ,webserver::http::HttpResponse::ptr rsp
                ,webserver::http::HttpSession::ptr session) {
            rsp->setBody(req->toString());
            return 0;
    });

    sd->addGlobServlet("/webserver/*", [](webserver::http::HttpRequest::ptr req
                ,webserver::http::HttpResponse::ptr rsp
                ,webserver::http::HttpSession::ptr session) {
            rsp->setBody("Glob:\r\n" + req->toString());
            return 0;
    });
    sd->addGlobServlet("/sylarx/*", [](webserver::http::HttpRequest::ptr req
                ,webserver::http::HttpResponse::ptr rsp
                ,webserver::http::HttpSession::ptr session) {
            rsp->setBody(XX(<html>
<head><title>404 Not Found</title></head>
<body>
<center><h1>404 Not Found</h1></center>
<hr><center>nginx/1.16.0</center>
</body>
</html>
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
));
            return 0;
    });
    server->start();
}

int main(int argc, char* argv[]) {
    //webserver::IOManager iom(4);
    webserver::IOManager iom(1, true, "main");
    woker.reset(new webserver::IOManager(3, false, "worker"));
    iom.schedule(run);
    return 0;
}