// SM_tinyfsm.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <tinyfsm.hpp>

struct Off; // forward declaration


// ----------------------------------------------------------------------------
// 1. Event Declarations
//
struct Toggle : tinyfsm::Event { };


// ----------------------------------------------------------------------------
// 2. State Machine Base Class Declaration
//
struct Switch : tinyfsm::Fsm<Switch>
{
    virtual void react(Toggle const&) { };

    // alternative: enforce handling of Toggle in all states (pure virtual)
    //virtual void react(Toggle const &) = 0;

    virtual void entry(void) { };  /* entry actions in some states */
    void         exit(void) { };  /* no exit actions */

    // alternative: enforce entry actions in all states (pure virtual)
    //virtual void entry(void) = 0;
};


// ----------------------------------------------------------------------------
// 3. State Declarations
//
struct On : Switch
{
    void entry() override { std::cout << "* Switch is ON" << std::endl; };
    void react(Toggle const&) override { transit<Off>(); };
};

struct Off : Switch
{
    void entry() override { std::cout << "* Switch is OFF" << std::endl; };
    void react(Toggle const&) override { transit<On>(); };
};

FSM_INITIAL_STATE(Switch, Off)


// ----------------------------------------------------------------------------
// 4. State Machine List Declaration (dispatches events to multiple FSM's)
//
// In this example, we only have a single state machine, no need to use FsmList<>:
//using fsm_handle = tinyfsm::FsmList< Switch >;
using fsm_handle = Switch;


// ----------------------------------------------------------------------------
// Main
//
int main()
{
    // instantiate events
    Toggle toggle;

    fsm_handle::start();

    while (1)
    {
        char c;
        std::cout << std::endl << "t=Toggle, q=Quit ? ";
        std::cin >> c;
        switch (c) {
        case 't':
            std::cout << "> Toggling switch..." << std::endl;
            fsm_handle::dispatch(toggle);
            // alternative: instantiating causes no overhead (empty declaration)
            //fsm_handle::dispatch(Toggle());
            break;
        case 'q':
            return 0;
        default:
            std::cout << "> Invalid input" << std::endl;
        };
    }
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
