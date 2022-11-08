//
//  OrderMaster.hpp
//

#ifndef OrderMaster_hpp
#define OrderMaster_hpp

#include <string>
#include <map>

#include "Model.hpp"

/**
 * Class representing a row of the OrderMaster table.
 * @author Julian Koksal
 * @date 2022-11-07
 */
class OrderMaster : public Model
{
public:
    /**
     * Constructor creates a OrderMaster object initialized with the given values.
     * @param orderNumber The order number (primary key).
     * @param orderedBy Name of the customer of who placed the order.
     * @param orderDate The date and time of the order.
     * @param isComplete Is the order completed.
     */
    OrderMaster(int orderNumber = 0, std::string orderedBy = "", std::string orderDate = "", bool isComplete = 0);
    
    /**
     * Destructor.
     */
    ~OrderMaster();
    
    /**
     * Gets the order number.
     * @return orderNumber
     */
    int getOrderNumber();
    
    /**
     * Gets orderedBy.
     * @return orderedBy
     */
    std::string getOrderedBy();
    
    /**
     * Gets the order date.
     * @return orderDate
     */
    std::string getOrderDate();
    bool getIsComplete();
    
    /**
     * Sets orderedBy.
     * @param val The new value.
     */
    void setOrderedBy(std::string val);
    
    /**
     * Sets orderDate.
     * @param val The new value.
     */
    void setOrderDate(std::string val);
    
    /**
     * Sets isComplete.
     * @param val The new value.
     */
    void setIsComplete(bool val);
protected:
    virtual std::map<std::string, std::any> toMap() const override;
    virtual Model * fromMap(std::map<std::string, std::any> mMap) const override;
private:
    /** The order number (primary key). */
    int orderNumber;
    
    /** Name of the customer of who placed the order. */
    std::string orderedBy;
    
    /** The date and time of the order. */
    std::string orderDate;
    
    /** Is the order completed. */
    bool isComplete;
};

#endif /* OrderMaster_hpp */
