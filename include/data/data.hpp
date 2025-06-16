# ifndef DATA_HPP
# define DATA_HPP

# include <string>
# include <memory>

class Data {

public:
    virtual ~Data() = default;

    virtual std::shared_ptr<Data> clone() const = 0;
    
    virtual bool hasString() const = 0;
    virtual bool isNone() const = 0;
    virtual bool isScalar() const = 0;

    virtual std::string extractString() const = 0;

    virtual std::string info() const = 0;
    virtual std::string shortInfo() const = 0;


};

# endif