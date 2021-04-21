#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <vector>
#include <iostream>
#include "db_cxx.h"
#include "storage_engine.h"
#include "heap_storage.h"
using namespace std;

typedef u_int16_t u16;
/**
 * Constructor for the slottedpage
 * @Param block parameter needed to initialize the super class DbBlock
 * @Param block_id same as above
 * @Param is_new flag to indicate if needed to create a new block
 * */
SlottedPage::SlottedPage(Dbt& block, BlockID block_id, bool is_new): DbBlock(block, block_id, is_new){
    // need to create a new block
    if(is_new){
        // set the new block with 0 records
        this->num_records = 0;
        // set the end of the available space
        this->end_free = this->BLOCK_SZ -1;
        // set up the header with default size = 0
        this->put_header();
    }else{
        // initialize the slotted page from existing block
        get_header(this->num_records, this->end_free);
    }
}

/**
 * Add new record to the block
 * @Return the newly inserted data's id in the block
 * @Throws if the data is greater than the available space in this blcok, throw this error
 * */
RecordID SlottedPage::add(const Dbt* data) throw (DbBlockNoRoomError){
    if(has_room(data->get_size()))
        throw new DbBlockNoRoomError("Not enough room to store this data");
    
    // update the # of records and set it as the id
    u16 id = ++(this->num_records);
    // get the size of the data
    u16 size = (u16)data->get_size();
    // minus the size from the end free border
    this->end_free -= size;
    // the data's location is immediately behind the endfree
    u16 loc = this->end_free + 1;
    // update the header with new endfree and #of records
    put_header();

    // update the header with newly inserted id and size and location
    put_header(id, size, loc);
    
    // copy the data to the location in the data block
    memcpy(this->address(loc), data->get_data(), size);
    return id;
}



/**
 * Get the Dbt object (ptr, size) object provided by BerkDB
 * */
Dbt* SlottedPage::get(RecordID record_id) const{
    u16 size, loc;
    get_header(size, loc, record_id);
    if(loc == 0){
        return nullptr;
    }
    return new Dbt(this->address(loc), size);

}

/**
 * Replace a record with given data
 * */
void SlottedPage::put(RecordID record_id, const Dbt& data)throw(DbBlockNoRoomError){
    u16 size, loc;
    get_header(size, loc, record_id);
    u16 newSize = data.get_size();
    if(!has_room(newSize)){
        throw new DbBlockNoRoomError("Not enough room");
    }
    if(newSize > size){
        slide(loc, loc - (newSize - size));
        memcpy(address(loc - (newSize - size)), data.get_data(), newSize);

    }else{
        memcpy(address(loc), data.get_data(), newSize);
        slide(loc, loc + (size - newSize));
    }

    get_header(size, loc, record_id);
    put_header(record_id, newSize, loc);
}

/**
 * Delete the record from the record id given by 
 * frist get the header of current record's size and location
 * then mark this current record as available space by updating header
 * then shift the endfree forward by the amount of size
 * 
 **/
void SlottedPage::del(RecordID record_id){
    u16 size, loc;
    get_header(size, loc, record_id);
    put_header(record_id, 0, 0);
    slide(loc, loc + size);
}

/**
 * Initialize a new recordIds vector
 * step through each record number, get their header info
 * push record id into the records vector if the location of this record is not 0
 * */
RecordIDs* SlottedPage::ids(void) const{
    u16 size, loc;
    RecordIDs* records = new RecordIDs();
    for(u16 i = 1; i <= this->num_records; i++){
        get_header(size, loc, i);
        if(loc != 0){
            records->push_back(i);
        }
    }
    return records;
}

/**
 * Pass in the reference of the size and location variable
 * call the get_n() to get the values for each variable
 * */
void SlottedPage::get_header(u16& size, u16& loc, RecordID id) const {
    size = get_n(4 * id);
    loc = get_n( 4 * id + 2);
}

/**
 * Update the header
 * if id is 0 then it is the size and location of this block
 * otherwise, update the slot at the 4 times id because each size + location totally
 * take 4 bytes hence the slot for size should be 4 * id and location should be 4 * id + 2
 * */
void SlottedPage::put_header(RecordID id, u16 size, u16 loc){
    if(id == 0)
    {
        size = this->num_records;
        loc = this->end_free;
    }
    put_n((u16)(4 * id), size);
    put_n(((u16)4 * id + 2), loc);
}

/**
 * Calculate the available space by have endfree - (total number of records + the info of 
 * the block which also takes 4 bytes) * 4 and cast the result back to u16
 * */
bool SlottedPage::has_room(u16 size) const{
    u16 avail_size = this->end_free - (u16)((this->num_records + 1) * 4);
    return size <= avail_size;
}

void SlottedPage::slide(u16 start, u16 end){
    u16 shift = end - start;
    u16 size, loc;
    if(shift == 0)
        return;
    
    memcpy(this->address(end_free + 1), this->address(end_free + 1 + shift), shift);

    RecordIDs* records = ids();
    for(RecordIDs::iterator i = records->begin(); i != records->end(); i++){
        get_header(size, loc, *i);
        if(loc <= start){
            put_header(*i, size, loc+shift);
        }
    }

    end_free += shift;
    put_header();
}

// get 2-byte int at given offset to the block addr
u16 SlottedPage::get_n(u16 offset) const {
    return *(u16*)this->address(offset);
}

// put 2-byte int at the offset location of the block address
void SlottedPage::put_n(u16 offset, u16 n){
    *(u16*)this->address(offset) = n;
}

void* SlottedPage::address(u16 offset) const{
    return (void*)((char*)this->block.get_data() + offset);
}

/**
 * Heap File implementations
 */

void HeapFile::create(){
    this->db_open(DB_CREATE|DB_EXCL);
    this->get_new();
}

void HeapFile::drop(){
    this->close();
    remove(this->dbfilename.c_str());
}

void HeapFile::open(){
    this->db_open();
}

void HeapFile::close(){
    this->db.close(0);
    this->closed = true;
}

/**
 * Allocate a new block for the database file
 * Initialize a block first
 * Allocate memory for that block
 * create a Dbt object (data) for BerKDB to work with
 * incrementing the last record id of HeapFile and make it block id
 * Create the Dbt key and Call the BerkDB to put this empty block into databse
 * Then read it back. 
 * Finally initialize the SlottedPage on this data and return it.
 **/
SlottedPage* HeapFile::get_new(void){
    char block[DbBlock::BLOCK_SZ];
    memset(block, 0, sizeof(block));
    Dbt data(block, sizeof(block));

    int block_id = ++this->last;
    Dbt key(&block_id, sizeof(block_id));

    SlottedPage* page = new SlottedPage(data, this->last, true);
    this->db.put(nullptr, &key, &data, 0);
    this->db.get(nullptr, &key, &data, 0);
    return page;
}

/**
 * Request a block by intializing a char array with BLOCK_SZ(4096)
 * Create a Dbt object (data) with the ptr to the char array and its size
 * Create a Dbt key with the reference to the block_id and its size
 * Use the key to call BerKDB to get the wanted data and put it into the data object
 * we created
 * Finally initialize a SlottedPage object with the datablock and its block_id
 * */
SlottedPage* HeapFile::get(BlockID block_id){
    char block[DbBlock::BLOCK_SZ];
    Dbt data(block, sizeof(block));
    Dbt key(&block_id, sizeof(block_id));
    this->db.get(nullptr, &key, &data, 0);
    SlottedPage* page  = new SlottedPage(data, block_id, false);
    return page;
}

/**
 * Write a block to the database file
 * get data from the DbBlock object and intialize a new Dbt object: data
 * Get the block id from the DbBlock object
 * Initialize the Dbt key with the block id and the sizeof(blockid)
 * finally call BerkDb to put key and data into the database
 * */
void HeapFile::put(DbBlock* block){
    void* blockData = block->get_data();
    Dbt data(blockData, DbBlock::BLOCK_SZ);
    int block_id = block->get_block_id();
    Dbt key(&block_id, sizeof(block_id));
    this->db.put(nullptr, &key, &data, 0);
}

/**
 * Need to return a vector of Block id. 
 * Begin from block #1 to the last, add each one into the vector
 **/
BlockIDs* HeapFile::block_ids()const{
    BlockIDs* myBlocks = new BlockIDs();
    for(BlockID i = 1; i <= this->last; i++){
        myBlocks->push_back(i);
    }
    return myBlocks;
}

/**
 * Initialze a nullptr for the envHome and call BerkDB
 * to get the path to env home and formulate the name to the db file
 * Then open the db file with BerKDB
 * set the B_Tree status for it and set the closed flag false
 **/
void HeapFile::db_open(unsigned int flags){
    if(!this->closed){
        return;
    }

    const char* envHome = nullptr;
    _DB_ENV->get_home(&envHome);
    string path = envHome;
    this->dbfilename = "../" + path + "/" + this->name + ".db";
    this->db.open(nullptr, this->dbfilename.c_str(), nullptr, DB_RECNO, flags, 0);
    DB_BTREE_STAT *stat;
    this->db.stat(nullptr, &stat, DB_FAST_START);
    this->last = stat->bt_ndata;
    this->closed = false;
}

/**
 * Heap Table Implementations
 * */

HeapTable::HeapTable(Identifier table_name, ColumnNames columnNames, ColumnAttributes column_attrbutes): DbRelation(table_name, column_attrbutes), file(table_name){};

void HeapTable::create(){
    this->file.create();
}

void HeapTable::create_if_not_exists(){
    try{
        this->file.open();
    }catch(DbException &e){
        this->file.create();
    }
}

void HeapTable::drop(){
    this->file.drop();
}

void HeapTable::open(){
    this->file.open();
}

void HeapTable::close(){
    this->file.close();
}

Dbt* HeapTable::marshal(const ValueDict* row) const{
    // create an empty char array to hold the marshalled results
    char*bytes = new char[DbBlock::BLOCK_SZ];

    // set initial offset as 0 and column_num as 0
    unsigned int offset = 0, column_num = 0;

    
    // iterate through all the column attributes 
    // for each column get the (name, value) pair from the ValueDict object
    // check the data attribute of the column if INT then set the current data spot(bytes + offset) as the value
    // of this column
    // same idea for the string
    for(string& column_name: this->column_names){
        ColumnAttribute ca = this->column_attributes[column_num++];
        ValueDict::const_iterator column = row->find(column_name);
        Value value = column->second;
        if(ca.get_data_type() == ColumnAttribute::INT){
            *(int32_t*)(bytes + offset) = value.n;
            offset += sizeof(int32_t);
        }else if(ca.get_data_type() == ColumnAttribute::TEXT){
            unsigned int size = value.s.length();
            *(u16*)(bytes + offset) = size;
            offset += sizeof(u16);
            memcpy(bytes + offset, value.s.c_str(), size);
            offset += size;

        }else{
            throw DbRelationError("Only able to marshal INT and TEXT");
        }

    }
    char *right_size_bytes = new char[offset];
    memcpy(right_size_bytes, bytes, offset);
    delete[]bytes;
    Dbt *data = new Dbt(right_size_bytes, offset);
    return data;

}
/**
 * Directly opposite of the operations of marshall
 * Don't forget to add terminating ptr at the end of the string
 * */
ValueDict* HeapTable::unmarshal(Dbt* data) const{
    ValueDict* row = new ValueDict();
    char* bytes = (char*)data->get_data();
    unsigned int offset = 0, column_num = 0;
    Value val;
    for(string& column_name: this->column_names){
        ColumnAttribute ca = this->column_attributes[column_num++];
        val.data_type = ca.get_data_type();
        if(val.data_type == ColumnAttribute::INT){
            val.n = *(int32_t*)(bytes + offset);
            offset += sizeof(int32_t);
        }else if(val.data_type == ColumnAttribute::TEXT){
            unsigned int size = *(*u16)(bytes + offset);
            offset += sizeof(u16);
            char myData[DbBlock::BLOCK_SZ];
            memcpy(myData, bytes + offset, size);
            myData[size] = '\0';
            val.s = string(myData);
            offset += size;
        }else{
            throw new DbRelationError("Can only unmarshall TEXT or INT");
        }

        (*row)[column_name] = val;
    }
    return row;
}

bool HeapTable::selected(Handle handle, const ValueDict* where){
    if(where == nullptr)
        return true;
    ValueDict* row = this->project(handle, where);
    return *row == *where;
}
Handles* HeapTable::select(const ValueDict* where){
    // TO DO
    Handles* handles = new Handles();
    BlockIDs* block_ids = this->file.block_ids();
    for(auto const& block_id: *block_ids){
        SlottedPage* block = this->file.get(block_id);
        RecordIDs* record_ids = block->ids();
        for(auto const& record_id: *record_ids){
            Handle handle(block_id, record_id);
            if(selected(handle, where))
                handles->push_back(Handle(block_id, record_id));
        }
        delete record_ids;
        delete block;
    }
    delete block_ids;
    return handles;
}

Handles* HeapTable::select(){
    // TO DO
    return this->select(nullptr);
}


ValueDict* HeapTable::project(Handle handle){
    return project(handle, &this->column_names);
    

}
ValueDict* HeapTable::project(Handle handle, const ColumnNames* column_names){
    BlockID block_id = handle.first;
    RecordID record_id = handle.second;
    SlottedPage* block = this->file.get(block_id);
    Dbt* data = block->get(record_id);
    ValueDict* row = unmarshal(data);
    delete data;
    delete block;
    if(column_names->empty()){
        return row;
    }
    ValueDict* res = new ValueDict();
    for(string column_name: *column_names){
        if(row->find(column_name) == row->end()){
            throw DbRelationError("Column name: " + column_name + " does not exist");
        }
        (*res)[column_name] = (*row)[column_name];
    }
    delete row;
    return res;
    

}

ValueDict* HeapTable::validate(const ValueDict* row) const{
    ValueDict* fullRow = new ValueDict();
    for(string column_name: this->column_names){
        ValueDict::const_iterator column = row->find(column_name);
        Value val;
        if(column == row->end()){
            throw DbRelationError("Error: NULL and default not implemented");
        }else{
            val = column->second;
        }
        (*fullRow)[column_name] = val;
    }

    return fullRow;
}

Handle HeapTable::append(const ValueDict* row){
    u16 record_id;
    Dbt* data = marshal(row);

    SlottedPage* block = this->file.get(this->file.get_last_block_id());

    try{
        record_id = block->add(data);
    }catch(DbBlockNoRoomError const& e){
        block = this->file.get_new();
        record_id = block->add(data);
    }
    this->file.put(block);
    delete block;
    delete[](char*)data->get_data();
    delete data;
    return Handle(this->file.get_last_block_id(), record_id);

}