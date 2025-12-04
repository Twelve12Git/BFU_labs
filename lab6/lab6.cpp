#include <iostream>

template<typename T>
T** alloc_matrix(size_t n, size_t m) {
    T** matrix = static_cast<T**>(malloc(sizeof(T*)*n));
    for(size_t i = 0; i < m; i++) matrix[i] = static_cast<T*>(malloc(sizeof(T)*m));
    return matrix;
}

template <typename T>
void realloc_matrix(T*** matrix_ptr, size_t n, size_t m) { 
    T**& matrix = *matrix_ptr;
    matrix = static_cast<T**>(realloc(matrix, sizeof(T*) * n));
    for (size_t i = 0; i < n; i++)matrix[i] = static_cast<T*>(realloc(matrix[i], sizeof(T) * m));
}

template <typename T>
void free_matrix(T*** matrix_ptr, size_t n) {
    T** matrix = *matrix_ptr;
    
    for (size_t i = 0; i < n; i++) {
        if (matrix[i] != nullptr) {
            free(matrix[i]);
            matrix[i] = nullptr;
        }
    }
    
    free(matrix);
    *matrix_ptr = nullptr;
}

template <typename T>
void exclude_row(T*** matrix_ptr, size_t n, size_t row_index) {
    T** matrix = *matrix_ptr;
    free(matrix[row_index]);
    for(size_t i = row_index; i < n-1; i++) matrix[i] = matrix[i+1];
}

template<typename T>
void print_matrix(T** matrix, size_t n, size_t m){
    for (size_t i = 0; i < n; i++){
        for(size_t j = 0; j < m; j++){
            std::cout << matrix[i][j] << "\t";
        }
        std::cout << std::endl;
    }
    std::cout << "-----"  << std::endl;
}

template<typename T>
int* get_rows_contains_zero(T** matrix, size_t n, size_t m){
    T* indexes = static_cast<T*>(malloc(sizeof(T)));
    indexes[0] = 0;
    for (size_t i = 0; i < n; i++){
        for(size_t j = 0; j < m; j++){
            if (matrix[i][j] == 0){
                indexes[0]++;
                indexes = static_cast<T*>(realloc(indexes, (indexes[0]+1)*sizeof(T)));
                indexes[indexes[0]] = i;
                break;
            }
        }
    }
    return indexes;
}

int main(){
    // лаба 6 вариант 4 (четный)
    //////////// 1 ////////////
    using data_t = int;
    data_t** matrix = alloc_matrix<data_t>(2, 2);
    do{
        std::cout << "Enter A B C D: " << std::endl;
        std::cin >> matrix[0][0] >> matrix[0][1] >> matrix[1][0] >> matrix[1][1];
    } while (matrix[0][0] < 0 || matrix[0][1] < 0);

    print_matrix(matrix, 2, 2);

    size_t n = matrix[0][0]+2, m = matrix[0][1]+2;

    realloc_matrix(&matrix, n, m);
    
    for(int i = n-1; i >= 0; i--){
        for(int j = m-1; j >= 0; j--){
            if(i > n-3 && j > m-3) matrix[i][j] = matrix[i-n+2][j-m+2];
            else matrix[i][j] = matrix[n-1][m-2]*i + matrix[n-1][m-1]*j;
        }
    }

    print_matrix(matrix, n, m);
    // int row_indx = 0; // так-то массивы из функций очень странно возвращать без размера
    // exclude_row(&matrix, n--, row_indx);
    data_t* row_to_exclude_indexes = get_rows_contains_zero(matrix, n, m);
    for (size_t i = 1; i <= row_to_exclude_indexes[0]; i++){
        exclude_row(&matrix, n--, row_to_exclude_indexes[i]);
    };
    print_matrix(matrix, n, m);
    free_matrix(&matrix, n);
    free(row_to_exclude_indexes);

    //////////// 2 ////////////
    int a, b; std::cin>>a>>b;
    int *a_ptr = &a, *b_ptr = &b;

    *a_ptr*=2;
    *b_ptr*=2;
    std::cout << a << "\t" << b << std::endl;

    int tmp_ptr = *a_ptr;
    *a_ptr = *b_ptr;
    *b_ptr = tmp_ptr;
    std::cout << a << "\t" << b << std::endl;

    return 0;
}
