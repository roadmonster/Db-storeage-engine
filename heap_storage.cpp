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

SlottedPage::SlottedPage(Dbt& block, BlockID block_id, bool is_new): DbBlock(block, block_id, is_new){
    if(is_new){
        this->num_records = 0;
        this->end_free = DbBlock::BLOCK_SZ -1;
        put_header();
    }else{
        get_header(this->num_records, this->end_free);
    }
}

RecodeID SlottedPage::add(const Dbt* data) throw (DbBlockNoRoomError){
    if(!has_room(data->get_size())){
        throw DbBlockNoRoomError("not enough room for new record");
    }

    u16 id = ++this->num_records;
    u16 size = (u16) data->get_size();
    this->end_free -= size;
    u16 loc = this->end_free + 1;
    put_header();
    put_header(id, size, loc);
    memccpy(this->address(loc), data->get_data(), size);
    return id;
}


Dbt* SlottedPage::get(RecordID recode_id){
    u16 size, loc;
    get_header(size, loc, record_id);
    if(loc == 0){
        return nullptr;
    }
    return new Dbt(this->address(loc), size);

}

void SlottedPage::put(RecordID record_id, const Dbt& data)throw(DbBlockNoRoomError){

}

void SlottedPage::del(RecordID recode_id){

}

RecordIDs* SlottedPage::ids(void){

}

void SlottedPage::get_header(u16& size, u16& loc, RecordID id){

}

void SlottedPage::put_header(RecordID id, u16 size, u16 loc){

}

bool SlottedPage::has_room(u16 size){
    u16 avail_size = this->end_free - (u16)((this->num_records + 1) * 4);
    return size <= avail_size;
}

void SlottedPage::slide(u16 start, u16 end){

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

}

void HeapFile::drop(){

}

void HeapFile::open(){

}

void HeapFile::close(){

}

SlottedPage* HeapFile::get_new(void){

}

