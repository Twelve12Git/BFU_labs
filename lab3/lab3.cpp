#include <iostream>
#include <limits>

/*
ВАРИАНТ 4
Для пункта 1 последовательность целых чисел {A1, A2, …, AN} вводим с клавиатуры. Числа последовательности хранить не нужно. Количество чисел в последовательности вводится с клавиатуры (корректно). Если в последовательности нет чисел, удовлетворяющих условию, вывести сообщение о состоянии.
Вариант 4
1) Найти сумму всех отрицательных чисел, наибольшее из таких чисел и количество его повторений.
2) Найти наибольшую цифру числа.
*/

int main() {
    // ----- ----- 1 ----- -----
    std::cout << "1)" << std::endl;

    int a, n; std::cin >> n;

    auto condition = [](const int& x){return x < 0;};

    int sum{}, max_num{std::numeric_limits<int>::min()}, max_num_repeats;
    while (n--)
    {
        std::cin >> a;
        if (!condition(a)) continue;

        sum += a;
        int tmp = max_num;
        max_num = std::max(max_num, a);
        max_num_repeats = (max_num == tmp) ? (a == max_num) ? max_num_repeats+1 : max_num_repeats : 1;
    }
    
    if (sum) {
        std::cout << "Sum: " << sum << std::endl;
        std::cout << "The max number is " << max_num <<" it's appeared " << max_num_repeats << " times." << std::endl;
    } else {
        std::cout << "None of numbers match the condition." << std::endl;
    }

    // ----- ----- 2 ----- -----
    std::cout  << std::endl << "2)" << std::endl;

    int x, max_digit{-1}; std::cin >> x;
    int tmp = std::abs(x);
    while(tmp){
        max_digit = std::max(max_digit, tmp%10);
        tmp /= 10;
    }
    std::cout << "Max digit of " << x << " is " << max_digit << std::endl;
    
    return 0;
}

// g++ lab3.cpp -o out && ./out < input.txt
// ./build/bin/lab3