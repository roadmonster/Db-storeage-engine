#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <vector>
#include <iostream>
#include "db_cxx.h"
#include "heap_storage.h"
#include "storage_engine.h"
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
RecodeID SlottedPage::add(const Dbt* data) throw (DbBlockNoRoomError){
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
    memccpy(this->address(loc), data->get_data(), size);
    return id;
}



/**
 * Get the Dbt object (ptr, size) object provided by BerkDB
 * */
Dbt* SlottedPage::get(RecordID recode_id){
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
        memccpy(address(loc - (newSize - size)), data.get_data(), newSize);

    }else{
        memccpy(address(loc), data.get_data(), newSize);
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
RecordIDs* SlottedPage::ids(void){
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
void SlottedPage::get_header(u16& size, u16& loc, RecordID id){
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
    if(id == 0){
        size = this->num_records;
        loc = this->end_free
    }
    put_n((u16)(4 * id), size);
    put_n(((u16)4 * id + 2), loc);
}

/**
 * Calculate the available space by have endfree - (total number of records + the info of 
 * the block which also takes 4 bytes) * 4 and cast the result back to u16
 * */
bool SlottedPage::has_room(u16 size){
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
u16 SlottedPage::get_n(u16 offset){
    return *(*u16)this->address(offset);
}

// put 2-byte int at the offset location of the block address
void SlottedPage::put_n(u16 offset, u16 n){
    *(u16*)this->address(offset) = n;
}

void* SlottedPage::address(u16 offset){
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

SlottedPage* HeapFile::get(BlockID block_id){
    char block[DbBlock::BLOCK_SZ];
    Dbt data(block, sizeof(block));
    Dbt key(&block_id, sizeof(block_id));
    this->db.get(nullptr, &key, &data, 0);
    SlottedPage* page  = new SlottedPage(data, block_id, false);
    return page;
}

void HeapFile::put(DbBlock* block){
    void* blockData = block->get_data();
    Dbt data(blockData, DbBlock::BLOCK_SZ);
    int block_id = block->get_block_id();
    Dbt key(&block_id, sizeof(block_id));
    this->db.put(nullptr, &key, &data, 0);
}

BlockIDs* HeapFile::block_ids(){
    BlockIDs* myBlocks = new BlockIDs();
    for(BlockID i = 1; i <= this->last; i++){
        myBlocks->push_back(i);
    }
    return myBlocks;
}

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

