# webserver
C++服务器框架

## 开发环境
Debain11（WSL）
gcc 12
cmake

### 项目结构
>.<br>
>├── bin<br>
>├── build<br>
>├── cmake<br>
>├── CMakeLists.txt<br>
>├── lib<br>
>├── README.md<br>
>├── src<br>
>└── tests<br>
## 日志系统
>1. 仿照log4J进行开发
>
>   Logger (定义日志等级类别)(debug日志，普通日志)
>
>           Formatter(定义日志格式)
>
>   Appender (日志输出地)  派生 FileLogAppender StdoutLogAppender
>   
>   
>   
>   LogFormatter（日志格式器） 包含多个FormatItem
>   
>   LogEvent (日志事件)包含 系统信息、线程信息、日志内容
>   LogEventWarp 日志事件包装类  析构的时候调用日志
>
>   LoggerManager 日志器管理器
>   

## 配置系统
>   ConfigVarBase(配置变量基类) 
>        --toString() 和 fromString() 两个纯虚函数将参数值转换成 YAML String 和从 YAML String 转成参数的值
>
>   ConfigVar(配置变量类) -- 保存对应变量的参数值 保存对应类型 T 的参数值。 对应类型为 <string, T>
>1. 根据不同的非内置基本类型 T，FromStr 和 ToStr 具有不同的模板偏特化实现。
>2. 内含一个 std::map，存放配置变更的回调函数。
>3. 提供 setValue() 和 getValue() 函数，其中在 setValue() 函数中，会调用 std::map 中存放的所有回调函数进行通知配置变更。
>4. 关于回调函数，提供 addListener()、delListener()、getListener()、clearListener() 四个函数给外界使用。
>
>
>
>   Config(ConfigVar 的管理类)
>1. 管理所有的配置项，通过静态函数 GetDatas() 获取缓存std::unordered_map<std::stringConfigVarBase::ptr>。
>2. 提供 Lookup(const std::string& name, const T& default_value, const std::string& description = “”) 函数
>3. 给外界获取/创建对应参数名的配置参数。
>4. 提供 Lookup(const std::string& name) 函数给外界查找配置参数。
>5. 提供 LoadFromYaml(const YAML::Node& root) 函数给外界使用YAML::Node初始化配置模块。
>6. 提供 LoadFromConfDir(const std::string& path, bool force = false) 给外界加载path文件夹里面的配置文件。
>7. 值得注意的是 Config 所有函数和变量均为静态的

Config --> Yaml

yamp-cpp: github 搜
mkdir build && cd build && cmake .. && make install

```cpp
YAML::Node node = YAML::LoadFile(filename);
node.IsMap()
for(auto it = node.begin();
    it != node.end(); ++it) {
        it->first, it->second
}

node.IsSequence()
for(size_t i = 0; i < node.size(); ++i) {
    
}

node.IsScalar();
```

## 线程库
线程库
Thread, Mutex, 

同步锁模块

## 协程库封装
定义协程接口
ucontext.h   ucontext_t

```mermaid
graph TD;
    A[Thread->main_fiber] <-->sub_fiber1;
    A<-->sub_fiber2;
```

## 协程调度模块
```
        1 - N            1 - M
    scheduler --> thread --> fiber
scheduler的功能
1. 线程池， 分配一组线程
2. 协程调度器， 将协程，指定到相应的线程执行


N : M

m_threads
<function<void()>, fiber, threadid> m_fibers

schedule(func/fiber);

概述
一个 N-M 的协程调度器，N 个线程运行 M 个协程，协程可以在线程之间进行切换，协程也可以绑定到指定线程运行。
实现协程调度之后，可以解决前一章协程模块中子协程不能运行另一个子协程的缺陷，子协程可以通过向调度器添加调度任务的方式来运行另一个子协程。
协程调度器调度的是协程，函数（可执行对象）被包装成协程。
Scheduler
协程调度器类。
t_scheduler_fiber 保存当前线程的调度协程，加上 Fiber 模块的 t_fiber 和 t_thread_fiber，每个线程总共可以记录三个协程的上下文信息。

```

## socket函数库

## Http协议

## 封装分布式协议库