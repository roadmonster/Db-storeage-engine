/**
 * @file schema_tables.cpp -implementations of schema table classes: Table, Column
 * @author Kevin
 * @see "Seattle University, CPSC5300"
 **/

#include "schema_tables.h"
#include "ParseTreeToString.h"

void initialize_schema_tables(){
    Tables tables;
    tables.create_if_not_exists();
    tables.close();
    Columns columns;
    columns.create_if_not_exists();
    columns.close();
}

bool is_acceptable_identifier(Identifier identifier){
    if(ParseTreeToString::is_reserved_word(identifier))
        return true;
    
    try{
        std::stoi(identifier);
        return false;
    }catch(std::exception& e){

    }
    for(auto const& c: identifier){
        if(!isalnum(c) && c!= '$' && c != '_')
            return false;
    }

    return true;
}

bool is_acceptable_data_type(std::string dt){
    return dt == "INT" || dt == "TEXT";
}

const Identifier Tables:TABLE_NAME = "_tables";
Columns* Tables::columns_table = nullptr;
std::map<Identifier, DbRelation*> Tables::table_cache;

ColumnNames& Tables::COLUMN_NAMES(){
    static ColumnNames cn;
    if(cn.empty)
        cn.push_back("table_name");
    return cn;
}

ColumnAttributes& Tables::COLUMN_ATTRIBUTES(){
    static ColumnAttributes col_attrs;
    if(col_attrs.empty())
    {   
        ColumnAttribute ca(ColumnAttribute::TEXT);
        col_attrs.push_back(ca);
    }
    return col_attrs;
        
}

Tables::Tables():HeapTable(TABLE_NAME, COLUMN_NAMES(), COLUMN_ATTRIBUTES()){
    Tables::table_cache[TABLE_NAME] = this;
    if(Tables::columns_table == nullptr)
        columns_table = new Columns();
    
    Tables::table_cache[columns_table->TABLE_NAME] = columns_table;
}

void Tables::create(){
    HeapTable::create();
    ValueDict row;
    row["table_name"] = Value("_tables");
    insert(&row);
    row["table_name"] = Value("_columns");
    insert(&row);
}

Handle Tables::insert(const ValueDict* row){
    Handles* handles = select(row);
    bool unique = handles->empty();
    delete handles;
    if(!unique)
        throw DbRelationError(row->at("table_name").s + "already exists.");
    return HeapTable::insert(row);
}

void Tables::del(Handle handle){
    ValueDict* row = project(handle);
    Identifier table_name = row->at("table_name").s;
    if(Tables::table_cache.find(table_name) != Tables::table_cache.end()){
        DbRelation* table = Tables::table_cache.at(table_name);
        Tables::table_cache.erase(table_name);
        delete table;
    }
}

void Tables::get_columns(Identifier table_name, ColumnNames &column_names, ColumnAttributes& column_attributes){

}

DbRelation& Tables::get_tables(Identifier table_namae){

}

/**
 * schema_tables::Columns implemantations 
 **/
const Identifier Columns::TABLE_NAME = "_columns";

ColumnNames& Columns::COLUMN_NAMES(){

}

ColumnAttributes& Columns::COLUMN_ATTRIBUTES(){

}

Columns::Columns(): HeapTable(TABLE_NAME, COLUMN_NAMES(), COLUMN_ATTRIBUTES()){

}

void Columns::create(){

}

Handle Columns::insert(const ValueDict* row){

}




