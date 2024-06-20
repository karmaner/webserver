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
>   Appender (日志输出地)  派生 FileLogAppender  StdoutLogAppender
>   
>   
>   
>   LogFormatter（日志格式器） 包含多个FormatItem
>   
>


## 协程库封装

## socket函数库

## Http协议

## 封装分布式协议库