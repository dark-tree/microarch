#pragma once
#include <systemc>
#include <verilated.h>
#include <verilated_fst_sc.h>
#include <memory>
#include <iostream>
#include "Vtop.h"
#include <functional>


using namespace sc_core;
using namespace sc_dt;

class TestRunner{
private:

    int tests_ran;
    int tests_passed;

    static TestRunner* TestRunner_;

    TestRunner()
    {
        tests_ran = 0;
        tests_passed = 0;
        std::cout<<"Test session started..."<<std::endl;
    }

public:

    TestRunner(TestRunner& other) = delete;
    void operator=(const TestRunner&) = delete;

    static TestRunner& getInstance();

    bool runTest(
        std::string module_name,
        std::string test_name,
        std::function<void(
            Vtop*,
            std::function<bool(
                bool,
                bool,
                std::string
            )>,
            std::string&,
            std::string&
        )> test,
        int argc,
        char* argv[]
    )
    {
        if(sc_core::sc_curr_simcontext)
        {
            delete sc_core::sc_curr_simcontext;
        }
        sc_core::sc_curr_simcontext = NULL;

        Verilated::debug(0);			//Debug level
        Verilated::traceEverOn(true);		//We need to tell verilator, that tracing will be used
        Verilated::randReset(2); 		//Reset randomization policy
        Verilated::commandArgs(argc, argv); 	//Passing command args (required)

        Vtop* module = new Vtop{module_name.c_str()};

        bool test_passed = true;

        // We are passing to the testing function needed context (module) and a function for making assertions.
        test(
            module,
            [&test_passed](
                bool expected,
                bool actual,
                std::string assertion_name
            )
            {
                if(expected!=actual)
                {
                    test_passed = false;
                    std::cout<<"Assertion failed: "<<assertion_name<<std::endl;
                    std::cout<<"Expected: "<<expected<<", got: "<<actual<<std::endl;
                    return false;
                }
                return true;
            },
            module_name,
            test_name
        );


        std::cout<<test_name<<" on module "<<module_name<<"...\t"<<(test_passed? "[PASSED]" : "[FAILED]")<<std::endl;

        tests_ran++;
        if(test_passed)
        {
            tests_passed++;
        }


        return test_passed;
    }

    void printRaport()
    {
        std::cout<<"Test session completed: "<<tests_passed<<" out of "<<tests_ran<<" tests have passed."<<std::endl;
    }

};

TestRunner* TestRunner::TestRunner_ = nullptr;

TestRunner& TestRunner::getInstance()
{
    if(TestRunner_ == nullptr)
    {
        TestRunner_ = new TestRunner();
    }
    return *TestRunner_;
}


#define SET_CLOCK(port_name, period, delay) \
    sc_clock port_name{"clk", 10, SC_NS, 0.5, 3,	SC_NS, true};  \
    module->port_name(port_name);

#define SET_SIGNAL(port_name)  \
    sc_signal<bool> port_name;  \
    module->port_name(port_name);

#define START_SIMULATION \
    sc_start(SC_ZERO_TIME); \
    auto tfp = new VerilatedFstSc;  \
    module->trace(tfp, 99);   \
    std::string file_name = "tracedump/traces_"; \
    file_name += module_name;   \
    file_name += '_'; \
    file_name += test_name; \
    file_name += ".fst"; \
    tfp->open(file_name.c_str());

#define TEST(module_name_, test_name_) \
    runner.runTest( \
        #module_name_,  \
        #test_name_,  \
        []( \
            Vtop* module, \
            std::function<bool( \
                bool, \
                bool, \
                std::string \
            )> assert_signal, \
            std::string& module_name, \
            std::string& test_name  \
        ){

#define END_SIMULATION  \
            module->final();  \
            tfp->close(); \
        },  \
        argc, \
        argv \
    );

#define TESTS \
    int sc_main(int argc, char* argv[]) \
    { \
        TestRunner& runner = TestRunner::getInstance();


#define END_TESTS \
        runner.printRaport(); \
        return 0; \
    }

#define STEP(time) sc_start(time, SC_NS);
