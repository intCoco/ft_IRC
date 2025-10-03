#include "check.hpp"
#include "tests.hpp"
#include <cstdlib>

void testValidArgs()
{
    std::cout << "--- START TEST VALID ARGS ---" << std::endl;
    Tests test;
    test.startSuite("Valid Arguments Tests");
    
    {
        char* validArgs[] = {(char*)"./ircserv", (char*)"6667", (char*)"password"};
        Data result;
        bool success = true;
        
        try
        {
            result = Check::checkAll(3, validArgs);
            if (result.port != 6667 || result.password != "password")
            {
                success = false;
            }
        }
        catch (...)
        {
            success = false;
        }
        
        test.assert_true("Valid args port 6667 password", success);
    }
    
    //CHECK MIN PORT
    {
        char* validArgs[] = {(char*)"./ircserv", (char*)"1024", (char*)"mypass"};
        Data result;
        bool success = true;
        
        try
        {
            result = Check::checkAll(3, validArgs);
            if (result.port != 1024 || result.password != "mypass")
            {
                success = false;
            }
        }
        catch (...)
        {
            success = false;
        }
        
        test.assert_true("Valid args port 1024 mypass", success);
    }
    
    //CHECK MAX PORT 65535
    {
        char* validArgs[] = {(char*)"./ircserv", (char*)"65535", (char*)"test123"};
        Data result;
        bool success = true;
        
        try
        {
            result = Check::checkAll(3, validArgs);
            if (result.port != 65535 || result.password != "test123")
            {
                success = false;
            }
        }
        catch (...)
        {
            success = false;
        }
        
        test.assert_true("Valid args port 65535 test123", success);
    }
    
    test.printTests();
    std::cout << "--- END TEST VALID ARGS ---" << std::endl;
}

void testInvalidArgs()
{
    std::cout << "--- START TEST INVALID ARGS ---" << std::endl;
    Tests test;
    test.startSuite("Invalid Arguments Tests");
    
    //PORT NBR TO LITTLE
    {
        char* invalidArgs[] = {(char*)"./ircserv", (char*)"1023", (char*)"password"};
        bool failed = false;
        
        try
        {
            Check::checkAll(3, invalidArgs);
        }
        catch (...)
        {
            failed = true;
        }
        
        test.assert_true("Port too small 1023 fails", failed);
    }
    
    //PORT NBR TO BIG
    {
        char* invalidArgs[] = {(char*)"./ircserv", (char*)"65536", (char*)"password"};
        bool failed = false;
        
        try
        {
            Check::checkAll(3, invalidArgs);
        }
        catch (...)
        {
            failed = true;
        }
        
        test.assert_true("Port too large 65536 fails", failed);
    }
    
    //PORT IS NOT NUMBER
    {
        char* invalidArgs[] = {(char*)"./ircserv", (char*)"abc", (char*)"password"};
        bool failed = false;
        
        try
        {
            Check::checkAll(3, invalidArgs);
        }
        catch (...)
        {
            failed = true;
        }
        
        test.assert_true("Non-numeric port abc fails", failed);
    }
    
    //EMPTY PASSWORD
    {
        char* invalidArgs[] = {(char*)"./ircserv", (char*)"6667", (char*)""};
        bool failed = false;
        
        try
        {
            Check::checkAll(3, invalidArgs);
        }
        catch (...)
        {
            failed = true;
        }
        
        test.assert_true("Empty password fails", failed);
    }
    
    //NOT ENOUGH ARGS
    {
        char* invalidArgs[] = {(char*)"./ircserv", (char*)"6667"};
        bool failed = false;
        
        try
        {
            Check::checkAll(2, invalidArgs);
        }
        catch (...)
        {
            failed = true;
        }
        
        test.assert_true("Wrong argc too few fails", failed);
    }
    
    // CHECK TO MANY ARGS
    {
        char* invalidArgs[] = {(char*)"./ircserv", (char*)"6667", (char*)"pass", (char*)"extra"};
        bool failed = false;
        
        try
        {
            Check::checkAll(4, invalidArgs);
        }
        catch (...)
        {
            failed = true;
        }
        
        test.assert_true("Wrong argc too many fails", failed);
    }
    
    test.printTests();
    std::cout << "--- END TEST INVALID ARGS ---" << std::endl;
}

void runAllCheckTests()
{
    std::cout << "\n--- START ALL CHECK TESTS ---" << std::endl;
    
    testValidArgs();
    testInvalidArgs();
    
    std::cout << "--- END ALL CHECK TESTS ---" << std::endl;
}

int main()
{
    std::cout << "--- START UNIT TESTS FOR FT_IRC ---" << std::endl;
    
    runAllCheckTests();
    
    std::cout << "--- END UNIT TESTS FOR FT_IRC ---" << std::endl;
    return (0);
}