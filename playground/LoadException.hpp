#ifndef LOADEXCEPTION_HPP
#define LOADEXCEPTION_HPP

#include <exception>

class LoadException : public std::exception {
private:
    const char* _message;

public:
    LoadException(const char* message) {
        _message = message;
    }
    
    virtual const char* what() {
        return _message;
    }
};

#endif//LOADEXCEPTION_HPP
