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

// class SqlRunner{

//    DbEnv *_DB_ENV;

//    bool initial = false;

//    void initialize_env(char* envHome){
//       // get the ptr to envrionment home
//       const char* home = getenv(envHome);
//       // initiallize the DbEnv
//       DbEnv env(0U);
//       // configure the DbEnv
//       env.set_message_stream(&std::cout);
//       env.set_error_stream(&std::cerr);
//       // open the environment with in mem cache
//       env.open(envHome, DB_CREATE|DB_INIT_MPOOL, 0);

//       // initialize the db
//       Db db(&env, 0);
//       // set the flag to true
//       initial = true;

//    }

//    // Main util method to parse each componenet of the stmt
//    void parseExpression(const Expr* expr){
//       // judge the expression type
//       switch (expr->type)
//       {
//          case kExprStart:
//             cout << "*";
//             break;
//          case kExprColumnRef:
//             if(expr->table != NULL){
//                cout<< expr->table << ".";
//             }
//             cout << express->name;
//             break;
//          case kExprOperator:
//             getExpression(expr->expr);
//             cout << " " << expr->opChar << " ";
//             getExpression (exception->expr2);
//             break;
//          case kExprLiteralFloat:
//             cout << expr->fval;
//             break;
//          case kExprLiteralInt:
//             cout << expr->ival;
//             break;
//          case kExprLiteralString:
//          cout << expr->name;
//             break;

//          default:
//             cout << "Invalid expression" << endl;
//             break;
//       }
//       if(expr->alias != NULL){
//          cout << "AS ";
//          cout << expr->alias << " ";
//       }
//    }

//    // parse the table componenent of the stmt
//    void parseTable(TableRef* tb){
//       // judge the type of the table stmt
//       switch (tb->type)
//       {
//          case kTableName:
//             cout << tb->name << " ";
//             break;
//          case kTableJoin:
//             parseJoin(tb->join);
//             break;

//          case kTableCrossProduct:
//             for(TableRef* t: *tb->list){
//                parseTable(t);
//             }
//             break;
//          default:
//             cout << "Invlid table" << endl;
//             break;
//       }

//       // check if has alias
//       if(tb->alias != NULL){
//          cout << "AS ";
//          cout << tb->alias << " ";
//       }

//    }

//    // parse the join statement
//    void parseJoin(JoinDefinition *joinStmt){
//       // parse the left part
//       parseTable(joinStmt->left);

//       // parse the join type
//       switch (joinStmt->type)
//       {
//          case kJoinInner:
//             cout << "INNER JOIN ";
//             break;
//          case kJoinOuter:
//             cout << "OUTER JOIN ";
//             break;

//          case kJoinLeft:
//             cout << "LEFT JOIN ";
//             break;

//          case kJoinRight:
//             cout << "RIGHT JOIN ";
//             break;
         
//          case kJoinLeftOuter:
//             cout << "LEFT OUTER JOIN ";
//             break;
         
//          case kJoinRightOuter:
//             cout << "RIGHT OUTER JOIN ";
//             break;
//          case kJoinCross: 
//             cout << "JOIN CROSS";
//             break;
//          case kJoinNatural: 
//             cout << "NATURAL JOIN ";
//             break;
//          default: 
//             break;

//       // parse the right table
//       parseTable(joinStmt->right);
//       cout << "ON ";

//       // finally parse the condition
//       parseExpression(joinStmt->condition);
//       cout << " ";
//    }

//    void doSelect(const SelectStatement* selStmt){
//       int i = 0;

//       int size = (*selStmt->slectList).size();

//       cout << "SELECT ";
//       // step throught the select list and parse each expression within the vector
//       for(Expr* expr: *selStmt->selectList){
//          parseExpression(expr);
//          // increment the counter
//          i++;

//          // if the counter  == size means this is the last item to select
//          // hence no need to have ',' 
//          if(i < size)
//             cout << ", ";

//          cout << "FROM "
         
//          // parse the table after 'from'
//          parseTable(selStmt->fromTable);

//          if(selStmt->whereClause != NULL){
//             cout << "WHERE ";
//             parseExpression(selStmt->whereClause->expr);
//             cout<< " " << selStmt->whereClause->opChar << " ";
//             parseExpression(selStmt->whereClause->expr2);
//          }

//       }
//    }

//    void doCreate(const CreateStatement* createStmt){
//       int size = (*createStmt->columns).size();
//       int i = 0;

//       cout << "CREATE ";
//       switch (createStmt->type)
//       {
//          case CreateStatement::kTable:
//             cout << "TABLE " << createStmt->tablename << " (";
//             for(ColumnDefinition* c_def: *createStmt->columns){
//                cout << *c_def->name;
//                switch(c_def->type){
//                   case ColumnDefinition:: TEXT:
//                      cout << " TEXT"
//                      break;
//                   case ColumnDefinition::INT:
//                      cout << " INT";
//                      break;
//                   case ColumnDefinition::DOUBLE:
//                      cout << " DOUBLE";
//                      break;
//                   default: ;
//                }
//                i++;
//                if(i < size){
//                   cout << ", ";
//                }

//             }
//             cout << ")"
//             break;
         
         
//          default:
//             cout << "invalid statement \n";
//             break;
//       }
//    }

//    void doQuery(const SQLStatement* stmt){
//       switch(stmt->type){
//          case kStmtSelect:
//             doSelect((const SelectStatement*)stmt);
//             break;
//          case kStmtCreate:
//             doCreate((const CreateStatement*)stmt);
//             break;
         
//          default: break;
//       }
//       cout << endl;

//    }
//    public:
//    void run(char* path){
//       cout << "Enter quit to quit\n";
//       if(!initial){
//          initialize_env(path);
//       }

//       while(true){
//          string query;
//          cout<< "SQL>";

//          getline(cin, query);

//          if(query == "quit"){
//             return;
//          }

//          SQLParseResult* result = SQLParser::parseSQLString(query);

//          if(parse -> isValid()){
//             doQuery(parse->getStatement(0));
//          }else{
//             cout << "Invalid statement" << endl;
//          }
//       }
//    }
// };

// int main(int argc, char* argv[]){
//    if(argc != 2){
//       cout << "Requireing db env path\n Sample [./sql5300 filepath]\n";
//       exit(1);
//    }
//    char* path = argv[1];
//    SqlRunner myRunner;
//    myRunner.run(path);

// }

int main(int argc, char* argv[]){
   if(argc != 2){
      cerr << "Requireing db env path\n Sample [./sql5300 filepath]\n"
      exit(1);
   }
   
   char* envHome = argv[1];
   cout << "(sql5300: running with database environment at " << envHome << ")" << endl;
   DbEnv env(0U);
   env.set_message_stream(&cout);
   env.set_error_stream(&cerr);

   try{
      env.open(envHome, DB_CREATE|DB_INIT_MPOOL, 0);
   }catch(DbException& e){
      cerr << e.what() << endl;
      exit(1);
   }

   _DB_ENV = &env;

   while(true){
      cout << "SQL> ";
      string query;
      getline(cin, query);
      if(query.length() == 0){
         continue;
      }
      if(query == "quit")
         break;
      if(query == "test"){
         cout << "test_heap_storage: " << (test_heap_storage()? "ok": "failed") << endl;

      }
      SQLParseResult* result = SQLParser::parseSQLString(query);

      if(parse -> isValid()){
         for(u_int i = 0; i < result->size(); ++i){
            const SQLStatement* statement = result->getStatement(i);
            try{
               cout << ParseTreeToString::statement(statement) << endl;
               QueryResult* res = SQLExec::execute(statement);
               cout<< *res << endl;
               delete res;
            }catch(SQLExecError& e){
               cout << e.what() << endl;
            }
         }
      }else{
         cout << "Invalid statement" << endl;
      }


   }
}


