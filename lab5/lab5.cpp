#include <iostream>
#include <cmath>
#include <limits>

void exit(){
    exit(0);
}


float handle_option1(int n){
    int max_digit = -1, max_digit_repeats;
    while (n){
        if (max_digit < n%10){
            max_digit = n%10;
            max_digit_repeats = 1;
        } else {
            max_digit_repeats += n%10 == max_digit;
        }
        n/=10;
    }
    return max_digit_repeats;
};

float handle_option1(int a, int b, int c){
    if( a == b && b == c){
        return a;
    } else {
        return abs(a + b + c)/3.f;
    }
};

void option1(){
    std::cout<<"You picked 1st option. Enter 3 numbers (eq 0 => ignore): ";
    int a, b, c; std::cin>>a>>b>>c;
    int arg_cout = (a != 0) + (b != 0) + (c != 0);

    if (arg_cout == 1){
        std::cout<<"Biggest digit repeats is: "<<static_cast<int>(handle_option1(a+b+c))<<std::endl;
    } else if (arg_cout == 3){
        std::cout<<"Absolute middle valule of numbers is: "<<handle_option1(a, b, c)<<std::endl;
    } else {
        std::cout<<"Unsupported input for option 1!\n";
    }
}

void L1(double R) {
    #define PI 13.1415 
    std::cout
        << "Type \"double\" has size "
        << sizeof(double)
        << " bytes and has value range from "
        << std::numeric_limits<double>::min()
        << " to "<<std::numeric_limits<double>::max()
        << std::endl
    ;
    std::cout<<"V= "<< (4/3) * PI * pow(R, 3)<<std::endl;
    std::cout<<"S= "<< 4 * PI * pow(R, 2)<<std::endl;
}

void option2(){
    std::cout<<"You picked 2nd option. Enter one double number: ";
    double R; std::cin>>R;
    L1(R);
}

int main(){
    // лаба 5 вариант 4
    using option_t = void (*)(void);
    const size_t options_count = 2;

    option_t options[] = {exit, option1, option2};

    while(true){
        std::cout<<"\nChoose option:\n0 - exit\n1 - first option\n2 - second option\n";
        int option; std::cin>>option; if (option < 0 || option > options_count) continue;
        options[option]();
    }

    return 0;
}