#include "tests.hpp"

Tests::Tests()
{
    _totalTests = 0;
    _passedTests = 0;
    _currentSuite = "";
}

Tests::Tests(const Tests& other)
{
    _totalTests = other._totalTests;
    _passedTests = other._passedTests;
    _currentSuite = other._currentSuite;
}

Tests& Tests::operator=(const Tests& other)
{
    if (this != &other)
    {
        _totalTests = other._totalTests;
        _passedTests = other._passedTests;
        _currentSuite = other._currentSuite;
    }
    return (*this);
}

Tests::~Tests()
{
}

void Tests::startSuite(const std::string& suiteName)
{
    _currentSuite = suiteName;
    std::cout << "\n=== " << suiteName << " ===" << std::endl;
}

void Tests::assert_true(const std::string& testName, bool condition)
{
    _totalTests++;
    if (condition)
    {
        std::cout << testName << " : PASS" << std::endl;
        _passedTests++;
    }
    else
    {
        std::cout << testName << " : FAIL" << std::endl;
    }
}

void Tests::printTests()
{
    std::cout << "Tests = " << _passedTests << "/" << _totalTests << std::endl;
    
    if (_passedTests == _totalTests)
    {
        std::cout << "PERFECT" << std::endl;
    }
    else
    {
        std::cout << (_totalTests - _passedTests) << " KO" << std::endl;
    }
}

int Tests::getTotalTests() const
{
    return (_totalTests);
}

int Tests::getPassedTests() const
{
    return (_passedTests);
}