#include "src/webserver.h"
#include "src/ds/bitmap.h"
#include "src/ds/roaring_bitmap.h"

static webserver::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();
#if 0
void init(uint32_t size, std::set<uint32_t> v = {}) {
    //uint32_t size = rand() % 8 * 7 + 128;
    std::set<uint32_t> v0;
    webserver::ds::Bitmap::ptr b;
    if(!size) {
        size = rand() % 4096;
        b.reset(new webserver::ds::Bitmap(size));
        for(int i = 0; i < (int)(size / 2); ++i) {
            uint32_t t = rand() % size;
            v0.insert(t);
            b->set(t, true);
        }
    } else {
        b.reset(new webserver::ds::Bitmap(size));
        for(auto& i : v) {
            uint32_t t = i % size;
            v0.insert(t);
            b->set(t, true);
        }
    }

    webserver::ds::Bitmap::ptr c(new webserver::ds::Bitmap(size, 0xFF));
    *c &= *b;
    WEBSERVER_ASSERT(c->getCount() == v0.size());

    std::vector<uint32_t> v00(v0.begin(), v0.end());
    std::vector<uint32_t> v000(v0.rbegin(), v0.rend());
    //std::vector<uint32_t> vv = {
    //    32,47,63,79,88,89,104,107,109,113,116,152,165,182,187,207,220,222,239,240,251,263,271,287,288,303,305,308,320
    //};
    //uint32_t size = 328;
    //webserver::ds::Bitmap::ptr b(new webserver::ds::Bitmap(size));
    //for(auto& i : vv) {
    //    b->set(i, true);
    //}

    std::vector<uint32_t> v1;
    std::vector<uint32_t> v2;
    std::vector<uint32_t> v3;
    std::vector<uint32_t> v4;
    std::vector<uint32_t> v5;

    b->listPosAsc(v5);

    for(auto it = b->begin_new();
            !*it; it->next()) {
        v1.push_back(**it);
    }

    for(auto it = b->rbegin_new();
            !*it; it->next()) {
        v2.push_back(**it);
    }

    b->foreach([&v3](uint32_t i){
        v3.push_back(i);
        return true;
    });

    b->rforeach([&v4](uint32_t i){
        v4.push_back(i);
        return true;
    });

    webserver::ds::Bitmap::ptr bb(new webserver::ds::Bitmap(size, 0xFF));
    *b &= *bb;
#define XX(v) \
        WEBSERVER_LOG_INFO(g_logger) << #v ": " << webserver::Join(v.begin(), v.end(), ",");

#define XX_ASSERT(a, b) \
        if(a != b) { \
            std::cout << "size=" << size << std::endl; \
            XX(v0); \
            XX(v1); \
            XX(v3); \
            XX(v2); \
            XX(v4); \
            XX(v5); \
            WEBSERVER_ASSERT(a == b); \
        }

    XX_ASSERT(v00, v5);
    if(v2 != v4 || v1 != v3) {
        XX_ASSERT(v00, v1);
        XX_ASSERT(v000, v2);
        XX_ASSERT(v00, v3);
        XX_ASSERT(v000, v4);
        //WEBSERVER_ASSERT(v00 == v3);
        //WEBSERVER_ASSERT(v1 == v3);
        //WEBSERVER_ASSERT(v2 == v4);
#undef XX_ASSERT
#undef XX
    }
    //WEBSERVER_LOG_INFO(g_logger) << "size: " << size;
}

void test_compress(size_t size, std::set<uint32_t> v = {}) {
    webserver::ds::Bitmap::ptr b;
    if(size == 0) {
        size = rand() % 4096;
        b.reset(new webserver::ds::Bitmap(size));
        for(size_t i = 0; i < size / 100; ++i) {
            auto t = rand() % size;
            b->set(t, 1);
            v.insert(t);
        }
    } else {
        b.reset(new webserver::ds::Bitmap(size));
        for(auto& i : v) {
            b->set(i, 1);
        }
    }

    auto bc = b->compress();
    auto bb = bc->uncompress();

    if(*b != *bb) {
        WEBSERVER_LOG_INFO(g_logger) << "b  :" << b->toString();
        WEBSERVER_LOG_INFO(g_logger) << "bc :" << bc->toString();
        WEBSERVER_LOG_INFO(g_logger) << "bb :" << bb->toString();
        WEBSERVER_LOG_INFO(g_logger) << "size=" << size << " - " << webserver::Join(v.begin(), v.end(), ",");

        WEBSERVER_ASSERT(*b == *bb);
    }
    //std::cout << "size=" << size << " value_size=" << v.size()
    //          << " compress_rate=" << bc->getCompressRate() << std::endl;
}

void test_op(size_t size, std::set<uint32_t> v1, std::set<uint32_t> v2) {
    webserver::ds::Bitmap::ptr a;
    webserver::ds::Bitmap::ptr b;

    if(size == 0) {
        size = rand() % 4096;
        a.reset(new webserver::ds::Bitmap(size));
        b.reset(new webserver::ds::Bitmap(size));
        for(size_t i = 0; i < size / 16; ++i) {
            auto t = rand() % size;
            a->set(t, 1);
            v1.insert(t);
        }
        for(size_t i = 0; i < size / 32; ++i) {
            auto t = rand() % size;
            b->set(t, 1);
            v2.insert(t);
        }
    } else {
        a.reset(new webserver::ds::Bitmap(size));
        b.reset(new webserver::ds::Bitmap(size));
        for(auto& i : v1) {
            a->set(i, 1);
        }
        for(auto& i : v2) {
            b->set(i, 1);
        }
    }
    auto bc = b->compress();
    auto andv = *a & *bc;
    auto orv = *a | *bc;

    auto xora = ~*a;
    auto xora2 = ~*a;

    auto xorb = ~*bc;
    auto xorb2 = ~*bc;

    WEBSERVER_ASSERT(a->cross(*b) == a->cross(*bc));


    std::vector<uint32_t> and_sv;
    std::vector<uint32_t> or_sv;

    std::set_union(v1.begin(), v1.end(), v2.begin(), v2.end(), std::back_inserter(or_sv));
    std::set_intersection(v1.begin(), v1.end(), v2.begin(), v2.end(), std::back_inserter(and_sv));

    WEBSERVER_ASSERT(a->cross(*b) == (!and_sv.empty()));

    std::vector<uint32_t> and_sv2;
    std::vector<uint32_t> or_sv2;
    andv.foreach([&and_sv2](uint32_t i){
        and_sv2.push_back(i);
        return true;
    });
    orv.foreach([&or_sv2](uint32_t i){
        or_sv2.push_back(i);
        return true;
    });

#define XX(v) \
        WEBSERVER_LOG_INFO(g_logger) << #v ": " << webserver::Join(v.begin(), v.end(), ",");

#define XX_ASSERT(a, b) \
        if(a != b) { \
            std::cout << "size=" << size << std::endl;\
            XX(v1); \
            XX(v2); \
            XX(and_sv); \
            XX(and_sv2); \
            XX(or_sv); \
            XX(or_sv2); \
            WEBSERVER_ASSERT(a == b); \
        }
        XX_ASSERT(and_sv, and_sv2);
        XX_ASSERT(or_sv, or_sv2);
        if(xora2 != *a) {
            std::cout << "xora: " << xora.toString() << std::endl;
            std::cout << "xora2: " << xora2.toString() << std::endl;
            std::cout << "a: " << a->toString() << std::endl;
            XX_ASSERT(xora2, *a);
        }
        if(xorb2 != *bc) {
            std::cout << "xorb: " << xorb.toString() << std::endl;
            std::cout << "xorb2: " << xorb2.toString() << std::endl;
            std::cout << "b: " << b->toString() << std::endl;
            XX_ASSERT(xorb2, *bc);
        }
        //WEBSERVER_ASSERT(v00 == v3);
        //WEBSERVER_ASSERT(v1 == v3);
        //WEBSERVER_ASSERT(v2 == v4);
#undef XX_ASSERT
#undef XX

}

int main(int argc, char** argv) {
    srand(time(0));
    {
        webserver::ds::Bitmap::ptr b(new webserver::ds::Bitmap(504));
    }
    //init(64, {4,5,7,8,14,17,18,19,20,21,23,27,29,32,35,39,40,41,43,46,48,50,51,53,62});
    //init(32, {1,3,4,6,7,10,11,12,15,18,22,24,26,29});
    //init(30, {0,2,3,8,10,12,14,15,18,20,27,28,29});
    //init(30, {1,3,4,7,9,10,12,17,19,22,25,26});

    //init(41, {0,7,8,10,12,16,18,22,23,24,27,29,31,32,34,35,37,41,42,46,48,51,53,54,58,60,61,63,64,67,71});
    //init(60, {2,7,10,11,12,14,15,16,18,19,20,21,22,23,28,29,30,34,38,40,41,45,49,50,54,55});
    init(72, {2,7,9,10,13,14,17,19,20,22,28,31,32,35,36,39,41,44,45,47,51,54,59,60,61,62,64,67,69});
    test_compress(41, {36});
    //test_op(97, {3,15,22,50,74,78}, {28,58});
    test_op(67, {21,38,66}, {41,50});
    for(int i = 0; i < 10000000; ++i) {
        init(0);
        test_compress(0);
        test_op(0, {}, {});
    }
    return 0;
}
#endif

std::vector<webserver::ds::Bitmap::ptr> vs;
std::vector<webserver::ds::Bitmap::ptr> vs2;
std::vector<webserver::ds::RoaringBitmap::ptr> vs3;

int N = 1;
int M = 250000000;

void init() {
    vs.resize(N);
    vs2.resize(N);
    vs3.resize(N);
    for(int i = 0; i < N; ++i) {
        webserver::ds::Bitmap::ptr r(new webserver::ds::Bitmap(M));
        webserver::ds::RoaringBitmap::ptr r2(new webserver::ds::RoaringBitmap());
        for(int i = 0; i < M / 1; ++i) {
            uint32_t v = rand() % M;
            r->set(v, true);
            r2->set(v, true);
        }
        vs[i] = r;
        vs2[i] = r;
        vs3[i] = r2;
    }
}

void x_compress() {
    for(auto& i : vs) {
        i = i->compress();
    }

    for(auto& i : vs3) {
        i = i->compress();
    }
}

void x_uncompress() {
    for(auto& i : vs) {
        i = i->uncompress();
    }

    for(auto& i : vs3) {
        i = i->uncompress();
    }

}

void write_to_file(const std::string& name) {
    {
        webserver::ByteArray::ptr ba(new webserver::ByteArray);
        webserver::ds::Bitmap::ptr a;
        for(auto& i : vs) {
            a = i;
            i->writeTo(ba);
        }
        ba->setPosition(0);
        ba->writeToFile(name);

        std::cout << "compress= " << a->isCompress()
                  << " rate=" << a->getCompressRate()
                  << " size=" << a->getSize()
                  << " count=" << a->getCount()
                  << std::endl;


        a.reset(new webserver::ds::Bitmap(0));
        a->readFrom(ba);

        std::cout << "*compress= " << a->isCompress()
                  << " rate=" << a->getCompressRate()
                  << " size=" << a->getSize()
                  << " count=" << a->getCount()
                  << std::endl;

    }

    {
        webserver::ByteArray::ptr ba(new webserver::ByteArray);
        webserver::ds::RoaringBitmap::ptr a;
        for(auto& i : vs3) {
            a = i;
            i->writeTo(ba);
        }
        ba->setPosition(0);
        ba->writeToFile(name + ".roaring");

        std::cout << "-compress= " << a->toString()
                  << std::endl;


        a.reset(new webserver::ds::RoaringBitmap);
        a->readFrom(ba);

        std::cout << "=compress= " << a->toString()
                  << std::endl;

    }

}

void load_from_file(const std::string& name) {
    webserver::ByteArray::ptr ba(new webserver::ByteArray);
    ba->readFromFile(name);
    ba->setPosition(0);
    webserver::ds::Bitmap::ptr a(new webserver::ds::Bitmap(0));
    a->readFrom(ba);
    std::cout << "compress= " << a->isCompress()
              << " rate=" << a->getCompressRate()
              << " size=" << a->getSize()
              << " count=" << a->getCount()
              << std::endl;
}

void test_uncompress() {
    for(int i = 0; i < N; ++i) {
        for(int n = i + 1; n < N; ++n) {
            vs2[i]->uncompress();
        }
    }
}

void test_uncompress2() {
    for(int i = 0; i < N; ++i) {
        for(int n = i + 1; n < N; ++n) {
            webserver::ds::Bitmap::ptr b(new webserver::ds::Bitmap(M));
            *b |= *vs2[i];
        }
    }
}

void test_uncompress3() {
    for(int i = 0; i < N; ++i) {
        for(int n = i + 1; n < N; ++n) {
            webserver::ds::Bitmap::ptr b(new webserver::ds::Bitmap(M));
            *b |= *vs2[i];

            WEBSERVER_ASSERT(*b == *vs2[i]->uncompress());
        }
    }
}

void test_uncompress4() {
    for(int i = 0; i < N; ++i) {
        for(int n = i + 1; n < N; ++n) {
            webserver::ds::Bitmap::ptr b(new webserver::ds::Bitmap(M, 0xff));
            *b &= *vs2[i];

            WEBSERVER_ASSERT(*b == *vs2[i]->uncompress());
        }
    }
}



void test_and() {
    for(int i = 0; i < N; ++i) {
        for(int n = i + 1; n < N; ++n) {
            auto r = vs2[i]->uncompress();
            *r &= *vs[n];
        }
    }
}

void test_or() {
    for(int i = 0; i < N; ++i) {
        for(int n = i + 1; n < N; ++n) {
            auto r = vs2[i]->uncompress();
            *r |= *vs[n];
        }
    }
}

void run(const char* name, std::function<void()> cb) {
    time_t t1 = time(0);
    std::cout << name << " begin" << std::endl;
    cb();
    std::cout << name << " end used=" << (time(0) - t1) << std::endl;
}

void check() {
    //for(auto& i : vs) {
    //}
}

void test_roaring_bitmap() {
    webserver::ds::RoaringBitmap::ptr rb(new webserver::ds::RoaringBitmap);
    for(int i = 0; i < 10; ++i) {
        rb->set(rand(), true);
    }

    for(auto it = rb->begin();
            it != rb->end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    std::cout << "count=" << rb->getCount()
              << " " << rb->toString() << std::endl;
    rb = rb->compress();
    std::cout << "count=" << rb->getCount()
              << " " << rb->toString() << std::endl;
    for(auto it = rb->rbegin();
            it != rb->rend(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

}

int main(int argc, char** argv) {
    test_roaring_bitmap();
    run("init", init);
    run("test_and", test_and);
    run("test_or", test_or);
    run("write_to_file", std::bind(write_to_file, "test.dat"));
    run("compress", x_compress);
    //run("uncompress", test_uncompress);
    //run("uncompress2", test_uncompress2);
    //run("uncompress3", test_uncompress3);
    //run("uncompress4", test_uncompress4);
    run("write_to_file", std::bind(write_to_file, "test2.dat"));
    run("uncompress", x_uncompress);
    run("write_to_file", std::bind(write_to_file, "test3.dat"));
    run("load_from_file", std::bind(load_from_file, "test2.dat"));
    run("test_and", test_and);
    run("test_or", test_or);
    return 0;
}

