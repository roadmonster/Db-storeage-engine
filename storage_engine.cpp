#include "storage_engine.h"

/**
 * check if data type match
 * check if data type is INT
 * if not INT then must be String
 * check if equals
 **/
bool Value::operator==(const Value& other) const{
    if(this->data_type != other.data_type)
        return false;
    if(this->data_type == ColumnAttribute::INT)
        return this->n == other.n;
    return this->s == other.s;
}

/**
 * Dereference this and call the overloaded == operator with Value object
 * passed in as reference, if the == 's negative is true, then the two value
 * are not equal
 **/
bool Value::operator!=(const Value& other) const{
    return !(*this == other);
}

/**
 * ValueDict is identifier/ value dict
 * step through this dict and pass all the keys back to the list of column names
 * finally call the DbRelation::project(Handle, ColumnNames)
 **/
ValueDict* DbRelation::project(Handle handle, const ValueDict* where){
    ColumnNames t;
    for(auto const& column: *where){
        t.push_back(column.first);
    }
    return this->project(handle, &t);
}