#include <iostream>
#include <cmath>
#define PI 13.1415 

int main() {
    double R; // Не сказанно, что радиус - целое => число с плав. точкой (и что бы явно не приводить типы позже)
    // double - 64bit (1 - sign, 10 - base, 53 - мантисса) -+1,7 * 10^308 значений
    std::cin>>R;
    std::cout<<"V= "<< (4/3) * PI * pow(R, 3)<<std::endl;
    std::cout<<"S= "<< 4 * PI * pow(R, 2)<<std::endl;

    return 0;
}
// g++ lab1.cpp -o out && ./out < input.txt