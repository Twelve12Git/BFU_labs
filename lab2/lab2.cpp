#include <iostream>
#include <bitset>

void print_bin(int x) {
    if (!x) return;
    print_bin(x/2);
    std::cout<<x%2;
}

int main() {
    /*
    лаба 2 вариант 4

    Для пункта 1 число А вводиться с клавиатуры, номер бита i тоже. Проверить правильность ввода i и определить по заданию. вывести битовую цепочку А.

    1) Если i бит А не равен 0 – Ввести натуральные числа A, B и C. Если A меньше B и B меньше C, то вывести C-B-A, в противном случае если если A кратно C, то вывести A/С+B, в остальных случаях вывести A+B+C..
    Иначе – побитово сложить число А с собой и показать результат.

    2) Ввести с клавиатуры N – номер месяца в году, с помощью switch вывести кол-во дней в нём (февраль вывести 28). Предусмотреть обработку ошибочного ввода N.
    */

    // 1)
    std::cout<<"Enter A, i: ";
    int A, B, C, i; std::cin>>A>>i;

    if (!(1 <= i && i <= sizeof(int)*8)) {
        std::cout<<"Wrong input\n";
        return 0;
    }
    if (!(A&(1<<(i-1)))){ // если бит с номером i равен нулю (я не уверен как нужно преобразовывать в бинарный вид)
        std::cout<<"Bit i ("<<i<<") is 0;\nbinary A is:\n";
        // print_bin(A); std::cout<<std::endl;
        // for(int i = sizeof(int)*8; i>0; i--) std::cout<<((A&(1<<(i-1))) != 0); std::cout<<std::endl;
        std::cout<<(std::bitset<sizeof(int)*8>(A).to_string())<<std::endl; // На лекции показывали так
    }else {
        std::cout<<"Enter A, B, C: "; std::cin>>A>>B>>C;
        std::cout<< (((A < B && B < C)) ? C-B-A : (A%C==0) ? A/C+B : A+B+C)<<std::endl;
    }

    // 2)
    std::cout<<"Enter month number: ";
    int N; std::cin>>N;
    switch (N){ // Это даже с помощью условий на rvalue ссылках не оптимизировать. Оно вообще в жизни встречается?
        case 1: case 3: case 5: case 7: case 8: case 10: case 12:
            std::cout<<"Days: 30"<<std::endl;
            break;
        case 4: case 6: case 9: case 11:
            std::cout<<"Days: 31"<<std::endl;
            break;
        case 2:
            std::cout<<"Days: 28"<<std::endl;
            break;
        default:
            std::cout<<"Wrong input";
    }
    return 0;
}
//  ./build/bin/lab2 
// g++ lab2.cpp -o out && ./out < input0.txt && ./out < input1.txt && ./out < input2.txt && ./out < input3.txt && ./out < input4.txt