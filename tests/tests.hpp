#ifndef TESTS_HPP
#define TESTS_HPP

#include <iostream>
#include <string>

class Tests
{
private:
    int _totalTests;
    int _passedTests;
    std::string _currentSuite;
    
public:
    Tests();
    Tests(const Tests& other);
    Tests& operator=(const Tests& other);
    ~Tests();
    
    void startSuite(const std::string& suiteName);
    void assert_true(const std::string& testName, bool condition);
    void printTests();
    
    int getTotalTests() const;
    int getPassedTests() const;
};

#endif