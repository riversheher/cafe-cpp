//
// Created by Sharon Nikritin on 2022-11-05.
//
#include "AuthToken.hpp"

AuthToken ::AuthToken(std::string tokenID, std::string createdDate) {
    this -> tableName = "AuthToken";
    this -> columns = { "tokenID", "createdDate" };
    this -> keys = { columns[0] };
    this -> isAutoGeneratedKey = false;

    this -> tokenID = tokenID;
    this -> createdDate = createdDate;

}

AuthToken ::~AuthToken() {

}

std::map<std::string, std::any> AuthToken:: toMap() const
{
    std::map<std::string, std::any> mMap;
    mMap[columns[0]] = tokenID;
    mMap[columns[1]] = createdDate;

    return mMap;
}

Model * AuthToken::fromMap(std::map<std::string, std::any> mMap) const
{

    std::string mID = std::any_cast<std::string>(mMap[columns[0]]);
    std::string mCreatedDate = std::any_cast<std::string>(mMap[columns[1]]);
    AuthToken *model = new AuthToken(mID, mCreatedDate);

    return model;
}

std::string AuthToken::getTokenID() {
    return tokenID;
}

bool AuthToken::isValid() {
    return true;
}


