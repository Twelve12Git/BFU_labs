#include <iostream>

// лаба 7 вариант 4
#define USE_VECTOR false// переключить вектор-массив

using data_t = int;
struct option
{
    using handler_t = void (*)(void);
    const char* description;
    handler_t handler;
};
#if USE_VECTOR // Пункт 1
#include <vector>
#include <algorithm>
#include <limits>

static std::vector<data_t> arr;

int main(){
    // лаба 7 вариант 4
    const size_t options_count = 7;

    option options[] = {
        {"Exit", [](){exit(0);}},
        {"Show array", [](){for(const data_t& el: arr) std::cout<<el<<"\t"; std::cout<<std::endl;}},
        {"Add element to begin", [](){data_t el; std::cin>>el; arr.insert(arr.begin(), 1);}},
        {"Add element to end", [](){data_t el; std::cin>>el; arr.push_back(el);}},
        {"Clear array", [](){arr.clear();}},
        {"Find element", [](){
            data_t el; std::cin>>el; 
            std::cout << "Value " << el << " found at index: " << 
            std::distance(arr.begin(),std::find(arr.begin(), arr.end(), el)) << std::endl;
            }
        },
        {"Do special action", [](){
                data_t min_el = std::numeric_limits<data_t>::max();
                for(data_t el: arr) {
                    if (el != 0) min_el = std::min(abs(el), min_el);
                }
                arr.resize(arr.size()+min_el, 0);
            }
        }
    };

    while(true){
        std::cout<<"Select option: \n";
        for(int i = 0; i < options_count; i++) std::cout << "(" <<i<< ") " << options[i].description << std::endl;
        size_t option; std::cin>>option; if (option > options_count) continue;
        options[option].handler();
    }

    return 0;
}
#else // пункт 2
#include <array>
#include <cstdlib>

std::array<int, 10> sort_arr_and_copy(std::array<int, 10> arr){
    for(int i = 0; i < arr.size(); i++){
        for(int j = i+1; j < arr.size(); j++){
            if (arr[i] < arr[j]) std::swap(arr[i], arr[j]);
        }
    }
    return arr;
}

void sort_arr(std::array<int, 10>& arr){
    for(int i = 0; i < arr.size(); i++){
        for(int j = i+1; j < arr.size(); j++){
            if (arr[i] < arr[j]) std::swap(arr[i], arr[j]);
        }
    }
}

void sort_arr(std::array<int, 10>* arr){
    for(int i = 0; i < arr->size(); i++){
        for(int j = i+1; j < arr->size(); j++){
            if ((*arr)[i] < (*arr)[j]) std::swap((*arr)[i], (*arr)[j]);
        }
    }
}

void print_arr(const std::array<int, 10>& arr){
    for(auto el: arr) std::cout<<el<<"\t"; std::cout<<std::endl;
}

int main(){
    std::array<int, 10> arr;

    for(int i = 0; i < 10; i++) arr[i] = std::rand() % 21 - 10;
    print_arr(arr);
    arr = sort_arr_and_copy(arr);
    print_arr(arr);
    std::cout<<"\n------------\n";
    for(int i = 0; i < 10; i++) arr[i] = std::rand() % 21 - 10;
    print_arr(arr);
    sort_arr(arr);
    print_arr(arr);
    std::cout<<"\n------------\n";
    for(int i = 0; i < 10; i++) arr[i] = std::rand() % 21 - 10;
    print_arr(arr);
    sort_arr(&arr);
    print_arr(arr);

    return 0;
}
#endif