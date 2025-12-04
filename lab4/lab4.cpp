#include <iostream>

#define VARIANT 4

int main(){
    // лаба 4 вариант 4

    ////////// 1 ////////// 
    int a[VARIANT + 3];

    bool all_nums_divisible_5 = true;

    for(int i = 0; i < VARIANT + 3; i++){
        std::cin>>a[i];
        all_nums_divisible_5 = all_nums_divisible_5 && a[i]%5 == 0;
    }

    if (all_nums_divisible_5){
        for(int i = 0; i < VARIANT+3; i++){
            for(int j = i+1; j < VARIANT+3; j++){
                if (a[i] > a[j]) std::swap(a[i], a[j]);
            }
        }
    }

    for(int i = 0; i < VARIANT + 3; i++) std::cout<<a[i]<<"\t"; std::cout<<std::endl;

    ////////// 2 //////////
    int b[3][4], dp[3][4];
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 4; j++){
            std::cin>>b[i][j];
            dp[i][j] = (b[i][j] < 0) + ((i > 0) ? dp[i-1][j] : 0);
        }
    }

    int max_negative_nums_col_indx = -1, max_negative_nums_col_value = 0;
    for(int i = 0; i < 4; i++){
        if(dp[2][i] > max_negative_nums_col_value) {
            max_negative_nums_col_value = dp[2][i];
            max_negative_nums_col_indx = i;
        }
    }

    if (max_negative_nums_col_indx != -1) {
        for (int i = 0; i < 3; i++){
            b[i][max_negative_nums_col_indx] = -1;
        }
    }

    // std::cout<<max_negative_nums_col_indx<<" "<<max_negative_nums_col_value<<"\n";

    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 4; j++) std::cout<<b[i][j]<<"\t";
        std::cout<<std::endl;
    }

    return 0;
}