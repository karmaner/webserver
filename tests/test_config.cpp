#include "src/basic/config.h"
#include "src/basic/log.h"
#include <yaml-cpp/yaml.h>
#include <iostream>

webserver::ConfigVar<int>::ptr g_int_value_config = webserver::Config::Lookup("system.port", (int)8080, "system port");

webserver::ConfigVar<float>::ptr g_float_value_config =
    webserver::Config::Lookup("system.value", (float)10.2f, "system value");

void print_yaml(const YAML::Node &node, int level)
{
    if (node.IsScalar())
    {
        WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT())
            << std::string(level * 4, ' ') << node.Scalar() << " - " << node.Type() << " - " << level;
    }
    else if (node.IsNull())
    {
        WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT())
            << std::string(level * 4, ' ') << "NULL - " << node.Type() << " - " << level;
    }
    else if (node.IsMap())
    {
        for (auto it = node.begin(); it != node.end(); ++it)
        {
            WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT())
                << std::string(level * 4, ' ') << it->first << " - " << it->second.Type() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    }
    else if (node.IsSequence())
    {
        for (size_t i = 0; i < node.size(); ++i)
        {
            WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT())
                << std::string(level * 4, ' ') << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}

void test_yaml()
{
    YAML::Node root = YAML::LoadFile("/home/webserver/workspace/webserver/bin/conf/log.yml");
    print_yaml(root, 0);

    WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << root.Scalar();
}

void test_config()
{
    WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << "before: " << g_int_value_config->getValue();
    WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << "before: " << g_float_value_config->toString();

    YAML::Node root = YAML::LoadFile("/home/karmaner/myRepo/webserver/bin/conf/log.yml");
    webserver::Config::LoadFromYaml(root);

    WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << "after: " << g_int_value_config->getValue();
    WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << "after: " << g_float_value_config->toString();
}

static webserver::Logger::ptr s_logger = WEBSERVER_LOG_NAME("system");

int main(int argc, char **argv)
{
    // test_yaml();
    WEBSERVER_LOG_INFO(s_logger) << "Hi ok" << "nice man";
    test_config();
    return 0;
}