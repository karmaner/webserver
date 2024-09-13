#ifndef __SRC_ORM_TABLE_H__
#define __SRC_ORM_TABLE_H__

#include "column.h"
#include "index.h"
#include <fstream>

namespace webserver {
namespace orm {

class Table {
public:
    typedef std::shared_ptr<Table> ptr;
    const std::string& getName() const { return m_name;}
    const std::string& getNamespace() const { return m_namespace;}
    const std::string& getDesc() const { return m_desc;}

    const std::vector<Column::ptr>& getCols() const { return m_cols;}
    const std::vector<Index::ptr>& getIdxs() const { return  m_idxs;}
    bool init(const tinyxml2::XMLElement& node);

    void gen(const std::string& path);

    std::string getFilename() const;
private:
    void gen_inc(const std::string& path);
    void gen_src(const std::string& path);
    std::string genToStringInc();
    std::string genToStringSrc(const std::string& class_name);
    std::string genToInsertSQL(const std::string& class_name);
    std::string genToUpdateSQL(const std::string& class_name);
    std::string genToDeleteSQL(const std::string& class_name);

    std::vector<Column::ptr> getPKs() const;
    Column::ptr getCol(const std::string& name) const;

    std::string genWhere() const;

    void gen_dao_inc(std::ofstream& ofs);
    void gen_dao_src(std::ofstream& ofs);
private:
    std::string m_name;
    std::string m_namespace;
    std::string m_desc;
    std::string m_subfix = "_info";
    std::string m_dbclass = "webserver::SQLite3";
    std::vector<Column::ptr> m_cols;
    std::vector<Index::ptr> m_idxs;
};

}
}

#endif