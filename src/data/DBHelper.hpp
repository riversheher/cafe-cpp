//
//  DBHelper.hpp
//

#ifndef DBHelper_hpp
#define DBHelper_hpp

#include <string>
#include <any>
#include <vector>
#include <set>
#include <map>
#include <typeinfo>
#include <iostream>

#include "sqlite3.h"

#include "Model.hpp"
#include "SqlCondition.hpp"

/**
 * @brief Data access layer class.
 *
 * Provides an abstract interface for accessing a SQLite3 database.
 * @author Julian Koksal
 * @date 2022-09-25
 */
class DBHelper
{
public:
    /**
     * @brief Destructor.
     *
     * Closes the sqlite3 database handle.
     */
    ~DBHelper();
    
    /**
     * @brief Gets the singleton instance of this class.
     * @return singleton instance of DBHelper
     */
    static const DBHelper & getInstance();
    
    /**
     * @brief Creates the MenuItem table. Used only in TestDBHelper
     */
    void createTableMenuItem() const;
    
    /**
     * @brief Reads all rows from the table represented by model and returns the result as a vector of models.
     * @param model Used to determine the table name, column names and types, and to cast the results. Must inherit from Model.
     * @param conditions Used to generate the WHERE clause of the select statement.
     * @param orderBy The field and direction used to generate the ORDER BY clause of the select statement. e.g. "price DESC".
     * @param columns The set of column names to select. If empty, all columns are selected.
     * @return The result of the select statement as a vector of models.
     */
    template<class T, class = std::enable_if_t<std::is_base_of<Model, T>::value>>
    std::vector<T> selectWhere(const T &model, std::vector<SqlCondition> conditions = std::vector<SqlCondition>(), std::string orderBy = "", std::set<std::string> columns = std::set<std::string>()) const
    {
        std::vector<T> result;
        // Helper reads from the database.
        std::vector<Model *> helperResult = selectWhereHelper(model, conditions, orderBy, columns);
        // Converts the results to static T objects and deletes the dynamic objects created by selectWhereHelper().
        for (int i = 0; i < helperResult.size(); i++)
        {
            T *row = (T *)helperResult[i];
            result.push_back(*row);
            delete row;
        }
        return result;
    }
    
    /**
     * @brief Inserts the given model to its associated table in the database.
     * @param model The model to insert to the database.
     */
    long long insert(const Model &model) const;
    
    /**
     * @brief Updates the given model in the database by its primary key(s).
     * @param model The model to update, identified by its keys member.
     */
    void update(const Model &model) const;
    
    /**
     * @brief Conditionally updates the table that model represents with the values of model.
     * @param conditions Used to generate the WHERE clause of the update statement.
     * @param columns The set of column names to update. If empty, all columns are updated.
     */
    void updateWhere(const Model &model, std::vector<SqlCondition> conditions, std::set<std::string> columns) const;
    
    /**
     * @brief Deletes the given model from the database by its primary key(s).
     * @param model The model to delete.
     */
    void destroy(const Model &model) const;
    
    /**
     * @brief Conditionally deletes from the table that model represents.
     *
     * Use with caution! Will delete every row in the table if conditions is empty.
     * @param model Used to determine the table name.
     * @param conditions Used to generate the WHERE clause of the delete statement.
     */
    void destroyWhere(const Model &model, std::vector<SqlCondition> conditions) const;
    
private:
    /** @brief Singleton instance of DBHelper. */
    static const DBHelper * instance;
    
    /** @brief SQLite3 handle. */
    sqlite3* db;
    
    /**
     * @brief Constructor.
     */
    DBHelper();
    
    /**
     * @brief Copy constructor. Not implemented to prevent copying of singleton instance.
     */
    DBHelper(const DBHelper &other);
    
    /**
     * @brief Assignment operator overload. Not implemented to prevent assignment of singleton instance.
     */
    DBHelper& operator=(const DBHelper &other);
    
    /**
     * @brief Helper method reads all rows from the table represented by model, and returns the result as a vector of Model pointers.
     * @param model Used to determine the table name, column names and types, and to cast the results.
     * @param conditions Used to generate the WHERE clause of the select statement.
     * @param orderBy The field and direction used to generate the ORDER BY clause of the select statement. e.g. "price DESC".
     * @param columns The set of column names to select. If empty, all columns are selected.
     * @return The result of the select statement as a vector of Model pointers.
     */
    std::vector<Model *> selectWhereHelper(const Model &model, std::vector<SqlCondition> conditions, std::string orderBy, std::set<std::string> columns) const;
    
    /**
     * @brief Opens the sqlite3 database handle.
     */
    void openDB();
    
    /**
     * @brief Closes the sqlite3 database handle.
     */
    void closeDB();
};

#endif /* DBHelper_hpp */
