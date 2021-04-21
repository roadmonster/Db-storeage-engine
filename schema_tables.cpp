/**
 * @file schema_tables.cpp -implementations of schema table classes: Table, Column
 * @author Kevin
 * @see "Seattle University, CPSC5300"
 **/

#include "schema_tables.h"
#include "ParseTreeToString.h"

void initialize_schema_tables(){

}

bool is_acceptable_identifier(Identifier identifier){

}

bool is_acceptable_data_type(std::string dt){

}

const Identifier Tables:TABLE_NAME = "_tables";
Columns* Tables::columns_table = nullptr;
std::map<Identifier, DbRelation*> Tables::table_cache;

ColumnNames& Tables::COLUMN_NAMES(){

}

ColumnAttributes& Tables::COLUMN_ATTRIBUTES(){

}

Tables::Tables():HeapTable(TABLE_NAME, COLUMN_NAMES(), COLUMN_ATTRIBUTES()){

}

void Tables::create(){

}

Handle Tables::insert(const ValueDict* row){

}

void Tables::del(Handle handle){

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




