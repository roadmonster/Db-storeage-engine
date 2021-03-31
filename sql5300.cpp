/**
 * Purpose: Milestone 1 -Framework SQL AST parser
 *
 * @file sql5300.cpp
 * @author Hao Li
 */

#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <cerrno>
#include "SQLParser.h"
#include "sqlhelper.h"
#include "db_cxx.h"

using namespace std;
using namespace hsql;

string toLowercase(string word);

string operatorToString(const Expr* expr);

string expressionToString(const Expr*expr);

string columnToString(const ColumnDefinition *col);

string tableToString(const TableRef *table);

string executeCreate(const CreateStatement *stmt);

string executeSelect(const SelectStatement *stmt);

string execute(const SQLStatement *stmt);

int main(int argc, char*argv[]){

   bool promptUser = true;
   string query;

   if(argc != 2){
      cerr << "Error: invalid command input. \nFormat is [sql5300: directory]\n";
      return 1;
   }

   char* dbenv= argv[1];
   DbEnv *myEnv = new DbEnv(0U);
   
   try{
      myEnv->open(dbenv, DB_CREATE | DB_INIT_MPOOL, 0);
   }catch(DbException &e){
      cerr << "Error: unable to open the " << dbenv << "database environment \n";
      cerr << e.what() << endl;
      return 1;
   }

   cout << "(" << argv[0] << ": running with database environment at " << dbenv << ")" << endl;
   
   do{
      cout << "SQL>";
      getline(cin, query);
      while(query.length() == 0){
         cout << "SQL>";
	 getline(cin,query);
      }

      if(toLowercase(query) == "quit"){
         promptUser = false;
      }else{
         SQLParserResult *result = SQLParser::parserSQLString(query);
	 if(result->isValid()){
	    for(unsigned i = 0; i < result->size(); i++){
	       cout << execute(result->getStatement(i)) << endl;
	    
	    }
	 }else{
	    cout<< "Error invalid SQL:" << query << endl;
	 }
      }
   }while(promptUser);



}
