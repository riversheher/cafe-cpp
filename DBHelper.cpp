//
//  DBHelper.cpp
//  DAL Test
//
//  Created by Julian Koksal on 2022-09-25.
//

#include "sqlite3.h"
#include "DBHelper.hpp"

DBHelper::DBHelper()
{
    openDB();
}

DBHelper::~DBHelper()
{
    closeDB();
}

void DBHelper::createTableMenuItem()
{
    std::string query;
    query = "CREATE TABLE IF NOT EXISTS MenuItem (name text not null primary key,price real not null);";
    
    int result;
    char* error_msg;
    
    result = sqlite3_exec(db, query.c_str(), NULL, NULL, &error_msg);
    
    if (error_msg != NULL)
    {
        free(error_msg);
        throw std::runtime_error("Error creating table.");
    }
}

void DBHelper::insert(Model *model)
{
    std::map<std::string, std::any> mMap = model->toMap();
    
    std::string query;
    query  = "INSERT INTO " + model->tableName + " ";
    query += "VALUES (?";
    for (int i = 1; i < mMap.size(); i++)
    {
        query += ",?";
    }
    query += ");";
    
    sqlite3_stmt *statement;
    int prepareResult = sqlite3_prepare_v2(db, query.c_str(), -1, &statement, NULL);
    if (prepareResult != SQLITE_OK)
    {
        sqlite3_finalize(statement);
        throw std::runtime_error("Error preparing insert statement. SQLite3 error code: " + std::to_string(prepareResult));
    }
    
    // Iterates the fields of model as represented in mMap and binds their values to the SQL statement.
    int index = 1; // SQL statement parameter index.
    for (std::map<std::string, std::any>::iterator it = mMap.begin(); it != mMap.end(); it++)
    {
        int bindResult = -1;
        
        if (it->first == model->autoGeneratedKey)
        {
            // NULL is binded to the auto generated key column so that SQLite3 will automatically choose a unique value.
            bindResult = sqlite3_bind_null(statement, index);
        }
        else
        {
            const std::type_info *attrType = &it->second.type();
            if (*attrType == typeid(bool))
            {
                bindResult = sqlite3_bind_int(statement, index, std::any_cast<bool>(it->second));
            }
            if (*attrType == typeid(int))
            {
                bindResult = sqlite3_bind_int(statement, index, std::any_cast<int>(it->second));
            }
            if (*attrType == typeid(double))
            {
                bindResult = sqlite3_bind_double(statement, index, std::any_cast<double>(it->second));
            }
            if (*attrType == typeid(std::string))
            {
                bindResult = sqlite3_bind_text(statement, index, std::any_cast<std::string>(it->second).c_str(), -1, NULL);
            }
        }
        
        if (bindResult != SQLITE_OK)
        {
            sqlite3_finalize(statement);
            if (bindResult == -1)
            {
                throw std::runtime_error("Error in call to DBHelper::insert(). The value of '"
                                         + it->first + "' is not a supported data type.");
            }
            throw std::runtime_error("Error binding insert statement. SQLite3 error code: " + std::to_string(bindResult));
        }
        
        index++;
    }
    
    sqlite3_step(statement);
    
    int finalizeResult = sqlite3_finalize(statement);
    if (finalizeResult != SQLITE_OK)
    {
        throw std::runtime_error("Error inserting to '" + model->tableName + "'. SQLite3 error code: " + std::to_string(finalizeResult));
    }
}

void DBHelper::update(Model *model)
{
    std::set<std::string> keys = model->keys;
    // If the table has no primary keys then DBHelper::update cannot be used. Use DBHelper::updateWhere instead.
    if (keys.empty())
    {
        throw std::runtime_error("Error in call to DBHelper::update(). '" + model->tableName + "' has no keys.");
    }
    
    std::map<std::string, std::any> mMap = model->toMap();
    
    std::string query;
    query  = "UPDATE " + model->tableName + " SET ";
    // Iterates field names of model to generate the SET command.
    for (std::map<std::string, std::any>::iterator it = mMap.begin(); it != mMap.end(); it++)
    {
        // Primary keys should not be updated.
        if (!keys.count(it->first))
        {
            query += it->first + " = ?,";
        }
    }
    query  = query.substr(0, query.size() - 1);
    query += " WHERE 1=1";
    // Iterates key fields of model to generate the WHERE clause.
    for (std::set<std::string>::iterator it = keys.begin(); it != keys.end(); it++)
    {
        query += " AND " + *it + " = ?";
    }
    query += ";";
    
    sqlite3_stmt *statement;
    int prepareResult = sqlite3_prepare_v2(db, query.c_str(), -1, &statement, NULL);
    if (prepareResult != SQLITE_OK)
    {
        sqlite3_finalize(statement);
        throw std::runtime_error("Error preparing update statement. SQLite3 error code: " + std::to_string(prepareResult));
    }
    
    // Iterates the fields of model as represented in mMap and binds their values to the SET command part of the SQL statement.
    int index = 1; // SQL statement parameter index.
    for (std::map<std::string, std::any>::iterator it = mMap.begin(); it != mMap.end(); it++)
    {
        if (!keys.count(it->first))
        {
            int bindResult = -1;
            
            const std::type_info *attrType = &it->second.type();
            if (*attrType == typeid(bool))
            {
                bindResult = sqlite3_bind_int(statement, index, std::any_cast<bool>(it->second));
            }
            if (*attrType == typeid(int))
            {
                bindResult = sqlite3_bind_int(statement, index, std::any_cast<int>(it->second));
            }
            if (*attrType == typeid(double))
            {
                bindResult = sqlite3_bind_double(statement, index, std::any_cast<double>(it->second));
            }
            if (*attrType == typeid(std::string))
            {
                bindResult = sqlite3_bind_text(statement, index, std::any_cast<std::string>(it->second).c_str(), -1, NULL);
            }
            
            if (bindResult != SQLITE_OK)
            {
                sqlite3_finalize(statement);
                if (bindResult == -1)
                {
                    throw std::runtime_error("Error in call to DBHelper::update(). The value of '"
                                             + it->first + "' is not a supported data type.");
                }
                throw std::runtime_error("Error binding update statement. SQLite3 error code: " + std::to_string(bindResult));
            }
            
            index++;
        }
    }
    
    // Iterates the key fields of model and binds their values to the WHERE clause of the SQL statement.
    for (std::set<std::string>::iterator it = keys.begin(); it != keys.end(); it++)
    {
        int bindResult = -1;
        
        const std::type_info *attrType = &mMap[*it].type();
        if (*attrType == typeid(int))
        {
            bindResult = sqlite3_bind_int(statement, index, std::any_cast<int>(mMap[*it]));
        }
        if (*attrType == typeid(std::string))
        {
            bindResult = sqlite3_bind_text(statement, index, std::any_cast<std::string>(mMap[*it]).c_str(), -1, NULL);
        }
        
        if (bindResult != SQLITE_OK)
        {
            sqlite3_finalize(statement);
            if (bindResult == -1)
            {
                throw std::runtime_error("Error in call to DBHelper::update(). The value of '"
                                         + *it + "' is not a supported data type.");
            }
            throw std::runtime_error("Error binding update statement. SQLite3 error code: " + std::to_string(bindResult));
        }
        
        index++;
    }
    
    sqlite3_step(statement);
    
    int finalizeResult = sqlite3_finalize(statement);
    if (finalizeResult != SQLITE_OK)
    {
        throw std::runtime_error("Error updating '" + model->tableName + "'. SQLite3 error code: " + std::to_string(finalizeResult));
    }
}

void DBHelper::updateWhere(Model *model, std::vector<SqlCondition> conditions)
{
    std::map<std::string, std::any> mMap = model->toMap();
    std::set<std::string> keys = model->keys;
    
    std::string query;
    query  = "UPDATE " + model->tableName + " SET ";
    // Iterates the field names of model to generate the SET command.
    for (std::map<std::string, std::any>::iterator it = mMap.begin(); it != mMap.end(); it++)
    {
        // Primary keys should not be updated.
        if (!keys.count(it->first))
        {
            query += it->first + " = ?,";
        }
    }
    query  = query.substr(0, query.size() - 1);
    query += " WHERE 1=1";
    // Iterates the conditions parameter to generate the WHERE clause.
    for (int i = 0; i < conditions.size(); i++)
    {
        query += " AND " + conditions[i].field + " " + conditions[i].op + " ?";
    }
    query += ";";
    
    sqlite3_stmt *statement;
    int prepareResult = sqlite3_prepare_v2(db, query.c_str(), -1, &statement, NULL);
    if (prepareResult != SQLITE_OK)
    {
        sqlite3_finalize(statement);
        throw std::runtime_error("Error preparing update statement. SQLite3 error code: " + std::to_string(prepareResult));
    }
    
    // Iterates the fields of model as represented in mMap and binds their values to the SET command part of the SQL statement.
    int index = 1; // SQL statement parameter index.
    for (std::map<std::string, std::any>::iterator it = mMap.begin(); it != mMap.end(); it++)
    {
        if (!keys.count(it->first))
        {
            int bindResult = -1;
            
            const std::type_info *attrType = &it->second.type();
            if (*attrType == typeid(bool))
            {
                bindResult = sqlite3_bind_int(statement, index, std::any_cast<bool>(it->second));
            }
            if (*attrType == typeid(int))
            {
                bindResult = sqlite3_bind_int(statement, index, std::any_cast<int>(it->second));
            }
            if (*attrType == typeid(double))
            {
                bindResult = sqlite3_bind_double(statement, index, std::any_cast<double>(it->second));
            }
            if (*attrType == typeid(std::string))
            {
                bindResult = sqlite3_bind_text(statement, index, std::any_cast<std::string>(it->second).c_str(), -1, NULL);
            }
            
            if (bindResult != SQLITE_OK)
            {
                sqlite3_finalize(statement);
                if (bindResult == -1)
                {
                    throw std::runtime_error("Error in call to DBHelper::updateWhere(). The value of '"
                                             + it->first + "' is not a supported data type.");
                }
                throw std::runtime_error("Error binding update statement. SQLite3 error code: " + std::to_string(bindResult));
            }
            
            index++;
        }
    }
    
    // Iterates the conditions parameter and binds their values to the WHERE clause of the SQL statement.
    for (int i = 0; i < conditions.size(); i++)
    {
        int bindResult = -1;
        
        const std::type_info *attrType = &conditions[i].value.type();
        if (*attrType == typeid(bool))
        {
            bindResult = sqlite3_bind_int(statement, index, std::any_cast<bool>(conditions[i].value));
        }
        if (*attrType == typeid(int))
        {
            bindResult = sqlite3_bind_int(statement, index, std::any_cast<int>(conditions[i].value));
        }
        if (*attrType == typeid(double))
        {
            bindResult = sqlite3_bind_double(statement, index, std::any_cast<double>(conditions[i].value));
        }
        if (*attrType == typeid(std::string))
        {
            bindResult = sqlite3_bind_text(statement, index, std::any_cast<std::string>(conditions[i].value).c_str(), -1, NULL);
        }
        
        if (bindResult != SQLITE_OK)
        {
            sqlite3_finalize(statement);
            if (bindResult == -1)
            {
                throw std::runtime_error("Error in call to DBHelper::updateWhere(). The value of '"
                                         + conditions[i].field + "' is not a supported data type.");
            }
            throw std::runtime_error("Error binding update statement. SQLite3 error code: " + std::to_string(bindResult));
        }
        
        index++;
    }
    
    sqlite3_step(statement);
    
    int finalizeResult = sqlite3_finalize(statement);
    if (finalizeResult != SQLITE_OK)
    {
        throw std::runtime_error("Error updating '" + model->tableName + "'. SQLite3 error code: " + std::to_string(finalizeResult));
    }
}

void DBHelper::destroy(Model *model)
{
    std::set<std::string> keys = model->keys;
    // If the table has no primary keys then DBHelper::destroy cannot be used. Use DBHelper::destroyWhere instead.
    if (keys.empty())
    {
        throw std::runtime_error("Error in call to DBHelper::destroy(). '" + model->tableName + "' has no keys.");
    }
    
    std::map<std::string, std::any> mMap = model->toMap();
    
    std::string query;
    query  = "DELETE FROM " + model->tableName + " WHERE 1=1";
    // Iterates the key fields of model to generate the WHERE clause.
    for (std::set<std::string>::iterator it = keys.begin(); it != keys.end(); it++)
    {
        query += " AND " + *it + " = ?";
    }
    query += ";";
    
    sqlite3_stmt *statement;
    int prepareResult = sqlite3_prepare_v2(db, query.c_str(), -1, &statement, NULL);
    if (prepareResult != SQLITE_OK)
    {
        sqlite3_finalize(statement);
        throw std::runtime_error("Error preparing delete statement. SQLite3 error code: " + std::to_string(prepareResult));
    }
    
    // Iterates the key fields of model and binds their values to the WHERE clause of the SQL statement.
    int index = 1; // SQL statement parameter index.
    for (std::set<std::string>::iterator it = keys.begin(); it != keys.end(); it++)
    {
        int bindResult = -1;
        
        const std::type_info *attrType = &mMap[*it].type();
        if (*attrType == typeid(int))
        {
            bindResult = sqlite3_bind_int(statement, index, std::any_cast<int>(mMap[*it]));
        }
        if (*attrType == typeid(std::string))
        {
            bindResult = sqlite3_bind_text(statement, index, std::any_cast<std::string>(mMap[*it]).c_str(), -1, NULL);
        }
        
        if (bindResult != SQLITE_OK)
        {
            sqlite3_finalize(statement);
            if (bindResult == -1)
            {
                throw std::runtime_error("Error in call to DBHelper::destroy(). The value of '"
                                         + *it + "' is not a supported data type.");
            }
            throw std::runtime_error("Error binding update statement. SQLite3 error code: " + std::to_string(bindResult));
        }
        
        index++;
    }
    
    sqlite3_step(statement);
    
    int finalizeResult = sqlite3_finalize(statement);
    if (finalizeResult != SQLITE_OK)
    {
        throw std::runtime_error("Error deleting from '" + model->tableName + "'. SQLite3 error code: " + std::to_string(finalizeResult));
    }
}

void DBHelper::destroyWhere(Model *model, std::vector<SqlCondition> conditions)
{
    std::string query;
    query  = "DELETE FROM " + model->tableName + " WHERE 1=1";
    // Iterates the conditions parameter to generate the WHERE clause.
    for (int i = 0; i < conditions.size(); i++)
    {
        query += " AND " + conditions[i].field + " " + conditions[i].op + " ?";
    }
    query += ";";
    
    sqlite3_stmt *statement;
    int prepareResult = sqlite3_prepare_v2(db, query.c_str(), -1, &statement, NULL);
    if (prepareResult != SQLITE_OK)
    {
        sqlite3_finalize(statement);
        throw std::runtime_error("Error preparing delete statement. SQLite3 error code: " + std::to_string(prepareResult));
    }
    
    // Iterates the conditions parameter and binds their values to the WHERE clause of the SQL statement.
    for (int i = 0; i < conditions.size(); i++)
    {
        int bindResult = -1;
        
        const std::type_info *attrType = &conditions[i].value.type();
        if (*attrType == typeid(bool))
        {
            bindResult = sqlite3_bind_int(statement, i + 1, std::any_cast<bool>(conditions[i].value));
        }
        if (*attrType == typeid(int))
        {
            bindResult = sqlite3_bind_int(statement, i + 1, std::any_cast<int>(conditions[i].value));
        }
        if (*attrType == typeid(double))
        {
            bindResult = sqlite3_bind_double(statement, i + 1, std::any_cast<double>(conditions[i].value));
        }
        if (*attrType == typeid(std::string))
        {
            bindResult = sqlite3_bind_text(statement, i + 1, std::any_cast<std::string>(conditions[i].value).c_str(), -1, NULL);
        }
        
        if (bindResult != SQLITE_OK)
        {
            sqlite3_finalize(statement);
            if (bindResult == -1)
            {
                throw std::runtime_error("Error in call to DBHelper::destroyWhere(). The value of '"
                                         + conditions[i].field + "' is not a supported data type.");
            }
            throw std::runtime_error("Error binding update statement. SQLite3 error code: " + std::to_string(bindResult));
        }
    }
    
    sqlite3_step(statement);
    
    int finalizeResult = sqlite3_finalize(statement);
    if (finalizeResult != SQLITE_OK)
    {
        throw std::runtime_error("Error deleting from '" + model->tableName + "'. SQLite3 error code: " + std::to_string(finalizeResult));
    }
}

std::vector<Model *> DBHelper::selectWhereHelper(Model *model, std::vector<SqlCondition> conditions, std::string orderBy, std::set<std::string> columns)
{
    std::map<std::string, std::any> mMap = model->toMap();
    
    std::string query;
    query  = "SELECT ";
    // Iterates the field names of model to generate the list of columns.
    // If a non-empty columns parameter is given then only the field names that are also in columns will be included.
    for (std::map<std::string, std::any>::iterator it = mMap.begin(); it != mMap.end(); it++)
    {
        if (columns.empty() || columns.count(it->first))
        {
            query += it->first + ",";
        }
    }
    query = query.substr(0, query.size() - 1);
    query += " FROM " + model->tableName;
    query += " WHERE 1=1";
    // Iterates the conditions parameter to generate the WHERE clause.
    for (int i = 0; i < conditions.size(); i++)
    {
        query += " AND " + conditions[i].field + " " + conditions[i].op + " ?";
    }
    if (!orderBy.empty())
    {
        query += " ORDER BY " + orderBy;
    }
    query += ";";
    
    sqlite3_stmt *statement;
    int prepareResult = sqlite3_prepare_v2(db, query.c_str(), -1, &statement, nullptr);
    if (prepareResult != SQLITE_OK)
    {
        throw std::runtime_error("Error preparing select statement. SQLite3 error code: " + std::to_string(prepareResult));
    }
    
    // Iterates the conditions parameter and binds their values to the WHERE clause of the SQL statement.
    for (int i = 0; i < conditions.size(); i++)
    {
        int bindResult = -1;
        
        const std::type_info *attrType = &conditions[i].value.type();
        if (*attrType == typeid(bool))
        {
            bindResult = sqlite3_bind_int(statement, i + 1, std::any_cast<bool>(conditions[i].value));
        }
        if (*attrType == typeid(int))
        {
            bindResult = sqlite3_bind_int(statement, i + 1, std::any_cast<int>(conditions[i].value));
        }
        if (*attrType == typeid(double))
        {
            bindResult = sqlite3_bind_double(statement, i + 1, std::any_cast<double>(conditions[i].value));
        }
        if (*attrType == typeid(std::string))
        {
            bindResult = sqlite3_bind_text(statement, i + 1, std::any_cast<std::string>(conditions[i].value).c_str(), -1, nullptr);
        }
        
        if (bindResult != SQLITE_OK)
        {
            sqlite3_finalize(statement);
            if (bindResult == -1)
            {
                throw std::runtime_error("Error in call to DBHelper::selectWhere(). The value of the SqlCondition with field '"
                                         + conditions[i].field + "' is not a supported data type.");
            }
            throw std::runtime_error("Error binding select statement. SQLite3 error code: " + std::to_string(bindResult));
        }
    }
    
    std::vector<Model *> result;
    int stepResult = sqlite3_step(statement);
    int columnCount = sqlite3_column_count(statement);
    while (stepResult == SQLITE_ROW)
    {
        // The map is used to determine column types and use the appropriate function to assign results back to the map.
        std::map<std::string, std::any> row = model->toMap();
        for (int i = 0; i < columnCount; i++)
        {
            const char *columnName = sqlite3_column_name(statement, i);
            const std::type_info *attrType = &row[columnName].type();
            if (*attrType == typeid(bool)) {
                row[columnName] = sqlite3_column_int(statement, i);
            }
            if (*attrType == typeid(int)) {
                row[columnName] = sqlite3_column_int(statement, i);
            }
            if (*attrType == typeid(double))
            {
                row[columnName] = sqlite3_column_double(statement, i);
            }
            if (*attrType == typeid(std::string))
            {
                row[columnName] = std::string((const char*)sqlite3_column_text(statement, i));
            }
        }
        // Creates a new object from the map.
        // Uses dynamic memory! Results must be deleted later.
        result.push_back(model->fromMap(row));
        stepResult = sqlite3_step(statement);
    }
    
    int finalizeResult = sqlite3_finalize(statement);
    if (finalizeResult != SQLITE_OK)
    {
        throw std::runtime_error("Error reading from database. SQLite3 error code: " + std::to_string(finalizeResult));
    }
    
    return result;
}

void DBHelper::openDB()
{
    const char* file_name = "data.db";
    int result;
    result = sqlite3_open(file_name, &db);
    if (result != SQLITE_OK) {
        closeDB();
        throw std::runtime_error("Error opening database.");
    }
}

void DBHelper::closeDB()
{
    int result;
    result = sqlite3_close(db);
    if (result != SQLITE_OK)
    {
        throw std::runtime_error("Error closing database.");
    }
}
