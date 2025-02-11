// SM_tinyfsm.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
//using the following library 
// https://github.com/digint/tinyfsm ;

#include <iostream>
#include <chrono>
#include <thread>
#include <tinyfsm.hpp>

//enum for case statement in main
enum caseStates {
    caseBoot,
    caseIdle,
    caseReady,
    caseAscent,
    caseCoast,
    caseMain_Descent,
    caseDrouge_Descent,
    caseLanded
};

//golbal variable for switch statement in main{}
caseStates case_state;

//states
struct Off; // forward declaration
struct Other;
struct Boot;
struct Idle;
struct Ready;
struct Ascent;
struct Coast;
struct Main_Descent;
struct Drogue_Descent;
struct Landed;

// ----------------------------------------------------------------------------
// Constants for transitions
// 
const float A = 111.54; // ft/s^2
const float B = 49.2; //ft
const float C = 1; // s
const float D = 13.1; // s 
const float E = 1; // s
const float F = 1000; // ft
const float G = 10; // s


// ----------------------------------------------------------------------------
// 1. Event Declarations
//
struct Toggle : tinyfsm::Event { };
struct Heartbeat : tinyfsm::Event { };
struct Signal_Liftoff : tinyfsm::Event { };
struct Liftoff : tinyfsm::Event { };
struct Seperation : tinyfsm::Event { };
struct Apogee : tinyfsm::Event{ };
struct Altitude : tinyfsm::Event { };
struct Touchdown : tinyfsm::Event{ };


// ----------------------------------------------------------------------------
// 2. State Machine Base Class Declaration
//
struct Switch : tinyfsm::Fsm<Switch>
{
    virtual void react(Toggle const&) { std::cout << "not implemented for state" << std::endl; };
    virtual void react(Heartbeat const&) { std::cout << "not implemented for state" << std::endl; } 
    virtual void react(Signal_Liftoff const&) { std::cout << "not implemented for state" << std::endl;  };
    virtual void react(Liftoff const&) { std::cout << "not implemented for state" << std::endl;  };
    virtual void react(Seperation const&) { std::cout << "not implemented for state" << std::endl;  };
    virtual void react(Apogee const&) { std::cout << "not implemented for state" << std::endl;  };
    virtual void react(Altitude const&) { std::cout << "not implemented for state" << std::endl;  };
    virtual void react(Touchdown const&) { std::cout << "not implemented for state" << std::endl;  };

    // alternative: enforce handling of Toggle in all states (pure virtual)
    //virtual void react(Toggle const &) = 0;

    virtual void entry(void) { };  /* entry actions in some states */
    virtual void exit(void) { };  /* no exit actions */

    // alternative: enforce entry actions in all states (pure virtual)
    //virtual void entry(void) = 0;

    //this should only be written to by Boot state
    static float altitude;
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
    void react(Toggle const&) override { transit<Other>(); };
};

struct Other : Switch
{
    void entry() override { std::cout << "We created another state!!!" << std::endl; };
    void react(Toggle const&) override { transit<On>();}
};

struct Boot : Switch
{
    int counter = 0;
    void entry() override {
        /*start ros node or whatever to detect
        heartbeets*/ 
        //set altitude here?? static altitude variable in parent class
    };

    void react(Heartbeat const&) {
        /*look at heart beat and after some condition
        transition to next state*/
        if (counter > 2) {
            case_state = caseIdle;
            transit<Idle>();
            
        }
        counter++;
    }

    void exit() override {
        /*unsubscribe to ros node ??*/
        //case_state = caseIdle;
    }
};

struct Idle : Switch
{
    int counter = 0;
    void entry() override {
        /*set up web socket or whatever to detect signal*/
        /*listen to something for acceleration.. we using ROS
        for this??*/
        std::cout << "entering Idle state" << std::endl;
    }
    
    void react(Signal_Liftoff const&) {
        /*check for signal or acceleration*/

        /*
        * //if either happen do we log here or do it in exit or
        * //start the logging in the next state
        if (signal){
            case_state = caseReady;
            transit<Ready>();
        }
        if (acceleration > A || altitude_gain > (D - C + 1){
            case_state = caseAscent;
            transit<Ascent>();

        }*/
        if (counter > 2) {
            case_state = caseAscent;
            transit<Idle>();
        }
        counter++;
    }

    void exit() override {
        /*clean anything up as we exit the state*/
        std::cout << "leaving Idle state" << std::endl;
    }
};

struct Ready : Switch
{
    /*does acceleration come from ros or some other source
    or do we calcualte it*/
    int counter = 0;
    void entry() override {
        std::cout << "Entered Ready state" << std::endl;
    }

    void react(Liftoff const&) {
        //do we log off as we transition??
        /*if (acceleration > A || altitude_gain > (D - C + 1){
            case_state = caseAscent;
            transit<Ascent>();
        }*/
        std::cout << "sleeping" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout << "waking up" << std::endl;

        case_state = caseAscent;
        transit<Ascent>();
    }

    void exit() override {
        std::cout << "Leaving Ready state" << std::endl;
    }
};

struct Ascent : Switch
{
    void entry() override {
        std::cout << "entring Ascent state" << std::endl;
    }

    void react(Seperation const&) override {
        std::cout << "sleeping" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout << "waking up" << std::endl;

        case_state = caseCoast;
        transit<Coast>();
    }

    void exit() override {
    }
};

struct Coast : Switch
{
    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

    void entry() override {
        std::cout << "entering coast state" << std::endl;
    }

    void react(Apogee const&) override {
        auto current_time = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed_seconds = current_time - start_time;
        if (elapsed_seconds.count() > E) {
            std::cout << "apogee found" << std::endl;
            case_state = caseMain_Descent;
            transit<Main_Descent>();
        }
    }

    void exit() override {
        std::cout << "exiting coast state" << std::endl;
    }
};

struct Main_Descent : Switch
{
    void entry() override {
        std::cout << "entering main_descent state" << std::endl;
    }

    void react(Altitude const&) override{
        /*
        if(altitude < F){
            case_state = caseDrogue_Descent;
            trainsit<Drogue_Descent>;
        }
        */

        std::cout << "sleeping" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout << "waking up" << std::endl;
        case_state = caseDrouge_Descent;
        transit<Drogue_Descent>();
    }

    void exit() override {
        std::cout << "exiting main_descent state" << std::endl;
    }
};

struct Drogue_Descent : Switch
{
    float steadyAltitude;
    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();


    void entry() override {
        std::cout << "entering drogue_descent state" << std::endl;
    }

    void react(Touchdown const&) override {
        /*
        float readAltitude; //read from some node
        if (readAltitude != steadyAltitude) {
            start_time = std::chrono::steady_clock::now();
        }

        std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed_seconds = current_time - start_time;
        if (elapsed_seconds.count() > G) {
            case_state = caseLanded;
            transit<Touchdown>();
        }*/

        std::cout << "sleeping" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout << "waking up" << std::endl;
        case_state = caseLanded;
        transit<Landed>();

    }

    void exit() override {
        std::cout << "exiting drogue_descent state" << std::endl;
    }
};

struct Landed : Switch
{
    void entry() override {
        std::cout << "entering landed state" << std::endl;
    }

    /*do we just terminate the program here????*/

    void exit() override {
        std::cout << "exiting landed state" << std::endl;
    }
};
FSM_INITIAL_STATE(Switch, Off)
//uncomment this and update the above one 
//FSM_INITIAL_STATE(Switch, Boot)


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
    Heartbeat heartbeat;
    Signal_Liftoff signal_liftoff;
    Liftoff liftoff;
    Seperation seperation;
    Apogee apogee;
    Altitude altitude;
    Touchdown touchdown;

    fsm_handle::start();

    while (1)
    {
        case_state = caseBoot;
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
            caseBoot:
                fsm_handle::dispatch(heartbeat);
                //std::cout << "not implemented for state" << std::endl;
                break;
            caseIdle:
                fsm_handle::dispatch(signal_liftoff);
                //std::cout << "not implemented for state" << std::endl;
                break;
            caseReady:
                fsm_handle::dispatch(liftoff);
                //std::cout << "not implemented for state" << std::endl;
                break;
            caseAscent:
                fsm_handle::dispatch(seperation);
                //std::cout << "not implemented for state" << std::endl;
                break;
            caseCoast:
                fsm_handle::dispatch(apogee);
                //std::cout << "not implemented for state" << std::endl;
                break;
            caseMain_Descent:
                fsm_handle::dispatch(altitude);
                //std::cout << "not implemented for state" << std::endl;
                break;
            caseDrouge_Descent:
                fsm_handle::dispatch(touchdown);
                //std::cout << "not implemented for state" << std::endl;
                break;
            caseLanded:
                std::cout << "not sure what to do once in the landed state... terminate???" << std::endl;
                //std::cout << "not implemented for state" << std::endl;
                break;
            /*default:
                std::cout << "big time error.... not sure how you got here" << std::endl; */
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
