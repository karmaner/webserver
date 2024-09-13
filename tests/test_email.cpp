#include "src/email/email.h"
#include "src/email/smtp.h"

void test() {
    webserver::EMail::ptr email = webserver::EMail::Create(
            "user@163.com", "passwd",
            "hello world", "<B>hi xxx</B>hell world", {"564628276@qq.com"});
    webserver::EMailEntity::ptr entity = webserver::EMailEntity::CreateAttach("webserver/webserver.h");
    if(entity) {
        email->addEntity(entity);
    }

    entity = webserver::EMailEntity::CreateAttach("webserver/address.cc");
    if(entity) {
        email->addEntity(entity);
    }

    auto client = webserver::SmtpClient::Create("smtp.163.com", 465, true);
    if(!client) {
        std::cout << "connect smtp.163.com:25 fail" << std::endl;
        return;
    }

    auto result = client->send(email, true);
    std::cout << "result=" << result->result << " msg=" << result->msg << std::endl;
    std::cout << client->getDebugInfo() << std::endl;
    //result = client->send(email, true);
    //std::cout << "result=" << result->result << " msg=" << result->msg << std::endl;
    //std::cout << client->getDebugInfo() << std::endl;
}

int main(int argc, char** argv) {
    webserver::IOManager iom(1);
    iom.schedule(test);
    iom.stop();
    return 0;
}
