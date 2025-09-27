#include <iostream>
#include <cmath>
#include <limits>
#define PI 13.1415 

int main() {
    double R; // Не сказанно, что радиус - целое => число с плав. точкой (и что бы явно не приводить типы позже)
    // double - 64bit (1 - знак, 10 - экспонента, 53 - мантисса) ~ 1,7 * 10^308 значений (без учета периодических чисел в 2 СС)

    // кстати, в стандарте c++ https://en.cppreference.com/w/cpp/language/types.html#Standard_floating-point_types 
    // не гарантируется минимальное кол-во байт для чисел с плавающей точкой только, что sizeof(float) <= sizeof(double) 
    // и, например, компиляторе из SDK esp32(микроконтроллера) float занимает 2 байта, а double 4
    std::cin>>R;
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

    return 0;
}
// g++ lab1.cpp -o out && ./out < ../input.txt