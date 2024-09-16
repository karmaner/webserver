#include "redis.h"
#include "src/webserver.h"
#include "src/basic/log.h"

namespace webserver {

static webserver::Logger::ptr g_logger = WEBSERVER_LOG_NAME("system");
static webserver::ConfigVar<std::map<std::string, std::map<std::string, std::string> > >::ptr g_redis =
    webserver::Config::Lookup("redis.config", std::map<std::string, std::map<std::string, std::string> >(), "redis config");

static std::string get_value(const std::map<std::string, std::string>& m
                                ,const std::string& key
                                ,const std::string& def = "") {
    auto it = m.find(key);
    return it == m.end() ? def : it->second;
}

redisReply* RedisReplyClone(redisReply* r) {
    redisReply* c = (redisReply*)calloc(1, sizeof(*c));
    c->type = r->type;

    switch(r->type) {
        case REDIS_REPLY_INTEGER:
            c->integer = r->integer;
            break;
        case REDIS_REPLY_ARRAY:
            if(r->element != NULL && r->elements > 0) {
                c->element = (redisReply**)calloc(r->elements, sizeof(r));
                c->elements = r->elements;
                for(size_t i = 0; i < r->elements; ++i) {
                    c->element[i] = RedisReplyClone(r->element[i]);
                }
            }
            break;
        case REDIS_REPLY_ERROR:
        case REDIS_REPLY_STATUS:
        case REDIS_REPLY_STRING:
            if(r->str == NULL) {
                c->str = NULL;
            } else {
                //c->str = strndup(r->str, r->len);
                c->str = (char*)malloc(r->len + 1);
                memcpy(c->str, r->str, r->len);
                c->str[r->len] = '\0';
            }
            c->len = r->len;
            break;
    }
    return c;
}

Redis::Redis() {
    m_type = IRedis::REDIS;
}

Redis::Redis(const std::map<std::string, std::string>& conf) {
    m_type = IRedis::REDIS;
    auto tmp = get_value(conf, "host");
    auto pos = tmp.find(":");
    m_host = tmp.substr(0, pos);
    m_port = webserver::TypeUtil::Atoi(tmp.substr(pos + 1));
    m_logEnable = webserver::TypeUtil::Atoi(get_value(conf, "log_enable", "1"));

    tmp = get_value(conf, "timeout_com");
    if(tmp.empty()) {
        tmp = get_value(conf, "timeout");
    }
    uint64_t v = webserver::TypeUtil::Atoi(tmp);

    m_cmdTimeout.tv_sec = v / 1000;
    m_cmdTimeout.tv_usec = v % 1000 * 1000;
}

bool Redis::reconnect() {
    return redisReconnect(m_context.get());
}

bool Redis::connect() {
    return connect(m_host, m_port, 50);
}

bool Redis::connect(const std::string& ip, int port, uint64_t ms) {
    m_host = ip;
    m_port = port;
    m_connectMs = ms;
    if(m_context) {
        return true;
    }
    timeval tv = {(int)ms / 1000, (int)ms % 1000 * 1000};
    auto c = redisConnectWithTimeout(ip.c_str(), port, tv);
    if(c) {
        if(m_cmdTimeout.tv_sec || m_cmdTimeout.tv_usec) {
            setTimeout(m_cmdTimeout.tv_sec * 1000 + m_cmdTimeout.tv_usec / 1000);
        }
        m_context.reset(c, redisFree);
        return true;
    }
    return false;
}

bool Redis::setTimeout(uint64_t v) {
    m_cmdTimeout.tv_sec = v / 1000;
    m_cmdTimeout.tv_usec = v % 1000 * 1000;
    redisSetTimeout(m_context.get(), m_cmdTimeout);
    return true;
}

ReplyPtr Redis::cmd(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    ReplyPtr rt = cmd(fmt, ap);
    va_end(ap);
    return rt;
}

ReplyPtr Redis::cmd(const char* fmt, va_list ap) {
    auto r = (redisReply*)redisvCommand(m_context.get(), fmt, ap);
    if(!r) {
        if(m_logEnable) {
            WEBSERVER_LOG_ERROR(g_logger) << "redisCommand error: (" << fmt << ")(" << m_host << ":" << m_port << ")(" << m_name << ")";
        }
        return nullptr;
    }
    ReplyPtr rt(r, freeReplyObject);
    if(r->type != REDIS_REPLY_ERROR) {
        return rt;
    }
    if(m_logEnable) {
        WEBSERVER_LOG_ERROR(g_logger) << "redisCommand error: (" << fmt << ")(" << m_host << ":" << m_port << ")(" << m_name << ")"
                    << ": " << r->str;
    }
    return nullptr;
}

ReplyPtr Redis::cmd(const std::vector<std::string>& argv) {
    std::vector<const char*> v;
    std::vector<size_t> l;
    for(auto& i : argv) {
        v.push_back(i.c_str());
        l.push_back(i.size());
    }

    auto r = (redisReply*)redisCommandArgv(m_context.get(), argv.size(), &v[0], &l[0]);
    if(!r) {
        if(m_logEnable) {
            WEBSERVER_LOG_ERROR(g_logger) << "redisCommandArgv error: (" << m_host << ":" << m_port << ")(" << m_name << ")";
        }
        return nullptr;
    }
    ReplyPtr rt(r, freeReplyObject);
    if(r->type != REDIS_REPLY_ERROR) {
        return rt;
    }
    if(m_logEnable) {
        WEBSERVER_LOG_ERROR(g_logger) << "redisCommandArgv error: (" << m_host << ":" << m_port << ")(" << m_name << ")"
                    << r->str;
    }
    return nullptr;
}

ReplyPtr Redis::getReply() {
    redisReply* r = nullptr;
    if(redisGetReply(m_context.get(), (void**)&r) == REDIS_OK) {
        ReplyPtr rt(r, freeReplyObject);
        return rt;
    }
    if(m_logEnable) {
        WEBSERVER_LOG_ERROR(g_logger) << "redisGetReply error: (" << m_host << ":" << m_port << ")(" << m_name << ")";
    }
    return nullptr;
}

int Redis::appendCmd(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int rt = appendCmd(fmt, ap);
    va_end(ap);
    return rt;

}

int Redis::appendCmd(const char* fmt, va_list ap) {
    return redisvAppendCommand(m_context.get(), fmt, ap);
}

int Redis::appendCmd(const std::vector<std::string>& argv) {
    std::vector<const char*> v;
    std::vector<size_t> l;
    for(auto& i : argv) {
        v.push_back(i.c_str());
        l.push_back(i.size());
    }
    return redisAppendCommandArgv(m_context.get(), argv.size(), &v[0], &l[0]);
}

RedisCluster::RedisCluster() {
    m_type = IRedis::REDIS_CLUSTER;
}

RedisCluster::RedisCluster(const std::map<std::string, std::string>& conf) {
    m_type = IRedis::REDIS_CLUSTER;
    m_host = get_value(conf, "host");
    m_logEnable = webserver::TypeUtil::Atoi(get_value(conf, "log_enable", "1"));
    auto tmp = get_value(conf, "timeout_com");
    if(tmp.empty()) {
        tmp = get_value(conf, "timeout");
    }
    uint64_t v = webserver::TypeUtil::Atoi(tmp);

    m_cmdTimeout.tv_sec = v / 1000;
    m_cmdTimeout.tv_usec = v % 1000 * 1000;
}


////RedisCluster
bool RedisCluster::reconnect() {
    return true;
    //return redisReconnect(m_context.get());
}

bool RedisCluster::connect() {
    return connect(m_host, m_port, 50);
}

bool RedisCluster::connect(const std::string& ip, int port, uint64_t ms) {
    m_host = ip;
    m_port = port;
    m_connectMs = ms;
    if(m_context) {
        return true;
    }
    timeval tv = {(int)ms / 1000, (int)ms % 1000 * 1000};
    auto c = redisClusterConnectWithTimeout(ip.c_str(), tv, 0);
    if(c) {
        m_context.reset(c, redisClusterFree);
        return true;
    }
    return false;
}

bool RedisCluster::setTimeout(uint64_t ms) {
    //timeval tv = {(int)ms / 1000, (int)ms % 1000 * 1000};
    //redisSetTimeout(m_context.get(), tv);
    return true;
}

ReplyPtr RedisCluster::cmd(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    ReplyPtr rt = cmd(fmt, ap);
    va_end(ap);
    return rt;
}

ReplyPtr RedisCluster::cmd(const char* fmt, va_list ap) {
    auto r = (redisReply*)redisClustervCommand(m_context.get(), fmt, ap);
    if(!r) {
        if(m_logEnable) {
            WEBSERVER_LOG_ERROR(g_logger) << "redisCommand error: (" << fmt << ")(" << m_host << ":" << m_port << ")(" << m_name << ")";
        }
        return nullptr;
    }
    ReplyPtr rt(r, freeReplyObject);
    if(r->type != REDIS_REPLY_ERROR) {
        return rt;
    }
    if(m_logEnable) {
        WEBSERVER_LOG_ERROR(g_logger) << "redisCommand error: (" << fmt << ")(" << m_host << ":" << m_port << ")(" << m_name << ")"
                    << ": " << r->str;
    }
    return nullptr;
}

ReplyPtr RedisCluster::cmd(const std::vector<std::string>& argv) {
    std::vector<const char*> v;
    std::vector<size_t> l;
    for(auto& i : argv) {
        v.push_back(i.c_str());
        l.push_back(i.size());
    }

    auto r = (redisReply*)redisClusterCommandArgv(m_context.get(), argv.size(), &v[0], &l[0]);
    if(!r) {
        if(m_logEnable) {
            WEBSERVER_LOG_ERROR(g_logger) << "redisCommandArgv error: (" << m_host << ":" << m_port << ")(" << m_name << ")";
        }
        return nullptr;
    }
    ReplyPtr rt(r, freeReplyObject);
    if(r->type != REDIS_REPLY_ERROR) {
        return rt;
    }
    if(m_logEnable) {
        WEBSERVER_LOG_ERROR(g_logger) << "redisCommandArgv error: (" << m_host << ":" << m_port << ")(" << m_name << ")"
                    << r->str;
    }
    return nullptr;
}

ReplyPtr RedisCluster::getReply() {
    redisReply* r = nullptr;
    if(redisClusterGetReply(m_context.get(), (void**)&r) == REDIS_OK) {
        ReplyPtr rt(r, freeReplyObject);
        return rt;
    }
    if(m_logEnable) {
        WEBSERVER_LOG_ERROR(g_logger) << "redisGetReply error: (" << m_host << ":" << m_port << ")(" << m_name << ")";
    }
    return nullptr;
}

int RedisCluster::appendCmd(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int rt = appendCmd(fmt, ap);
    va_end(ap);
    return rt;

}

int RedisCluster::appendCmd(const char* fmt, va_list ap) {
    return redisClustervAppendCommand(m_context.get(), fmt, ap);
}

int RedisCluster::appendCmd(const std::vector<std::string>& argv) {
    std::vector<const char*> v;
    std::vector<size_t> l;
    for(auto& i : argv) {
        v.push_back(i.c_str());
        l.push_back(i.size());
    }
    return redisClusterAppendCommandArgv(m_context.get(), argv.size(), &v[0], &l[0]);
}

FoxRedis::FoxRedis(webserver::FoxThread* thr, const std::map<std::string, std::string>& conf)
    :m_thread(thr)
    ,m_status(UNCONNECTED)
    ,m_event(nullptr) {
    m_type = IRedis::FOX_REDIS;
    auto tmp = get_value(conf, "host");
    auto pos = tmp.find(":");
    m_host = tmp.substr(0, pos);
    m_port = webserver::TypeUtil::Atoi(tmp.substr(pos + 1));
    m_ctxCount = 0;
    m_logEnable = webserver::TypeUtil::Atoi(get_value(conf, "log_enable", "1"));

    tmp = get_value(conf, "timeout_com");
    if(tmp.empty()) {
        tmp = get_value(conf, "timeout");
    }
    uint64_t v = webserver::TypeUtil::Atoi(tmp);

    m_cmdTimeout.tv_sec = v / 1000;
    m_cmdTimeout.tv_usec = v % 1000 * 1000;
}

void FoxRedis::ConnectCb(const redisAsyncContext* c, int status) {
    FoxRedis* ar = static_cast<FoxRedis*>(c->data);
    if(!status) {
        WEBSERVER_LOG_INFO(g_logger) << "FoxRedis::ConnectCb "
                    << c->c.tcp.host << ":" << c->c.tcp.port
                    << " success";
        ar->m_status = CONNECTED;
    } else {
        WEBSERVER_LOG_ERROR(g_logger) << "FoxRedis::ConnectCb "
                    << c->c.tcp.host << ":" << c->c.tcp.port
                    << " fail, error:" << c->errstr;
        ar->m_status = UNCONNECTED;
    }
}

void FoxRedis::DisconnectCb(const redisAsyncContext* c, int status) {
    WEBSERVER_LOG_INFO(g_logger) << "FoxRedis::DisconnectCb "
                << c->c.tcp.host << ":" << c->c.tcp.port
                << " status:" << status;
    FoxRedis* ar = static_cast<FoxRedis*>(c->data);
    ar->m_status = UNCONNECTED;
}

void FoxRedis::CmdCb(redisAsyncContext *ac, void *r, void *privdata) {
    Ctx* ctx = static_cast<Ctx*>(privdata);
    if(!ctx) {
        return;
    }
    if(ctx->timeout) {
        delete ctx;
        //if(ctx && ctx->fiber) {
        //    WEBSERVER_LOG_ERROR(g_logger) << "redis cmd: '" << ctx->cmd << "' timeout("
        //                << (ctx->rds->m_cmdTimeout.tv_sec * 1000
        //                        + ctx->rds->m_cmdTimeout.tv_usec / 1000)
        //                << "ms)";
        //    ctx->scheduler->schedule(&ctx->fiber);
        //    ctx->cancelEvent();
        //}
        return;
    }

    auto m_logEnable = ctx->rds->m_logEnable;

    redisReply* reply = (redisReply*)r;
    if(ac->err) {
        if(m_logEnable) {
            webserver::replace(ctx->cmd, "\r\n", "\\r\\n");
            WEBSERVER_LOG_ERROR(g_logger) << "redis cmd: '" << ctx->cmd
                        << "' "
                        << "(" << ac->err << ") " << ac->errstr;
        }
        if(ctx->fctx->fiber) {
            ctx->fctx->scheduler->schedule(&ctx->fctx->fiber);
        }
    } else if(!reply) {
        if(m_logEnable) {
            webserver::replace(ctx->cmd, "\r\n", "\\r\\n");
            WEBSERVER_LOG_ERROR(g_logger) << "redis cmd: '" << ctx->cmd
                        << "' "
                        << "reply: NULL";
        }
        if(ctx->fctx->fiber) {
            ctx->fctx->scheduler->schedule(&ctx->fctx->fiber);
        }
    } else if(reply->type == REDIS_REPLY_ERROR) {
        if(m_logEnable) {
            webserver::replace(ctx->cmd, "\r\n", "\\r\\n");
            WEBSERVER_LOG_ERROR(g_logger) << "redis cmd: '" << ctx->cmd
                        << "' "
                        << "reply: " << reply->str;
        }
        if(ctx->fctx->fiber) {
            ctx->fctx->scheduler->schedule(&ctx->fctx->fiber);
        }
    } else {
        if(ctx->fctx->fiber) {
            ctx->fctx->rpy.reset(RedisReplyClone(reply), freeReplyObject);
            ctx->fctx->scheduler->schedule(&ctx->fctx->fiber);
        }
    }
    ctx->cancelEvent();
    delete ctx;
}

void FoxRedis::TimeCb(int fd, short event, void* d) {
    FoxRedis* ar = static_cast<FoxRedis*>(d);
    redisAsyncCommand(ar->m_context.get(), CmdCb, nullptr, "ping");
}

struct Res {
    redisAsyncContext* ctx;
    struct event* event;
};

//void DelayTimeCb(int fd, short event, void* d) {
//    WEBSERVER_LOG_INFO(g_logger) << "DelayTimeCb";
//    Res* res = static_cast<Res*>(d);
//    redisAsyncFree(res->ctx);
//    evtimer_del(res->event);
//    event_free(res->event);
//    delete res;
//}

bool FoxRedis::init() {
    if(m_thread == webserver::FoxThread::GetThis()) {
        return pinit();
    } else {
        m_thread->dispatch(std::bind(&FoxRedis::pinit, this));
    }
    return true;
}

void FoxRedis::delayDelete(redisAsyncContext* c) {
    //if(!c) {
    //    return;
    //}

    //Res* res = new Res();
    //res->ctx = c;
    //struct event* event = event_new(m_thread->getBase(), -1, EV_TIMEOUT, DelayTimeCb, res);
    //res->event = event;
    //
    //struct timeval tv = {60, 0};
    //evtimer_add(event, &tv);
}

bool FoxRedis::pinit() {
    //WEBSERVER_LOG_INFO(g_logger) << "pinit m_status=" << m_status;
    if(m_status != UNCONNECTED) {
        return true;
    }
    auto ctx = redisAsyncConnect(m_host.c_str(), m_port);
    if(!ctx) {
        WEBSERVER_LOG_ERROR(g_logger) << "redisAsyncConnect (" << m_host << ":" << m_port
                    << ") null";
        return false;
    }
    if(ctx->err) {
        WEBSERVER_LOG_ERROR(g_logger) << "Error:(" << ctx->err << ")" << ctx->errstr;
        return false;
    }
    ctx->data = this;
    redisLibeventAttach(ctx, m_thread->getBase());
    redisAsyncSetConnectCallback(ctx, ConnectCb);
    redisAsyncSetDisconnectCallback(ctx, DisconnectCb);
    m_status = CONNECTING;
    //m_context.reset(ctx, redisAsyncFree);
    m_context.reset(ctx, webserver::nop<redisAsyncContext>);
    //m_context.reset(ctx, std::bind(&FoxRedis::delayDelete, this, std::placeholders::_1));
    if(m_event == nullptr) {
        m_event = event_new(m_thread->getBase(), -1, EV_TIMEOUT | EV_PERSIST, TimeCb, this);
        struct timeval tv = {120, 0};
        evtimer_add(m_event, &tv);
    }
    TimeCb(0, 0, this);
    return true;
}

ReplyPtr FoxRedis::cmd(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    auto r = cmd(fmt, ap);
    va_end(ap);
    return r;
}

ReplyPtr FoxRedis::cmd(const char* fmt, va_list ap) {
    char* buf = nullptr;
    //int len = vasprintf(&buf, fmt, ap);
    int len = redisvFormatCommand(&buf, fmt, ap);
    if(len == -1) {
        WEBSERVER_LOG_ERROR(g_logger) << "redis fmt error: " << fmt;
        return nullptr;
    }
    //Ctx::ptr ctx(new Ctx(this));
    //if(buf) {
    //    ctx->cmd.append(buf, len);
    //    free(buf);
    //}
    //ctx->scheduler = webserver::Scheduler::GetThis();
    //ctx->fiber = webserver::Fiber::GetThis();
    //ctx->thread = m_thread;

    FCtx fctx;
    fctx.cmd.append(buf, len);
    free(buf);
    fctx.scheduler = webserver::Scheduler::GetThis();
    fctx.fiber = webserver::Fiber::GetThis();

    m_thread->dispatch(std::bind(&FoxRedis::pcmd, this, &fctx));
    webserver::Fiber::YieldToHold();
    return fctx.rpy;
}

ReplyPtr FoxRedis::cmd(const std::vector<std::string>& argv) {
    //Ctx::ptr ctx(new Ctx(this));
    //ctx->parts = argv;
    FCtx fctx;
    do {
        std::vector<const char*> args;
        std::vector<size_t> args_len;
        for(auto& i : argv) {
            args.push_back(i.c_str());
            args_len.push_back(i.size());
        }
        char* buf = nullptr;
        int len = redisFormatCommandArgv(&buf, argv.size(), &(args[0]), &(args_len[0]));
        if(len == -1 || !buf) {
            WEBSERVER_LOG_ERROR(g_logger) << "redis fmt error";
            return nullptr;
        }
        fctx.cmd.append(buf, len);
        free(buf);
    } while(0);

    //ctx->scheduler = webserver::Scheduler::GetThis();
    //ctx->fiber = webserver::Fiber::GetThis();
    //ctx->thread = m_thread;

    fctx.scheduler = webserver::Scheduler::GetThis();
    fctx.fiber = webserver::Fiber::GetThis();

    m_thread->dispatch(std::bind(&FoxRedis::pcmd, this, &fctx));
    webserver::Fiber::YieldToHold();
    return fctx.rpy;
}

void FoxRedis::pcmd(FCtx* fctx) {
    if(m_status == UNCONNECTED) {
        WEBSERVER_LOG_INFO(g_logger) << "redis (" << m_host << ":" << m_port << ") unconnected "
                    << fctx->cmd;
        init();
        if(fctx->fiber) {
            fctx->scheduler->schedule(&fctx->fiber);
        }
        return;
    }
    Ctx* ctx(new Ctx(this));
    ctx->thread = m_thread;
    ctx->init();
    ctx->fctx = fctx;
    ctx->cmd = fctx->cmd;

    if(!ctx->cmd.empty()) {
        //redisAsyncCommand(m_context.get(), CmdCb, ctx.get(), ctx->cmd.c_str());
        redisAsyncFormattedCommand(m_context.get(), CmdCb, ctx, ctx->cmd.c_str(), ctx->cmd.size());
    //} else if(!ctx->parts.empty()) {
    //    std::vector<const char*> argv;
    //    std::vector<size_t> argv_len;
    //    for(auto& i : ctx->parts) {
    //        argv.push_back(i.c_str());
    //        argv_len.push_back(i.size());
    //    }
    //    redisAsyncCommandArgv(m_context.get(), CmdCb, ctx.get(), argv.size(),
    //            &(argv[0]), &(argv_len[0]));
    }
}

FoxRedis::~FoxRedis() {
    if(m_event) {
        evtimer_del(m_event);
        event_free(m_event);
    }
}

FoxRedis::Ctx::Ctx(FoxRedis* r)
    :ev(nullptr)
    ,timeout(false)
    ,rds(r)
    //,scheduler(nullptr)
    ,thread(nullptr) {
    webserver::Atomic::addFetch(rds->m_ctxCount, 1);
}

FoxRedis::Ctx::~Ctx() {
    //cancelEvent();
    WEBSERVER_ASSERT(thread == webserver::FoxThread::GetThis());
    //WEBSERVER_ASSERT(destory == 0);
    webserver::Atomic::subFetch(rds->m_ctxCount, 1);
    //++destory;
    //cancelEvent();
    if(ev) {
        evtimer_del(ev);
        event_free(ev);
        ev = nullptr;
    }

}

void FoxRedis::Ctx::cancelEvent() {
    //if(ev) {
    //    if(thread == webserver::FoxThread::GetThis()) {
    //        evtimer_del(ev);
    //        event_free(ev);
    //    } else {
    //        auto e = ev;
    //        thread->dispatch([e](){
    //            evtimer_del(e);
    //            event_free(e);
    //        });
    //    }
    //    ev = nullptr;
    //}
    //ref = nullptr;
}

bool FoxRedis::Ctx::init() {
    ev = evtimer_new(rds->m_thread->getBase(), EventCb, this);
    evtimer_add(ev, &rds->m_cmdTimeout);
    return true;
}

void FoxRedis::Ctx::EventCb(int fd, short event, void* d) {
    Ctx* ctx = static_cast<Ctx*>(d);
    ctx->timeout = 1;
    if(ctx->rds->m_logEnable) {
        webserver::replace(ctx->cmd, "\r\n", "\\r\\n");
        WEBSERVER_LOG_INFO(g_logger) << "redis cmd: '" << ctx->cmd << "' reach timeout "
                   << (ctx->rds->m_cmdTimeout.tv_sec * 1000
                            + ctx->rds->m_cmdTimeout.tv_usec / 1000) << "ms";
    }
    if(ctx->fctx->fiber) {
        ctx->fctx->scheduler->schedule(&ctx->fctx->fiber);
    }
    ctx->cancelEvent();
    //ctx->ref = nullptr;
}

FoxRedisCluster::FoxRedisCluster(webserver::FoxThread* thr, const std::map<std::string, std::string>& conf)
    :m_thread(thr)
    ,m_status(UNCONNECTED)
    ,m_event(nullptr) {
    m_ctxCount = 0;

    m_type = IRedis::FOX_REDIS_CLUSTER;
    m_host = get_value(conf, "host");
    m_logEnable = webserver::TypeUtil::Atoi(get_value(conf, "log_enable", "1"));
    auto tmp = get_value(conf, "timeout_com");
    if(tmp.empty()) {
        tmp = get_value(conf, "timeout");
    }
    uint64_t v = webserver::TypeUtil::Atoi(tmp);

    m_cmdTimeout.tv_sec = v / 1000;
    m_cmdTimeout.tv_usec = v % 1000 * 1000;
}

void FoxRedisCluster::ConnectCb(const redisAsyncContext* c, int status) {
    if(!status) {
        WEBSERVER_LOG_INFO(g_logger) << "FoxRedisCluster::ConnectCb "
                    << c->c.tcp.host << ":" << c->c.tcp.port
                    << " success";
    } else {
        WEBSERVER_LOG_ERROR(g_logger) << "FoxRedisCluster::ConnectCb "
                    << c->c.tcp.host << ":" << c->c.tcp.port
                    << " fail, error:" << c->errstr;
    }
}

void FoxRedisCluster::DisconnectCb(const redisAsyncContext* c, int status) {
    WEBSERVER_LOG_INFO(g_logger) << "FoxRedisCluster::DisconnectCb "
                << c->c.tcp.host << ":" << c->c.tcp.port
                << " status:" << status;
}

void FoxRedisCluster::CmdCb(redisClusterAsyncContext *ac, void *r, void *privdata) {
    Ctx* ctx = static_cast<Ctx*>(privdata);
    if(ctx->timeout) {
        delete ctx;
        //if(ctx && ctx->fiber) {
        //    WEBSERVER_LOG_ERROR(g_logger) << "redis cmd: '" << ctx->cmd << "' timeout("
        //                << (ctx->rds->m_cmdTimeout.tv_sec * 1000
        //                        + ctx->rds->m_cmdTimeout.tv_usec / 1000)
        //                << "ms)";
        //    ctx->scheduler->schedule(&ctx->fiber);
        //    ctx->cancelEvent();
        //}
        return;
    }
    auto m_logEnable = ctx->rds->m_logEnable;
    //ctx->cancelEvent();
    redisReply* reply = (redisReply*)r;
    //++ctx->callback_count;
    if(ac->err) {
        if(m_logEnable) {
            webserver::replace(ctx->cmd, "\r\n", "\\r\\n");
            WEBSERVER_LOG_ERROR(g_logger) << "redis cmd: '" << ctx->cmd
                        << "' "
                        << "(" << ac->err << ") " << ac->errstr;
        }
        if(ctx->fctx->fiber) {
            ctx->fctx->scheduler->schedule(&ctx->fctx->fiber);
        }
    } else if(!reply) {
        if(m_logEnable) {
            webserver::replace(ctx->cmd, "\r\n", "\\r\\n");
            WEBSERVER_LOG_ERROR(g_logger) << "redis cmd: '" << ctx->cmd
                        << "' "
                        << "reply: NULL";
        }
        if(ctx->fctx->fiber) {
            ctx->fctx->scheduler->schedule(&ctx->fctx->fiber);
        }
    } else if(reply->type == REDIS_REPLY_ERROR) {
        if(m_logEnable) {
            webserver::replace(ctx->cmd, "\r\n", "\\r\\n");
            WEBSERVER_LOG_ERROR(g_logger) << "redis cmd: '" << ctx->cmd
                        << "' "
                        << "reply: " << reply->str;
        }
        if(ctx->fctx->fiber) {
            ctx->fctx->scheduler->schedule(&ctx->fctx->fiber);
        }
    } else {
        if(ctx->fctx->fiber) {
            ctx->fctx->rpy.reset(RedisReplyClone(reply), freeReplyObject);
            ctx->fctx->scheduler->schedule(&ctx->fctx->fiber);
        }
    }
    //ctx->ref = nullptr;
    delete ctx;
    //ctx->tref = nullptr;
}

void FoxRedisCluster::TimeCb(int fd, short event, void* d) {
    //FoxRedisCluster* ar = static_cast<FoxRedisCluster*>(d);
    //redisAsyncCommand(ar->m_context.get(), CmdCb, nullptr, "ping");
}

bool FoxRedisCluster::init() {
    if(m_thread == webserver::FoxThread::GetThis()) {
        return pinit();
    } else {
        m_thread->dispatch(std::bind(&FoxRedisCluster::pinit, this));
    }
    return true;
}

void FoxRedisCluster::delayDelete(redisAsyncContext* c) {
    //if(!c) {
    //    return;
    //}

    //Res* res = new Res();
    //res->ctx = c;
    //struct event* event = event_new(m_thread->getBase(), -1, EV_TIMEOUT, DelayTimeCb, res);
    //res->event = event;
    //
    //struct timeval tv = {60, 0};
    //evtimer_add(event, &tv);
}

bool FoxRedisCluster::pinit() {
    if(m_status != UNCONNECTED) {
        return true;
    }
    WEBSERVER_LOG_INFO(g_logger) << "FoxRedisCluster pinit:" << m_host;
    auto ctx = redisClusterAsyncConnect(m_host.c_str(), 0);
    if(!ctx) {
        WEBSERVER_LOG_ERROR(g_logger) << "redisClusterAsyncConnect (" << m_host
                    << ") null";
        return false;
    }
    if(ctx->err) {
        WEBSERVER_LOG_ERROR(g_logger) << "Error:(" << ctx->err << ")" << ctx->errstr;
        return false;
    }
    ctx->data = this;
    redisClusterLibeventAttach(ctx, m_thread->getBase());
    redisClusterAsyncSetConnectCallback(ctx, ConnectCb);
    redisClusterAsyncSetDisconnectCallback(ctx, DisconnectCb);
    m_status = CONNECTED;
    //m_context.reset(ctx, redisAsyncFree);
    m_context.reset(ctx, webserver::nop<redisClusterAsyncContext>);
    //m_context.reset(ctx, std::bind(&FoxRedisCluster::delayDelete, this, std::placeholders::_1));
    if(m_event == nullptr) {
        m_event = event_new(m_thread->getBase(), -1, EV_TIMEOUT | EV_PERSIST, TimeCb, this);
        struct timeval tv = {120, 0};
        evtimer_add(m_event, &tv);
        TimeCb(0, 0, this);
    }
    return true;
}

ReplyPtr FoxRedisCluster::cmd(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    auto r = cmd(fmt, ap);
    va_end(ap);
    return r;
}

ReplyPtr FoxRedisCluster::cmd(const char* fmt, va_list ap) {
    char* buf = nullptr;
    //int len = vasprintf(&buf, fmt, ap);
    int len = redisvFormatCommand(&buf, fmt, ap);
    if(len == -1 || !buf) {
        WEBSERVER_LOG_ERROR(g_logger) << "redis fmt error: " << fmt;
        return nullptr;
    }
    FCtx fctx;
    fctx.cmd.append(buf, len);
    free(buf);
    fctx.scheduler = webserver::Scheduler::GetThis();
    fctx.fiber = webserver::Fiber::GetThis();
    //Ctx::ptr ctx(new Ctx(this));
    //if(buf) {
    //    ctx->cmd.append(buf, len);
    //    free(buf);
    //}
    //ctx->scheduler = webserver::Scheduler::GetThis();
    //ctx->fiber = webserver::Fiber::GetThis();
    //ctx->thread = m_thread;

    m_thread->dispatch(std::bind(&FoxRedisCluster::pcmd, this, &fctx));
    webserver::Fiber::YieldToHold();
    return fctx.rpy;
}

ReplyPtr FoxRedisCluster::cmd(const std::vector<std::string>& argv) {
    //Ctx::ptr ctx(new Ctx(this));
    //ctx->parts = argv;
    FCtx fctx;
    do {
        std::vector<const char*> args;
        std::vector<size_t> args_len;
        for(auto& i : argv) {
            args.push_back(i.c_str());
            args_len.push_back(i.size());
        }
        char* buf = nullptr;
        int len = redisFormatCommandArgv(&buf, argv.size(), &(args[0]), &(args_len[0]));
        if(len == -1 || !buf) {
            WEBSERVER_LOG_ERROR(g_logger) << "redis fmt error";
            return nullptr;
        }
        fctx.cmd.append(buf, len);
        free(buf);
    } while(0);

    fctx.scheduler = webserver::Scheduler::GetThis();
    fctx.fiber = webserver::Fiber::GetThis();

    m_thread->dispatch(std::bind(&FoxRedisCluster::pcmd, this, &fctx));
    webserver::Fiber::YieldToHold();
    return fctx.rpy;
}

void FoxRedisCluster::pcmd(FCtx* fctx) {
    if(m_status != CONNECTED) {
        WEBSERVER_LOG_INFO(g_logger) << "redis (" << m_host << ") unconnected "
                    << fctx->cmd;
        init();
        if(fctx->fiber) {
            fctx->scheduler->schedule(&fctx->fiber);
        }
        return;
    }
    Ctx* ctx(new Ctx(this));
    ctx->thread = m_thread;
    ctx->init();
    ctx->fctx = fctx;
    ctx->cmd = fctx->cmd;
    //ctx->ref = ctx;
    //ctx->tref = ctx;
    if(!ctx->cmd.empty()) {
        //redisClusterAsyncCommand(m_context.get(), CmdCb, ctx.get(), ctx->cmd.c_str());
        redisClusterAsyncFormattedCommand(m_context.get(), CmdCb, ctx, &ctx->cmd[0], ctx->cmd.size());
    //} else if(!ctx->parts.empty()) {
    //    std::vector<const char*> argv;
    //    std::vector<size_t> argv_len;
    //    for(auto& i : ctx->parts) {
    //        argv.push_back(i.c_str());
    //        argv_len.push_back(i.size());
    //    }
    //    redisClusterAsyncCommandArgv(m_context.get(), CmdCb, ctx.get(), argv.size(),
    //            &(argv[0]), &(argv_len[0]));
    }
}

FoxRedisCluster::~FoxRedisCluster() {
    if(m_event) {
        evtimer_del(m_event);
        event_free(m_event);
    }
}

FoxRedisCluster::Ctx::Ctx(FoxRedisCluster* r)
    :ev(nullptr)
    ,timeout(false)
    ,rds(r)
    //,scheduler(nullptr)
    ,thread(nullptr) {
    //,cancel_count(0)
    //,destory(0)
    //,callback_count(0) {
    fctx = nullptr;
    webserver::Atomic::addFetch(rds->m_ctxCount, 1);
}

FoxRedisCluster::Ctx::~Ctx() {
    WEBSERVER_ASSERT(thread == webserver::FoxThread::GetThis());
    //WEBSERVER_ASSERT(destory == 0);
    webserver::Atomic::subFetch(rds->m_ctxCount, 1);
    //++destory;
    //cancelEvent();

    if(ev) {
        evtimer_del(ev);
        event_free(ev);
        ev = nullptr;
    }
}

void FoxRedisCluster::Ctx::cancelEvent() {
    //WEBSERVER_LOG_INFO(g_logger) << "cancelEvent " << webserver::FoxThread::GetThis()
    //           << " - " << thread
    //           << " - " << webserver::IOManager::GetThis()
    //           << " - " << cancel_count;
    //if(thread != webserver::FoxThread::GetThis()) {
    //    WEBSERVER_LOG_INFO(g_logger) << "cancelEvent " << webserver::FoxThread::GetThis()
    //               << " - " << thread
    //               << " - " << webserver::IOManager::GetThis()
    //               << " - " << cancel_count;

    //    //WEBSERVER_LOG_INFO(g_logger) << "cancelEvent thread=" << thread << " " << thread->getId()
    //    //           << " this=" << webserver::FoxThread::GetThis();
    //    //WEBSERVER_ASSERT(thread == webserver::FoxThread::GetThis());
    //}
    //WEBSERVER_ASSERT(!webserver::IOManager::GetThis());
    ////if(webserver::Atomic::addFetch(cancel_count) > 1) {
    ////    return;
    ////}
    ////WEBSERVER_ASSERT(!webserver::Fiber::GetThis());
    ////webserver::RWMutex::WriteLock lock(mutex);
    //if(++cancel_count > 1) {
    //    return;
    //}
    //if(ev) {
    //    auto e = ev;
    //    ev = nullptr;
    //    //lock.unlock();
    //    //evtimer_del(e);
    //    //event_free(e);
    //    if(thread == webserver::FoxThread::GetThis()) {
    //        evtimer_del(e);
    //        event_free(e);
    //    } else {
    //        thread->dispatch([e](){
    //            evtimer_del(e);
    //            event_free(e);
    //        });
    //    }
    //}
    //ref = nullptr;
}

bool FoxRedisCluster::Ctx::init() {
    WEBSERVER_ASSERT(thread == webserver::FoxThread::GetThis());
    ev = evtimer_new(rds->m_thread->getBase(), EventCb, this);
    evtimer_add(ev, &rds->m_cmdTimeout);
    return true;
}

void FoxRedisCluster::Ctx::EventCb(int fd, short event, void* d) {
    Ctx* ctx = static_cast<Ctx*>(d);
    if(!ctx->ev) {
        return;
    }
    ctx->timeout = 1;
    if(ctx->rds->m_logEnable) {
        webserver::replace(ctx->cmd, "\r\n", "\\r\\n");
        WEBSERVER_LOG_INFO(g_logger) << "redis cmd: '" << ctx->cmd << "' reach timeout "
                   << (ctx->rds->m_cmdTimeout.tv_sec * 1000
                            + ctx->rds->m_cmdTimeout.tv_usec / 1000) << "ms";
    }
    ctx->cancelEvent();
    if(ctx->fctx->fiber) {
        ctx->fctx->scheduler->schedule(&ctx->fctx->fiber);
    }
    //ctx->ref = nullptr;
    //delete ctx;
    //ctx->tref = nullptr;
}

IRedis::ptr RedisManager::get(const std::string& name) {
    webserver::RWMutex::WriteLock lock(m_mutex);
    auto it = m_datas.find(name);
    if(it == m_datas.end()) {
        return nullptr;
    }
    if(it->second.empty()) {
        return nullptr;
    }
    auto r = it->second.front();
    it->second.pop_front();
    if(r->getType() == IRedis::FOX_REDIS
            || r->getType() == IRedis::FOX_REDIS_CLUSTER) {
        it->second.push_back(r);
        return std::shared_ptr<IRedis>(r, webserver::nop<IRedis>);
    }
    lock.unlock();
    auto rr = dynamic_cast<ISyncRedis*>(r);
    if((time(0) - rr->getLastActiveTime()) > 30) {
        if(!rr->cmd("ping")) {
            if(!rr->reconnect()) {
                webserver::RWMutex::WriteLock lock(m_mutex);
                m_datas[name].push_back(r);
                return nullptr;
            }
        }
    }
    rr->setLastActiveTime(time(0));
    return std::shared_ptr<IRedis>(r, std::bind(&RedisManager::freeRedis
                        ,this, std::placeholders::_1));
}

void RedisManager::freeRedis(IRedis* r) {
    webserver::RWMutex::WriteLock lock(m_mutex);
    m_datas[r->getName()].push_back(r);
}

RedisManager::RedisManager() {
    init();
}

void RedisManager::init() {
    m_config = g_redis->getValue();
    size_t done = 0;
    size_t total = 0;
    for(auto& i : m_config) {
        auto type = get_value(i.second, "type");
        auto pool = webserver::TypeUtil::Atoi(get_value(i.second, "pool"));
        total += pool;
        for(int n = 0; n < pool; ++n) {
            if(type == "redis") {
                webserver::Redis* rds(new webserver::Redis(i.second));
                rds->connect();
                rds->setLastActiveTime(time(0));
                webserver::RWMutex::WriteLock lock(m_mutex);
                m_datas[i.first].push_back(rds);
                webserver::Atomic::addFetch(done, 1);
            } else if(type == "redis_cluster") {
                webserver::RedisCluster* rds(new webserver::RedisCluster(i.second));
                rds->connect();
                rds->setLastActiveTime(time(0));
                webserver::RWMutex::WriteLock lock(m_mutex);
                m_datas[i.first].push_back(rds);
                webserver::Atomic::addFetch(done, 1);
            } else if(type == "fox_redis") {
                auto conf = i.second;
                auto name = i.first;
                webserver::FoxThreadMgr::GetInstance()->dispatch("redis", [this, conf, name, &done](){
                    webserver::FoxRedis* rds(new webserver::FoxRedis(webserver::FoxThread::GetThis(), conf));
                    rds->init();
                    rds->setName(name);

                    webserver::RWMutex::WriteLock lock(m_mutex);
                    m_datas[name].push_back(rds);
                    webserver::Atomic::addFetch(done, 1);
                });
            } else if(type == "fox_redis_cluster") {
                auto conf = i.second;
                auto name = i.first;
                webserver::FoxThreadMgr::GetInstance()->dispatch("redis", [this, conf, name, &done](){
                    webserver::FoxRedisCluster* rds(new webserver::FoxRedisCluster(webserver::FoxThread::GetThis(), conf));
                    rds->init();
                    rds->setName(name);

                    webserver::RWMutex::WriteLock lock(m_mutex);
                    m_datas[name].push_back(rds);
                    webserver::Atomic::addFetch(done, 1);
                });
            } else {
                webserver::Atomic::addFetch(done, 1);
            }
        }
    }

    while(done != total) {
        usleep(5000);
    }
}

std::ostream& RedisManager::dump(std::ostream& os) {
    os << "[RedisManager total=" << m_config.size() << "]" << std::endl;
    for(auto& i : m_config) {
        os << "    " << i.first << " :[";
        for(auto& n : i.second) {
            os << "{" << n.first << ":" << n.second << "}";
        }
        os << "]" << std::endl;
    }
    return os;
}

ReplyPtr RedisUtil::Cmd(const std::string& name, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    ReplyPtr rt = Cmd(name, fmt, ap);
    va_end(ap);
    return rt;
}

ReplyPtr RedisUtil::Cmd(const std::string& name, const char* fmt, va_list ap) {
    auto rds = RedisMgr::GetInstance()->get(name);
    if(!rds) {
        return nullptr;
    }
    return rds->cmd(fmt, ap);
}

ReplyPtr RedisUtil::Cmd(const std::string& name, const std::vector<std::string>& args) {
    auto rds = RedisMgr::GetInstance()->get(name);
    if(!rds) {
        return nullptr;
    }
    return rds->cmd(args);
}

}
